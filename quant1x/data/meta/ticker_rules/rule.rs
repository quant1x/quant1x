// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// CodeRule — 证券代码规则, 与 Python data/meta/ticker_rules/rule.py 对齐

use super::super::exchange::Exchange;
use super::super::instrument::InstrumentType;

/// 证券代码规则
#[derive(Debug, Clone)]
pub struct CodeRule {
    pub exchange: Exchange,
    pub prefix: RulePrefix,
    pub instrument_type: InstrumentType,
    pub name: &'static str,
    pub desc: &'static str,
}

/// 规则前缀: 可以是字符串前缀或数字范围
#[derive(Debug, Clone)]
pub enum RulePrefix {
    Str(&'static str),
    Range { start: &'static str, end: &'static str },
}

impl RulePrefix {
    /// 检查代码是否匹配此前缀
    pub fn matches(&self, code: &str) -> bool {
        match self {
            RulePrefix::Str(prefix) => {
                if prefix.is_empty() {
                    true // 空前缀匹配一切(如美股默认规则)
                } else {
                    code.starts_with(prefix)
                }
            }
            RulePrefix::Range { start, end } => {
                // 对于数字范围, 按字符串比较(因为代码可能是前导零的数字字符串)
                code >= *start && code <= *end
            }
        }
    }

    /// 返回前缀长度(用于最佳匹配排序)
    pub fn match_length(&self) -> usize {
        match self {
            RulePrefix::Str(s) => s.len(),
            RulePrefix::Range { start, end: _ } => {
                // 对于范围, 返回起始值的长度作为匹配长度
                start.len()
            }
        }
    }

    /// 返回范围的最大可能长度
    pub fn max_value_length(&self) -> usize {
        match self {
            RulePrefix::Str(s) => s.len(),
            RulePrefix::Range { start, end } => {
                std::cmp::max(start.len(), end.len())
            }
        }
    }
}

/// 根据代码前缀匹配最优规则
/// 对应 Python 的 match_rule 函数
pub fn match_rule(code: &str, rules: &[CodeRule]) -> CodeRule {
    let code = code.to_uppercase();
    let trimmed = code.trim();
    let mut best_match: Option<&CodeRule> = None;
    let mut best_len = 0usize;

    for entry in rules {
        let prefix = &entry.prefix;
        if prefix.matches(trimmed) {
            let len = prefix.match_length();
            if len > best_len {
                best_len = len;
                best_match = Some(entry);
            } else if best_len == 0 && len == 0 {
                // 空前缀在无其他匹配时使用
                best_match = Some(entry);
                break;
            }
        }
    }

    match best_match {
        Some(rule) => rule.clone(),
        None => CodeRule {
            exchange: Exchange::UNKNOWN,
            prefix: RulePrefix::Str(""),
            instrument_type: InstrumentType::UNKNOWN,
            name: "",
            desc: "未匹配到规则",
        },
    }
}

/// 全局规则(跨市场优先)
pub fn global_rules() -> Vec<CodeRule> {
    vec![
        CodeRule {
            exchange: Exchange::SSE,
            prefix: RulePrefix::Str("880"),
            instrument_type: InstrumentType::SECTOR,
            name: "板块指数",
            desc: "通达信",
        },
        CodeRule {
            exchange: Exchange::SSE,
            prefix: RulePrefix::Str("881"),
            instrument_type: InstrumentType::SECTOR,
            name: "板块指数",
            desc: "通达信",
        },
    ]
}
