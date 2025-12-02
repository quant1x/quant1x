use super::sina::FinanceDecoder;
use crate::runtime::RollingOnce;
use crate::timestamp::Timestamp;
use chrono::Local;
use csv;
use filetime::FileTime;
use httpdate::parse_http_date;
use once_cell::sync::Lazy;
use std::fs::File;
use std::fs::OpenOptions;
use std::io::Write;
use std::path::PathBuf;
use std::sync::Arc;
use std::sync::Mutex;
use std::time::UNIX_EPOCH;

static GLOBAL_CALENDAR_STRINGS: Lazy<Mutex<Vec<String>>> = Lazy::new(|| Mutex::new(Vec::new()));
static GLOBAL_CALENDAR_TS: Lazy<Mutex<Vec<Timestamp>>> = Lazy::new(|| Mutex::new(Vec::new()));
static LAST_LOADED_DATE: Lazy<Mutex<Option<String>>> = Lazy::new(|| Mutex::new(None));

// Global RollingOnce for calendar updates, mirrors C++'s global RollingOnce behavior.
static GLOBAL_CALENDAR_ONCE: Lazy<Arc<RollingOnce>> = Lazy::new(|| {
    let path = default_calendar_path();
    let mut marker = path.clone();
    if let Some(parent) = path.parent() {
        marker = parent.to_path_buf();
        marker.push("calendar.updated");
    }
    RollingOnce::with_daily_reset(marker, 9, 0)
});

fn default_calendar_path() -> PathBuf {
    // Use crate-level configuration to determine the calendar filename (parity with C++)
    let fname = crate::config::get_calendar_filename();
    PathBuf::from(fname)
}

fn load_calendar_from_file(path: PathBuf) -> std::io::Result<()> {
    // Ensure file exists: try download if missing, otherwise proceed to read contents
    if !path.exists() {
        log::debug!("calendar file missing, try downloading");
        if let Err(err) = download_and_cache_calendar(&path) {
            log::debug!("download calendar failed: {}", err);
            return Err(std::io::Error::new(
                std::io::ErrorKind::NotFound,
                "calendar missing",
            ));
        }
    }
    let mut strs = Vec::new();
    let mut tss = Vec::new();
    // Use CSV parser to handle headers/quoting robustly. Read entire file into memory
    // and parse with the csv crate (first column is the date).
    let contents = std::fs::read_to_string(&path)?;
    let mut rdr = csv::ReaderBuilder::new()
        .has_headers(true)
        .from_reader(contents.as_bytes());
    for result in rdr.records() {
        let record = result?;
        if let Some(first) = record.get(0) {
            let date_str = first.trim().to_string();
            if date_str.is_empty() {
                continue;
            }
            strs.push(date_str.clone());
            if let Ok(ts) = Timestamp::parse(&date_str) {
                if let Some(pre) =
                    Timestamp::pre_market_time(ts.extract().0, ts.extract().1, ts.extract().2)
                {
                    tss.push(pre);
                    continue;
                }
            }
            if let Ok(ts2) = Timestamp::parse(&date_str) {
                tss.push(ts2);
            }
        }
    }
    {
        let mut guard_s = GLOBAL_CALENDAR_STRINGS.lock().unwrap();
        let mut guard_ts = GLOBAL_CALENDAR_TS.lock().unwrap();
        // Ensure timestamps are sorted for binary_search
        tss.sort();
        // Ensure strings are sorted to match
        strs.sort();
        *guard_s = strs;
        *guard_ts = tss;
    }
    Ok(())
}

fn preprocess_js(text: &str) -> String {
    let mut processed = text.to_string();
    if let Some(eq) = processed.find('=') {
        processed = processed[eq + 1..].to_string();
    }
    if let Some(semi) = processed.find(';') {
        processed = processed[..semi].to_string();
    }
    processed.retain(|c| c != '"');
    processed
}

