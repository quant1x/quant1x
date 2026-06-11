use crate::exchange::SecurityType;
use std::fmt;

// Note: Exchange codes are exposed via `ExchangeCode` constants below.

// ExchangeCode newtype mirrors Go's `ExchangeCode` (string alias) and provides
// conversion to `ExchangeId`.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ExchangeCode(pub &'static str);

impl ExchangeCode {
    pub const fn new(s: &'static str) -> Self {
        ExchangeCode(s)
    }

    pub fn as_str(&self) -> &str {
        self.0
    }

    pub fn id(&self) -> ExchangeId {
        match self.0 {
            x if x == EXCHANGE_SZSE.as_str() => ExchangeId::ShenZhen,
            x if x == EXCHANGE_SSE.as_str() => ExchangeId::ShangHai,
            x if x == EXCHANGE_BSE.as_str() => ExchangeId::BeiJing,
            x if x == EXCHANGE_HKEX.as_str() => ExchangeId::HongKong,
            x if x == EXCHANGE_US.as_str() => ExchangeId::USA,
            _ => ExchangeId::Unknown,
        }
    }
}

impl std::fmt::Display for ExchangeCode {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.0)
    }
}

// Common ExchangeCode constants (match Go names)
pub const EXCHANGE_UNKNOWN: ExchangeCode = ExchangeCode::new("unknown");
pub const EXCHANGE_SSE: ExchangeCode = ExchangeCode::new("sh");
pub const EXCHANGE_SZSE: ExchangeCode = ExchangeCode::new("sz");
pub const EXCHANGE_BSE: ExchangeCode = ExchangeCode::new("bj");
pub const EXCHANGE_HKEX: ExchangeCode = ExchangeCode::new("hk");
pub const EXCHANGE_US: ExchangeCode = ExchangeCode::new("us");

#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ExchangeId {
    Unknown = 255,
    ShenZhen = 0,
    ShangHai = 1,
    BeiJing = 2,
    HongKong = 21,
    USA = 22,
}

impl fmt::Display for ExchangeId {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let s = match self {
            ExchangeId::Unknown => "unknown",
            ExchangeId::ShenZhen => EXCHANGE_SZSE.as_str(),
            ExchangeId::ShangHai => EXCHANGE_SSE.as_str(),
            ExchangeId::BeiJing => EXCHANGE_BSE.as_str(),
            ExchangeId::HongKong => EXCHANGE_HKEX.as_str(),
            ExchangeId::USA => EXCHANGE_US.as_str(),
        };
        write!(f, "{}", s)
    }
}

#[derive(Debug, Clone)]
pub struct ExchangeInfo {
    pub id: ExchangeId,
    pub code: String,
    pub name: String,
    pub description: Option<String>,
    pub is_active: bool,
}

impl ExchangeInfo {
    pub fn new(code: &str, name: &str, desc: Option<&str>, id: ExchangeId) -> Self {
        Self {
            id,
            code: code.to_string(),
            name: name.to_string(),
            description: desc.map(|s| s.to_string()),
            is_active: true,
        }
    }

    pub fn validate(&self) -> Result<(), String> {
        if self.code.is_empty() {
            return Err("exchange code cannot be empty".into());
        }
        if self.name.is_empty() {
            return Err("exchange name cannot be empty".into());
        }
        Ok(())
    }
}

impl fmt::Display for ExchangeInfo {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}({})", self.name, self.code)
    }
}

#[derive(Debug, Clone)]
pub struct SecurityCode {
    pub market: ExchangeId,
    pub symbol: String,
    pub typ: SecurityType,
}

impl SecurityCode {
    pub fn new(market: ExchangeId, symbol: &str, typ: SecurityType) -> Self {
        Self {
            market,
            symbol: symbol.to_string(),
            typ,
        }
    }

    pub fn validate(&self) -> Result<(), String> {
        if self.symbol.is_empty() {
            return Err("security code symbol cannot be empty".into());
        }
        Ok(())
    }
}

impl fmt::Display for SecurityCode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}{}", self.market, self.symbol)
    }
}


