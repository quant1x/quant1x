use csv;
use std::fs;
use std::path::PathBuf;
use std::time::{SystemTime, UNIX_EPOCH};
use reqwest;

/// Integration test: when the local calendar cache is missing, the loader should
/// download, decode using CalendarDecoder, and write the cache file.
#[test]
fn download_and_cache_calendar_creates_file() {
    // Use the crate's helper so the test follows the same config precedence.
    let path = PathBuf::from(quant1x::get_calendar_filename());

    // If the cache exists, remove it to force a download.
    if path.exists() {
        fs::remove_file(&path).expect("remove old calendar cache");
    }

    // Also remove any existing marker so RollingOnce will run the update closure
    let mut marker = path.clone();
    if let Some(parent) = path.parent() {
        marker = parent.to_path_buf();
        marker.push("calendar.updated");
    }
    if marker.exists() {
        let _ = fs::remove_file(&marker);
    }

    // Now call into the public API that triggers loading. The exchange module
    // re-exports its functions at the crate root, so call `quant1x::get_calendar_list()`.
    // This should attempt to open the file, then download and write it.
    let list = quant1x::get_calendar_list();

    // After the call, the cache file should exist and contain at least one line.
    assert!(
        path.exists(),
        "calendar cache file was not created: {:?}",
        path
    );

    let contents = fs::read_to_string(&path).expect("read created calendar cache");
    // Parse CSV robustly (skip header) and collect date rows from first column
    let mut rdr = csv::ReaderBuilder::new()
        .has_headers(true)
        .from_reader(contents.as_bytes());
    let mut date_lines: Vec<String> = Vec::new();
    for result in rdr.records() {
        let record = result.expect("csv record");
        if let Some(first) = record.get(0) {
            let s = first.trim();
            if s.len() == 10 && s.as_bytes()[4] == b'-' && s.as_bytes()[7] == b'-' {
                date_lines.push(s.to_string());
            }
        }
    }
    assert!(
        !date_lines.is_empty(),
        "calendar cache file has no date rows"
    );

    // And the returned in-memory list should have the same length as the date rows.
    assert_eq!(
        date_lines.len(),
        list.len(),
        "in-memory list and file date rows differ"
    );

    // Verify cache mtime matches remote Last-Modified (if available) and
    // marker mtime is recent (local completion time). Marker is located next
    // to the calendar file under meta path as `calendar.updated`.
    let cache_secs = fs::metadata(&path).unwrap().modified().unwrap().duration_since(UNIX_EPOCH).unwrap().as_secs() as i64;

    // fetch HEAD to read Last-Modified
    match reqwest::blocking::Client::new().head("https://finance.sina.com.cn/realstock/company/klc_td_sh.txt").send() {
        Ok(resp) => {
            if let Some(lm) = resp.headers().get(reqwest::header::LAST_MODIFIED) {
                if let Ok(lm_str) = lm.to_str() {
                    if let Ok(parsed) = httpdate::parse_http_date(lm_str) {
                        if let Ok(dur) = parsed.duration_since(UNIX_EPOCH) {
                            let remote_secs = dur.as_secs() as i64;
                            println!("remote Last-Modified secs: {}", remote_secs);
                            println!("cache secs: {}", cache_secs);
                            assert_eq!(cache_secs, remote_secs, "cache mtime does not match remote Last-Modified");
                        }
                    }
                }
            }
        }
        Err(e) => {
            println!("failed to fetch remote headers: {}", e);
        }
    }

    // marker file
    let mut marker = path.clone();
    if let Some(parent) = path.parent() {
        marker = parent.to_path_buf();
        marker.push("calendar.updated");
    }
    if marker.exists() {
        if let Ok(mi) = fs::metadata(&marker) {
            if let Ok(mtime) = mi.modified() {
                if let Ok(dur) = mtime.duration_since(UNIX_EPOCH) {
                    let marker_secs = dur.as_secs() as i64;
                    let now_secs = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_secs() as i64;
                    println!("marker secs: {}  now secs: {}", marker_secs, now_secs);
                    assert!((now_secs - marker_secs) < 10, "marker mtime not recent");
                }
            }
        }
    }
}
