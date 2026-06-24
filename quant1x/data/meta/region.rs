// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

/// 市场区域, 用于收敛货币和时区
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Region {
    CN,
    HK,
    US,
    UK,
    EU,
    SG,
    JP,
    OFFSHORE,
    ONSHORE,
    GLB,
    UNKNOWN,
}

impl Region {
    /// 主要货币
    pub fn currency(self) -> &'static str {
        match self {
            Region::CN => "CNY",
            Region::HK => "HKD",
            Region::US => "USD",
            Region::UK => "GBP",
            Region::EU => "EUR",
            Region::SG => "SGD",
            Region::JP => "JPY",
            Region::OFFSHORE => "USD",
            Region::ONSHORE => "CNY",
            _ => "USD",
        }
    }

    /// 主要时区
    pub fn timezone(self) -> &'static str {
        match self {
            Region::CN => "Asia/Shanghai",
            Region::HK => "Asia/Hong_Kong",
            Region::US => "America/New_York",
            Region::UK => "Europe/London",
            Region::EU => "Europe/Berlin",
            Region::SG => "Asia/Singapore",
            Region::JP => "Asia/Tokyo",
            Region::OFFSHORE => "America/New_York",
            Region::ONSHORE => "Asia/Shanghai",
            _ => "UTC",
        }
    }

    pub fn as_str(self) -> &'static str {
        match self {
            Region::CN => "CN",
            Region::HK => "HK",
            Region::US => "US",
            Region::UK => "UK",
            Region::EU => "EU",
            Region::SG => "SG",
            Region::JP => "JP",
            Region::OFFSHORE => "OS",
            Region::ONSHORE => "ON",
            Region::GLB => "GLB",
            Region::UNKNOWN => "UNKNOWN",
        }
    }
}

impl std::fmt::Display for Region {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.as_str())
    }
}
