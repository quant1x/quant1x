use std::fmt;

pub const EXCHANGE_SSE: &str = "sh";
pub const EXCHANGE_SZSE: &str = "sz";
pub const EXCHANGE_BJSE: &str = "bj";
pub const EXCHANGE_HK: &str = "hk";
pub const EXCHANGE_US: &str = "us";

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ExchangeId {
    ShenZhen,
    ShangHai,
    BeiJing,
    HongKong,
    USA,
}

impl fmt::Display for ExchangeId {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let s = match self {
            ExchangeId::ShenZhen => EXCHANGE_SZSE,
            ExchangeId::ShangHai => EXCHANGE_SSE,
            ExchangeId::BeiJing => EXCHANGE_BJSE,
            ExchangeId::HongKong => EXCHANGE_HK,
            ExchangeId::USA => EXCHANGE_US,
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
}

impl SecurityCode {
    pub fn new(market: ExchangeId, symbol: &str) -> Self {
        Self { market, symbol: symbol.to_string() }
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
