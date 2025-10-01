use std::str::FromStr;

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum MarketType {
    ShenZhen,
    ShangHai,
    BeiJing,
    HongKong,
    USA,
}

impl MarketType {
    pub fn as_str(&self) -> &'static str {
        match self {
            MarketType::ShangHai => "sh",
            MarketType::ShenZhen => "sz",
            MarketType::BeiJing => "bj",
            MarketType::HongKong => "hk",
            MarketType::USA => "us",
        }
    }
}

/// 退市标识
pub const STOCK_DELISTING: &str = "DELISTING";

pub fn get_security_code(market: MarketType, symbol: &str) -> String {
    match market {
        MarketType::USA => format!("{}{}", MarketType::USA.as_str(), symbol),
        MarketType::HongKong => format!("{}{}", MarketType::HongKong.as_str(), &symbol[..5]),
        MarketType::BeiJing => format!("{}{}", MarketType::BeiJing.as_str(), &symbol[..6]),
        MarketType::ShenZhen => format!("{}{}", MarketType::ShenZhen.as_str(), &symbol[..6]),
        _ => format!("{}{}", MarketType::ShangHai.as_str(), &symbol[..6]),
    }
}

fn starts_with_any(s: &str, prefixes: &[&str]) -> bool {
    prefixes.iter().any(|&prefix| s.starts_with(prefix))
}

fn ends_with_any(s: &str, suffixes: &[&str]) -> bool {
    suffixes.iter().any(|&suffix| s.ends_with(suffix))
}

pub fn get_market(symbol: &str) -> String {
    let symbol = symbol.trim();
    let mut market = "sh".to_string();

    let market_flags = ["sh", "sz", "SH", "SZ", "bj", "BJ", "hk", "HK", "us", "US"];

    if starts_with_any(symbol, &market_flags) {
        market = symbol[..2].to_lowercase();
    } else if ends_with_any(symbol, &market_flags) {
        let length = symbol.len();
        market = symbol[length-2..].to_lowercase();
    } else if starts_with_any(symbol, &["50", "51", "60", "68", "90", "110", "113", "132", "204"]) {
        market = "sh".to_string();
    } else if starts_with_any(symbol, &["00", "12", "13", "18", "15", "16", "18", "20", "30", "39", "115", "1318"]) {
        market = "sz".to_string();
    } else if starts_with_any(symbol, &["5", "6", "9", "7"]) {
        market = "sh".to_string();
    } else if starts_with_any(symbol, &["88"]) {
        market = "sh".to_string();
    } else if starts_with_any(symbol, &["4", "8"]) {
        market = "bj".to_string();
    }

    market
}

pub fn get_market_id(symbol: &str) -> MarketType {
    match get_market(symbol).as_str() {
        "sh" => MarketType::ShangHai,
        "sz" => MarketType::ShenZhen,
        "bj" => MarketType::BeiJing,
        _ => MarketType::ShangHai,
    }
}

pub fn detect_market(symbol: &str) -> (MarketType, String, String) {
    let mut code = symbol.trim().to_string();
    let mut market = MarketType::ShangHai;
    let market_flags = ["sh", "sz", "SH", "SZ", "bj", "BJ", "hk", "HK", "us", "US"];

    if starts_with_any(&code, &market_flags) {
        let (m, c) = code.split_at(2);
        market = MarketType::from_str(&m.to_lowercase()).unwrap_or(MarketType::ShangHai);
        code = if c.starts_with('.') {
            c[1..].to_string()
        } else {
            c.to_string()
        };
    } else if ends_with_any(&code, &market_flags) {
        let length = code.len();
        let (c, m) = code.split_at(length-2);
        market = MarketType::from_str(&m.to_lowercase()).unwrap_or(MarketType::ShangHai);
        code = c[..c.len()-1].to_string();
    } else if starts_with_any(&code, &["50", "51", "60", "68", "90", "110", "113", "132", "204"]) {
        market = MarketType::ShangHai;
    } else if starts_with_any(&code, &["00", "12", "13", "18", "15", "16", "18", "20", "30", "39", "115", "1318"]) {
        market = MarketType::ShenZhen;
    } else if starts_with_any(&code, &["5", "6", "9", "7"]) {
        market = MarketType::ShangHai;
    } else if starts_with_any(&code, &["88"]) {
        market = MarketType::ShangHai;
    } else if starts_with_any(&code, &["4", "8"]) {
        market = MarketType::BeiJing;
    }

    (market, market.as_str().to_string(), code)
}

