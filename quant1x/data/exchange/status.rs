use crate::data::meta::timestamp::Timestamp;
use std::fs;
use std::time::UNIX_EPOCH;

/// Get file modification time as `Timestamp` (milliseconds since epoch).
///
/// Returns `Some(Timestamp)` on success, or `None` if metadata cannot be read.
pub fn get_filename_modified_time(fname: &str) -> Option<Timestamp> {
    let meta = match fs::symlink_metadata(fname) {
        Ok(m) => m,
        Err(_) => return None,
    };
    let mtime = match meta.modified() {
        Ok(t) => t,
        Err(_) => return None,
    };
    let dur = match mtime.duration_since(UNIX_EPOCH) {
        Ok(d) => d,
        Err(_) => return None,
    };
    let ms = dur.as_millis() as i64;
    Some(Timestamp::new(ms))
}

/// Check whether the given file should be initialized.
///
/// Mirrors C++ `should_initialize_file`: returns true on metadata error,
/// otherwise delegates to `session::can_initialize`.
pub fn should_initialize_file(fname: &str) -> bool {
    match get_filename_modified_time(fname) {
        Some(mod_time) => crate::exchange::session::can_initialize(Some(mod_time)),
        None => true,
    }
}

/// Check whether a file should be updated in real time.
///
/// Mirrors C++ `should_update_file`: returns true on metadata error,
/// otherwise asks `session::can_update_in_realtime` and returns the boolean.
pub fn should_update_file(fname: &str) -> bool {
    match get_filename_modified_time(fname) {
        Some(mod_time) => crate::exchange::session::can_update_in_realtime(Some(mod_time)).0,
        None => true,
    }
}
