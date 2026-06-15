// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// market — 市场/证券代码识别与纠正，与 Python data/market.py 对齐

use crate::data::meta::exchange::Exchange;
use crate::data::meta::instrument::{Instrument, InstrumentType};
use crate::data::meta::ticker_rules;
use crate::data::meta::ticker_rules::rule::{match_rule, global_rules};

use std::collections::HashSet;
use once_cell::sync::Lazy;

// ============================================================
// 常量: 交易所标识集合 — 对应 Python 的 PREFIX_EXCHANGE_IDENTIFIERS / ALL_EXCHANGE_IDENTIFIERS
// ============================================================
static PREFIX_EXCHANGE_IDENTIFIERS: Lazy<HashSet<&'static str>> = Lazy::new(|| {
    let mut s = HashSet::new();
    s.insert(Exchange::SSE.identifier());
    s.insert(Exchange::SZSE.identifier());
    s.insert(Exchange::BSE.identifier());
    s.insert(Exchange::HKEX.identifier());
    s
});

static ALL_EXCHANGE_IDENTIFIERS: Lazy<HashSet<&'static str>> = Lazy::new(|| {
    let mut s = HashSet::new();
    s.insert(Exchange::SSE.identifier());
    s.insert(Exchange::SZSE.identifier());
    s.insert(Exchange::BSE.identifier());
    s.insert(Exchange::HKEX.identifier());
    s.insert(Exchange::HKFE.identifier());
    s.insert(Exchange::USA.identifier());
    s
});

// ============================================================
// detect_instrument_type_by_rule — 根据交易所和代码检测证券类型
// ============================================================

/// 根据交易所和代码，使用对应规则检测证券类型
/// 对应 Python 的 detect_instrument_type_by_rule
pub fn detect_instrument_type_by_rule(exchange: Exchange, code: &str) -> InstrumentType {
    let rules = match exchange {
        Exchange::SSE => ticker_rules::sse_rules(),
        Exchange::SZSE => ticker_rules::szse_rules(),
        Exchange::BSE => ticker_rules::bse_rules(),
        Exchange::HKEX => ticker_rules::hkex_rules(),
        Exchange::USA => ticker_rules::usa_rules(),
        _ => return InstrumentType::UNKNOWN,
    };
    let cr = match_rule(code, &rules);
    cr.instrument_type
}

// ============================================================
// detect_symbol — 检测并解析证券代码
// ============================================================

