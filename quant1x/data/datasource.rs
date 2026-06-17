// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// datasource — 数据源抽象接口, 与 Python data/datasource.py 对齐(旧版本)

use crate::data::meta::exchange::Exchange;
use crate::data::meta::instrument::Instrument;
use crate::data::schema::{Bar, Sector, Transaction};

/// 板块类别: 用于区分不同逻辑类型的股票分组
///
/// 与 Python `PlateCategory(Enum)` 对齐
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PlateCategory {
    /// 未知
    Unknown = 0,
    /// 行业
    Industry = 2,
    /// 地区
    Region = 3,
    /// 概念
    Thematic = 4,
    /// 风格
    Style = 5,
    /// 指数
    Index = 6,
    /// 研究行业
    ResearchIndustry = 12,
}

impl PlateCategory {
    /// 返回类别代码(数字值), 对应 Python `PlateCategory.code`
    pub fn code(self) -> i32 {
        self as i32
    }

    /// 返回中文显示名, 对应 Python `PlateCategory.label`
    pub fn label(self) -> &'static str {
        match self {
            PlateCategory::Unknown => "未知",
            PlateCategory::Industry => "行业",
            PlateCategory::Region => "地区",
            PlateCategory::Thematic => "概念",
            PlateCategory::Style => "风格",
            PlateCategory::Index => "指数",
            PlateCategory::ResearchIndustry => "研究行业",
        }
    }
}

impl std::fmt::Display for PlateCategory {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.label())
    }
}

/// 市场接口抽象 trait
///
/// 所有具体市场(如 A 股, 港股, 美股)应实现此 trait. 
/// 与 Python `DataHandler(ABC)` 对齐. 
pub trait DataHandler: Send + Sync {
    /// 返回该市场对应的市场列表
    ///
    /// 对应 Python `get_market_list(self) -> List[Exchange]`
    fn get_market_list(&self) -> Vec<Exchange>;

    /// 返回指定市场对应的指数列表
    ///
    /// 对应 Python `get_index_list(self, market: Union[List, str] = "all") -> List[Instrument]`
    ///
    /// market: 市场标识, 可以是字符串或列表. "all" 表示所有市场
    fn get_index_list(&self, market: Option<&[String]>) -> Vec<Instrument>;

    /// 获取指定类别的板块列表
    ///
    /// 对应 Python `get_sector_list(self, category: PlateCategory = PlateCategory.UNKNOWN) -> List[Sector]`
    fn get_sector_list(&self, category: PlateCategory) -> Vec<Sector>;

    /// 返回指定市场对应的所有证券列表
    ///
    /// 对应 Python `list_instruments(self, market: Union[List, str] = "all") -> List[Instrument]`
    fn list_instruments(&self, market: Option<&[String]>) -> Vec<Instrument>;

    /// 获取指定证券代码对应的证券信息
    ///
    /// 对应 Python `get_instrument(self, symbol: str) -> Instrument`
    ///
    /// symbol: 证券代码, 如 "sh600000"
    /// 当找不到指定代码的合约时返回 None
    fn get_instrument(&self, symbol: &str) -> Option<Instrument>;

    /// 获取指定证券代码的K线数据
    ///
    /// 对应 Python `klines(self, symbol: str, start_date: str | None = None, end_date: str | None = None, freq: str | None = None)`
    ///
    /// Python 版本返回 DataFrame, Rust 版本返回 Vec<Bar>
    fn klines(
        &self,
        symbol: &str,
        start_date: Option<&str>,
        end_date: Option<&str>,
        freq: Option<&str>,
    ) -> Option<Vec<Bar>>;

    /// 获取指定证券代码的交易数据
    ///
    /// 对应 Python `transactions(self, symbol: str, date: str | None = None)`
    ///
    /// date: 交易日期, 如 "2020-01-01", None 表示当天
    /// Python 版本返回 DataFrame, Rust 版本返回 Vec<Transaction>
    fn transactions(
        &self,
        symbol: &str,
        date: Option<&str>,
    ) -> Option<Vec<Transaction>>;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_plate_category_code() {
        assert_eq!(PlateCategory::Unknown.code(), 0);
        assert_eq!(PlateCategory::Industry.code(), 2);
        assert_eq!(PlateCategory::Region.code(), 3);
        assert_eq!(PlateCategory::Thematic.code(), 4);
        assert_eq!(PlateCategory::Style.code(), 5);
        assert_eq!(PlateCategory::Index.code(), 6);
        assert_eq!(PlateCategory::ResearchIndustry.code(), 12);
    }

    #[test]
    fn test_plate_category_label() {
        assert_eq!(PlateCategory::Unknown.label(), "未知");
        assert_eq!(PlateCategory::Industry.label(), "行业");
        assert_eq!(PlateCategory::ResearchIndustry.label(), "研究行业");
    }

    #[test]
    fn test_plate_category_display() {
        assert_eq!(format!("{}", PlateCategory::Industry), "行业");
    }

    #[test]
    fn test_sector_default() {
        let sector = Sector::default();
        assert!(sector.name.is_empty());
        assert!(sector.code.is_empty());
        assert_eq!(sector.sector_type, 0);
        assert_eq!(sector.count, 0);
        assert!(sector.block.is_empty());
        assert!(sector.constituent_stocks.is_empty());
    }
}
