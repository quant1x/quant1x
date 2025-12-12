use std::env;
use std::path::PathBuf;

/// Return the preferred home directory following C++ priority:
/// QUANT1X_HOME, GOX_HOME, HOME (or USERPROFILE on Windows), then fallback to temp dir.
pub fn homedir() -> Option<PathBuf> {
    if let Ok(v) = env::var("QUANT1X_HOME") {
        if !v.is_empty() {
            return Some(PathBuf::from(v));
        }
    }
    if let Ok(v) = env::var("GOX_HOME") {
        if !v.is_empty() {
            return Some(PathBuf::from(v));
        }
    }
    if let Ok(v) = env::var("HOME") {
        if !v.is_empty() {
            return Some(PathBuf::from(v));
        }
    }
    #[cfg(target_os = "windows")]
    {
        if let Ok(v) = env::var("USERPROFILE") {
            if !v.is_empty() {
                return Some(PathBuf::from(v));
            }
        }
    }
    // Fallback to standard dirs::home_dir
    if let Some(p) = dirs::home_dir() {
        return Some(p);
    }
    // Last resort: temp dir
    Some(std::env::temp_dir())
}

pub fn expand_user(path: &str) -> Result<String, String> {
    if path.is_empty() {
        return Ok(path.to_string());
    }

    if !path.starts_with('~') {
        return Ok(path.to_string());
    }

    if path.len() > 1 && !path.starts_with("~/") && !path.starts_with("~\\") {
        return Err("cannot expand user-specific home dir".to_string());
    }

    let home = homedir().ok_or("Could not find home directory")?;
    let home_str = home.to_str().ok_or("Invalid home directory path")?;

    if path == "~" {
        return Ok(home_str.to_string());
    }

    // Handle both / and \ separators
    let separator = std::path::MAIN_SEPARATOR;
    let path_rest = &path[2..]; // Skip "~/" or "~\"

    Ok(format!("{}{}{}", home_str, separator, path_rest))
}
