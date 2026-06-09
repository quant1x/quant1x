// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// Instrument — 证券信息结构体，与 Python data/meta/instrument.py 对齐

use super::exchange::Exchange;
use super::region::Region;

/// 资产子类型（高4位），语义由主类型 InstrumentType 决定
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum Subtype {
    Default = 0x00,
    Chinext = 0x10, // 创业板
    Star = 0x20,    // 科创板
    B = 0x30,       // B股
    H = 0x40,       // H股
    Gem = 0x50,     // 港交所创业板
    ExchangeTraded = 0x60,
    Listed = 0x70,
    OpenEnded = 0x80,
    Mutual = 0xB0,
    Private = 0xC0,
    Money = 0xD0,
    Special = 0xE0,
    Temporary = 0xF0,
}

/// 合约类型（低4位=资产大类，高4位=子类型扩展）
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct InstrumentType(pub u16);

impl InstrumentType {
    // 基础类型常量
    pub const UNKNOWN: Self = InstrumentType(0x00);
    pub const INDEX: Self = InstrumentType(0x01);
    pub const STOCK: Self = InstrumentType(0x02);
    pub const FUND: Self = InstrumentType(0x03);
    pub const BOND: Self = InstrumentType(0x04);
    pub const FOREX: Self = InstrumentType(0x05);
    pub const COMMODITY: Self = InstrumentType(0x06);
    pub const FUTURE: Self = InstrumentType(0x07);
    pub const OPTION: Self = InstrumentType(0x08);
    pub const WARRANT: Self = InstrumentType(0x09);
    pub const MACRO: Self = InstrumentType(0x0F);

    // 组合类型
    pub const BSTOCK: Self = InstrumentType(Subtype::B as u16 | Self::STOCK.0);
    pub const HSTOCK: Self = InstrumentType(Subtype::H as u16 | Self::STOCK.0);
    pub const IPO: Self = InstrumentType(Subtype::Special as u16 | Self::STOCK.0);
    pub const CHINEXT_MARKET: Self = InstrumentType(Subtype::Chinext as u16 | Self::STOCK.0);
    pub const STAR_MARKET: Self = InstrumentType(Subtype::Star as u16 | Self::STOCK.0);
    pub const GEM_MARKET: Self = InstrumentType(Subtype::Gem as u16 | Self::STOCK.0);
    pub const TEMPORARY_STOCK: Self = InstrumentType(Subtype::Temporary as u16 | Self::STOCK.0);
    pub const ETF: Self = InstrumentType(Subtype::ExchangeTraded as u16 | Self::FUND.0);
    pub const LOF: Self = InstrumentType(Subtype::Listed as u16 | Self::FUND.0);
    pub const OPEN_ENDED_FUND: Self = InstrumentType(Subtype::OpenEnded as u16 | Self::FUND.0);
    pub const MONEY_FUND: Self = InstrumentType(Subtype::Money as u16 | Self::FUND.0);
    pub const MACRO_INDICATOR: Self = InstrumentType(Self::MACRO.0);
    pub const SECTOR: Self = InstrumentType(Subtype::Special as u16 | Self::INDEX.0);
    pub const NEEQ: Self = InstrumentType(0xFE);
    pub const OTHER: Self = InstrumentType(0xFF);

    /// 提取基础资产类型（低4位）
    pub fn base_type(self) -> InstrumentType {
        InstrumentType(self.0 & 0x0F)
    }

    /// 提取子类型扩展位（高4位）
    pub fn subtype(self) -> u16 {
        self.0 & 0xF0
    }

    pub fn is_stock(self) -> bool {
        self.base_type() == Self::STOCK
    }

    pub fn is_index(self) -> bool {
        self.base_type() == Self::INDEX
    }
}

