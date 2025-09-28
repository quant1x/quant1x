use std::sync::Arc;
use crate::cache::{self, DataAdapter, Kind};
use crate::Timestamp;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct KLine {
    #[serde(rename = "Date")]
    pub date: String,
    #[serde(rename = "Open")]
    pub open: f64,
    #[serde(rename = "Close")]
    pub close: f64,
    #[serde(rename = "High")]
    pub high: f64,
    #[serde(rename = "Low")]
    pub low: f64,
    #[serde(rename = "Volume")]
    pub volume: f64,
    #[serde(rename = "Amount")]
    pub amount: f64,
    #[serde(rename = "Up")]
    pub up: i32,
    #[serde(rename = "Down")]
    pub down: i32,
    #[serde(rename = "Datetime")]
    pub datetime: String,
    #[serde(rename = "AdjustmentCount")]
    pub adjustment_count: i32,
}

impl KLine {
    pub fn headers() -> Vec<String> {
        vec!["Date".into(), "Open".into(), "Close".into(), "High".into(), "Low".into(), "Volume".into(), "Amount".into(), "Up".into(), "Down".into(), "Datetime".into(), "AdjustmentCount".into()]
    }
}

#[derive(Debug)]
pub struct DataKLine;

impl cache::Schema for DataKLine {
    fn kind(&self) -> Kind { crate::cache::PLUGIN_MASK_BASE_DATA | 3 }
    fn owner(&self) -> String { crate::cache::DEFAULT_DATA_PROVIDER.to_string() }
    fn key(&self) -> String { "day".to_string() }
    fn name(&self) -> String { "日K线".to_string() }
    fn usage(&self) -> String { "日K线".to_string() }
}

impl DataAdapter for DataKLine {
    fn print(&self, _code: &str, _dates: &[Timestamp]) {}

