#![allow(dead_code)]
use serde::{Deserialize, Serialize};
use serde_yaml;
use std::path::PathBuf;

// Typed application-level configuration parsed once at startup.
use std::collections::HashMap;
use std::sync::LazyLock;

/// 盘前9点
pub const PRE_MARKET_HOUR: u32 = 9;
/// 盘前9点0分
pub const PRE_MARKET_MINUTE: u32 = 0;
/// 盘前9点0分0秒
pub const PRE_MARKET_SECOND: u32 = 0;
/// 每天9点整
pub static GLOBAL_CRON_EXPR_DAILY_INIT: LazyLock<String> = LazyLock::new(|| {
    format!( "{} {} {} * * *", PRE_MARKET_SECOND, PRE_MARKET_MINUTE, PRE_MARKET_HOUR)
});

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct DataSection {
    #[serde(default)]
    pub concurrency: Option<HashMap<String, usize>>,
    #[serde(default)]
    pub cache: Option<CacheSection>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct CacheSection {
    #[serde(default)]
    pub kline: Option<HashMap<String, bool>>,
}

/// Return the path to the config filename (quant1x.yaml) after lazy init.
pub fn config_filename() -> String {
    crate::core::get_configfile_path().to_string()
}

/// Whether running in debug mode per config file (default false)
pub fn is_debug() -> bool {
    let m = crate::core::get_config_map();
    if let Some(v) = m.get("debug") {
        return v.as_bool().unwrap_or(false);
    }
    false
}

pub fn default_home_path() -> String {
    crate::core::get_base_path().to_string()
}

pub fn default_cache_path() -> String {
    crate::core::get_data_path().to_string()
}

pub fn get_meta_path() -> String {
    let p = crate::core::get_meta_path();
    p.to_string_lossy().to_string()
}

pub fn get_logs_path() -> String {
    crate::core::get_logs_path().to_string()
}

pub fn get_calendar_filename() -> String {
    let mut p = std::path::PathBuf::from(get_meta_path());
    p.push("calendar");
    p.to_string_lossy().to_string()
}

pub fn get_security_filename() -> String {
    let mut p = std::path::PathBuf::from(get_meta_path());
    p.push("securities.csv");
    p.to_string_lossy().to_string()
}

// helper: get file paths following C++ layout
pub fn get_xdxr_path() -> String {
    let mut p = std::path::PathBuf::from(default_cache_path());
    p.push("xdxr");
    p.to_string_lossy().to_string()
}

pub fn get_day_path() -> String {
    let mut p = std::path::PathBuf::from(default_cache_path());
    p.push("day");
    p.to_string_lossy().to_string()
}

pub fn get_bar_path(freq: &str) -> String {
    let mut p = std::path::PathBuf::from(default_cache_path());
    p.push(freq);
    p.to_string_lossy().to_string()
}

pub fn get_minute_path() -> String {
    let mut p = std::path::PathBuf::from(default_cache_path());
    p.push("minutes");
    p.to_string_lossy().to_string()
}

// cache id utils (very small port of C++ helpers)
pub fn cache_id(code: &str) -> String {
    // simplistic: just return code; C++ uses market prefix + code
    code.to_string()
}

pub fn cache_id_path(code: &str) -> String {
    let id = cache_id(code);
    if id.len() <= 3 {
        id
    } else {
        format!("{}/{}", &id[..id.len() - 3], id)
    }
}

pub fn get_holding_path() -> String {
    format!("{}/holding", default_cache_path())
}

/// Return the full filename for an xdxr cache file for `code`.
/// Mirrors C++ behavior: files are stored under <cache>/xdxr/<prefix>/<code>.csv
pub fn get_xdxr_filename(code: &str) -> String {
    // try to mirror C++ layout which keeps a prefix path to avoid too many files
    let suffix_len = 3usize;
    let mut path = std::path::PathBuf::from(get_xdxr_path());
    if code.len() > suffix_len {
        let prefix = &code[..code.len() - suffix_len];
        // sanitize prefix to avoid accidental drive letters or path separators
        let safe_prefix: String = prefix
            .chars()
            .map(|c| {
                if c == ':' || c == '\\' || c == '/' {
                    '_'
                } else {
                    c
                }
            })
            .collect();
        path.push(safe_prefix);
    }
    // ensure directory exists when caller needs to write
    let filename = format!("{}.csv", code);
    path.push(filename);
    path.to_string_lossy().to_string()
}

/// Return the full filename for a day bar cache file for `code`.
/// DEPRECATED: Use the Instrument-based path generation in bar.rs instead.
/// Kept for backward compatibility with old callers.
pub fn get_bar_filename(code: &str, forward: bool) -> String {
    // 兼容旧格式: code 如 "sh600000" (长度可能不是8)
    // 对于 Instrument 格式, 使用 detect_symbol 解析后按新规则生成路径
    let inst = crate::data::market::detect_symbol(code);
    if !inst.can_construct_symbol() {
        // 回退到旧逻辑
        if code.len() < 3 {
            log::error!("invalid security code: {}", code);
            return String::new();
        }
        let sub = &code[..code.len() - 3];
        let mut path = std::path::PathBuf::from(get_day_path());
        path.push(sub);
        let ext = if forward { "csv" } else { "raw" };
        path.push(format!("{}.{}", code, ext));
        return path.to_string_lossy().to_string();
    }
    let symbol = inst.symbol();
    let sub = format!("day/{}", inst.cache_dir());
    let mut path = std::path::PathBuf::from(default_cache_path());
    path.push(&sub);
    let ext = if forward { "csv" } else { "raw" };
    path.push(format!("{}.{}", symbol, ext));
    path.to_string_lossy().to_string()
}

/// Return the full filename for a bar cache file for a specific frequency.
/// DEPRECATED: Use the Instrument-based path generation in bar_minute.rs instead.
pub fn get_bar_filename_ex(code: &str, freq: &str) -> String {
    let inst = crate::data::market::detect_symbol(code);
    if !inst.can_construct_symbol() {
        // 回退到旧逻辑
        if code.len() < 3 {
            log::error!("invalid security code: {}", code);
            return String::new();
        }
        let mut path = std::path::PathBuf::from(get_bar_path(freq));
        let sub = &code[..code.len() - 3];
        path.push(sub);
        path.push(format!("{}.csv", code));
        return path.to_string_lossy().to_string();
    }
    let symbol = inst.symbol();
    let sub = format!("{}/{}", freq, inst.cache_dir());
    let mut path = std::path::PathBuf::from(default_cache_path());
    path.push(&sub);
    path.push(format!("{}.csv", symbol));
    path.to_string_lossy().to_string()
}

/// Return the full filename for a minute KLine cache file for `code`.
/// We'll mirror a simple layout under <cache>/minutes/<prefix>/<code>.csv similar to other helpers.
/// Return the full filename for a minute KLine cache file for `code` and `cache_date`.
/// Mirrors C++ get_minute_filename(code, cache_date) which expects `cache_date` in YYYYMMDD
pub fn get_minute_filename(code: &str, cache_date: &str) -> String {
    if code.len() != 8 {
        log::error!("invalid security code length (expected 8): {}", code);
        return String::new();
    }
    let date = cache_date.replace('-', "");
    if date.len() != 8 {
        log::error!(
            "invalid cache_date format for minute filename: {}",
            cache_date
        );
        return String::new();
    }
    let year = &date[0..4];
    let mut path = std::path::PathBuf::from(get_minute_path());
    path.push(year);
    path.push(date);
    path.push(format!("{}.csv", code));
    path.to_string_lossy().to_string()
}

// Minute KLine configuration (mirror C++ config::MinuteKLineConfig)
#[derive(Debug, Clone)]
pub struct MinuteKLineConfig {
    pub frequency: String,
    pub minutes: usize,
    pub enabled: bool,
}

impl Default for MinuteKLineConfig {
    fn default() -> Self {
        Self {
            frequency: "1min".to_string(),
            minutes: 1,
            enabled: false,
        }
    }
}

/// Read minute bar configuration from global yaml data (data.cache.kline)
/// Mirrors C++ datasets::get_minute_bar_config which requires exactly one entry
pub fn get_minute_bar_config() -> MinuteKLineConfig {
    let mut cfg = MinuteKLineConfig::default();
    // Prefer typed config (parsed at startup). This makes config handling
    // single-entry and less error-prone. If typed config is present, use
    // data.cache.kline and pick the first entry (if multiple exist) while
    // logging a warning. Otherwise fall back to legacy YAML logic but still
    // pick the first entry instead of panicking or disabling by default.
    // Try to read typed `data` section from core config map (if present)
    let cmap = crate::core::get_config_map();
    if let Some(data_val) = cmap.get("data") {
        if let Ok(typed) = serde_yaml::from_value::<DataSection>(data_val.clone()) {
            if let Some(kmap) = typed.cache.and_then(|c| c.kline) {
                if !kmap.is_empty() {
                    if kmap.len() > 1 {
                        log::warn!(
                            "typed config: multiple kline entries found, selecting the first one"
                        );
                    }
                    if let Some((k, v)) = kmap.into_iter().next() {
                        cfg.frequency = k;
                        cfg.enabled = v;
                    }
                }
            }
        }
    }
    // parse minutes from frequency using pandas::parse_frequency (implemented in pandas module)
    if cfg.enabled {
        match crate::pandas::parse_frequency(&cfg.frequency) {
            Ok((minutes, freq_norm)) => {
                cfg.minutes = minutes as usize;
                cfg.frequency = freq_norm;
            }
            Err(e) => {
                log::error!(
                    "failed to parse minute frequency '{}': {}",
                    cfg.frequency,
                    e
                );
                // keep defaults but mark disabled to avoid accidental fetches
                cfg.enabled = false;
            }
        }
    }
    cfg
}

/// Return concurrency for a specific data module/adapter key.
///
/// Looks up YAML at `data.concurrency.<key>` then `data.concurrency.default`.
/// Falls back to detected parallelism and caps at 8 to preserve prior behavior.
pub fn get_concurrency_for(key: &str) -> usize {
    // Read from typed config parsed at startup (TYPED_CONFIG). If not present
    // or no matching key, fall back to detected parallelism.
    // Try typed config `data.concurrency` from core config map
    if let Some(data_val) = crate::core::get_config_map().get("data") {
        if let Ok(typed) = serde_yaml::from_value::<DataSection>(data_val.clone()) {
            if let Some(map) = typed.concurrency {
                if let Some(v) = map.get(key) {
                    let mut result = std::cmp::min(*v as usize, 8);
                    if let Some(max) = crate::contrib::data::tdx::client::pool_max_connections() {
                        result = std::cmp::min(result, max);
                    }
                    return result;
                }
                if let Some(v) = map.get("default") {
                    let mut result = std::cmp::min(*v as usize, 8);
                    if let Some(max) = crate::contrib::data::tdx::client::pool_max_connections() {
                        result = std::cmp::min(result, max);
                    }
                    return result;
                }
            }
        }
    }

    // fallback to detected parallelism, cap to 8
    let default = std::thread::available_parallelism()
        .map(|n| n.get())
        .unwrap_or(4);
    let mut result = std::cmp::min(default, 8);
    if let Some(max) = crate::contrib::data::tdx::client::pool_max_connections() {
        result = std::cmp::min(result, max);
    }
    result
}

// Note: runtime hot-reload and ad-hoc YAML caching were removed.
// Configuration is parsed once (typed) via serde into `AppConfig` and stored
// in `TYPED_CONFIG`. Callers should use `get_concurrency_for` which reads
// from the typed config and falls back to sensible defaults.

/// Return the path where block/sector metadata files (tdxzs.cfg, tdxhy.cfg, etc.) are located.
/// We mirror the C++ behavior by looking for a bundled resources/meta directory inside
/// the crate workspace; fall back to <cache>/resources/meta if not present.
pub fn get_block_path() -> String {
    // Return the meta path (where block/sector metadata lives). Ensure the
    // directory exists when possible and return the path as a string. This
    // mirrors the original behavior: block metadata files live under the
    // user's meta directory returned by `get_meta_path()`.
    let p2 = std::path::PathBuf::from(get_meta_path());
    let _ = std::fs::create_dir_all(&p2);
    p2.to_string_lossy().to_string()
}

fn get_quarter_by_date(date: &str) -> String {
    let date = date.replace("-", "");
    if date.len() < 6 {
        return "0000Q0".to_string();
    }
    let year = &date[0..4];
    let month_str = &date[4..6];
    let month: u32 = month_str.parse().unwrap_or(0);
    if month == 0 || month > 12 {
        return "0000Q0".to_string();
    }
    let quarter = (month - 1) / 3 + 1;
    format!("{}Q{}", year, quarter)
}

pub fn top10_holders_filename(code: &str, date: &str) -> String {
    let id_path = cache_id_path(code);
    let quarter = get_quarter_by_date(date);
    let holding_path = get_holding_path();
    let full_path_str = format!("{}/{}/{}.csv", holding_path, quarter, id_path);
    let path = std::path::PathBuf::from(&full_path_str);
    if let Some(parent) = path.parent() {
        let _ = std::fs::create_dir_all(parent);
    }
    full_path_str
}

pub fn quarterly_cache_path(date: &str) -> String {
    let quarter = get_quarter_by_date(date);
    let mut path = std::path::PathBuf::from(default_cache_path());
    path.push("infoq");
    path.push(quarter);
    path.to_string_lossy().to_string()
}

pub fn quarterly_filename(date: &str, keyword: &str) -> String {
    let mut path = std::path::PathBuf::from(quarterly_cache_path(date));
    path.push(format!("{}.csv", keyword));
    path.to_string_lossy().to_string()
}

pub fn reports_filename(date: &str) -> String {
    quarterly_filename(date, "reports")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_quarter_by_date() {
        assert_eq!(get_quarter_by_date("2023-01-01"), "2023Q1");
        assert_eq!(get_quarter_by_date("2023-03-31"), "2023Q1");
        assert_eq!(get_quarter_by_date("2023-04-01"), "2023Q2");
        assert_eq!(get_quarter_by_date("2023-06-30"), "2023Q2");
        assert_eq!(get_quarter_by_date("2023-07-01"), "2023Q3");
        assert_eq!(get_quarter_by_date("2023-09-30"), "2023Q3");
        assert_eq!(get_quarter_by_date("2023-10-01"), "2023Q4");
        assert_eq!(get_quarter_by_date("2023-12-31"), "2023Q4");
        assert_eq!(get_quarter_by_date("20230101"), "2023Q1");
        assert_eq!(get_quarter_by_date("invalid"), "0000Q0");
        assert_eq!(get_quarter_by_date("2023-13-01"), "0000Q0"); // invalid month
    }

    #[test]
    fn test_cache_id_path() {
        assert_eq!(cache_id_path("sh600000"), "sh600/sh600000");
        assert_eq!(cache_id_path("sz000001"), "sz000/sz000001");
        assert_eq!(cache_id_path("bj830000"), "bj830/bj830000");
        assert_eq!(cache_id_path("sh123"), "sh/sh123"); // short code
    }

    #[test]
    #[ignore = "requires config file"]
    fn test_top10_holders_filename() {
        // This test depends on the default cache path, which might vary.
        // We check the suffix structure.
        let filename = top10_holders_filename("sh600000", "2023-03-31");
        assert!(filename.ends_with("holding/2023Q1/sh600/sh600000.csv"));

        let filename2 = top10_holders_filename("sz000001", "2023-12-31");
        assert!(filename2.ends_with("holding/2023Q4/sz000/sz000001.csv"));
    }

    #[test]
    #[ignore = "requires config file"]
    fn test_get_bar_filename() {
        let filename = get_bar_filename("sh600000", true);
        assert!(filename
            .replace('\\', "/")
            .ends_with("day/sh600/sh600000.csv"));

        let filename_raw = get_bar_filename("sh600000", false);
        assert!(filename_raw
            .replace('\\', "/")
            .ends_with("day/sh600/sh600000.raw"));
    }

    #[test]
    #[ignore = "requires config file"]
    fn test_get_minute_filename() {
        let filename = get_minute_filename("sh600000", "2023-01-01");
        assert!(filename
            .replace('\\', "/")
            .ends_with("minutes/2023/20230101/sh600000.csv"));

        let filename2 = get_minute_filename("sh600000", "20230101");
        assert!(filename2
            .replace('\\', "/")
            .ends_with("minutes/2023/20230101/sh600000.csv"));
    }
}
