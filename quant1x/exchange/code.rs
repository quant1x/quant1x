// Minimal Rust implementation of market flag helper mirroring C++ `GetMarketFlag`
pub fn get_market_flag(market: u8) -> &'static str {
    match market {
        0 => "sz",
        2 => "bj",
        21 => "hk",
        22 => "us",
        1 => "sh",
        _ => "sh",
    }
}

/// Build a security code string like C++ `exchange::GetMarketFlag(...) + code`
pub fn security_code(market: u8, code: &str) -> String {
    format!("{}{}", get_market_flag(market), code)
}