// Minimal Rust implementation of market flag helper mirroring C++ `GetMarketFlag`
pub fn get_market_flag(market: u8) -> &'static str {
    match market {
        MARKET_SHENZHEN => MARKET_FLAG_SZ,
        MARKET_BEIJING => MARKET_FLAG_BJ,
        MARKET_HONGKONG => MARKET_FLAG_HK,
        MARKET_USA => MARKET_FLAG_US,
        MARKET_SHANGHAI => MARKET_FLAG_SH,
        _ => MARKET_FLAG_SH,
    }
}

/// Build a security code string like C++ `exchange::GetMarketFlag(...) + code`
pub fn security_code(market: u8, code: &str) -> String {
    format!("{}{}", get_market_flag(market), code)
}

/// Detect market id, market flag and pure code from a security string.
/// Mirrors C++ exchange::DetectMarket behavior for common prefixes.
pub fn detect_market(security_code: &str) -> (u8, String, String) {
    let s = security_code.trim();
    let lower = s.to_lowercase();

    // market flags and prefix tables - mirror C++ lists
    // market flag constants
    const MARKET_FLAGS: [&str; 5] = [
        MARKET_FLAG_SH,
        MARKET_FLAG_SZ,
        MARKET_FLAG_BJ,
        MARKET_FLAG_HK,
        MARKET_FLAG_US,
    ];
    let shanghai_main = ["50", "51", "60", "68", "90", "110", "113", "132", "204"];
    let shanghai_special = ["5", "6", "9", "7"];
    let shanghai_other = ["88"];
    let shenzhen_main = [
        "00", "12", "13", "18", "15", "16", "18", "20", "30", "39", "115", "1318",
    ];
    let beijing_main = ["40", "43", "83", "87", "88", "420", "820", "899", "920"];

    // 1) explicit prefix like sh600000 or sh.600000
    for &flag in &MARKET_FLAGS {
        if lower.starts_with(flag) {
            let pure = if s.len() > 2 && &s[2..3] == "." {
                s[3..].to_string()
            } else {
                s[2..].to_string()
            };
            let market_id = match flag {
                MARKET_FLAG_SH => MARKET_SHANGHAI,
                MARKET_FLAG_SZ => MARKET_SHENZHEN,
                MARKET_FLAG_BJ => MARKET_BEIJING,
                MARKET_FLAG_HK => MARKET_HONGKONG,
                MARKET_FLAG_US => MARKET_USA,
                _ => MARKET_SHANGHAI,
            };
            return (market_id, flag.to_string(), pure);
        }
    }

    // 2) explicit suffix like 600000.sh (we follow C++ behavior and strip last 3 chars)
    for &flag in &MARKET_FLAGS {
        let suffix = format!(".{}", flag);
        if lower.ends_with(&suffix) {
            let len = s.len();
            if len > 3 {
                let market_code = flag.to_string();
                let pure = s[..len - 3].to_string();
                let market_id = match flag {
                    MARKET_FLAG_SH => MARKET_SHANGHAI,
                    MARKET_FLAG_SZ => MARKET_SHENZHEN,
                    MARKET_FLAG_BJ => MARKET_BEIJING,
                    MARKET_FLAG_HK => MARKET_HONGKONG,
                    MARKET_FLAG_US => MARKET_USA,
                    _ => MARKET_SHANGHAI,
                };
                return (market_id, market_code, pure);
            }
        }
    }

    // 3) no explicit marker: use prefix tables
    for &p in &shanghai_main {
        if lower.starts_with(p) {
            return (MARKET_SHANGHAI, MARKET_FLAG_SH.to_string(), s.to_string());
        }
    }
    for &p in &shenzhen_main {
        if lower.starts_with(p) {
            return (MARKET_SHENZHEN, MARKET_FLAG_SZ.to_string(), s.to_string());
        }
    }
    for &p in &shanghai_special {
        if lower.starts_with(p) {
            return (MARKET_SHANGHAI, MARKET_FLAG_SH.to_string(), s.to_string());
        }
    }
    for &p in &shanghai_other {
        if lower.starts_with(p) {
            return (MARKET_SHANGHAI, MARKET_FLAG_SH.to_string(), s.to_string());
        }
    }
    for &p in &beijing_main {
        if lower.starts_with(p) {
            return (MARKET_BEIJING, MARKET_FLAG_BJ.to_string(), s.to_string());
        }
    }

    // default fallback: heuristic same as prior implementation
    if s.starts_with('6') {
        (MARKET_SHANGHAI, MARKET_FLAG_SH.to_string(), s.to_string())
    } else {
        (MARKET_SHENZHEN, MARKET_FLAG_SZ.to_string(), s.to_string())
    }
}

