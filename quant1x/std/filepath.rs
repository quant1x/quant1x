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
    let path = path.trim();
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

    if path == "~" {
        return Ok(home.to_string_lossy().to_string());
    }

    // Use join for safe path concatenation (handles separators correctly)
    // Must skip "~/" (index 2) to avoid treating it as absolute path
    Ok(home.join(&path[2..]).to_string_lossy().to_string())
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::env;

    #[test]
    fn test_homedir() {
        // Save original env vars
        let original_q1x = env::var("QUANT1X_HOME").ok();
        let original_gox = env::var("GOX_HOME").ok();
        let original_home = env::var("HOME").ok();
        #[cfg(target_os = "windows")]
        let original_userprofile = env::var("USERPROFILE").ok();

        // Test QUANT1X_HOME priority
        env::set_var("QUANT1X_HOME", "/tmp/q1x");
        assert_eq!(homedir().unwrap(), PathBuf::from("/tmp/q1x"));
        env::remove_var("QUANT1X_HOME");

        // Test GOX_HOME priority
        env::set_var("GOX_HOME", "/tmp/gox");
        assert_eq!(homedir().unwrap(), PathBuf::from("/tmp/gox"));
        env::remove_var("GOX_HOME");

        // Restore env vars
        if let Some(v) = original_q1x {
            env::set_var("QUANT1X_HOME", v);
        }
        if let Some(v) = original_gox {
            env::set_var("GOX_HOME", v);
        }
        if let Some(v) = original_home {
            env::set_var("HOME", v);
        }
        #[cfg(target_os = "windows")]
        if let Some(v) = original_userprofile {
            env::set_var("USERPROFILE", v);
        }
    }

    #[test]
    fn test_expand_user() {
        let home = homedir().unwrap();
        let home_str = home.to_string_lossy();

        // Test empty path
        assert_eq!(expand_user("").unwrap(), "");

        // Test non-tilde path
        assert_eq!(expand_user("/usr/bin").unwrap(), "/usr/bin");

        // Test just tilde
        assert_eq!(expand_user("~").unwrap(), home_str);

        // Test tilde with path
        let expected = home.join("data").to_string_lossy().to_string();
        assert_eq!(expand_user("~/data").unwrap(), expected);

        // Test invalid tilde usage
        assert!(expand_user("~user").is_err());
    }
}