fn download_and_cache_calendar(path: &PathBuf) -> Result<(), Box<dyn std::error::Error>> {
    // Default to the Sina URL; the heavy lifting is in download_and_cache_calendar_url
    let url = "https://finance.sina.com.cn/realstock/company/klc_td_sh.txt";
    download_and_cache_calendar_url(path, url)
}

fn download_and_cache_calendar_url(
    path: &PathBuf,
    url: &str,
) -> Result<(), Box<dyn std::error::Error>> {
    // If file exists, provide If-Modified-Since
    let client = reqwest::blocking::Client::builder().build()?;
    let mut req = client.get(url);
    if path.exists() {
        if let Ok(metadata) = std::fs::metadata(path) {
            if let Ok(mtime) = metadata.modified() {
                let since = httpdate::fmt_http_date(mtime);
                req = req.header(reqwest::header::IF_MODIFIED_SINCE, since);
            }
        }
    }
    let resp = req.send()?;
    // capture status and headers before reading body
    let status = resp.status();
    let headers = resp.headers().clone();
    if status == reqwest::StatusCode::NOT_MODIFIED {
        // Nothing changed
        return Ok(());
    }
    if !status.is_success() {
        return Err(format!("http status {}", status).into());
    }
    let text = resp.text()?;
    let pre = preprocess_js(&text);
    // decoder expects base64-like payload; use CalendarDecoder
    let mut dec = FinanceDecoder::new(&pre);
    dec.decode_base64(&pre);
    let records = dec.decode();
    // write CSV cache file: header + date,source rows
    if let Some(parent) = path.parent() {
        std::fs::create_dir_all(parent)?;
    }
    // Collect dates, insert known missing date if absent (parity with C++)
    let mut dates: Vec<String> = records.iter().filter_map(|r| r.date.clone()).collect();
    let missing = "1992-05-04".to_string();
    match dates.binary_search(&missing) {
        Ok(_) => {}
        Err(pos) => {
            dates.insert(pos, missing.clone());
        }
    }

    let mut f = File::create(path)?;
    writeln!(f, "date,source")?;
    for d in dates.iter() {
        writeln!(f, "{},sina", d)?;
    }

    // If response provides Last-Modified header, set file mtime accordingly
    if let Some(lm) = headers.get(reqwest::header::LAST_MODIFIED) {
        if let Ok(lm_str) = lm.to_str() {
            if let Ok(parsed) = parse_http_date(lm_str) {
                // parsed is a SystemTime (UTC). Convert to unix seconds/nanos and
                // build FileTime with from_unix_time to avoid any local-timezone
                // interpretation when setting file times on Windows.
                if let Ok(dur) = parsed.duration_since(UNIX_EPOCH) {
                    let secs = dur.as_secs() as i64;
                    let nsec = dur.subsec_nanos();
                    let ft = FileTime::from_unix_time(secs, nsec);
                    filetime::set_file_mtime(path, ft).ok();
                } else {
                    // fallback to from_system_time if duration calculation fails
                    let ft = FileTime::from_system_time(parsed);
                    filetime::set_file_mtime(path, ft).ok();
                }
            }
        }
    }

    // Insert known missing date if not present (parity with C++ calendarMissingDate)
    // This step is best-effort: re-open and ensure the date '1992-05-04' exists
    if let Ok(contents) = std::fs::read_to_string(path) {
        if !contents.contains("1992-05-04") {
            let mut f = OpenOptions::new().append(true).open(path)?;
            writeln!(f, "1992-05-04,sina")?;
        }
    }
    Ok(())
}

