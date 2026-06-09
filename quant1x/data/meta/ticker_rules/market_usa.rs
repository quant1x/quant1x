// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// USA 美国证券交易所规则 — 与 Python data/meta/ticker_rules/market_usa.py 对齐

use super::rule::{CodeRule, RulePrefix};
use super::super::exchange::Exchange;
use super::super::instrument::InstrumentType;
use std::collections::HashMap;

/// USA 美国证券交易所规则
pub fn usa_rules() -> Vec<CodeRule> {
    vec![
        CodeRule { exchange: Exchange::OFFSHORE, prefix: RulePrefix::Str("IXIC"), instrument_type: InstrumentType::INDEX, name: "指数", desc: "纳斯达克指数" },
        CodeRule { exchange: Exchange::OFFSHORE, prefix: RulePrefix::Str("DAX"), instrument_type: InstrumentType::INDEX, name: "指数", desc: "德国DAX指数" },
        CodeRule { exchange: Exchange::EXTENDED, prefix: RulePrefix::Str("US"), instrument_type: InstrumentType::SECTOR, name: "指数", desc: "美国板块指数" },
        CodeRule { exchange: Exchange::USA, prefix: RulePrefix::Str(""), instrument_type: InstrumentType::STOCK, name: "挂牌公司普通股", desc: "" },
    ]
}

/// 美股 ticker -> 行情协议代码映射
fn ticker_to_code_map() -> HashMap<&'static str, &'static str> {
    let mut m = HashMap::new();
    m.insert("IXIC", "A_IXIC"); // 纳斯达克指数
    m.insert("DAX", "B_DAX");   // 德国DAX指数
    m
}

/// 将美国股票代码转换为行情标准的代码
/// 对应 Python 的 usa_ticker_to_code
pub fn usa_ticker_to_code(ticker: &str) -> String {
    let upper = ticker.to_uppercase();
    ticker_to_code_map()
        .get(upper.as_str())
        .map(|&s| s.to_string())
        .unwrap_or(upper)
}

/// 将美国股票协议代码转换为对应的股票代码
/// 对应 Python 的 usa_code_to_ticker
pub fn usa_code_to_ticker(code: &str) -> String {
    let map = ticker_to_code_map();
    for (ticker, mapped_code) in map.iter() {
        if *mapped_code == code {
            return ticker.to_string();
        }
    }
    String::new()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_usa_ticker_to_code() {
        assert_eq!(usa_ticker_to_code("IXIC"), "A_IXIC");
        assert_eq!(usa_ticker_to_code("AAPL"), "AAPL");
    }

    #[test]
    fn test_usa_code_to_ticker() {
        assert_eq!(usa_code_to_ticker("A_IXIC"), "IXIC");
        assert_eq!(usa_code_to_ticker("B_DAX"), "DAX");
        assert_eq!(usa_code_to_ticker("UNKNOWN"), "");
    }
}
