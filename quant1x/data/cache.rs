use crate::data::meta::timestamp::Timestamp;
use std::fs;
use std::time::UNIX_EPOCH;

/// 获取今天初始化时间，对应 Python `cache.get_today_initialized_time`。
pub fn get_today_initialized_time() -> Timestamp {
    let now = Timestamp::now();
    now.pre_market_time_from_current().unwrap_or(now)
}

/// 获取文件最后修改时间，对应 Python `cache.get_filename_modified_time`。
///
/// - 文件不存在 → `Timestamp::zero()`
/// - OS 错误（权限、竞争条件等）→ `Timestamp::zero()`
pub fn get_filename_modified_time(fname: &str) -> Timestamp {
    let meta = match fs::symlink_metadata(fname) {
        Ok(m) => m,
        Err(_) => return Timestamp::zero(),
    };
    let mtime = match meta.modified() {
        Ok(t) => t,
        Err(_) => return Timestamp::zero(),
    };
    let dur = match mtime.duration_since(UNIX_EPOCH) {
        Ok(d) => d,
        Err(_) => return Timestamp::zero(),
    };
    Timestamp::new(dur.as_millis() as i64)
}

/// 增量更新缓存清理的最大天数，对应 Python `MaxCachedDaysToDropOnIncrementalUpdate`。
pub const MAX_CACHED_DAYS_TO_DROP_ON_INCREMENTAL_UPDATE: i32 = 1;

const DEFAULT_BAR_PERIOD: &str = "D";

/// 根据周期标识返回中文名称，对应 Python `cache.get_period_name`。
pub fn get_period_name(period: &str) -> String {
    let upper = period.to_uppercase();
    match upper.as_str() {
        "W" => "周".to_string(),
        "M" => "月".to_string(),
        "Q" => "季".to_string(),
        "Y" => "年".to_string(),
        "D" => "日".to_string(),
        _ => upper,
    }
}

/// 日期格式化，对应 Python `cache.date_format`。
pub fn date_format(date: &str, layout: &str) -> String {
    // 尝试多种常见日期格式解析
    let parsed = chrono::NaiveDate::parse_from_str(date, "%Y-%m-%d")
        .or_else(|_| chrono::NaiveDate::parse_from_str(date, "%Y/%m/%d"))
        .or_else(|_| chrono::NaiveDate::parse_from_str(date, "%Y.%m.%d"))
        .or_else(|_| chrono::NaiveDate::parse_from_str(date, "%B %d, %Y"))
        .or_else(|_| chrono::NaiveDate::parse_from_str(date, "%b %d, %Y"))
        .or_else(|_| chrono::NaiveDate::parse_from_str(date, "%Y%m%d"));
    match parsed {
        Ok(d) => d.format(layout).to_string(),
        Err(_) => date.to_string(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_filename_modified_time_nonexistent() {
        let ts = get_filename_modified_time("/nonexistent/cache_test");
        assert_eq!(ts, Timestamp::zero());
    }

    #[test]
    fn test_get_period_name() {
        assert_eq!(get_period_name("W"), "周");
        assert_eq!(get_period_name("M"), "月");
        assert_eq!(get_period_name("Q"), "季");
        assert_eq!(get_period_name("Y"), "年");
        assert_eq!(get_period_name("D"), "日");
    }

    #[test]
    fn test_date_format() {
        assert_eq!(date_format("2024-06-01", "%Y-%m-%d"), "2024-06-01");
        assert_eq!(date_format("2024/06/01", "%Y-%m-%d"), "2024-06-01");
        assert_eq!(date_format("2024.06.01", "%Y-%m-%d"), "2024-06-01");
    }
}
