// Lightweight Rust implementations mirroring quant1x std strings helpers

/// Convert ASCII letters to lower-case; leaves other characters unchanged.
pub fn to_lower<S: AsRef<str>>(input: S) -> String {
    input.as_ref().chars().map(|c| c.to_ascii_lowercase()).collect()
}

/// Convert ASCII letters to upper-case; leaves other characters unchanged.
pub fn to_upper<S: AsRef<str>>(input: S) -> String {
    input.as_ref().chars().map(|c| c.to_ascii_uppercase()).collect()
}

/// Trim ASCII whitespace characters (space, tab, LF, CR) from both ends.
pub fn trim<S: AsRef<str>>(s: S) -> String {
    s.as_ref()
        .trim_matches(|c: char| c == ' ' || c == '\t' || c == '\n' || c == '\r')
        .to_string()
}

/// Return true if `s` starts with any of the provided `prefixes`.
pub fn starts_with<S: AsRef<str>, P: AsRef<str>>(s: S, prefixes: &[P]) -> bool {
    let s = s.as_ref();
    if s.is_empty() || prefixes.is_empty() {
        return false;
    }
    for p in prefixes {
        let p = p.as_ref();
        if s.len() >= p.len() && s.starts_with(p) {
            return true;
        }
    }
    false
}

/// Return true if `s` ends with any of the provided `suffixes`.
pub fn ends_with<S: AsRef<str>, P: AsRef<str>>(s: S, suffixes: &[P]) -> bool {
    let s = s.as_ref();
    if s.is_empty() || suffixes.is_empty() {
        return false;
    }
    for suf in suffixes {
        let suf = suf.as_ref();
        if s.len() >= suf.len() && s.ends_with(suf) {
            return true;
        }
    }
    false
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_case_and_trim() {
        assert_eq!(to_lower("AbC-好"), "abc-好");
        assert_eq!(to_upper("aBc-好"), "ABC-好");
        assert_eq!(trim("  \thello\n"), "hello");
    }

    #[test]
    fn test_start_end() {
        let p = ["pre", "no"];
        assert!(starts_with("prefix", &p));
        let s = ["fix", "x"]; // suffix candidates
        assert!(ends_with("prefix", &s));
    }
}