pub fn assert_index_by_market_and_code(market_id: MarketType, symbol: &str) -> bool {
    match market_id {
        MarketType::ShangHai => starts_with_any(symbol, &["000", "880", "881"]),
        MarketType::ShenZhen => starts_with_any(symbol, &["399"]),
        _ => false,
    }
}

pub fn assert_index_by_security_code(security_code: &str) -> bool {
    let (market_id, _, code) = detect_market(security_code);
    assert_index_by_market_and_code(market_id, &code)
}

pub fn assert_block_by_security_code(security_code: &mut String) -> bool {
    let (market_id, flag, code) = detect_market(security_code);
    if market_id != MarketType::ShangHai {
        return false;
    }
    if !starts_with_any(&code, &["880", "881"]) {
        return false;
    }
    *security_code = format!("{}{}", flag, code);
    true
}

pub fn assert_etf_by_market_and_code(market_id: MarketType, symbol: &str) -> bool {
    if let MarketType::ShangHai = market_id {
        starts_with_any(symbol, &["510"]) && symbol.len() == 6
    } else {
        false
    }
}

pub fn assert_stock_by_market_and_code(market_id: MarketType, symbol: &str) -> bool {
    match market_id {
        MarketType::ShangHai => starts_with_any(symbol, &["60", "68", "510"]),
        MarketType::ShenZhen => starts_with_any(symbol, &["00", "30"]),
        _ => false,
    }
}

pub fn assert_stock_by_security_code(security_code: &str) -> bool {
    let (market_id, _, code) = detect_market(security_code);
    assert_stock_by_market_and_code(market_id, &code)
}

pub fn correct_security_code(security_code: &str) -> String {
    if security_code.is_empty() {
        return String::new();
    }
    let (_, flag, code) = detect_market(security_code);
    format!("{}{}", flag, code)
}

#[derive(Debug, PartialEq)]
pub enum TargetKind {
    Stock,
    Index,
    Block,
    ETF,
}

pub fn assert_code(security_code: &str) -> TargetKind {
    let (market_id, _, code) = detect_market(security_code);

    if market_id == MarketType::ShangHai && starts_with_any(&code, &["880", "881"]) {
        return TargetKind::Block;
    }
    if market_id == MarketType::ShangHai && starts_with_any(&code, &["000"]) {
        return TargetKind::Index;
    }
    if market_id == MarketType::ShenZhen && starts_with_any(&code, &["399"]) {
        return TargetKind::Index;
    }
    if market_id == MarketType::ShangHai && starts_with_any(&code, &["510"]) {
        return TargetKind::ETF;
    }
    TargetKind::Stock
}

impl FromStr for MarketType {
    type Err = ();

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "sh" => Ok(MarketType::ShangHai),
            "sz" => Ok(MarketType::ShenZhen),
            "bj" => Ok(MarketType::BeiJing),
            "hk" => Ok(MarketType::HongKong),
            "us" => Ok(MarketType::USA),
            _ => Err(()),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_market() {
        // 测试市场标识解析
        assert_eq!(get_market("SH600000"), "sh");
        assert_eq!(get_market("sz000001"), "sz");
        assert_eq!(get_market("600000"), "sh");
        assert_eq!(get_market("000001"), "sz");
        assert_eq!(get_market("430001"), "bj");
        assert_eq!(get_market("HK00001"), "hk");
        assert_eq!(get_market("USAAPL"), "us");
        assert_eq!(get_market("880001"), "sh"); // 通达信板块
    }

    #[test]
    fn test_detect_market() {
        // 测试市场检测功能
        let (market, flag, code) = detect_market("SH600000");
        assert_eq!(market, MarketType::ShangHai);
        assert_eq!(flag, "sh".to_string());
        assert_eq!(code, "600000".to_string());

        let (market, flag, code) = detect_market("000001.SZ");
        assert_eq!(market, MarketType::ShenZhen);
        assert_eq!(flag, "sz".to_string());
        assert_eq!(code, "000001".to_string());

        let (market, flag, code) = detect_market("430001.BJ");
        assert_eq!(market, MarketType::BeiJing);
        assert_eq!(flag, "bj".to_string());
        assert_eq!(code, "430001".to_string());
    }

    #[test]
    fn test_get_security_code() {
        // 测试证券代码生成
        assert_eq!(get_security_code(MarketType::ShangHai, "600000"), "sh600000");
        assert_eq!(get_security_code(MarketType::ShenZhen, "000001"), "sz000001");
        assert_eq!(get_security_code(MarketType::HongKong, "00001"), "hk00001");
        assert_eq!(get_security_code(MarketType::USA, "AAPL"), "usAAPL");
    }

