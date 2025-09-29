use crate::runtime::RollingOnce;
use crate::timestamp::Timestamp;
use once_cell::sync::Lazy;
use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::{Arc, Mutex};
// protocol-level operations are handled inside level1::security_list::fetch_security_list

static GLOBAL_SECURITY_MAP: Lazy<Mutex<HashMap<String, SecurityInfo>>> =
    Lazy::new(|| Mutex::new(HashMap::new()));

// Global RollingOnce for securities (daily reset at 09:00)
static GLOBAL_SECURITY_ONCE: Lazy<Arc<RollingOnce>> = Lazy::new(|| {
    // create marker next to security filename (parent dir) with name security.updated
    let mut marker = PathBuf::from(crate::config::get_security_filename());
    if let Some(parent) = marker.parent() {
        marker = parent.to_path_buf();
        marker.push("security.updated");
    }
    RollingOnce::with_daily_reset(marker, 9, 0)
});

#[derive(Clone, Debug)]
pub struct SecurityInfo {
    pub code: String,
    pub name: String,
    pub lot_size: u16,
    pub price_precision: u8,
}

impl SecurityInfo {
    pub fn new(code: String, name: String, lot_size: u16, price_precision: u8) -> Self {
        Self {
            code,
            name,
            lot_size,
            price_precision,
        }
    }
}

fn init_securities_impl() -> Result<(), Box<dyn std::error::Error>> {
    // Determine cache filename
    let filename = crate::config::get_security_filename();
    let path = PathBuf::from(&filename);

    // Check last modified time and compare to today's pre-market
    let need_update = match std::fs::metadata(&path) {
        Ok(meta) => {
            if let Ok(mtime) = meta.modified() {
                let msecs = mtime
                    .duration_since(std::time::UNIX_EPOCH)
                    .map(|d| d.as_millis() as i64)
                    .unwrap_or(0);
                let cache_time = Timestamp::from(msecs);
                let check_point = Timestamp::now()
                    .pre_market_time_from_current()
                    .unwrap_or(Timestamp::now());
                // if now >= check_point and cache_time < check_point then stale
                let now = Timestamp::now();
                if now >= check_point && cache_time < check_point {
                    true
                } else {
                    false
                }
            } else {
                true
            }
        }
        Err(_) => true,
    };

    if need_update {
        log::info!("security: updating securities list into {}", filename);

        use crate::exchange::{MARKET_FLAG_SH, MARKET_FLAG_SZ, MARKET_SHANGHAI, MARKET_SHENZHEN};

        // markets to query (SZ then SH) mirror previous behavior
        let markets: [u16; 2] = [MARKET_SHENZHEN as u16, MARKET_SHANGHAI as u16];
        let mut all: Vec<(String, u16, u8, String, f64)> = Vec::new();

        for &market in markets.iter() {
            let mut start: u16 = 0;
            loop {
                match crate::level1::security_list::fetch_security_list(market as u16, start) {
                    Some(resp) => {
                        log::info!(
                            "security list market={} start={} count={}",
                            market,
                            start,
                            resp.count
                        );
                        let cnt = resp.list.len();
                        for e in resp.list.into_iter() {
                            let prefix = if market == MARKET_SHANGHAI as u16 {
                                MARKET_FLAG_SH
                            } else {
                                MARKET_FLAG_SZ
                            };
                            let code = format!("{}{}", prefix, e.code);
                            all.push((code, e.vol_unit, e.decimal_point, e.name, e.pre_close));
                        }
                        // security_list_max in C++ is 1000
                        if cnt < 1000 {
                            break;
                        }
                        start = start.wrapping_add(1000u16);
                    }
                    None => {
                        log::error!(
                            "security list request failed for market {} start {}",
                            market,
                            start
                        );
                        break;
                    }
                }
            }
        }

        // Write CSV with header (Code, VolUnit, DecimalPoint, Name, PreClose)
        fn csv_escape(s: &str) -> String {
            if s.contains(',') || s.contains('"') || s.contains('\n') {
                let mut out = String::from("\"");
                out.push_str(&s.replace("\"", "\"\""));
                out.push('"');
                out
            } else {
                s.to_string()
            }
        }

        if !all.is_empty() {
            if let Some(parent) = std::path::Path::new(&filename).parent() {
                let _ = std::fs::create_dir_all(parent);
            }
            let mut tmp = tempfile::NamedTempFile::new_in(
                std::path::Path::new(&filename)
                    .parent()
                    .unwrap_or_else(|| std::path::Path::new(".")),
            )?;
            use std::io::Write;
            writeln!(tmp.as_file_mut(), "Code,VolUnit,DecimalPoint,Name,PreClose")?;
            for s in all.iter() {
                writeln!(
                    tmp.as_file_mut(),
                    "{},{},{},{},{}",
                    s.0,
                    s.1,
                    s.2,
                    csv_escape(&s.3),
                    s.4
                )?;
            }
            let _ = tmp.persist(&filename);
        }
    }

    // Load CSV into memory and parse fields: Code,VolUnit,DecimalPoint,Name,PreClose
    if let Ok(file) = std::fs::File::open(&filename) {
        let mut rdr = csv::ReaderBuilder::new()
            .has_headers(true)
            .flexible(true)
            .trim(csv::Trim::All)
            .from_reader(file);

        let mut map = GLOBAL_SECURITY_MAP.lock().unwrap();
        map.clear();

        for result in rdr.records() {
            if let Ok(rec) = result {
                // rec: Code,VolUnit,DecimalPoint,Name,PreClose (Name may contain commas)
                let code = rec.get(0).unwrap_or("").trim().to_string();
                if code.is_empty() {
                    continue;
                }
                let vol_unit = rec.get(1).and_then(|s| s.parse::<u16>().ok()).unwrap_or(0);
                let decimal_point = rec.get(2).and_then(|s| s.parse::<u8>().ok()).unwrap_or(0);
                let name = rec.get(3).unwrap_or("").to_string();

                let code_fixed = correct_security_code(&code);
                let info = SecurityInfo::new(code_fixed.clone(), name, vol_unit, decimal_point);
                map.insert(code_fixed, info);
            }
        }
    }

    Ok(())
}

