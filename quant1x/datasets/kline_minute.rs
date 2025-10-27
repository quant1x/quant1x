use crate::cache::{self, DataAdapter, Kind};
use crate::Timestamp;
use serde::{Deserialize, Serialize};
use std::sync::Arc;

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct MinuteKLine {
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

impl MinuteKLine {
    pub fn headers() -> Vec<String> {
        vec![
            "Date".into(),
            "Open".into(),
            "Close".into(),
            "High".into(),
            "Low".into(),
            "Volume".into(),
            "Amount".into(),
            "Up".into(),
            "Down".into(),
            "Datetime".into(),
            "AdjustmentCount".into(),
        ]
    }
}

// 推断成交量单位 (参照 C++ 实现)。遍历获取的 SecurityBar 列表, 使用第一条有效记录的
// (Amount / Vol) 与典型价格(平均 OCHL)和 High 比较, 向上取整到 10 的次幂以推断单位。
fn infer_bar_vol_unit(hs: &Vec<Vec<crate::level1::SecurityBar>>) -> f64 {
    for vec in hs.iter() {
        for row in vec.iter() {
            if row.amount <= 0.0 || row.vol <= 0.0 {
                continue;
            }
            let mut typical = (row.open + row.close + row.high + row.low) / 4.0;
            if typical <= 0.0 {
                typical = row.close;
            }
            if typical <= 0.0 {
                continue;
            }
            let implied = row.amount / row.vol;
            if !implied.is_finite() || implied <= 0.0 || row.high <= 0.0 {
                return 1.0;
            }
            if implied <= row.high {
                return 1.0;
            }
            let ratio = implied / row.high;
            if !ratio.is_finite() || ratio <= 1.0 {
                return 1.0;
            }
            let expd = ratio.log10().ceil();
            let mut expi = expd as i32;
            if expi < 0 {
                expi = 0;
            }
            if expi > 9 {
                expi = 9;
            }
            return 10f64.powi(expi);
        }
    }
    1.0
}

#[derive(Debug)]
pub struct DataMinuteKLine;

impl cache::Schema for DataMinuteKLine {
    fn kind(&self) -> Kind {
        crate::datasets::BaseMinuteKLine
    }
    fn owner(&self) -> String {
        crate::cache::DEFAULT_DATA_PROVIDER.to_string()
    }
    fn key(&self) -> String {
        "min".to_string()
    }
    fn name(&self) -> String {
        "分钟K线".to_string()
    }
    fn usage(&self) -> String {
        "分钟K线".to_string()
    }
}

impl DataAdapter for DataMinuteKLine {
    fn print(&self, _code: &str, _dates: &[Timestamp]) {}

    fn update(&self, code: &str, _date: Timestamp) {
        // 读取分钟 K 线配置（必须与 C++ 中的 datasets::get_minute_kline_config 保持一致）
        let mkc = crate::config::get_minute_kline_config();
        if !mkc.enabled {
            log::debug!("[DataMinuteKLine] minute kline not enabled in config");
            return;
        }
        // 使用配置中的频率构建分钟 K 线缓存文件名
        let filename = crate::config::get_kline_filename_ex(code, &mkc.frequency);
        if filename.is_empty() {
            log::error!(
                "[DataMinuteKLine] cannot build minute filename for {}",
                code
            );
            return;
        }
        log::debug!("[DataMinuteKLine] cache filename: {}", filename);

        // 确保父目录存在
        let path = std::path::Path::new(&filename);
        if let Some(parent) = path.parent() {
            if let Err(e) = std::fs::create_dir_all(parent) {
                log::error!(
                    "[DataMinuteKLine] failed to create parent dir {:?}: {}",
                    parent,
                    e
                );
                return;
            }
        }

        // 常量设置（与 C++ 保持一致）
        const MAX_KLINE_LOOKBACK_DAYS: usize = 1;
        const SECURITY_BARS_MAX: usize = 800;
        const CN_DEFAULT_TOTALFZNUM: usize = 240; // default trading minutes in a day

        // 加载本地缓存（如果存在）
        let cache_filename = filename.clone();
        let cache_klines: Vec<MinuteKLine> = read_minute_kline_from_csv(&cache_filename);
        let klines_length = cache_klines.len();
        // 从配置推断周期和对应的 K 线类型
        let period = if mkc.minutes > 0 { mkc.minutes } else { 1 };
        let mut number_of_day = CN_DEFAULT_TOTALFZNUM / period;
        if number_of_day == 0 {
            number_of_day = 1;
        }
        // map period -> level1 category (mirror C++ switch)
        let kline_type: crate::level1::KLineType = match period {
            5 => crate::level1::KLineType::_5Min,
            15 => crate::level1::KLineType::_15Min,
            30 => crate::level1::KLineType::_30Min,
            60 => crate::level1::KLineType::_1Hour,
            _ => crate::level1::KLineType::_1Min, // default
        };

        // Align klines offset to a fixed block size (floor alignment), mirroring the C++ logic.
        // Ensure (klines_length - klines_offset) is an integer multiple of `min_fixed_offset`.
        let min_fixed_offset = MAX_KLINE_LOOKBACK_DAYS * number_of_day;
        let mut klines_offset = min_fixed_offset;
        let mut adjust_times = 0i32;
        // 如果没有缓存，则使用一个非常早的默认日期
        let mut current_start_date =
            crate::Timestamp::pre_market_time(1990, 12, 19).unwrap_or(crate::Timestamp::zero());
        if klines_length > 0 {
            if klines_offset > klines_length {
                klines_offset = klines_length;
            }
            // candidate: 原始候选起点索引
            let candidate = if klines_length > klines_offset {
                klines_length - klines_offset
            } else {
                0
            };
            // 使用 floor 对齐到 min_fixed_offset 的倍数，确保 (klines_length - klines_offset) 为该块大小的整数倍
            let mut aligned = if min_fixed_offset > 0 {
                (candidate / min_fixed_offset) * min_fixed_offset
            } else {
                0
            };
            // 边界保护
            if aligned >= klines_length {
                aligned = 0;
            }
            // 重新计算 klines_offset，使得 klines_length - klines_offset == aligned
            klines_offset = klines_length - aligned;
            // 根据对齐后的索引取出对应的日期作为拉取起点，并保留该边界行的 adjustment_count
            let kline = &cache_klines[aligned];
            // kline.date 是字符串, 尝试解析并转换为盘前时间
            if let Ok(mut ts) = crate::Timestamp::parse(&kline.date) {
                ts = crate::Timestamp::pre_market_time_from_current(&ts).unwrap_or(ts);
                current_start_date = ts;
            }
            adjust_times = kline.adjustment_count;

            // 如果 aligned 看起来不是某个交易日的首条记录，记录警告以便人工审查。
            if aligned > 0 && cache_klines[aligned - 1].date == cache_klines[aligned].date {
                log::warn!(
                    "[DataMinuteKLine] aligned index {} is not day-first for {} (date={})",
                    aligned,
                    code,
                    cache_klines[aligned].date
                );
            }
        }

        // 构建从起始日期到今日盘前的日期范围
        let mut current_end_date =
            crate::Timestamp::pre_market_time_from_current(&crate::Timestamp::now())
                .unwrap_or(crate::Timestamp::now());
        let ts_range = crate::exchange::date_range(current_start_date, current_end_date, false);
        if ts_range.is_empty() {
            log::debug!("[DataMinuteKLine] empty date range for {}", code);
            return;
        }
        log::info!(
            "[DataMinuteKLine] updating {} from {} to {} ({} days) with period {} minutes",
            code,
            current_start_date.only_date(),
            current_end_date.only_date(),
            ts_range.len(),
            period
        );
        // 与 C++ 行为对齐：将分钟总条目数限制在 u16 最大值（65535），并使用 number_of_day 将天数转换为分钟条目数
        let max_entries: usize = 65535;
        let total_days = ts_range.len();
        let max_days = if number_of_day > 0 {
            max_entries / number_of_day
        } else {
            total_days
        };
        let days = std::cmp::min(max_days, total_days);
        if days == 0 {
            log::debug!("[DataMinuteKLine] empty date range for {}", code);
            return;
        }
        let total = days * number_of_day;
        // 从后往前取days的交易日期
        current_start_date = ts_range[total_days - days];
        current_end_date = ts_range[total_days - 1];
        log::info!(
            "[DataMinuteKLine] fetching {} days from {} to {} ({} entries)",
            days,
            current_start_date.only_date(),
            current_end_date.only_date(),
            total
        );
        // 从 level1 分页拉取分钟数据（C++ 中日线用类目 9；分钟线依据频率使用 1..8，这里简化使用 1 作为分钟类目）
        let mut hs: Vec<Vec<crate::level1::SecurityBar>> = Vec::new();
        let step = SECURITY_BARS_MAX;
        let mut start_idx: usize = 0;
        while start_idx < total {
            let remaining = total - start_idx;
            let count = std::cmp::min(step, remaining) as u16;
            // 最低可观测性：在拉取每页数据前记录请求参数
            log::info!(
                "[DataMinuteKLine] fetch request: code={} kline_type={:?} start_idx={} count={} total={}",
                code,
                kline_type,
                start_idx,
                count,
                total
            );
            match crate::datasets::kline_raw::fetch_kline(code, start_idx as u32, count, kline_type)
            {
                Some(resp) if !resp.list.is_empty() => {
                    let response_len = resp.list.len();
                    hs.push(resp.list);
                    if response_len < count as usize {
                        break;
                    }
                    start_idx = start_idx.saturating_add(count as usize);
                }
                _ => {
                    log::warn!(
                        "[DataMinuteKLine] fetch_kline returned empty for {} start={}",
                        code,
                        start_idx
                    );
                    break;
                }
            }
        }

        if hs.is_empty() {
            // 空列表不保存CSV，包括表头
            return;
        }

        // reverse pages to ascending time (C++ behavior)
        hs.reverse();

        let mut incremental_klines: Vec<MinuteKLine> = Vec::new();
        let bar_vol_unit = infer_bar_vol_unit(&hs);
        for page in hs.iter() {
            for row in page.iter() {
                let date_time =
                    crate::Timestamp::pre_market_time(row.year, row.month as u32, row.day as u32)
                        .unwrap_or(crate::Timestamp::now());
                if date_time < current_start_date || date_time > current_end_date {
                    continue;
                }
                let kx = MinuteKLine {
                    date: date_time.only_date(),
                    open: row.open,
                    close: row.close,
                    high: row.high,
                    low: row.low,
                    volume: row.vol * bar_vol_unit,
                    amount: row.amount,
                    up: row.up_count as i32,
                    down: row.down_count as i32,
                    datetime: row.datetime.clone(),
                    adjustment_count: 0,
                };
                incremental_klines.push(kx);
            }
        }

        let is_fresh_fetch_require_adjustment = adjust_times == 1;
        let dividends = crate::datasets::xdxr::load_xdxr(code);
        if is_fresh_fetch_require_adjustment {
            apply_forward_adjustment_for_event!(&mut incremental_klines, current_start_date, &dividends);
        }

        // merge cache and incremental
        let mut klines: Vec<MinuteKLine> = Vec::new();
        if klines_length > klines_offset {
            klines.extend_from_slice(&cache_klines[..(klines_length - klines_offset)]);
        }
        if klines.is_empty() {
            klines = incremental_klines.clone();
        } else {
            klines.extend(incremental_klines.into_iter());
        }

        if !is_fresh_fetch_require_adjustment {
            apply_forward_adjustment_for_event!(&mut klines, current_start_date, &dividends);
        }

        // persist
        if klines.is_empty() {
            // 空列表不保存CSV，包括表头
            return;
        }
        let tmp = format!("{}.tmp", filename);
        match std::fs::File::create(&tmp) {
            Ok(f) => {
                let mut w = csv::Writer::from_writer(f);
                if let Err(e) = w.write_record(MinuteKLine::headers()) {
                    log::error!("[DataMinuteKLine] write header failed: {}", e);
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
                        log::error!("[DataMinuteKLine] write row failed: {}", e);
                    }
                }
                let _ = w.flush();
                if let Err(e) = std::fs::rename(&tmp, &filename) {
                    log::error!(
                        "[DataMinuteKLine] rename failed {} -> {}: {}",
                        tmp,
                        filename,
                        e
                    );
                }
            }
            Err(e) => {
                log::error!("[DataMinuteKLine] create tmp {} failed: {}", tmp, e);
            }
        }
    }
}

// read minute CSV
fn read_minute_kline_from_csv(filename: &str) -> Vec<MinuteKLine> {
    let mut klines: Vec<MinuteKLine> = Vec::new();
    match std::fs::File::open(filename) {
        Ok(f) => {
            let mut rdr = csv::ReaderBuilder::new().has_headers(true).from_reader(f);
            match rdr
                .deserialize::<MinuteKLine>()
                .collect::<Result<Vec<MinuteKLine>, csv::Error>>()
            {
                Ok(v) => klines = v,
                Err(e) => log::error!(
                    "[DataMinuteKLine] failed to deserialize minute file {}: {}",
                    filename,
                    e
                ),
            }
        }
        Err(_) => {}
    }
    klines
}

pub fn init() {
    let plugin = Arc::new(DataMinuteKLine) as Arc<dyn DataAdapter>;
    cache::register(plugin);
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_minute_kline_update() {
        let adapter: DataMinuteKLine = DataMinuteKLine;
        let code = "sh510050";
        let date = Timestamp::now();
        adapter.update(code, date);
    }
}
