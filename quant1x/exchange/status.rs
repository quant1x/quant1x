use crate::timestamp::Timestamp;
use std::fs;
use std::io;
use std::time::UNIX_EPOCH;

/// Get file modification time as `Timestamp` (milliseconds since epoch).
pub fn get_filename_modified_time(fname: &str) -> Result<Timestamp, io::Error> {
    let meta = fs::symlink_metadata(fname)?;
    let mtime = meta.modified()?;
    let dur = mtime.duration_since(UNIX_EPOCH).map_err(|_| {
        io::Error::new(io::ErrorKind::Other, "file modified time is before UNIX_EPOCH")
    })?;
    let ms = dur.as_millis() as i64;
    Ok(Timestamp::new(ms))
}

/// Check whether a file should be updated.
///
/// - If reading metadata fails, conservatively returns `true`.
/// - Otherwise delegates to `can_initialize` in `session`.
pub fn should_update_file(fname: &str) -> bool {
    match get_filename_modified_time(fname) {
        Ok(mod_time) => crate::exchange::session::can_initialize(Some(mod_time)),
        Err(_) => true,
    }
}
