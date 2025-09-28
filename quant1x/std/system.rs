use std::path::PathBuf;
use std::env;

/// Return the preferred home directory following C++ priority:
/// QUANT1X_HOME, GOX_HOME, HOME (or USERPROFILE on Windows), then fallback to temp dir.
pub fn homedir() -> Option<PathBuf> {
    if let Ok(v) = env::var("QUANT1X_HOME") {
        if !v.is_empty() { return Some(PathBuf::from(v)); }
    }
    if let Ok(v) = env::var("GOX_HOME") {
        if !v.is_empty() { return Some(PathBuf::from(v)); }
    }
    if let Ok(v) = env::var("HOME") {
        if !v.is_empty() { return Some(PathBuf::from(v)); }
    }
#[cfg(target_os = "windows")]
    {
        if let Ok(v) = env::var("USERPROFILE") {
            if !v.is_empty() { return Some(PathBuf::from(v)); }
        }
    }
    // Fallback to standard dirs::home_dir
    if let Some(p) = dirs::home_dir() { return Some(p); }
    // Last resort: temp dir
    Some(std::env::temp_dir())
}