/// 证券信息结构体，与 Python data/meta/instrument.py 的 Instrument dataclass 对齐
#[derive(Debug, Clone)]
pub struct Instrument {
    /// 交易所
    pub exchange: Exchange,
    /// 证券类型
    pub instrument_type: InstrumentType,
    /// 交易所分配的证券代码（ticker）
    pub ticker: String,
    /// 证券名称
    pub name: String,
    /// 每手股数
    pub lot_size: i32,
    /// 价格小数位数
    pub price_precision: i32,
    /// 扩展市场代码
    pub ext_market: i32,
    /// 扩展类别代码
    pub ext_category: i32,
    /// 证券代码别名
    pub alias_ticker: String,
}

impl Default for Instrument {
    fn default() -> Self {
        Self {
            exchange: Exchange::UNKNOWN,
            instrument_type: InstrumentType::UNKNOWN,
            ticker: String::new(),
            name: String::new(),
            lot_size: 0,
            price_precision: 0,
            ext_market: 0,
            ext_category: 0,
            alias_ticker: String::new(),
        }
    }
}

impl Instrument {
    /// 创建默认的未知 Instrument
    pub fn unknown() -> Self {
        Self::default()
    }

    /// 构建交易符号字符串
    /// CN 市场: {identifier}{ticker}，如 sh600000
    /// 非 CN 市场: {ticker}.{identifier}，如 aapl.us (ticker 转小写)
    pub fn symbol(&self) -> String {
        if self.exchange.region() == Region::CN {
            format!("{}{}", self.exchange.identifier(), self.ticker)
        } else {
            format!("{}.{}", self.ticker.to_lowercase(), self.exchange.identifier())
        }
    }

    /// 检查是否可以构造有效的交易符号
    pub fn can_construct_symbol(&self) -> bool {
        self.exchange != Exchange::UNKNOWN && self.instrument_type != InstrumentType::UNKNOWN
    }

    /// 检查证券是否有效
    pub fn is_valid(&self) -> bool {
        self.exchange != Exchange::UNKNOWN
            && self.instrument_type != InstrumentType::UNKNOWN
            && self.lot_size > 0
            && self.price_precision > 0
    }

    /// 获取证券代码（优先返回 alias_ticker）
    pub fn code(&self) -> &str {
        if self.alias_ticker.is_empty() {
            &self.ticker
        } else {
            &self.alias_ticker
        }
    }
}

impl std::fmt::Display for Instrument {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.symbol())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_instrument_type_basics() {
        assert_eq!(InstrumentType::STOCK.base_type(), InstrumentType::STOCK);
        assert_eq!(InstrumentType::ETF.base_type(), InstrumentType::FUND);
        assert_eq!(InstrumentType::SECTOR.base_type(), InstrumentType::INDEX);
        assert!(InstrumentType::STOCK.is_stock());
        assert!(!InstrumentType::INDEX.is_stock());
        assert!(InstrumentType::INDEX.is_index());
    }

    #[test]
    fn test_instrument_symbol_cn() {
        let inst = Instrument {
            exchange: Exchange::SSE,
            instrument_type: InstrumentType::STOCK,
            ticker: "600000".to_string(),
            name: "浦发银行".to_string(),
            lot_size: 100,
            price_precision: 2,
            ext_market: 0,
            ext_category: 0,
            alias_ticker: String::new(),
        };
        assert_eq!(inst.symbol(), "sh600000");
    }

    #[test]
    fn test_instrument_symbol_us() {
        let inst = Instrument {
            exchange: Exchange::USA,
            instrument_type: InstrumentType::STOCK,
            ticker: "AAPL".to_string(),
            name: "Apple Inc.".to_string(),
            lot_size: 1,
            price_precision: 2,
            ext_market: 0,
            ext_category: 0,
            alias_ticker: String::new(),
        };
        assert_eq!(inst.symbol(), "aapl.us");
    }

    #[test]
    fn test_instrument_symbol_hk() {
        let inst = Instrument {
            exchange: Exchange::HKEX,
            instrument_type: InstrumentType::STOCK,
            ticker: "00700".to_string(),
            name: "腾讯控股".to_string(),
            lot_size: 100,
            price_precision: 2,
            ext_market: 0,
            ext_category: 0,
            alias_ticker: String::new(),
        };
        assert_eq!(inst.symbol(), "00700.hk");
    }
}
