// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// market — 通达信市场/类别映射表
// 对应 Python contrib/data/tdx/market.py

use crate::data::meta::exchange::Exchange;
use crate::data::meta::instrument::InstrumentType;

/// 根据市场编号和类别编号查找对应的交易所和资产类型
/// 与 Python find_exchange_by_market_and_category(market, category) 对齐
pub fn find_exchange_by_market_and_category(
    market: i32,
    category: i32,
) -> (Exchange, InstrumentType) {
    if market == 0 || market == 1 {
        return (Exchange::TEMP, InstrumentType::OTHER);
    }
    match (market, category) {
        (1, 1) => (Exchange::TEMP, InstrumentType::STOCK),
        (1, 12) => (Exchange::TEMP, InstrumentType::OPTION),
        (43, 1) => (Exchange::TEMP, InstrumentType::BSTOCK),
        (44, 1) => (Exchange::TEMP, InstrumentType::NEEQ),
        // 香港市场
        (27, 5) => (Exchange::HKEX, InstrumentType::INDEX),
        (31, 2) => (Exchange::HKEX, InstrumentType::STOCK),
        (48, 2) => (Exchange::HKEX, InstrumentType::GEM_MARKET),
        (22, 2) => (Exchange::HKEX, InstrumentType::BOND),
        (32, 2) => (Exchange::HKEX, InstrumentType::WARRANT),
        (49, 2) => (Exchange::HKEX, InstrumentType::FUND),
        (71, 2) => (Exchange::HKSC, InstrumentType::STOCK),
        // 期权
        (8, 12) => (Exchange::SSE, InstrumentType::OPTION),
        (9, 12) => (Exchange::SZSE, InstrumentType::OPTION),
        (4, 12) => (Exchange::CZCE, InstrumentType::OPTION),
        (5, 12) => (Exchange::DCE, InstrumentType::OPTION),
        (6, 12) => (Exchange::SHFE, InstrumentType::OPTION),
        (7, 12) => (Exchange::CFFEX, InstrumentType::OPTION),
        (67, 12) => (Exchange::GFEX, InstrumentType::OPTION),
        // 期货
        (28, 3) => (Exchange::CZCE, InstrumentType::FUTURE),
        (29, 3) => (Exchange::DCE, InstrumentType::FUTURE),
        (30, 3) => (Exchange::SHFE, InstrumentType::FUTURE),
        (46, 11) => (Exchange::SGE, InstrumentType::FUTURE),
        (55, 3) => (Exchange::SGE, InstrumentType::COMMODITY),
        (47, 3) => (Exchange::CFFEX, InstrumentType::FUTURE),
        (47, 5) => (Exchange::CFFEX, InstrumentType::INDEX),
        (66, 3) => (Exchange::GFEX, InstrumentType::FUTURE),
        (23, 3) => (Exchange::HKFE, InstrumentType::FUTURE),
        // 国际指数
        (12, 5) => (Exchange::OFFSHORE, InstrumentType::INDEX),
        // 基金
        (33, 8) => (Exchange::OFFEX, InstrumentType::FUND),
        (34, 9) => (Exchange::OFFEX, InstrumentType::MONEY_FUND),
        // 宏观指标
        (38, 10) => (Exchange::MACRO, InstrumentType::MACRO_INDICATOR),
        // 商品指数
        (42, 3) => (Exchange::ONSHORE, InstrumentType::INDEX),
        // OTC
        (45, 6) => (Exchange::OTC, InstrumentType::OTHER),
        // 中证指数
        (62, 5) => (Exchange::CSI, InstrumentType::INDEX),
        // 扩展板块指数
        (70, 5) => (Exchange::EXTENDED, InstrumentType::INDEX),
        // 美股
        (74, 13) => (Exchange::USA, InstrumentType::STOCK),
        // 英股
        (75, 14) => (Exchange::GBR, InstrumentType::STOCK),
        // 新加坡
        (78, 15) => (Exchange::SGX, InstrumentType::STOCK),
        // 代码镜像
        (100, 11) => (Exchange::MIRROR, InstrumentType::OTHER),
        // 国证指数
        (102, 5) => (Exchange::CNI, InstrumentType::INDEX),
        _ => (Exchange::TEMP, InstrumentType::OTHER),
    }
}

