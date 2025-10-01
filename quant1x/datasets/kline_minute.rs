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
    // Read minute kline config (must mirror C++ datasets::get_minute_kline_config)
    let mkc = crate::config::get_minute_kline_config();
    if !mkc.enabled {
        log::debug!("[DataMinuteKLine] minute kline not enabled in config");
        return;
    }
    // build minute filename using normalized frequency from config
    let filename = crate::config::get_kline_filename_ex(code, &mkc.frequency);
        if filename.is_empty() {
            log::error!("[DataMinuteKLine] cannot build minute filename for {}", code);
            return;
        }
        log::debug!("[DataMinuteKLine] cache filename: {}", filename);

        // ensure parent dir
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

        // constants mirroring C++
        const MAX_KLINE_LOOKBACK_DAYS: usize = 1;
        const SECURITY_BARS_MAX: usize = 800;
        const CN_DEFAULT_TOTALFZNUM: usize = 240; // default trading minutes in a day

        // load existing cache
        let cache_filename = filename.clone();
        let cache_klines: Vec<MinuteKLine> = read_minute_kline_from_csv(&cache_filename);
        let klines_length = cache_klines.len();
        // derive period and kline type from configuration
        let period = if mkc.minutes > 0 { mkc.minutes } else { 1 };
        let mut number_of_day = CN_DEFAULT_TOTALFZNUM / period;
        if number_of_day == 0 {
            number_of_day = 1;
        }
        // map period -> level1 category (mirror C++ switch)
        let kline_type: u16 = match period {
            5 => 0,   // _5MIN
            15 => 1,  // _15MIN
            30 => 2,  // _30MIN
            60 => 3,  // _1HOUR
            _ => 8,   // _1MIN (default)
        };

        let mut klines_offset = MAX_KLINE_LOOKBACK_DAYS * number_of_day;
        let mut adjust_times = 0i32;
        let mut current_start_date =
            crate::Timestamp::pre_market_time(1990, 12, 19).unwrap_or(crate::Timestamp::zero());
        if klines_length > 0 {
            if klines_offset > klines_length {
                klines_offset = klines_length;
            }
            let kline = &cache_klines[klines_length - klines_offset];
            if let Ok(ts) = crate::Timestamp::parse(&kline.date) {
                current_start_date = ts;
            }
            adjust_times = kline.adjustment_count;
        }

        // build date range from start to today's pre-market
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
        // Align behavior with C++: limit total number of minute entries by u16 max (65535)
        // and convert days -> minute entries using `number_of_day` (minutes-per-day / period)
        let max_entries: usize = 65535;
        let total_days = ts_range.len();
        let max_days = if number_of_day > 0 { max_entries / number_of_day } else { total_days };
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
        // fetch pages from level1 using minute category (9 is day in C++ for KLine; for minute we use 1..8 categories depending on minute freq)
        // C++ used category '9' for day; for minute categories it's typically 1..8. We'll use category 1 here as minute bars
        let mut hs: Vec<Vec<crate::level1::SecurityBar>> = Vec::new();
        let step = SECURITY_BARS_MAX;
        let mut start_idx: usize = 0;
        while start_idx < total {
            let remaining = total - start_idx;
            let count = std::cmp::min(step, remaining) as u16;
            match crate::level1::fetch_security_bars(code, kline_type, 1, start_idx as u32, count) {
                Some(resp) => {
                    if resp.list.is_empty() {
                        break;
                    }
                    hs.push(resp.list);
                    if (resp.count as usize) < count as usize {
                        break;
                    }
                    start_idx = start_idx.saturating_add(count as usize);
                }
                None => {
                    log::warn!(
                        "[DataMinuteKLine] fetch_security_bars returned None for {} start={}",
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
        for page in hs.iter() {
            for row in page.iter() {
                let date_time = crate::Timestamp::pre_market_time(row.year, row.month as u32, row.day as u32)
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

        let is_fresh_fetch_require_adjustment = adjust_times == 1;
        let dividends = crate::datasets::xdxr::load_xdxr(code);
        if is_fresh_fetch_require_adjustment {
            calculate_pre_adjust(&mut incremental_klines, current_start_date, &dividends);
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
            calculate_pre_adjust(&mut klines, current_start_date, &dividends);
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
                    log::error!("[DataMinuteKLine] rename failed {} -> {}: {}", tmp, filename, e);
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

// calculate pre-adjustment for minute klines (reuse same algorithm as day KLine)
fn calculate_pre_adjust(
    klines: &mut Vec<MinuteKLine>,
    start_date: crate::Timestamp,
    dividends: &Vec<crate::level1::xdxr::XdxrInfo>,
) {
    if klines.is_empty() {
        return;
    }
    let last_day = klines.last().unwrap().date.clone();
    let ts_last_day = crate::Timestamp::parse(&last_day).unwrap_or(crate::Timestamp::now());
    let ts_last_day =
        crate::Timestamp::pre_market_time_from_current(&ts_last_day).unwrap_or(ts_last_day);
    let last_day_next = crate::exchange::next_trading_day(ts_last_day).only_date();
    let start_date_only = start_date.only_date();

    let xdxr_infos: Vec<crate::level1::xdxr::XdxrInfo> = dividends
        .iter()
        .filter(|x| {
            if x.category as i32 != 1 {
                return false;
            }
            if let Ok(dts) = crate::Timestamp::parse(&x.date) {
                return last_day_next >= dts.only_date();
            }
            false
        })
        .cloned()
        .collect();

    for info in xdxr_infos.iter() {
        if info.date <= start_date_only {
            continue;
        }
        let (m, a) = info.adjust_factor();
        let klines_size = klines.len();
        for i in 0..klines_size {
            if klines[i].date >= info.date {
                break;
            }
            klines[i].open = klines[i].open * m + a;
            klines[i].close = klines[i].close * m + a;
            klines[i].high = klines[i].high * m + a;
            klines[i].low = klines[i].low * m + a;
            let ap = if klines[i].volume != 0.0 {
                klines[i].amount / klines[i].volume
            } else {
                0.0
            };
            let ap_adjusted = ap * m + a;
            if ap_adjusted != 0.0 {
                klines[i].volume = klines[i].amount / ap_adjusted;
            }
            klines[i].adjustment_count += 1;
        }
    }
}

pub fn init() {
    let plugin = Arc::new(DataMinuteKLine) as Arc<dyn DataAdapter>;
    cache::register(plugin);
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_read_kline_from_csv() {
        let adapter = DataMinuteKLine;
        let code = "sh510050";
        let date = Timestamp::now();
        adapter.update(code, date);
    }
}