    fn update(&self, code: &str, _date: Timestamp) {
        // 尝试从 level1 获取日线数据；若无法获取则降级为本地缓存（仅写入表头）的行为。
        // 构建与 C++ 等效的 kline 文件名。
        // 使用集中式的 config helper 构建 kline 缓存文件路径（与 C++ 保持一致）。
        let filename = crate::config::get_kline_filename(code, true);
        if filename.is_empty() {
            log::error!("[DataKLine] cannot build kline filename for {}", code);
            return;
        }
        log::debug!("[DataKLine] cache filename: {}", filename);

        // 确保父目录存在
        let path = std::path::Path::new(&filename);
        if let Some(parent) = path.parent() {
            if let Err(e) = std::fs::create_dir_all(parent) {
                log::error!("[DataKLine] failed to create parent dir {:?}: {}", parent, e);
                return;
            }
        }
        // 遵循 C++ 的逻辑：分页步长（security_bars_max）、回溯天数、合并及除权预处理
        const MAX_KLINE_LOOKBACK_DAYS: usize = 1;
        const SECURITY_BARS_MAX: usize = 800;

        // 先加载已有缓存（若存在），以确定起始日期和调整次数（与 C++ 保持一致）
        let cache_filename = filename.clone();
        let cache_klines: Vec<KLine> = read_kline_from_csv(&cache_filename);
        let klines_length = cache_klines.len();
        let mut klines_offset_days = MAX_KLINE_LOOKBACK_DAYS;
        let mut adjust_times = 0i32;
        // 默认起始日期：1990-12-19（市场首次上市日期）
        let mut current_start_date = crate::Timestamp::pre_market_time(1990, 12, 19).unwrap_or(crate::Timestamp::zero());
        if klines_length > 0 {
            if klines_offset_days > klines_length {
                klines_offset_days = klines_length;
            }
            let kline = &cache_klines[klines_length - klines_offset_days];
            // parse date back to Timestamp
            if let Ok(ts) = crate::Timestamp::parse(&kline.date) {
                current_start_date = ts;
            }
            adjust_times = kline.adjustment_count;
        }

        // 从当前时间确定结束日期（尽可能使用今日的盘前时间作为结束日期）
        let current_end_date = crate::Timestamp::pre_market_time_from_current(&crate::Timestamp::now()).unwrap_or(crate::Timestamp::now());
        // 构建日期范围
        let ts_range = crate::exchange::date_range(current_start_date, current_end_date, false);
        if ts_range.is_empty() {
            log::debug!("[DataKLine] empty date range for {}", code);
            return;
        }
        let total = ts_range.len();

        // 按日期范围分页从 level1 拉取数据（每页作为一个向量，保持页内顺序，之后再翻转页序以匹配 C++）
        let mut hs: Vec<Vec<crate::level1::SecurityBar>> = Vec::new();
        let step = SECURITY_BARS_MAX;
    let mut start_idx: usize = 0;
        while start_idx < total {
            let remaining = total - start_idx;
            let count = std::cmp::min(step, remaining) as u16;
            // 注意：fetch_security_bars 接受 start 为 u32，count 为 u16；C++ 基于日期索引使用 start/count
            match crate::level1::fetch_security_bars(code, 9, 1, start_idx as u32, count) {
                Some(resp) => {
                    if resp.list.is_empty() {
                        break;
                    }
                    // 直接把整页 push 进去，保留每页内部的顺序
                    hs.push(resp.list);
                    // if server returned less than requested count, stop
                    if (resp.count as usize) < count as usize {
                        break;
                    }
                    start_idx = start_idx.saturating_add(count as usize);
                }
                None => {
                    log::warn!("[DataKLine] fetch_security_bars returned None for {} start={}", code, start_idx);
                    break;
                }
            }
        }

        // 如果没有获取到任何页，则回退为仅写入表头的文件创建/重写（与之前行为一致）
        if hs.is_empty() {
            if !path.exists() {
                match std::fs::File::create(&filename) {
                    Ok(f) => {
                        let mut w = csv::Writer::from_writer(f);
                        if let Err(e) = w.write_record(KLine::headers()) { log::error!("[DataKLine] write header failed: {}", e); }
                        let _ = w.flush();
                    }
                    Err(e) => { log::error!("[DataKLine] create file {} failed: {}", filename, e); }
                }
            }
            return;
        }
        // C++ 会将分页结果反转为时间升序；保留每页内的顺序，只 reverse 外层 pages
        hs.reverse();

        // 根据抓取到的 pages 并按日期范围筛选，构建增量 K 线列表（按页内顺序迭代）
        let mut incremental_klines: Vec<KLine> = Vec::new();
        for page in hs.iter() {
            for row in page.iter() {
            // 为 bar 日期构造盘前时间戳
            let date_time = crate::Timestamp::pre_market_time(row.year, row.month as u32, row.day as u32).unwrap_or(crate::Timestamp::now());
            if date_time < ts_range[0] || date_time > ts_range[total-1] {
                continue;
            }
            let kx = KLine {
                date: date_time.only_date(),
                open: row.open,
                close: row.close,
                high: row.high,
                low: row.low,
                volume: row.vol * 100.0,
                amount: row.amount,
                up: row.up_count as i32,
                down: row.down_count as i32,
                datetime: row.datetime.clone(),
                adjustment_count: 0,
            };
            incremental_klines.push(kx);
            }
        }

        // 判断是否仅针对最新抓取的那一部分需要做预除权调整
        let is_fresh_fetch_require_adjustment = adjust_times == 1;
        // 仅从本地缓存加载除权除息数据（与 C++ 行为一致）
        let dividends = crate::datasets::xdxr::load_xdxr(code);
        if is_fresh_fetch_require_adjustment {
            calculate_pre_adjust(&mut incremental_klines, ts_range[0], &dividends);
        }

        // 按照 C++ 的合并逻辑合并缓存与增量数据
        let mut klines: Vec<KLine> = Vec::new();
        if klines_length > klines_offset_days {
            klines.extend_from_slice(&cache_klines[..(klines_length - klines_offset_days)]);
        }
        if klines.is_empty() {
            klines = incremental_klines.clone();
        } else {
            klines.extend(incremental_klines.into_iter());
        }

        if !is_fresh_fetch_require_adjustment {
            calculate_pre_adjust(&mut klines, ts_range[0], &dividends);
        }

        // 持久化保存
        let tmp = format!("{}.tmp", filename);
        match std::fs::File::create(&tmp) {
            Ok(f) => {
                let mut w = csv::Writer::from_writer(f);
                if let Err(e) = w.write_record(KLine::headers()) { log::error!("[DataKLine] write header failed: {}", e); }
                for row in klines.iter() {
                    let rec: Vec<String> = vec![row.date.clone(), row.open.to_string(), row.close.to_string(), row.high.to_string(), row.low.to_string(), row.volume.to_string(), row.amount.to_string(), row.up.to_string(), row.down.to_string(), row.datetime.clone(), row.adjustment_count.to_string()];
                    if let Err(e) = w.write_record(rec) { log::error!("[DataKLine] write row failed: {}", e); }
                }
                let _ = w.flush();
                if let Err(e) = std::fs::rename(&tmp, &filename) { log::error!("[DataKLine] rename failed {} -> {}: {}", tmp, filename, e); }
            }
            Err(e) => { log::error!("[DataKLine] create tmp {} failed: {}", tmp, e); }
        }
    }
}

