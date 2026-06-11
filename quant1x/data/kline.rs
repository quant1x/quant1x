use crate::data::meta::instrument::Instrument;
use crate::contrib::data::tdx::level1::std::KLineType;
use crate::meta::Timestamp;
use serde::{Deserialize, Serialize};
use std::sync::Arc;
use crate::data::adapter::DataAdapter;
use crate::apply_forward_adjustment_for_event;

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct KLine {
    #[serde(rename = "date")]
    pub date: String,
    #[serde(rename = "open")]
    pub open: f64,
    #[serde(rename = "close")]
    pub close: f64,
    #[serde(rename = "high")]
    pub high: f64,
    #[serde(rename = "low")]
    pub low: f64,
    #[serde(rename = "volume")]
    pub volume: f64,
    #[serde(rename = "amount")]
    pub amount: f64,
    #[serde(rename = "up")]
    pub up: i32,
    #[serde(rename = "down")]
    pub down: i32,
    #[serde(rename = "datetime")]
    pub datetime: String,
    #[serde(rename = "adjustment_count")]
    pub adjustment_count: i32,
}

impl KLine {
    pub fn headers() -> Vec<String> {
        vec![
            "date".into(),
            "open".into(),
            "close".into(),
            "high".into(),
            "low".into(),
            "volume".into(),
            "amount".into(),
            "up".into(),
            "down".into(),
            "datetime".into(),
            "adjustment_count".into(),
        ]
    }
}

#[derive(Debug)]
pub struct DataKLine;

impl crate::data::Schema for DataKLine {
    fn kind(&self) -> crate::Kind {
        crate::data::BaseKLine
    }
    fn owner(&self) -> String {
        crate::data::DEFAULT_DATA_PROVIDER.to_string()
    }
    fn key(&self) -> String {
        "day".to_string()
    }
    fn name(&self) -> String {
        "日K线".to_string()
    }
    fn usage(&self) -> String {
        "日K线".to_string()
    }
}

impl crate::data::DataAdapter for DataKLine {
    fn print(&self, _inst: &Instrument, _dates: &[Timestamp]) {}