// Market id constants (mirror C++ MarketType enum)
pub const MARKET_SHENZHEN: u8 = 0;
pub const MARKET_SHANGHAI: u8 = 1;
pub const MARKET_BEIJING: u8 = 2;
pub const MARKET_HONGKONG: u8 = 21;
pub const MARKET_USA: u8 = 22;

// Market flag string constants (mirror C++ constants)
pub const MARKET_FLAG_SH: &str = "sh";
pub const MARKET_FLAG_SZ: &str = "sz";
pub const MARKET_FLAG_BJ: &str = "bj";
pub const MARKET_FLAG_HK: &str = "hk";
pub const MARKET_FLAG_US: &str = "us";

/// Return true if the given market id and pure code represent an index.
pub fn assert_index_by_market_and_code(market_id: u8, symbol: &str) -> bool {
    let s = symbol.trim();
    if market_id == MARKET_SHANGHAI
        && (s.starts_with("000") || s.starts_with("880") || s.starts_with("881"))
    {
        return true;
    }
    if market_id == MARKET_SHENZHEN && s.starts_with("399") {
        return true;
    }
    // BeiJing index: 899
    if market_id == MARKET_BEIJING && s.starts_with("899") {
        return true;
    }
    false
}

/// Return true if the full security code represents an index.
pub fn assert_index_by_security_code(security_code: &str) -> bool {
    let (market_id, _, code) = detect_market(security_code);
    assert_index_by_market_and_code(market_id, &code)
}

/// If the provided full security code is a ShangHai block (880/881), normalize it in-place
/// to the canonical form (flag+code) and return true. Otherwise return false.
pub fn assert_block_by_security_code(security_code: &mut String) -> bool {
    let (market_id, flag, code) = detect_market(security_code);
    if market_id != MARKET_SHANGHAI {
        return false;
    }
    if !(code.starts_with("880") || code.starts_with("881")) {
        return false;
    }
    *security_code = format!("{}{}", flag, code);
    true
}

/// Return true if the given market id and pure code represent an ETF (ShangHai 510...)
pub fn assert_etf_by_market_and_code(market_id: u8, symbol: &str) -> bool {
    market_id == MARKET_SHANGHAI && symbol.trim().starts_with("510")
}

/// Return true if the given market id and pure code represent an ordinary stock
pub fn assert_stock_by_market_and_code(market_id: u8, symbol: &str) -> bool {
    let s = symbol.trim();
    if market_id == MARKET_SHANGHAI
        && (s.starts_with("60") || s.starts_with("68") || s.starts_with("510"))
    {
        return true;
    }
    if market_id == MARKET_SHENZHEN && (s.starts_with("00") || s.starts_with("30")) {
        return true;
    }
    if market_id == MARKET_BEIJING
        && (s.starts_with("40")
            || s.starts_with("43")
            || s.starts_with("83")
            || s.starts_with("87")
            || s.starts_with("88")
            || s.starts_with("420")
            || s.starts_with("820")
            || s.starts_with("920"))
    {
        return true;
    }
    false
}

/// Return true if the full security code represents a stock
pub fn assert_stock_by_security_code(security_code: &str) -> bool {
    let (market_id, _, code) = detect_market(security_code);
    assert_stock_by_market_and_code(market_id, &code)
}

/// Normalize a security code string to the canonical flag+code format. Empty input => empty output.
pub fn correct_security_code(symbol: &str) -> String {
    if symbol.is_empty() {
        return String::new();
    }
    let (_mid, flag, code) = detect_market(symbol);
    format!("{}{}", flag, code)
}

/// Classification of a security code
#[derive(Debug, PartialEq, Eq)]
pub enum TargetKind {
    Stock,
    Index,
    Block,
    Etf,
}

