use std::fs;
use std::path::PathBuf;

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

    // Now call into the public API that triggers loading. The exchange module
    // re-exports its functions at the crate root, so call `quant1x::get_calendar_list()`.
    // This should attempt to open the file, then download and write it.
    let list = quant1x::get_calendar_list();

    // After the call, the cache file should exist and contain at least one line.
    assert!(path.exists(), "calendar cache file was not created: {:?}", path);

    let contents = fs::read_to_string(&path).expect("read created calendar cache");
    // Count only date rows (skip CSV header). Lines like "YYYY-MM-DD,source" are valid.
    let date_lines: Vec<&str> = contents
        .lines()
        .map(|l| l.trim())
        .filter(|l| !l.is_empty())
        .filter(|l| {
            // skip header if present
            if l.to_lowercase().starts_with("date") { return false; }
            // expect format YYYY-MM-DD, maybe with a trailing ,source
            if let Some(first) = l.split(',').next() {
                let s = first.trim();
                return s.len() == 10 && s.as_bytes()[4] == b'-' && s.as_bytes()[7] == b'-';
            }
            false
        })
        .collect();
    assert!(!date_lines.is_empty(), "calendar cache file has no date rows");

    // And the returned in-memory list should have the same length as the date rows.
    assert_eq!(date_lines.len(), list.len(), "in-memory list and file date rows differ");
}