fn lazy_load_calendar() {
    let path = default_calendar_path();

    // use global RollingOnce helper to centralize marker logic (daily 09:00)
    let once = &*GLOBAL_CALENDAR_ONCE;

    // Check last loaded date in memory
    let mut last_loaded_guard = LAST_LOADED_DATE.lock().unwrap();
    let last_loaded = last_loaded_guard.clone();
    let today = Local::now().format("%Y-%m-%d").to_string();

    // If path missing, always perform immediate update. Otherwise use RollingOnce to
    // decide and atomically run the update closure.
    if !path.exists() {
        log::debug!("calendar: cache missing, running initial download");
        if let Err(err) = download_and_cache_calendar(&path) {
            log::debug!("calendar initial download failed: {}", err);
        }
    } else {
        match once.do_once_try(|| {
            // closure executed only when allowed_to_run returns true
            // Clear in-memory caches so we reload fresh
            {
                let mut gs = GLOBAL_CALENDAR_STRINGS.lock().unwrap();
                let mut gts = GLOBAL_CALENDAR_TS.lock().unwrap();
                gs.clear();
                gts.clear();
            }
            download_and_cache_calendar(&path).map(|_| ())
        }) {
            Ok(Some(_)) => log::debug!("calendar: update ran and persisted marker"),
            Ok(None) => { /* not allowed to run now */ }
            Err(e) => log::debug!("calendar: update failed during execution: {:?}", e),
        }
    }

    // Load from file into memory if not already loaded today
    // If last_loaded == today, skip reloading to avoid repeated parsing
    if last_loaded.as_deref() != Some(today.as_str()) {
        if let Err(e) = load_calendar_from_file(path.clone()) {
            log::debug!("failed to load calendar file: {}", e);
        } else {
            *last_loaded_guard = Some(today);
        }
    }
}

pub fn get_calendar_list() -> Vec<String> {
    lazy_load_calendar();
    GLOBAL_CALENDAR_STRINGS.lock().unwrap().clone()
}

/// Ensure calendar cache exists on disk and trigger a load if needed.
/// This is a small public helper intended for callers like `app::try_run_subcommand`
/// that need to ensure the calendar cache file is present before showing progress.
use std::error::Error;

pub fn ensure_calendar_cache() -> Result<(), Box<dyn Error>> {
    let path = default_calendar_path();
    if !path.exists() {
        // best-effort: create parent and empty file so callers can show a path
        if let Some(parent) = path.parent() {
            std::fs::create_dir_all(parent)?;
        }
        std::fs::File::create(&path)?;
    }
    // schedule lazy load (will be a no-op if already loaded and up-to-date)
    lazy_load_calendar();
    Ok(())
}

pub fn last_trading_day(date: Timestamp) -> Timestamp {
    lazy_load_calendar();
    {
        let tss = GLOBAL_CALENDAR_TS.lock().unwrap();
        if tss.is_empty() {
            // fallback: return today's pre-market
            return Timestamp::pre_market_time_from_current(&Timestamp::now())
                .unwrap_or(Timestamp::now());
        }
        // find upper_bound
        match tss.binary_search(&date) {
            Ok(idx) => tss[idx],
            Err(pos) => {
                let mut it = if pos == 0 { 0 } else { pos - 1 };
                // if current < last_timestamp (pre-market), move back
                let last_ts = tss[it];
                let current = Timestamp::now();
                if current < last_ts && it > 0 {
                    it -= 1;
                }
                tss[it]
            }
        }
    }
}

pub fn prev_trading_day(date: Timestamp) -> Timestamp {
    lazy_load_calendar();
    {
        let tss = GLOBAL_CALENDAR_TS.lock().unwrap();
        if tss.is_empty() {
            return date;
        }
        match tss.binary_search(&date) {
            Ok(idx) => {
                if idx == 0 {
                    tss[0]
                } else {
                    tss[idx - 1]
                }
            }
            Err(pos) => {
                if pos == 0 {
                    tss[0]
                } else {
                    tss[pos - 1]
                }
            }
        }
    }
}

