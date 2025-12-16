use crate::exchange::{
    ExchangeId, SecurityCode, EXCHANGE_BJSE, EXCHANGE_HK, EXCHANGE_SSE, EXCHANGE_SZSE, EXCHANGE_US,
};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SecurityType {
    TypeUnknown,
    TypeStock,
    TypeETF,
    TypeFund,
    TypeBond,
    TypeBStock,
    TypeIPO,
    TypeIndex,
    TypeBlock,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
// Reuse `SecurityType` and `SecurityCodeExt` from `exchange.rs` to match Go layout

// CodeRule equivalent (match Go's struct with Prefix, Type, Desc)
struct CodeRule {
    prefix: &'static str,
    typ: SecurityType,
    desc: &'static str,
}

// Rule tables (ported from code_rule.go)
const GLOBAL_RULES: &[CodeRule] = &[
    CodeRule {
        prefix: "880",
        typ: SecurityType::TypeBlock,
        desc: "板块指数(通达信)",
    },
    CodeRule {
        prefix: "881",
        typ: SecurityType::TypeBlock,
        desc: "板块指数(通达信)",
    },
];

const SSE_RULES: &[CodeRule] = &[
    CodeRule {
        prefix: "000",
        typ: SecurityType::TypeIndex,
        desc: "上证指数",
    },
    CodeRule {
        prefix: "51",
        typ: SecurityType::TypeETF,
        desc: "上交所ETF(510-519)",
    },
    CodeRule {
        prefix: "588",
        typ: SecurityType::TypeETF,
        desc: "科创板ETF",
    },
    CodeRule {
        prefix: "50",
        typ: SecurityType::TypeFund,
        desc: "LOF/封闭式基金",
    },
    CodeRule {
        prefix: "52",
        typ: SecurityType::TypeFund,
        desc: "其他基金",
    },
    CodeRule {
        prefix: "600",
        typ: SecurityType::TypeStock,
        desc: "主板A股",
    },
    CodeRule {
        prefix: "601",
        typ: SecurityType::TypeStock,
        desc: "主板A股",
    },
    CodeRule {
        prefix: "603",
        typ: SecurityType::TypeStock,
        desc: "主板A股",
    },
    CodeRule {
        prefix: "605",
        typ: SecurityType::TypeStock,
        desc: "主板A股",
    },
    CodeRule {
        prefix: "688",
        typ: SecurityType::TypeStock,
        desc: "科创板",
    },
    CodeRule {
        prefix: "689",
        typ: SecurityType::TypeStock,
        desc: "科创板CDR",
    },
    CodeRule {
        prefix: "900",
        typ: SecurityType::TypeBStock,
        desc: "B股",
    },
    CodeRule {
        prefix: "110",
        typ: SecurityType::TypeBond,
        desc: "债券",
    },
    CodeRule {
        prefix: "113",
        typ: SecurityType::TypeBond,
        desc: "可转债",
    },
    CodeRule {
        prefix: "118",
        typ: SecurityType::TypeBond,
        desc: "可交换债",
    },
    CodeRule {
        prefix: "120",
        typ: SecurityType::TypeBond,
        desc: "公司债",
    },
    CodeRule {
        prefix: "123",
        typ: SecurityType::TypeBond,
        desc: "可转债",
    },
    CodeRule {
        prefix: "127",
        typ: SecurityType::TypeBond,
        desc: "可转债",
    },
    CodeRule {
        prefix: "128",
        typ: SecurityType::TypeBond,
        desc: "可转债",
    },
    CodeRule {
        prefix: "730",
        typ: SecurityType::TypeIPO,
        desc: "新股申购",
    },
    CodeRule {
        prefix: "780",
        typ: SecurityType::TypeIPO,
        desc: "新股申购",
    },
];

const SZSE_RULES: &[CodeRule] = &[
    CodeRule {
        prefix: "399",
        typ: SecurityType::TypeIndex,
        desc: "深证指数",
    },
    CodeRule {
        prefix: "159",
        typ: SecurityType::TypeETF,
        desc: "深交所ETF",
    },
    CodeRule {
        prefix: "150",
        typ: SecurityType::TypeFund,
        desc: "LOF",
    },
    CodeRule {
        prefix: "160",
        typ: SecurityType::TypeFund,
        desc: "LOF",
    },
    CodeRule {
        prefix: "161",
        typ: SecurityType::TypeFund,
        desc: "LOF",
    },
    CodeRule {
        prefix: "162",
        typ: SecurityType::TypeFund,
        desc: "LOF",
    },
    CodeRule {
        prefix: "163",
        typ: SecurityType::TypeFund,
        desc: "LOF",
    },
    CodeRule {
        prefix: "164",
        typ: SecurityType::TypeFund,
        desc: "LOF",
    },
    CodeRule {
        prefix: "167",
        typ: SecurityType::TypeFund,
        desc: "LOF",
    },
    CodeRule {
        prefix: "168",
        typ: SecurityType::TypeFund,
        desc: "LOF",
    },
    CodeRule {
        prefix: "169",
        typ: SecurityType::TypeFund,
        desc: "LOF",
    },
    CodeRule {
        prefix: "184",
        typ: SecurityType::TypeFund,
        desc: "封闭式基金",
    },
    CodeRule {
        prefix: "000",
        typ: SecurityType::TypeStock,
        desc: "主板A股",
    },
    CodeRule {
        prefix: "001",
        typ: SecurityType::TypeStock,
        desc: "主板A股",
    },
    CodeRule {
        prefix: "002",
        typ: SecurityType::TypeStock,
        desc: "主板A股",
    },
    CodeRule {
        prefix: "003",
        typ: SecurityType::TypeStock,
        desc: "主板A股",
    },
    CodeRule {
        prefix: "300",
        typ: SecurityType::TypeStock,
        desc: "创业板",
    },
    CodeRule {
        prefix: "301",
        typ: SecurityType::TypeStock,
        desc: "创业板",
    },
    CodeRule {
        prefix: "200",
        typ: SecurityType::TypeBStock,
        desc: "B股",
    },
    CodeRule {
        prefix: "110",
        typ: SecurityType::TypeBond,
        desc: "可转债",
    },
    CodeRule {
        prefix: "111",
        typ: SecurityType::TypeBond,
        desc: "可转债",
    },
    CodeRule {
        prefix: "118",
        typ: SecurityType::TypeBond,
        desc: "可交换债",
    },
    CodeRule {
        prefix: "123",
        typ: SecurityType::TypeBond,
        desc: "可转债",
    },
    CodeRule {
        prefix: "127",
        typ: SecurityType::TypeBond,
        desc: "可转债",
    },
    CodeRule {
        prefix: "128",
        typ: SecurityType::TypeBond,
        desc: "可转债",
    },
];

const BJSE_RULES: &[CodeRule] = &[
    CodeRule {
        prefix: "920",
        typ: SecurityType::TypeStock,
        desc: "北交所股票(2024年起新上市)",
    },
    CodeRule {
        prefix: "83",
        typ: SecurityType::TypeStock,
        desc: "北交所股票(原精选层)",
    },
    CodeRule {
        prefix: "87",
        typ: SecurityType::TypeStock,
        desc: "北交所股票(原精选层)",
    },
    CodeRule {
        prefix: "88",
        typ: SecurityType::TypeStock,
        desc: "北交所股票(2022-2023年上市)",
    },
    CodeRule {
        prefix: "82",
        typ: SecurityType::TypeBond,
        desc: "优先股",
    },
    CodeRule {
        prefix: "89",
        typ: SecurityType::TypeBond,
        desc: "可转债",
    },
];

const HKSE_RULES: &[CodeRule] = &[
    CodeRule {
        prefix: "HSI",
        typ: SecurityType::TypeIndex,
        desc: "恒生指数",
    },
    CodeRule {
        prefix: "HSCEI",
        typ: SecurityType::TypeIndex,
        desc: "国企指数",
    },
    CodeRule {
        prefix: "HSCCI",
        typ: SecurityType::TypeIndex,
        desc: "红筹指数",
    },
    CodeRule {
        prefix: "028",
        typ: SecurityType::TypeETF,
        desc: "ETF",
    },
    CodeRule {
        prefix: "030",
        typ: SecurityType::TypeETF,
        desc: "ETF",
    },
    CodeRule {
        prefix: "031",
        typ: SecurityType::TypeETF,
        desc: "ETF",
    },
    CodeRule {
        prefix: "090",
        typ: SecurityType::TypeETF,
        desc: "ETF",
    },
    CodeRule {
        prefix: "091",
        typ: SecurityType::TypeETF,
        desc: "ETF",
    },
    CodeRule {
        prefix: "08",
        typ: SecurityType::TypeStock,
        desc: "港股(GEM)",
    },
    CodeRule {
        prefix: "0",
        typ: SecurityType::TypeStock,
        desc: "港股",
    },
    CodeRule {
        prefix: "1",
        typ: SecurityType::TypeBond,
        desc: "权证",
    },
    CodeRule {
        prefix: "2",
        typ: SecurityType::TypeBond,
        desc: "权证",
    },
    CodeRule {
        prefix: "4",
        typ: SecurityType::TypeBond,
        desc: "牛熊证",
    },
    CodeRule {
        prefix: "5",
        typ: SecurityType::TypeBond,
        desc: "牛熊证",
    },
    CodeRule {
        prefix: "6",
        typ: SecurityType::TypeBond,
        desc: "牛熊证",
    },
];

/// 根据给定的代码和规则列表匹配最符合的证券类型
///
/// 该函数通过比较代码前缀与规则列表中的前缀来匹配证券类型，
/// 返回匹配到的最长前缀对应的证券类型。
///
/// # 参数
/// * `code` - 待匹配的证券代码字符串
/// * `rules` - 证券代码规则列表，包含前缀和对应的证券类型
///
/// # 返回值
/// * `Some(SecurityType)` - 匹配到的最符合的证券类型
/// * `None` - 未匹配到任何规则
fn match_rule(code: &str, rules: &[CodeRule]) -> Option<SecurityType> {
    let mut best_len = 0usize;
    let mut matched: Option<SecurityType> = None;
    for r in rules.iter() {
        if code.starts_with(r.prefix) {
            let l = r.prefix.len();
            if l > best_len {
                best_len = l;
                matched = Some(r.typ);
            }
        }
    }
    matched
} // Closing brace for match_rule function

/// Detect 解析证券代码并返回 SecurityCode
pub fn detect(input: &str) -> SecurityCode {
    // Port of Go Detect: single-pass extraction then rule-based resolution
    let raw = input.trim();
    if raw.is_empty() {
        return SecurityCode::new(ExchangeId::ShangHai, "", SecurityType::TypeUnknown);
    }

    let pure_code = raw.to_lowercase();
    let mut symbol = String::new();
    let mut exchange_id = ExchangeId::Unknown;
    let mut typ = SecurityType::TypeUnknown;

    // All exchange flags (use exchange constants to stay in sync)
    let flags = [
        EXCHANGE_SSE.as_str(),
        EXCHANGE_SZSE.as_str(),
        EXCHANGE_BJSE.as_str(),
        EXCHANGE_HK.as_str(),
        EXCHANGE_US.as_str(),
    ];

    // 1. explicit market prefix
    if crate::std::strings::starts_with(&pure_code, &flags) {
        symbol = pure_code[2..].to_string();
        let flag = &pure_code[..2];
        exchange_id = if flag == EXCHANGE_SSE.as_str() {
            ExchangeId::ShangHai
        } else if flag == EXCHANGE_SZSE.as_str() {
            ExchangeId::ShenZhen
        } else if flag == EXCHANGE_BJSE.as_str() {
            ExchangeId::BeiJing
        } else if flag == EXCHANGE_HK.as_str() {
            ExchangeId::HongKong
        } else if flag == EXCHANGE_US.as_str() {
            ExchangeId::USA
        } else {
            ExchangeId::ShangHai
        };
    } else if crate::std::strings::ends_with(&pure_code, &flags)
        && pure_code.len() >= 3
        && pure_code.as_bytes()[pure_code.len() - 3] as char == '.'
    {
        // 2. explicit market suffix like 600000.sh or appl.us
        let len = pure_code.len();
        symbol = pure_code[..len - 3].to_string();
        let flag = &pure_code[len - 2..];
        exchange_id = if flag == EXCHANGE_SSE.as_str() {
            ExchangeId::ShangHai
        } else if flag == EXCHANGE_SZSE.as_str() {
            ExchangeId::ShenZhen
        } else if flag == EXCHANGE_BJSE.as_str() {
            ExchangeId::BeiJing
        } else if flag == EXCHANGE_HK.as_str() {
            ExchangeId::HongKong
        } else if flag == EXCHANGE_US.as_str() {
            ExchangeId::USA
        } else {
            ExchangeId::ShangHai
        };
    } else {
        // 3. plain form
        let code_len = pure_code.len();
        match code_len {
            4 => {
                if pure_code.chars().all(|c| c.is_ascii_lowercase()) {
                    exchange_id = ExchangeId::USA;
                    symbol = pure_code.clone();
                    typ = SecurityType::TypeStock;
                } else {
                    exchange_id = ExchangeId::Unknown;
                    symbol.clear();
                    typ = SecurityType::TypeUnknown;
                }
            }
            5 => {
                exchange_id = ExchangeId::HongKong;
                symbol = pure_code.clone();
            }
            6 => {
                // 6-digit: global rules first, then szse, bjse, sse (ordering per Go)
                if let Some(t) = match_rule(&pure_code, GLOBAL_RULES) {
                    return SecurityCode::new(ExchangeId::ShangHai, &pure_code, t);
                }
                if let Some(t) = match_rule(&pure_code, SZSE_RULES) {
                    return SecurityCode::new(ExchangeId::ShenZhen, &pure_code, t);
                }
                if let Some(t) = match_rule(&pure_code, BJSE_RULES) {
                    return SecurityCode::new(ExchangeId::BeiJing, &pure_code, t);
                }
                if let Some(t) = match_rule(&pure_code, SSE_RULES) {
                    return SecurityCode::new(ExchangeId::ShangHai, &pure_code, t);
                }
                // no match -- leave exchange_id unknown
                symbol = pure_code.clone();
            }
            _ => {}
        }
    }

    if exchange_id == ExchangeId::Unknown {
        return SecurityCode::new(ExchangeId::Unknown, "", SecurityType::TypeUnknown);
    }

    if typ == SecurityType::TypeUnknown {
        // derive type based on market rules
        let rules = match exchange_id {
            ExchangeId::ShangHai => SSE_RULES,
            ExchangeId::ShenZhen => SZSE_RULES,
            ExchangeId::BeiJing => BJSE_RULES,
            ExchangeId::HongKong => HKSE_RULES,
            ExchangeId::USA => &[],
            _ => &[],
        };
        if exchange_id == ExchangeId::USA {
            typ = SecurityType::TypeStock;
            return SecurityCode::new(exchange_id, &symbol, typ);
        }
        if let Some(t) = match_rule(&symbol, rules) {
            typ = t;
            return SecurityCode::new(exchange_id, &symbol, typ);
        } else {
            return SecurityCode::new(ExchangeId::Unknown, "", SecurityType::TypeUnknown);
        }
    } else {
        return SecurityCode::new(exchange_id, &symbol, typ);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::exchange::{ExchangeId, SecurityType};
    #[test]
    fn test_detect_scenarios_match_go() {
        let tests = vec![
            // From Go TestDetect_Scenarios: (name, in, expected Market, Symbol, Type)
            (
                "sh prefix",
                "sh600000",
                ExchangeId::ShangHai,
                "600000",
                SecurityType::TypeStock,
            ),
            (
                "plain 6-digit SSE",
                "600000",
                ExchangeId::ShangHai,
                "600000",
                SecurityType::TypeStock,
            ),
            (
                "sz prefix",
                "sz000001",
                ExchangeId::ShenZhen,
                "000001",
                SecurityType::TypeStock,
            ),
            (
                "hk suffix",
                "00700.hk",
                ExchangeId::HongKong,
                "00700",
                SecurityType::TypeStock,
            ),
            (
                "us suffix",
                "appl.us",
                ExchangeId::USA,
                "appl",
                SecurityType::TypeStock,
            ),
            (
                "us upper suffix",
                "APPL.US",
                ExchangeId::USA,
                "appl",
                SecurityType::TypeStock,
            ),
            // invalid / error formats
            (
                "too short numeric",
                "123",
                ExchangeId::Unknown,
                "",
                SecurityType::TypeUnknown,
            ),
            (
                "four digits numeric",
                "6006",
                ExchangeId::Unknown,
                "",
                SecurityType::TypeUnknown,
            ),
            (
                "four digits numeric dup",
                "6006",
                ExchangeId::Unknown,
                "",
                SecurityType::TypeUnknown,
            ),
            (
                "000001 (sz)",
                "000001",
                ExchangeId::ShenZhen,
                "000001",
                SecurityType::TypeStock,
            ),
            (
                "880005 (block->sh)",
                "880005",
                ExchangeId::ShangHai,
                "880005",
                SecurityType::TypeBlock,
            ),
            (
                "five digits -> hk",
                "60060",
                ExchangeId::HongKong,
                "60060",
                SecurityType::TypeBond,
            ),
            // From rule table
            (
                "global 880",
                "880000",
                ExchangeId::ShangHai,
                "880000",
                SecurityType::TypeBlock,
            ),
            (
                "global 881",
                "881000",
                ExchangeId::ShangHai,
                "881000",
                SecurityType::TypeBlock,
            ),
            // SSE
            (
                "sse ETF 51",
                "510000",
                ExchangeId::ShangHai,
                "510000",
                SecurityType::TypeETF,
            ),
            (
                "sse ETF 588",
                "588000",
                ExchangeId::ShangHai,
                "588000",
                SecurityType::TypeETF,
            ),
            (
                "sse fund 50",
                "500000",
                ExchangeId::ShangHai,
                "500000",
                SecurityType::TypeFund,
            ),
            (
                "sse fund 52",
                "520000",
                ExchangeId::ShangHai,
                "520000",
                SecurityType::TypeFund,
            ),
            (
                "sse stock 688",
                "688000",
                ExchangeId::ShangHai,
                "688000",
                SecurityType::TypeStock,
            ),
            (
                "sse stock 689",
                "689000",
                ExchangeId::ShangHai,
                "689000",
                SecurityType::TypeStock,
            ),
            (
                "sse bstock 900",
                "900000",
                ExchangeId::ShangHai,
                "900000",
                SecurityType::TypeBStock,
            ),
            (
                "sse ipo 730",
                "730000",
                ExchangeId::ShangHai,
                "730000",
                SecurityType::TypeIPO,
            ),
            // SZSE
            (
                "sz index 399",
                "399000",
                ExchangeId::ShenZhen,
                "399000",
                SecurityType::TypeIndex,
            ),
            (
                "sz etf 159",
                "159000",
                ExchangeId::ShenZhen,
                "159000",
                SecurityType::TypeETF,
            ),
            (
                "sz fund 150",
                "150000",
                ExchangeId::ShenZhen,
                "150000",
                SecurityType::TypeFund,
            ),
            (
                "sz gem 300",
                "300000",
                ExchangeId::ShenZhen,
                "300000",
                SecurityType::TypeStock,
            ),
            (
                "sz bstock 200",
                "200000",
                ExchangeId::ShenZhen,
                "200000",
                SecurityType::TypeBStock,
            ),
            // BJSE
            (
                "bj new 920",
                "920000",
                ExchangeId::BeiJing,
                "920000",
                SecurityType::TypeStock,
            ),
            (
                "bj 83",
                "830000",
                ExchangeId::BeiJing,
                "830000",
                SecurityType::TypeStock,
            ),
            (
                "bj 87",
                "870000",
                ExchangeId::BeiJing,
                "870000",
                SecurityType::TypeStock,
            ),
            (
                "bj bond 82",
                "820000",
                ExchangeId::BeiJing,
                "820000",
                SecurityType::TypeBond,
            ),
            // HK (5-digit)
            (
                "hk etf 028",
                "02800",
                ExchangeId::HongKong,
                "02800",
                SecurityType::TypeETF,
            ),
            (
                "hk stock 0",
                "00000",
                ExchangeId::HongKong,
                "00000",
                SecurityType::TypeStock,
            ),
        ];

        for (name, input, exp_market, exp_symbol, exp_type) in tests {
            let got = detect(input);
            assert_eq!(
                got.market, exp_market,
                "{}: market mismatch for {}",
                name, input
            );
            assert_eq!(
                got.symbol, exp_symbol,
                "{}: symbol mismatch for {}",
                name, input
            );
            assert_eq!(got.typ, exp_type, "{}: type mismatch for {}", name, input);
        }
    }
}