// 辅助函数：从 CSV 读取 K 线，功能类似 C++ 的 read_kline_from_csv
fn read_kline_from_csv(filename: &str) -> Vec<KLine> {
    // 使用基于 Serde 的 CSV 反序列化，使字段名可自动与表头匹配。
    let mut klines: Vec<KLine> = Vec::new();
    match std::fs::File::open(filename) {
        Ok(f) => {
            let mut rdr = csv::ReaderBuilder::new().has_headers(true).from_reader(f);
            // Deserialize all records into Vec<KLine> in one go; CSV+Serde maps headers -> struct fields
            match rdr.deserialize::<KLine>().collect::<Result<Vec<KLine>, csv::Error>>() {
                Ok(v) => klines = v,
                Err(e) => log::error!("[DataKLine] failed to deserialize kline file {}: {}", filename, e),
            }
        }
        Err(_) => { /* missing file -> return empty vector */ }
    }
    klines
}

// 辅助函数：计算预除权（与 C++ 实现相似）
fn calculate_pre_adjust(klines: &mut Vec<KLine>, start_date: crate::Timestamp, dividends: &Vec<crate::level1::xdxr::XdxrInfo>) {
    if klines.is_empty() { return; }
    let last_day = klines.last().unwrap().date.clone();
    let ts_last_day = crate::Timestamp::parse(&last_day).unwrap_or(crate::Timestamp::now());
    // 将该日期转换为该日的盘前时间
    let ts_last_day = crate::Timestamp::pre_market_time_from_current(&ts_last_day).unwrap_or(ts_last_day);
    let last_day_next = crate::exchange::next_trading_day(ts_last_day).only_date();
    let start_date_only = start_date.only_date();
    // 筛选除权除息记录：满足 last_day_next >= x.Date 并且 x.Category == 1
    let xdxr_infos: Vec<crate::level1::xdxr::XdxrInfo> = dividends.iter().filter(|x| {
        if x.category as i32 != 1 { return false; }
        if let Ok(dts) = crate::Timestamp::parse(&x.date) {
            return last_day_next >= dts.only_date();
        }
        false
    }).cloned().collect();
    let mut _times = xdxr_infos.len();
    for info in xdxr_infos.iter() {
        if info.date <= start_date_only { /* continue */ } else {
            let (m, a) = info.adjust_factor();
            let klines_size = klines.len();
            for i in 0..klines_size {
                if klines[i].date >= info.date { break; }
                klines[i].open = klines[i].open * m + a;
                klines[i].close = klines[i].close * m + a;
                klines[i].high = klines[i].high * m + a;
                klines[i].low = klines[i].low * m + a;
                // 使用 amount/volume 进行成交量的调整
                let ap = if klines[i].volume != 0.0 { klines[i].amount / klines[i].volume } else { 0.0 };
                let ap_adjusted = ap * m + a;
                if ap_adjusted != 0.0 { klines[i].volume = klines[i].amount / ap_adjusted; }
                klines[i].adjustment_count += 1;
            }
        }
        _times -= 1;
    }
}

pub fn init() {
    let plugin = Arc::new(DataKLine) as Arc<dyn DataAdapter>;
    cache::register(plugin);
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_read_kline_from_csv() {
        let adapter = DataKLine;
        let code = "sh510050";
        let date = Timestamp::now();
        adapter.update(code, date);
    }
}