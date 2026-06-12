use crate::runtime::RollingOnce;
use crate::meta::Timestamp;
use crate::market::{correct_security_code};
use crate::exchange::{get_market_flag};
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

        use crate::exchange::{
            MARKET_BEIJING, MARKET_FLAG_SH, MARKET_FLAG_SZ, MARKET_SHANGHAI, MARKET_SHENZHEN,
        };

        // markets to query (SZ then SH) mirror previous behavior
        let markets: [u16; 3] = [
            MARKET_SHENZHEN as u16,
            MARKET_SHANGHAI as u16,
            MARKET_BEIJING as u16,
        ];
        let mut all: Vec<SecurityInfo> = Vec::new();
        let count = crate::contrib::data::tdx::level1::std::security_list::PRE_REQUEST_MAX;
        for &market in markets.iter() {
            let mut start: u32 = 0;
            loop {
                match crate::contrib::data::tdx::level1::std::security_list::fetch_security_list(market as u16, start, count)
                {
                    Some(resp) => {
                        log::info!(
                            "security list market={} start={} count={}",
                            market,
                            start,
                            resp.count
                        );
                        let cnt = resp.list.len();
                        for e in resp.list.into_iter() {
                            let prefix = get_market_flag(market as u8);
                            let code = format!("{}{}", prefix, e.code);
                            // store as SecurityInfo (pre_close is not stored in SecurityInfo)
                            all.push(SecurityInfo::new(code, e.name, e.vol_unit, e.decimal_point));
                        }
                        if cnt < count as usize {
                            break;
                        }
                        start = start.wrapping_add(count);
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
            writeln!(tmp.as_file_mut(), "Code,VolUnit,DecimalPoint,Name")?;
            for s in all.iter() {
                // PreClose not part of SecurityInfo; write placeholder 0.0 to keep CSV columns compatible
                writeln!(
                    tmp.as_file_mut(),
                    "{},{},{},{}",
                    s.code,
                    s.lot_size,
                    s.price_precision,
                    csv_escape(&s.name),
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
                // rec: Code,VolUnit,DecimalPoint,Name
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

pub fn get_instrument_info(code: &str) -> Option<SecurityInfo> {
    // ensure initialized once (C++ calls global_security_once->Do(init_securities) here)
    let _ = GLOBAL_SECURITY_ONCE.do_once_try(|| -> Result<(), Box<dyn std::error::Error>> {
        init_securities_impl()?;
        Ok(())
    });

    let code_fixed = correct_security_code(code);
    let map = GLOBAL_SECURITY_MAP.lock().unwrap();
    map.get(&code_fixed).cloned()
}
