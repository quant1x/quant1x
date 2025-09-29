/// defaultBaseUnit equivalent from C++ security_quote.h
pub fn default_base_unit(_market_id: i32, code: &str) -> f64 {
    // follow the C++ logic: check prefixes
    if code.starts_with("60")
        || code.starts_with("68")
        || code.starts_with("00")
        || code.starts_with("30")
        || code.starts_with("39")
    {
        100.0
    } else if code.starts_with("510") {
        1000.0
    } else {
        100.0
    }
}

/// AssertIndexByMarketAndCode equivalent from C++ exchange::code.cpp
pub fn assert_index_by_market_and_code(market_id: i32, symbol: &str) -> bool {
    // MarketType::ShangHai == 1, ShenZhen == 0 in the C++ code
    if market_id == 1
        && (symbol.starts_with("000") || symbol.starts_with("880") || symbol.starts_with("881"))
    {
        return true;
    }
    if market_id == 0 && symbol.starts_with("399") {
        return true;
    }
    false
}