    fn update(&self, inst: &Instrument, _date: Timestamp) {
        let symbol = inst.symbol();
        // 对齐 Python tdx/kline.py: get_kline_filename(inst, freq)
        //   module_name = freq.cache_key()  # "day"
        //   symbol = inst.symbol()
        //   sub = f"{module_name}/{inst.cache_dir()}"
        //   return f'{config.data_path}/{sub}/{symbol}.csv'
        let filename = get_kline_filename(inst);
        if filename.is_empty() {
            log::error!("[DataKLine] cannot build kline filename for {}", symbol);
            return;
        }
        log::debug!("[DataKLine] cache filename: {}", filename);

        // 确保父目录存在
        let path = std::path::Path::new(&filename);
        if let Some(parent) = path.parent() {
            if let Err(e) = std::fs::create_dir_all(parent) {
                log::error!(
                    "[DataKLine] failed to create parent dir {:?}: {}",
                    parent,
                    e
                );
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
        let mut current_start_date =
            crate::meta::Timestamp::pre_market_time(1990, 12, 19).unwrap_or(crate::meta::Timestamp::zero());
        if klines_length > 0 {
            if klines_offset_days > klines_length {
                klines_offset_days = klines_length;
            }
            let kline = &cache_klines[klines_length - klines_offset_days];
            // parse date back to Timestamp
            if let Ok(ts) = crate::meta::Timestamp::parse(&kline.date) {
                current_start_date = ts;
            }
            adjust_times = kline.adjustment_count;
        }

        // 从当前时间确定结束日期（尽可能使用今日的盘前时间作为结束日期）
        let current_end_date =
            crate::meta::Timestamp::pre_market_time_from_current(&crate::meta::Timestamp::now())
                .unwrap_or(crate::meta::Timestamp::now());
        // 构建日期范围
        let ts_range = crate::meta::date_range(current_start_date, current_end_date, false);
        if ts_range.is_empty() {
            log::debug!("[DataKLine] empty date range for {}", symbol);
            return;
        }

        let total = ts_range.len();

        // 按日期范围分页从 level1 拉取数据（每页作为一个向量，保持页内顺序，之后再翻转页序以匹配 C++）
        let mut hs: Vec<Vec<crate::contrib::data::tdx::standard::SecurityBar>> = Vec::new();
        let step = SECURITY_BARS_MAX;
        let mut start_idx: usize = 0;
        while start_idx < total {
            let remaining = total - start_idx;
            let count = std::cmp::min(step, remaining) as u16;
            // 注意：start 使用日期索引，datasets 层直接调度 SecurityBars 请求
            match crate::data::kline_raw::fetch_kline(
                &symbol,
                start_idx as u32,
                count,
                KLineType::RiK,
            ) {
                Some(resp) => {
                    let response_count = resp.count as usize;
                    if resp.list.is_empty() {
                        break;
                    }
                    hs.push(resp.list);
                    if response_count < count as usize {
                        break;
                    }
                    start_idx = start_idx.saturating_add(count as usize);
                }
                None => {
                    log::warn!(
                        "[DataKLine] fetch_kline returned None for {} start={}",
                        symbol,
                        start_idx
                    );
                    break;
                }
            }
        }

        // // 如果没有获取到任何页，则不保存CSV（空列表不保存，包括表头）
        // if hs.is_empty() {
        //     return;
        // }
        // C++ 会将分页结果反转为时间升序；保留每页内的顺序，只 reverse 外层 pages
        hs.reverse();

        // 根据抓取到的 pages 并按日期范围筛选，构建增量 K 线列表（按页内顺序迭代）
        let mut incremental_klines: Vec<KLine> = Vec::new();
        for page in hs.iter() {
            for row in page.iter() {
                // 为 bar 日期构造盘前时间戳
                let date_time =
                    crate::meta::Timestamp::pre_market_time(row.year, row.month as u32, row.day as u32)
                        .unwrap_or(crate::meta::Timestamp::now());
                if date_time < ts_range[0] || date_time > ts_range[total - 1] {
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
        // 仅从本地缓存加载除权除息数据（对齐 Python: get_xdxr_list(inst)）
        let dividends = crate::data::xdxr::load_xdxr(&symbol);
        if is_fresh_fetch_require_adjustment {
            apply_forward_adjustment_for_event!(&mut incremental_klines, ts_range[0], &dividends);
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
            apply_forward_adjustment_for_event!(&mut klines, ts_range[0], &dividends);
        }

        // 持久化保存
        if klines.is_empty() {
            // 空列表不保存CSV，包括表头
            return;
        }
        let tmp = format!("{}.tmp", filename);
        match std::fs::File::create(&tmp) {
            Ok(f) => {
                let mut w = csv::Writer::from_writer(f);
                if let Err(e) = w.write_record(KLine::headers()) {
                    log::error!("[DataKLine] write header failed: {}", e);
                }
                for row in klines.iter() {
                    let rec: Vec<String> = vec![
                        row.date.clone(),
                        row.open.to_string(),
                        row.close.to_string(),
                        row.high.to_string(),
                        row.low.to_string(),
                        row.volume.to_string(),
                        row.amount.to_string(),
                        row.up.to_string(),
                        row.down.to_string(),
                        row.datetime.clone(),
                        row.adjustment_count.to_string(),
                    ];
                    if let Err(e) = w.write_record(rec) {
                        log::error!("[DataKLine] write row failed: {}", e);
                    }
                }
                let _ = w.flush();
                if let Err(e) = std::fs::rename(&tmp, &filename) {
                    log::error!("[DataKLine] rename failed {} -> {}: {}", tmp, filename, e);
                }
            }
            Err(e) => {
                log::error!("[DataKLine] create tmp {} failed: {}", tmp, e);
            }
        }
    }
}

// 辅助函数：从 CSV 读取 K 线，功能类似 C++ 的 read_kline_from_csv
pub fn load_klines(filename: &str) -> Vec<KLine> {
    // 使用基于 Serde 的 CSV 反序列化，使字段名可自动与表头匹配。
    let mut klines: Vec<KLine> = Vec::new();
    match std::fs::File::open(filename) {
        Ok(f) => {
            let mut rdr = csv::ReaderBuilder::new().has_headers(true).from_reader(f);
            // Deserialize all records into Vec<KLine> in one go; CSV+Serde maps headers -> struct fields
            match rdr
                .deserialize::<KLine>()
                .collect::<Result<Vec<KLine>, csv::Error>>()
            {
                Ok(v) => klines = v,
                Err(e) => log::error!(
                    "[DataKLine] failed to deserialize kline file {}: {}",
                    filename,
                    e
                ),
            }
        }
        Err(_) => { /* missing file -> return empty vector */ }
    }
    klines
}

fn read_kline_from_csv(filename: &str) -> Vec<KLine> {
    load_klines(filename)
}

/// 生成 K 线缓存文件路径，与 Python tdx/kline.py 的 get_kline_filename(inst, freq) 对齐
///   module_name = freq.cache_key()  # "day"
///   symbol = inst.symbol()
///   sub = f"{module_name}/{inst.cache_dir()}"
///   return f'{config.data_path}/{sub}/{symbol}.csv'
fn get_kline_filename(inst: &Instrument) -> String {
    let symbol = inst.symbol();
    let sub = format!("day/{}", inst.cache_dir());
    format!("{}/{}/{}.csv", crate::config::default_cache_path(), sub, symbol)
}

pub fn init() {
    let plugin = Arc::new(DataKLine) as Arc<dyn crate::data::DataAdapter>;
    crate::data::register(plugin);
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::data::market::detect_symbol;

    #[test]
    fn test_kline_update() {
        let adapter = DataKLine;
        let code = "sz002350";
        let inst = detect_symbol(code);
        assert!(inst.can_construct_symbol());
        let date = Timestamp::now();
        adapter.update(&inst, date);
    }
}