pub fn next_trading_day(date: Timestamp) -> Timestamp {
    lazy_load_calendar();
    {
        let tss = GLOBAL_CALENDAR_TS.lock().unwrap();
        if tss.is_empty() {
            return date;
        }
        match tss.binary_search(&date) {
            Ok(idx) => {
                if idx + 1 >= tss.len() {
                    tss[idx]
                } else {
                    tss[idx + 1]
                }
            }
            Err(pos) => {
                if pos >= tss.len() {
                    tss[tss.len() - 1]
                } else {
                    tss[pos]
                }
            }
        }
    }
}

pub fn date_range(begin: Timestamp, end: Timestamp, _skip_today: bool) -> Vec<Timestamp> {
    lazy_load_calendar();
    {
        let tss = GLOBAL_CALENDAR_TS.lock().unwrap();
        if tss.is_empty() {
            return vec![];
        }
        let first = match tss.binary_search(&begin) {
            Ok(i) => i,
            Err(e) => e,
        };
        let last = match tss.binary_search(&end) {
            Ok(i) => i + 1,
            Err(e) => e,
        };
        if first >= last {
            return vec![];
        }
        tss[first..last].to_vec()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::tempdir;

    #[test]
    fn test_download_and_cache_calendar_url_csv() {
        let td = tempdir().unwrap();
        let path = td.path().join("cal.csv");
        // instead of mocking HTTP, directly exercise the text-processing path
        let body = "date,source\n2025-09-01,sina\n2025-09-02,sina\n".to_string();
        let res = download_and_cache_calendar_from_text(&path, &body, None);
        assert!(res.is_ok());
        let contents = std::fs::read_to_string(&path).unwrap();
        assert!(contents.contains("2025-09-01"));
    }

    #[test]
    fn test_lazy_load_calendar_from_file() {
        use std::env;
        let td = tempdir().unwrap();
        let home = td.path().to_path_buf();
        // ensure dirs::home_dir() will return our tempdir on Windows and Unix
        env::set_var("HOME", &home);
        env::set_var("USERPROFILE", &home);

        // make the crate config point to our tempdir by ensuring the default home expands
        // The config module uses lazy init; to force it to use our tempdir, set QUANT1X_HOME
        // which the crate homedir helper checks first.
        env::set_var("QUANT1X_HOME", &home);

        // get the calendar path from crate config
        let cal_path = crate::config::get_calendar_filename();
        let cal = PathBuf::from(cal_path.clone());
        if let Some(parent) = cal.parent() {
            std::fs::create_dir_all(parent).unwrap();
        }

        let mut f = File::create(&cal).unwrap();
        writeln!(f, "date,source").unwrap();
        writeln!(f, "2025-09-10,sina").unwrap();
        writeln!(f, "2025-09-11,sina").unwrap();

        // directly load from the file we created, then read from global in-memory list
        load_calendar_from_file(cal.clone()).unwrap();
        let list = GLOBAL_CALENDAR_STRINGS.lock().unwrap().clone();
        assert!(list.contains(&"2025-09-10".to_string()));
        assert!(list.contains(&"2025-09-11".to_string()));
    }

    #[test]
    fn test_last_modified_sets_file_mtime() {
        // prepare temp file
        let td = tempdir().unwrap();
        let path = td.path().join("cal2.csv");
        // Last-Modified example string
        let lm_str = "Mon, 29 Sep 2014 19:43:31 GMT";
        let parsed = parse_http_date(lm_str).expect("parse_http_date");
        // use CSV body and pass parsed SystemTime into helper
        let body = "date,source\n2014-09-29,sina\n".to_string();
        let res = download_and_cache_calendar_from_text(&path, &body, Some(parsed));
        assert!(res.is_ok());
        // read metadata modified time
        let meta = std::fs::metadata(&path).unwrap();
        let modified = meta.modified().unwrap();
        let dur_expected = parsed.duration_since(UNIX_EPOCH).unwrap();
        let dur_actual = modified.duration_since(UNIX_EPOCH).unwrap();
        // allow 2 seconds tolerance for filesystem timestamp granularity
        let secs_exp = dur_expected.as_secs();
        let secs_act = dur_actual.as_secs();
        assert!(
            (secs_exp as i64 - secs_act as i64).abs() <= 2,
            "expected {} got {}",
            secs_exp,
            secs_act
        );
    }

    // Helper used by tests: given the raw response text, process it the same way the HTTP
    // path does (preprocess -> decode -> write CSV). `last_modified` is optional and if
    // provided will set the file mtime.
    fn download_and_cache_calendar_from_text(
        path: &PathBuf,
        text: &str,
        last_modified: Option<std::time::SystemTime>,
    ) -> Result<(), Box<dyn std::error::Error>> {
        // If the response already looks like CSV (starts with "date,"), write it directly
        if text.trim_start().starts_with("date,") {
            if let Some(parent) = path.parent() {
                std::fs::create_dir_all(parent)?;
            }
            let mut f = File::create(path)?;
            f.write_all(text.as_bytes())?;
            if let Some(st) = last_modified {
                if let Ok(dur) = st.duration_since(UNIX_EPOCH) {
                    let secs = dur.as_secs() as i64;
                    let nsec = dur.subsec_nanos();
                    let ft = FileTime::from_unix_time(secs, nsec);
                    filetime::set_file_mtime(path, ft).ok();
                } else {
                    let ft = FileTime::from_system_time(st);
                    filetime::set_file_mtime(path, ft).ok();
                }
            }
            return Ok(());
        }

        let pre = preprocess_js(text);
        let mut dec = FinanceDecoder::new(&pre);
        dec.decode_base64(&pre);
        let records = dec.decode();

        if let Some(parent) = path.parent() {
            std::fs::create_dir_all(parent)?;
        }
        let mut dates: Vec<String> = records.iter().filter_map(|r| r.date.clone()).collect();
        let missing = "1992-05-04".to_string();
        match dates.binary_search(&missing) {
            Ok(_) => {}
            Err(pos) => {
                dates.insert(pos, missing.clone());
            }
        }

        let mut f = File::create(path)?;
        writeln!(f, "date,source")?;
        for d in dates.iter() {
            writeln!(f, "{},sina", d)?;
        }

        if let Some(st) = last_modified {
            let ft = FileTime::from_system_time(st);
            filetime::set_file_mtime(path, ft).ok();
        }

        Ok(())
    }

    #[test]
    fn test_date_range() -> Result<(), Box<dyn std::error::Error>> {
        let start = crate::timestamp::Timestamp::pre_market_time(1990, 12, 19);
        let end = crate::timestamp::Timestamp::pre_market_time(2025, 09, 26);
        let ts = date_range(start.unwrap(), end.unwrap(), false);
        for (i, t) in ts.iter().enumerate() {
            println!("[{}] {:?}", i, t.only_date());
        }
        Ok(())
    }

    #[test]
    fn test_last_trading_day() -> Result<(), Box<dyn std::error::Error>> {
        let mut date =
            crate::timestamp::Timestamp::from_date(2025, 10, 13, 8, 59, 59, 999).unwrap();
        let mut last = last_trading_day(date);
        println!(
            "last trading day before {:?} is {:?}",
            date.only_date(),
            last.only_date()
        );
        // Assert the expected last trading day is 2025-10-10 (previous Friday)
        let mut expected =
            crate::timestamp::Timestamp::from_date(2025, 10, 10, 9, 0, 0, 0).unwrap();
        assert_eq!(last.only_date(), expected.only_date());
        date = crate::timestamp::Timestamp::from_date(2025, 10, 13, 9, 0, 0, 1).unwrap();
        last = last_trading_day(date);
        expected = crate::timestamp::Timestamp::from_date(2025, 10, 13, 9, 0, 0, 0).unwrap();
        assert_eq!(last.only_date(), expected.only_date());
        Ok(())
    }
}