/// Determine the kind of the security code (block/index/etf/stock)
pub fn assert_code(security_code: &str) -> TargetKind {
    let (market_id, _flag, code) = detect_market(security_code);
    if market_id == MARKET_SHANGHAI {
        // ShangHai: sector prefixes (880/881) -> Block
        if code.starts_with("880") || code.starts_with("881") {
            return TargetKind::Block;
        }
        // ShangHai: 000... -> Index
        if code.starts_with("000") {
            return TargetKind::Index;
        }
        // ShangHai: codes starting with '5' are ETF (per C++ logic)
        if code.starts_with('5') {
            return TargetKind::Etf;
        }
    }
    if market_id == MARKET_SHENZHEN {
        if code.starts_with("399") {
            return TargetKind::Index;
        }
        if code.starts_with("159") {
            return TargetKind::Etf;
        }
    }
    TargetKind::Stock
}

/// Check whether the security code is either an index or a stock (mirrors C++ checkIndexAndStock)
pub fn check_index_and_stock(security_code: &str) -> bool {
    if assert_index_by_security_code(security_code) {
        return true;
    }
    if assert_stock_by_security_code(security_code) {
        return true;
    }
    false
}

#[cfg(test)]
mod assert_tests {
    use super::*;

    #[test]
    fn test_assert_index_and_stock_variants() {
        // Index examples
        assert!(assert_index_by_security_code("sh000001"));
        assert!(assert_index_by_security_code("000001.sh"));
        assert!(assert_index_by_security_code("399001"));

        // Stock examples
        assert!(assert_stock_by_security_code("sh600000"));
        assert!(assert_stock_by_security_code("sz000001"));

        // ETF example
        assert!(assert_etf_by_market_and_code(MARKET_SHANGHAI, "510500"));
        assert_eq!(assert_code("sh880001"), TargetKind::Block);
        assert_eq!(assert_code("sh000001"), TargetKind::Index);
        assert_eq!(assert_code("sz399001"), TargetKind::Index);
        assert_eq!(assert_code("sh600000"), TargetKind::Stock);
    }

    #[test]
    fn test_assert_block_and_correct_code() {
        let mut s = String::from("880001");
        // Without flag, detect_market will infer ShangHai and assert_block should normalize
        assert!(assert_block_by_security_code(&mut s));
        assert_eq!(s, "sh880001");

        let mut s2 = String::from("sz000001");
        assert!(!assert_block_by_security_code(&mut s2));

        assert_eq!(correct_security_code("600000"), "sh600000".to_string());
        assert_eq!(correct_security_code(""), "".to_string());
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_detect_market_prefix_and_suffix() {
        let cases = vec![
            ("sh600000", 1u8, "sh", "600000"),
            ("sh.600000", 1u8, "sh", "600000"),
            ("600000.sh", 1u8, "sh", "600000"),
            ("sz000001", 0u8, "sz", "000001"),
            ("000001.sz", 0u8, "sz", "000001"),
            ("000001", 0u8, "sz", "000001"),
            ("600000", 1u8, "sh", "600000"),
            ("400001", 2u8, "bj", "400001"),
            ("880000", 1u8, "sh", "880000"),
            ("115000", 0u8, "sz", "115000"),
            ("hk00700", 21u8, "hk", "00700"),
            ("00700.hk", 21u8, "hk", "00700"),
            ("usAAPL", 22u8, "us", "AAPL"),
        ];

        let cases = cases
            .into_iter()
            .map(|(input, exp_id, exp_flag, exp_pure)| {
                let id_const = match exp_id {
                    0u8 => MARKET_SHENZHEN,
                    1u8 => MARKET_SHANGHAI,
                    2u8 => MARKET_BEIJING,
                    21u8 => MARKET_HONGKONG,
                    22u8 => MARKET_USA,
                    other => other,
                };
                (input, id_const, exp_flag, exp_pure)
            })
            .collect::<Vec<_>>();

        for (input, exp_id, exp_flag, exp_pure) in cases {
            let (id, flag, pure) = detect_market(input);
            assert_eq!(id, exp_id, "id mismatch for input: {}", input);
            assert_eq!(
                flag,
                exp_flag.to_string(),
                "flag mismatch for input: {}",
                input
            );
            assert_eq!(
                pure.to_lowercase(),
                exp_pure.to_lowercase(),
                "pure mismatch for input: {} -> {}",
                input,
                pure
            );
        }
    }
}