/// 检测并解析证券代码的市场类型及证券类型
/// 对应 Python 的 detect_symbol
///
/// 支持多种格式:
///   - 前缀形式: sh600000
///   - 后缀形式: 600000.sh 或 AAPL.us
///   - 纯数字形式: 600000 (自动推断交易所)
///   - 4字母全大写: AAPL (自动识别为美股)
///   - 5位数字: 00700 (自动识别为港股)
pub fn detect_symbol(input_str: &str) -> Instrument {
    let s = input_str.trim();
    if s.is_empty() {
        return Instrument::unknown();
    }
    let s_lower = s.to_lowercase();
    let pure_code = s_lower.clone();

    let mut ticker = String::new();
    let mut exchange = Exchange::UNKNOWN;
    let mut typ = InstrumentType::UNKNOWN;

    // 1. 判断前缀: sh600000
    if pure_code.len() >= 2 {
        let prefix = &pure_code[..2];
        if PREFIX_EXCHANGE_IDENTIFIERS.contains(prefix) {
            ticker = pure_code[2..].to_string();
            exchange = Exchange::parse(prefix).unwrap_or(Exchange::UNKNOWN);
            // 走指定市场规则
        }
    }

    // 2. 判断后缀: 600000.sh or AAPL.us
    if exchange == Exchange::UNKNOWN && pure_code.len() >= 3 {
        let suffix_start = pure_code.len() - 3;
        if pure_code.as_bytes()[suffix_start] == b'.' {
            let suffix = &pure_code[suffix_start + 1..];
            if ALL_EXCHANGE_IDENTIFIERS.contains(suffix) {
                ticker = pure_code[..suffix_start].to_string();
                exchange = Exchange::parse(suffix).unwrap_or(Exchange::UNKNOWN);
            }
        }
    }

    // 3. 纯数字或者字母（无显式前缀/后缀）
    if exchange == Exchange::UNKNOWN {
        let code_len = pure_code.len();
        match code_len {
            4 => {
                if pure_code.chars().all(|c| c.is_ascii_alphabetic()) {
                    exchange = Exchange::USA;
                    typ = InstrumentType::STOCK;
                    return Instrument {
                        exchange,
                        instrument_type: typ,
                        ticker: pure_code,
                        ..Default::default()
                    };
                }
            }
            5 => {
                if pure_code.chars().all(|c| c.is_ascii_digit()) {
                    exchange = Exchange::HKEX;
                    typ = InstrumentType::STOCK;
                    return Instrument {
                        exchange,
                        instrument_type: typ,
                        ticker: pure_code,
                        ..Default::default()
                    };
                }
            }
            6 => {
                // 3.1 全局规则优先匹配
                let cr = match_rule(&pure_code, &global_rules());
                if cr.exchange != Exchange::UNKNOWN {
                    return Instrument {
                        exchange: cr.exchange,
                        instrument_type: cr.instrument_type,
                        ticker: pure_code,
                        ..Default::default()
                    };
                }

                // 3.2 按市场匹配规则
                // 3.2.1 0、159和3开头，优先匹配深交所
                if pure_code.starts_with('0') || pure_code.starts_with("159") || pure_code.starts_with('3') {
                    let cr = match_rule(&pure_code, &ticker_rules::szse_rules());
                    if cr.exchange != Exchange::UNKNOWN {
                        return Instrument {
                            exchange: cr.exchange,
                            instrument_type: cr.instrument_type,
                            ticker: pure_code,
                            ..Default::default()
                        };
                    }
                }
                // 3.2.2 6和5开头，优先匹配上交所
                if pure_code.starts_with('6') || pure_code.starts_with('5') {
                    let cr = match_rule(&pure_code, &ticker_rules::sse_rules());
                    if cr.exchange != Exchange::UNKNOWN {
                        return Instrument {
                            exchange: cr.exchange,
                            instrument_type: cr.instrument_type,
                            ticker: pure_code,
                            ..Default::default()
                        };
                    }
                }
                // 3.2.3 匹配上交所
                let cr = match_rule(&pure_code, &ticker_rules::sse_rules());
                if cr.exchange != Exchange::UNKNOWN {
                    return Instrument {
                        exchange: cr.exchange,
                        instrument_type: cr.instrument_type,
                        ticker: pure_code,
                        ..Default::default()
                    };
                }
                // 3.2.4 匹配深交所
                let cr = match_rule(&pure_code, &ticker_rules::szse_rules());
                if cr.exchange != Exchange::UNKNOWN {
                    return Instrument {
                        exchange: cr.exchange,
                        instrument_type: cr.instrument_type,
                        ticker: pure_code,
                        ..Default::default()
                    };
                }
                // 3.2.5 匹配北交所
                let cr = match_rule(&pure_code, &ticker_rules::bse_rules());
                if cr.exchange != Exchange::UNKNOWN {
                    return Instrument {
                        exchange: cr.exchange,
                        instrument_type: cr.instrument_type,
                        ticker: pure_code,
                        ..Default::default()
                    };
                }
            }
            _ => {
                return Instrument::unknown();
            }
        }
    }

    // 4. 如果exchange是UNKNOWN，则返回未知
    if exchange == Exchange::UNKNOWN {
        return Instrument::unknown();
    }

    // 5. 如果typ是Unknown，按市场规则匹配
    if typ == InstrumentType::UNKNOWN {
        let rules = match exchange {
            Exchange::SSE => ticker_rules::sse_rules(),
            Exchange::SZSE => ticker_rules::szse_rules(),
            Exchange::BSE => ticker_rules::bse_rules(),
            Exchange::HKEX => ticker_rules::hkex_rules(),
            Exchange::USA => ticker_rules::usa_rules(),
            _ => return Instrument::unknown(),
        };

        let cr = match_rule(&ticker, &rules);
        if cr.instrument_type != InstrumentType::UNKNOWN {
            return Instrument {
                exchange: cr.exchange,
                instrument_type: cr.instrument_type,
                ticker,
                ..Default::default()
            };
        } else {
            return Instrument::unknown();
        }
    } else {
        return Instrument {
            exchange,
            instrument_type: typ,
            ticker,
            ..Default::default()
        };
    }
}

// ============================================================
// correct_security_code — 纠正证券代码格式
// ============================================================

/// 纠正证券代码格式，补全前缀或后缀
/// 对应 Python 的 correct_security_code
///
/// 支持多种格式:
///   - 前缀形式: sh600000
///   - 后缀形式: 600000.sh 或 AAPL.us
///   - 纯数字形式: 600000 (自动推断交易所)
///   - 4字母全大写: AAPL (自动识别为美股)
///   - 6位数字: 600000 (自动推断交易所)
///
/// 返回规范的 {identifier}{ticker} 或 {ticker}.{identifier} 格式
pub fn correct_security_code(code: &str) -> String {
    if code.is_empty() {
        return String::new();
    }
    let inst = detect_symbol(code);
    if inst.can_construct_symbol() {
        inst.symbol()
    } else {
        // 如果无法构造，回退到简单的 strip + detect 逻辑
        // 保持与旧 detect_market 的兼容性
        let s = code.trim().to_lowercase();
        let market_flags: [&str; 5] = ["sh", "sz", "bj", "hk", "us"];

        // 前缀形式
        for &flag in &market_flags {
            if s.starts_with(flag) && s.len() > flag.len() {
                let ticker = &s[flag.len()..];
                return format!("{}{}", flag, ticker);
            }
        }

        // 后缀形式
        for &flag in &market_flags {
            let suffix = format!(".{}", flag);
            if s.ends_with(&suffix) && s.len() > suffix.len() {
                let ticker = &s[..s.len() - suffix.len()];
                return format!("{}{}", flag, ticker);
            }
        }

        // 纯数字代码推断
        if s.len() == 6 && s.chars().all(|c| c.is_ascii_digit()) {
            if s.starts_with('6') || s.starts_with('5') || s.starts_with('9') {
                return format!("sh{}", s);
            } else {
                return format!("sz{}", s);
            }
        }

        code.to_string()
    }
}