    #[test]
    fn test_assert_code() {
        // 测试代码类型判断
        assert_eq!(assert_code("sh000001"), TargetKind::Index);
        assert_eq!(assert_code("sz399001"), TargetKind::Index);
        assert_eq!(assert_code("sh510050"), TargetKind::ETF);
        assert_eq!(assert_code("sh880001"), TargetKind::Block);
        assert_eq!(assert_code("sz000001"), TargetKind::Stock);
        assert_eq!(assert_code("bj430001"), TargetKind::Stock);
    }

    #[test]
    fn test_index_detection() {
        // 测试指数判断
        assert!(assert_index_by_security_code("sh000001"));
        assert!(assert_index_by_security_code("sz399001"));
        assert!(!assert_index_by_security_code("sz000001"));
        assert!(!assert_index_by_security_code("sh600000"));
    }

    #[test]
    fn test_block_correction() {
        // 测试板块代码修正
        let mut code = "880001.SH".to_string();
        assert!(assert_block_by_security_code(&mut code));
        assert_eq!(code, "sh880001");

        let mut code2 = "881001".to_string();
        assert!(assert_block_by_security_code(&mut code2)); // 881开头的是板块, 板块属于上海市场

        let mut code3 = "sh880001".to_string();
        assert!(assert_block_by_security_code(&mut code3));
        assert_eq!(code3, "sh880001");
    }

    #[test]
    fn test_code_correction() {
        // 测试代码修正功能
        assert_eq!(correct_security_code("SH600000"), "sh600000");
        assert_eq!(correct_security_code("000001.SZ"), "sz000001");
        assert_eq!(correct_security_code("430001.BJ"), "bj430001");
        assert_eq!(correct_security_code("AAPL.US"), "usAAPL");
    }

    #[test]
    fn test_edge_cases() {
        // 测试边界情况
        assert_eq!(get_market(""), "sh"); // 空字符串
        assert_eq!(get_market("123456"), "sz"); // 默认sz
        assert_eq!(detect_market("invalid").0, MarketType::ShangHai);
        assert_eq!(correct_security_code(""), "");
    }

    #[test]
    fn test_stock_detection() {
        // 测试股票判断功能
        assert!(assert_stock_by_security_code("sh600000"));   // 上海主板
        assert!(assert_stock_by_security_code("sh688001"));   // 科创板
        assert!(assert_stock_by_security_code("sz000001"));   // 深圳主板
        assert!(assert_stock_by_security_code("sz300001"));   // 创业板
        assert!(!assert_stock_by_security_code("sh000001"));  // 上证指数
        assert!(!assert_stock_by_security_code("sz399001"));  // 深证成指
        assert!(!assert_stock_by_security_code("bj430001"));  // 北交所代码
        assert!(assert_stock_by_security_code("sh510050"));  // ETF
    }

    #[test]
    fn test_etf_detection() {
        // 测试ETF判断功能
        assert!(assert_etf_by_market_and_code(MarketType::ShangHai, "510050"));  // 上证50ETF
        assert!(assert_etf_by_market_and_code(MarketType::ShangHai, "510300"));  // 沪深300ETF

        assert!(!assert_etf_by_market_and_code(MarketType::ShangHai, "600000"));  // 普通股票
        assert!(!assert_etf_by_market_and_code(MarketType::ShenZhen, "159915")); // 深市ETF
        assert!(!assert_etf_by_market_and_code(MarketType::BeiJing, "510050"));  // 非沪市
        assert!(!assert_etf_by_market_and_code(MarketType::ShangHai, "510"));    // 长度不足
    }

    #[test]
    fn test_get_market_id() {
        // 验证市场ID转换逻辑
        assert_eq!(get_market_id("SH600000"), MarketType::ShangHai);
        assert_eq!(get_market_id("sz000001"), MarketType::ShenZhen);
        assert_eq!(get_market_id("BJ430001"), MarketType::BeiJing);
        assert_eq!(get_market_id("hk00001"), MarketType::ShangHai); // 非A股返回默认值
        assert_eq!(get_market_id("invalid"), MarketType::ShangHai); // 默认值
    }

    #[test]
    fn test_stock_delisting_constant() {
        // 验证退市状态常量
        assert_eq!(STOCK_DELISTING, "DELISTING");
    }

    #[test]
    fn test_performance() {
        let start = std::time::Instant::now();
        for _ in 0..1_000_000 {
            correct_security_code("600519");
            //correct_security_code("300750.SZ");
            //correct_security_code("830099.BJ");
        }
        let duration = start.elapsed();
        println!("百万次解析耗时: {:?}, {:?}", duration, duration/1_000_000);
    }
}