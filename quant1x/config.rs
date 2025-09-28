#![allow(dead_code)]
use std::sync::OnceLock;
use serde::{Deserialize, Serialize};
use serde_yaml::Value as YamlValue;
use std::fs;
use std::path::PathBuf;

/// Crate-level configuration and runtime paths.
#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct BaseConfig {
    pub home_dir: String,
    pub filename: String,
    pub cache_dir: String,
    pub logs_dir: String,
    pub running_in_debug: bool,
    /// raw YAML data (optional)
    #[serde(skip_serializing_if = "Option::is_none")]
    pub data: Option<YamlValue>,
}

static GLOBAL_CONFIG: OnceLock<BaseConfig> = OnceLock::new();

fn expand_homedir(path: &str) -> Option<PathBuf> {
    // prefer the crate-level homedir helper which mirrors C++ precedence
    if let Some(home) = crate::std::homedir() {
        if path.starts_with("~/") || path == "~" {
            let mut p = home;
            let tail = if path == "~" { "" } else { &path[2..] };
            if !tail.is_empty() {
                p.push(tail);
            }
            return Some(p);
        }
    }
    Some(PathBuf::from(path))
}

fn default_data_path() -> String {
    "~/.q2x-rust".to_string()
}

fn lazy_init() -> BaseConfig {
    let mut cfg = BaseConfig::default();
    // init path
    let default_home = default_data_path();
    if let Some(home) = expand_homedir(&default_home) {
        // create dir
        if let Err(e) = fs::create_dir_all(&home) {
            log::warn!("failed to create config dir {:?}: {}", home, e);
        }
        // quant1x.yaml inside home
        let mut config_file = home.clone();
        config_file.push("quant1x.yaml");

        cfg.home_dir = home.to_string_lossy().to_string();
        cfg.filename = config_file.to_string_lossy().to_string();

        // try read yaml
        match fs::read_to_string(&cfg.filename) {
            Ok(s) => match serde_yaml::from_str::<YamlValue>(&s) {
                Ok(yaml) => {
                    cfg.running_in_debug = yaml.get("debug").and_then(|v| v.as_bool()).unwrap_or(false);
                    cfg.cache_dir = yaml.get("basedir").and_then(|v| v.as_str()).map(|s| {
                        // expand relative to home if starts with ~
                        expand_homedir(s).map(|p| p.to_string_lossy().to_string()).unwrap_or_else(|| cfg.home_dir.clone())
                    }).unwrap_or_else(|| cfg.home_dir.clone());
                    cfg.data = Some(yaml);
                }
                Err(e) => {
                    log::warn!("failed to parse config yaml {}: {}", cfg.filename, e);
                    cfg.cache_dir = cfg.home_dir.clone();
                }
            },
            Err(_) => {
                // no config file, use defaults
                cfg.cache_dir = cfg.home_dir.clone();
            }
        }
    } else {
        // fallback
        cfg.home_dir = String::from(".");
        cfg.filename = String::from("quant1x.yaml");
        cfg.cache_dir = String::from(".");
    }

    // logs dir
    cfg.logs_dir = format!("{}/logs", cfg.cache_dir);
    // try create logs dir
    let _ = fs::create_dir_all(&cfg.logs_dir);

    cfg
}

/// Get a reference to global BaseConfig, initialize lazily.
pub fn global_config() -> &'static BaseConfig {
    GLOBAL_CONFIG.get_or_init(|| lazy_init())
}

/// Return the path to the config filename (quant1x.yaml) after lazy init.
pub fn config_filename() -> String {
    global_config().filename.clone()
}

/// Whether running in debug mode per config file (default false)
pub fn is_debug() -> bool {
    global_config().running_in_debug
}

pub fn default_home_path() -> String {
    global_config().home_dir.clone()
}

pub fn default_cache_path() -> String {
    global_config().cache_dir.clone()
}

pub fn get_meta_path() -> String {
    let mut p = std::path::PathBuf::from(default_home_path());
    p.push("meta");
    p.to_string_lossy().to_string()
}

pub fn get_logs_path() -> String {
    let mut p = std::path::PathBuf::from(default_cache_path());
    p.push("logs");
    p.to_string_lossy().to_string()
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

pub fn get_kline_path(freq: &str) -> String {
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
    if id.len() <= 3 { id }
    else { format!("{}/{}", &id[..id.len()-3], id) }
}

pub fn get_holding_path() -> String { format!("{}/holding", default_cache_path()) }

/// Return the full filename for an xdxr cache file for `code`.
/// Mirrors C++ behavior: files are stored under <cache>/xdxr/<prefix>/<code>.csv
pub fn get_xdxr_filename(code: &str) -> String {
    // try to mirror C++ layout which keeps a prefix path to avoid too many files
    let suffix_len = 3usize;
    let mut path = std::path::PathBuf::from(get_xdxr_path());
    if code.len() > suffix_len {
        let prefix = &code[..code.len() - suffix_len];
        // sanitize prefix to avoid accidental drive letters or path separators
        let safe_prefix: String = prefix.chars().map(|c| if c == ':' || c == '\\' || c == '/' { '_' } else { c }).collect();
        path.push(safe_prefix);
    }
    // ensure directory exists when caller needs to write
    let filename = format!("{}.csv", code);
    path.push(filename);
    path.to_string_lossy().to_string()
}

/// Return the path where block/sector metadata files (tdxzs.cfg, tdxhy.cfg, etc.) are located.
/// We mirror the C++ behavior by looking for a bundled resources/meta directory inside
/// the crate workspace; fall back to <cache>/resources/meta if not present.
pub fn get_block_path() -> String {
    // prefer resources/meta inside the source tree (useful for tests and dev)
    let mut p = std::path::PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    p.push("resources");
    p.push("meta");
    if p.exists() {
        return p.to_string_lossy().to_string();
    }

    // fallback to cache-based resources/meta
    let mut p2 = std::path::PathBuf::from(default_cache_path());
    p2.push("resources");
    p2.push("meta");
    // ensure directory exists when possible
    let _ = std::fs::create_dir_all(&p2);
    p2.to_string_lossy().to_string()
}