pub fn init_securities() {
    // kept for compatibility: run the init logic directly (non-once)
    let _ = init_securities_impl();
}

pub fn get_security_info(code: &str) -> Option<SecurityInfo> {
    // ensure initialized once (C++ calls global_security_once->Do(init_securities) here)
    let _ = GLOBAL_SECURITY_ONCE.do_once_try(|| -> Result<(), Box<dyn std::error::Error>> {
        init_securities_impl()?;
        Ok(())
    });

    let code_fixed = correct_security_code(code);
    let map = GLOBAL_SECURITY_MAP.lock().unwrap();
    map.get(&code_fixed).cloned()
}

/// Basic normalization similar to C++/Python CorrectSecurityCode
fn correct_security_code(s: &str) -> String {
    let v = s.trim().to_lowercase();
    // handle forms like 600519.sh or 600519.SH
    if v.contains('.') {
        let parts: Vec<&str> = v.split('.').collect();
        if parts.len() >= 2 {
            let code = parts[0];
            let suf = parts[1];
            if suf == "sh" {
                return format!("sh{:0>6}", code);
            } else if suf == "sz" {
                return format!("sz{:0>6}", code);
            }
        }
    }
    // if starts with sh/sz
    if v.starts_with("sh") || v.starts_with("sz") {
        let prefix = &v[..2];
        let rest = v[2..].to_string();
        return format!("{}{:0>6}", prefix, rest);
    }
    // plain numeric code: infer market by leading digit
    let numeric = v.chars().filter(|c| c.is_ascii_digit()).collect::<String>();
    if numeric.starts_with('6') {
        format!("sh{:0>6}", numeric)
    } else {
        format!("sz{:0>6}", numeric)
    }
}