/// 根据交易所和资产类别查找对应的市场编号和类别编号
/// 与 Python find_market_by_exchange_and_asset_class(exchange, asset_class) 对齐
pub fn find_market_by_exchange_and_asset_class(
    exchange: Exchange,
    asset_class: InstrumentType,
) -> (i32, i32) {
    if exchange == Exchange::TEMP {
        return (1, 1);
    }
    match (exchange, asset_class) {
        (Exchange::HKEX, InstrumentType::INDEX) => (27, 5),
        (Exchange::HKEX, InstrumentType::STOCK) => (31, 2),
        (Exchange::HKEX, InstrumentType::GEM_MARKET) => (48, 2),
        (Exchange::HKEX, InstrumentType::BOND) => (22, 2),
        (Exchange::HKEX, InstrumentType::WARRANT) => (32, 2),
        (Exchange::HKEX, InstrumentType::FUND) => (49, 2),
        (Exchange::HKSC, InstrumentType::STOCK) => (71, 2),
        (Exchange::SSE, InstrumentType::OPTION) => (8, 12),
        (Exchange::SZSE, InstrumentType::OPTION) => (9, 12),
        (Exchange::CZCE, InstrumentType::OPTION) => (4, 12),
        (Exchange::DCE, InstrumentType::OPTION) => (5, 12),
        (Exchange::SHFE, InstrumentType::OPTION) => (6, 12),
        (Exchange::CFFEX, InstrumentType::OPTION) => (7, 12),
        (Exchange::GFEX, InstrumentType::OPTION) => (67, 12),
        (Exchange::CZCE, InstrumentType::FUTURE) => (28, 3),
        (Exchange::DCE, InstrumentType::FUTURE) => (29, 3),
        (Exchange::SHFE, InstrumentType::FUTURE) => (30, 3),
        (Exchange::SGE, InstrumentType::FUTURE) => (46, 11),
        (Exchange::SGE, InstrumentType::COMMODITY) => (55, 3),
        (Exchange::CFFEX, InstrumentType::FUTURE) => (47, 3),
        (Exchange::CFFEX, InstrumentType::INDEX) => (47, 5),
        (Exchange::GFEX, InstrumentType::FUTURE) => (66, 3),
        (Exchange::HKFE, InstrumentType::FUTURE) => (23, 3),
        (Exchange::OFFSHORE, InstrumentType::INDEX) => (12, 5),
        (Exchange::OFFEX, InstrumentType::FUND) => (33, 8),
        (Exchange::OFFEX, InstrumentType::MONEY_FUND) => (34, 9),
        (Exchange::MACRO, InstrumentType::MACRO_INDICATOR) => (38, 10),
        (Exchange::ONSHORE, InstrumentType::INDEX) => (42, 3),
        (Exchange::OTC, InstrumentType::OTHER) => (45, 6),
        (Exchange::CSI, InstrumentType::INDEX) => (62, 5),
        (Exchange::EXTENDED, InstrumentType::INDEX) => (70, 5),
        (Exchange::USA, InstrumentType::STOCK) => (74, 13),
        (Exchange::GBR, InstrumentType::STOCK) => (75, 14),
        (Exchange::SGX, InstrumentType::STOCK) => (78, 15),
        (Exchange::MIRROR, InstrumentType::OTHER) => (100, 11),
        (Exchange::CNI, InstrumentType::INDEX) => (102, 5),
        _ => (1, 1),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_find_exchange_by_market_and_category_hk() {
        let (exch, it) = find_exchange_by_market_and_category(31, 2);
        assert_eq!(exch, Exchange::HKEX);
        assert_eq!(it, InstrumentType::STOCK);
    }

    #[test]
    fn test_find_exchange_by_market_and_category_hk_index() {
        let (exch, it) = find_exchange_by_market_and_category(27, 5);
        assert_eq!(exch, Exchange::HKEX);
        assert_eq!(it, InstrumentType::INDEX);
    }

    #[test]
    fn test_find_exchange_by_market_and_category_us() {
        let (exch, it) = find_exchange_by_market_and_category(74, 13);
        assert_eq!(exch, Exchange::USA);
        assert_eq!(it, InstrumentType::STOCK);
    }

    #[test]
    fn test_find_market_by_exchange_and_asset_class() {
        let (mk, cat) =
            find_market_by_exchange_and_asset_class(Exchange::HKEX, InstrumentType::INDEX);
        assert_eq!(mk, 27);
        assert_eq!(cat, 5);
    }
}
