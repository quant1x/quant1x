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

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ParseRegionError;

impl std::fmt::Display for ParseRegionError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "unknown region")
    }
}

impl std::error::Error for ParseRegionError {}

impl std::str::FromStr for Region {
    type Err = ParseRegionError;

    fn from_str(value: &str) -> Result<Self, Self::Err> {
        match value {
            "CN" => Ok(Self::CN),
            "HK" => Ok(Self::HK),
            "US" => Ok(Self::US),
            "UK" => Ok(Self::UK),
            "EU" => Ok(Self::EU),
            "SG" => Ok(Self::SG),
            "JP" => Ok(Self::JP),
            "OS" => Ok(Self::OFFSHORE),
            "ON" => Ok(Self::ONSHORE),
            "GLB" => Ok(Self::GLB),
            "UNKNOWN" => Ok(Self::UNKNOWN),
            _ => Err(ParseRegionError),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{ParseRegionError, Region};
    use std::str::FromStr;

    #[test]
    fn parses_all_region_codes() {
        let cases = [
            ("CN", Region::CN),
            ("HK", Region::HK),
            ("US", Region::US),
            ("UK", Region::UK),
            ("EU", Region::EU),
            ("SG", Region::SG),
            ("JP", Region::JP),
            ("OS", Region::OFFSHORE),
            ("ON", Region::ONSHORE),
            ("GLB", Region::GLB),
            ("UNKNOWN", Region::UNKNOWN),
        ];

        for (value, expected) in cases {
            assert_eq!(Region::from_str(value), Ok(expected));
            assert_eq!(value.parse::<Region>(), Ok(expected));
            assert_eq!(expected.as_str(), value);
        }
    }

    #[test]
    fn rejects_invalid_region_codes() {
        for value in ["", "cn", " OS", "OS ", "CNH", "UNKNOWN "] {
            assert_eq!(value.parse::<Region>(), Err(ParseRegionError));
        }
    }
}

impl std::fmt::Display for Region {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.as_str())
    }
}