// ============================================================
// 每日初始化 cron 表达式 — 与 Python 的 cn_cron_expr_daily_init 对齐
// ============================================================
pub const PRE_MARKET_HOUR: u32 = 9;
pub const PRE_MARKET_MINUTE: u32 = 0;
pub const PRE_MARKET_SECOND: u32 = 0;

/// Cron 表达式: 每天 9:00 执行
pub fn cn_cron_expr_daily_init() -> String {
    format!("0 {} {} * * *", PRE_MARKET_HOUR, PRE_MARKET_MINUTE)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_detect_symbol_prefix_sh() {
        let inst = detect_symbol("sh600000");
        assert_eq!(inst.exchange, Exchange::SSE);
        assert_eq!(inst.ticker, "600000");
        assert!(inst.can_construct_symbol());
        assert_eq!(inst.symbol(), "sh600000");
    }

    #[test]
    fn test_detect_symbol_suffix_sh() {
        let inst = detect_symbol("600000.sh");
        assert_eq!(inst.exchange, Exchange::SSE);
        assert_eq!(inst.ticker, "600000");
        assert_eq!(inst.symbol(), "sh600000");
    }

    #[test]
    fn test_detect_symbol_suffix_sz() {
        let inst = detect_symbol("000001.sz");
        assert_eq!(inst.exchange, Exchange::SZSE);
        assert_eq!(inst.ticker, "000001");
        assert_eq!(inst.symbol(), "sz000001");
    }

    #[test]
    fn test_detect_symbol_pure_6digit_sh() {
        let inst = detect_symbol("600000");
        assert_eq!(inst.exchange, Exchange::SSE);
        assert_eq!(inst.ticker, "600000");
        assert_eq!(inst.symbol(), "sh600000");
    }

    #[test]
    fn test_detect_symbol_pure_6digit_sz() {
        let inst = detect_symbol("000001");
        assert_eq!(inst.exchange, Exchange::SZSE);
        assert_eq!(inst.ticker, "000001");
        assert_eq!(inst.symbol(), "sz000001");
    }

    #[test]
    fn test_detect_symbol_4alpha_us() {
        let inst = detect_symbol("AAPL");
        assert_eq!(inst.exchange, Exchange::USA);
        assert_eq!(inst.ticker, "aapl");
        assert_eq!(inst.symbol(), "aapl.us");
    }

    #[test]
    fn test_detect_symbol_5digit_hk() {
        let inst = detect_symbol("00700");
        assert_eq!(inst.exchange, Exchange::HKEX);
        assert_eq!(inst.ticker, "00700");
        assert_eq!(inst.symbol(), "00700.hk");
    }

    #[test]
    fn test_detect_symbol_suffix_us() {
        let inst = detect_symbol("aapl.us");
        assert_eq!(inst.exchange, Exchange::USA);
        assert_eq!(inst.ticker, "aapl");
        assert_eq!(inst.symbol(), "aapl.us");
    }

    #[test]
    fn test_detect_symbol_empty() {
        let inst = detect_symbol("");
        assert_eq!(inst.exchange, Exchange::UNKNOWN);
        assert!(!inst.can_construct_symbol());
    }

    #[test]
    fn test_correct_security_code() {
        assert_eq!(correct_security_code("600000"), "sh600000");
        assert_eq!(correct_security_code("000001"), "sz000001");
        assert_eq!(correct_security_code("600000.sh"), "sh600000");
        assert_eq!(correct_security_code("sh600000"), "sh600000");
        assert_eq!(correct_security_code("aapl.us"), "aapl.us");
        assert_eq!(correct_security_code(""), "");
    }

    #[test]
    fn test_detect_symbol_hkex_suffix() {
        let inst = detect_symbol("hsi.hk");
        assert_eq!(inst.exchange, Exchange::HKEX);
        assert_eq!(inst.ticker, "hsi");
        assert_eq!(inst.symbol(), "hsi.hk");
    }

    #[test]
    fn test_detect_symbol_hkex_prefix() {
        let inst = detect_symbol("hk00700");
        assert_eq!(inst.exchange, Exchange::HKEX);
        assert_eq!(inst.ticker, "00700");
        assert_eq!(inst.symbol(), "00700.hk");
    }

    #[test]
    fn test_detect_symbol_index() {
        let inst = detect_symbol("000001");
        assert_eq!(inst.exchange, Exchange::SZSE);
        assert_eq!(inst.ticker, "000001");
    }

    #[test]
    fn test_detect_symbol_etf() {
        let inst = detect_symbol("510050");
        assert_eq!(inst.exchange, Exchange::SSE);
        assert_eq!(inst.ticker, "510050");
        assert_eq!(inst.instrument_type, InstrumentType::ETF);
    }
}
