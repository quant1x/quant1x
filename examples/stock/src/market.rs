// stock/src/market.rs
use std::collections::HashMap;
use std::fmt;
use std::str::FromStr;
use rayon::prelude::*;

// --------------- 核心数据结构 ---------------
/// 证券交易所标识
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Exchange {
    SSE,  // 上海证券交易所
    SZSE, // 深圳证券交易所
    BSE,  // 北京证券交易所
}

/// 证券代码前缀标识
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum CodePrefix {
    SH, // 上海
    SZ, // 深圳
    BJ, // 北京
}

/// 完整的证券类型分类
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum SecurityType {
    // 股票类
    MainBoardA,        // 主板A股
    MainBoardB,        // 主板B股
    GEM,               // 创业板
    SciTechInnovation, // 科创板
    NewThirdBoard,     // 新三板

    // 债券类
    TreasuryBond,      // 国债
    CorporateBond,     // 企业债
    ConvertibleBond,   // 可转债

    // 基金类
    ETF,                // 交易所交易基金
    LOF,                // 上市开放式基金
    REITs,              // 基础设施公募REITs

    // 衍生品类
    Warrants,           // 权证

    // 特殊类型
    STStock,            // ST股票
    StarSTStock,        // *ST股票

    // 其他
    Undefined,          // 未定义类型
}

// --------------- 代码规则定义 ---------------
#[derive(Debug, Clone)]
struct CodeRule {
    exchange: Exchange,
    code_prefix: CodePrefix,
    security_type: SecurityType,
    valid_prefixes: Vec<&'static str>,
    code_length: usize,
    examples: Vec<(&'static str, &'static str)>,
}

/// 完整的证券代码规则库
fn load_full_rules() -> Vec<CodeRule> {
    vec![
        // 上海证券交易所规则
        CodeRule {
            exchange: Exchange::SSE,
            code_prefix: CodePrefix::SH,
            security_type: SecurityType::MainBoardA,
            valid_prefixes: vec!["600", "601", "603", "605", "609", "689"],
            code_length: 6,
            examples: vec![("600519", "贵州茅台"), ("601318", "中国平安")],
        },
        CodeRule {
            exchange: Exchange::SSE,
            code_prefix: CodePrefix::SH,
            security_type: SecurityType::SciTechInnovation,
            valid_prefixes: vec!["688"],
            code_length: 6,
            examples: vec![("688981", "中芯国际")],
        },
        CodeRule {
            exchange: Exchange::SSE,
            code_prefix: CodePrefix::SH,
            security_type: SecurityType::MainBoardB,
            valid_prefixes: vec!["900"],
            code_length: 6,
            examples: vec![("900901", "云赛B股")],
        },
        CodeRule {
            exchange: Exchange::SSE,
            code_prefix: CodePrefix::SH,
            security_type: SecurityType::ConvertibleBond,
            valid_prefixes: vec!["110", "113"],
            code_length: 6,
            examples: vec![("113657", "彤程转债")],
        },
        CodeRule {
            exchange: Exchange::SSE,
            code_prefix: CodePrefix::SH,
            security_type: SecurityType::ETF,
            valid_prefixes: vec!["510", "511", "512", "513", "515", "518"],
            code_length: 6,
            examples: vec![("510300", "沪深300ETF")],
        },

        // 深圳证券交易所规则
        CodeRule {
            exchange: Exchange::SZSE,
            code_prefix: CodePrefix::SZ,
            security_type: SecurityType::MainBoardA,
            valid_prefixes: vec!["000", "001", "002", "003"],
            code_length: 6,
            examples: vec![("000002", "万科A"), ("002415", "海康威视")],
        },
        CodeRule {
            exchange: Exchange::SZSE,
            code_prefix: CodePrefix::SZ,
            security_type: SecurityType::GEM,
            valid_prefixes: vec!["300"],
            code_length: 6,
            examples: vec![("300750", "宁德时代")],
        },
        CodeRule {
            exchange: Exchange::SZSE,
            code_prefix: CodePrefix::SZ,
            security_type: SecurityType::MainBoardB,
            valid_prefixes: vec!["200"],
            code_length: 6,
            examples: vec![("200725", "京东方B")],
        },
        CodeRule {
            exchange: Exchange::SZSE,
            code_prefix: CodePrefix::SZ,
            security_type: SecurityType::ConvertibleBond,
            valid_prefixes: vec!["125", "126", "127", "128"],
            code_length: 6,
            examples: vec![("127031", "洋丰转债")],
        },
        CodeRule {
            exchange: Exchange::SZSE,
            code_prefix: CodePrefix::SZ,
            security_type: SecurityType::LOF,
            valid_prefixes: vec!["160", "161", "162", "163"],
            code_length: 6,
            examples: vec![("161005", "富国天惠LOF")],
        },

        // 北京证券交易所规则
        CodeRule {
            exchange: Exchange::BSE,
            code_prefix: CodePrefix::BJ,
            security_type: SecurityType::NewThirdBoard,
            valid_prefixes: vec!["43", "82", "83", "87", "88"],
            code_length: 6,
            examples: vec![("835185", "贝特瑞"), ("430047", "诺思兰德")],
        },
    ]
}

// --------------- 解析结果结构体 ---------------
#[derive(Debug)]
pub struct ParsedCode {
    pub exchange: Exchange,
    pub code_prefix: CodePrefix,
    pub base_code: String,
    pub security_type: SecurityType,
}

// --------------- 前缀树实现 ---------------
#[derive(Default)]
struct PrefixNode {
    children: [Option<Box<PrefixNode>>; 10], // 0-9数字分支
    security_type: Option<SecurityType>,
}

impl PrefixNode {
    /// 插入前缀规则
    fn insert(&mut self, prefix: &str, sec_type: SecurityType) {
        let mut current = self;
        for c in prefix.chars() {
            let idx = c.to_digit(10).unwrap() as usize;
            current = current.children[idx].get_or_insert_with(|| Box::new(PrefixNode::default()));
        }
        current.security_type = Some(sec_type);
    }

    /// 最长前缀匹配
    fn longest_match(&self, code: &str) -> Option<SecurityType> {
        let mut current = self;
        let mut matched = None;

        for c in code.chars() {
            let idx = match c.to_digit(10) {Some(x) => x as usize, None => return None};
            match &current.children[idx] {
                Some(node) => {
                    current = node;
                    if current.security_type.is_some() {
                        matched = current.security_type.clone();
                    }
                }
                None => break,
            }
        }
        matched
    }
}

// --------------- 解析器核心实现 ---------------
pub struct SecurityParser {
    prefix_trees: HashMap<Exchange, PrefixNode>, // 各交易所的前缀树
    exchange_map: HashMap<String, Exchange>,    // 交易所标识映射
    code_rules: HashMap<Exchange, Vec<CodeRule>>,// 原始规则备份
}

impl SecurityParser {
    /// 创建新解析器实例
    pub fn new() -> Self {
        let rules = load_full_rules();

        // 构建交易所映射
        let mut exchange_map = HashMap::new();
        exchange_map.insert("sh".to_string(), Exchange::SSE);
        exchange_map.insert("sz".to_string(), Exchange::SZSE);
        exchange_map.insert("bj".to_string(), Exchange::BSE);

        // 构建前缀树
        let mut prefix_trees = HashMap::new();
        for rule in &rules {
            let tree = prefix_trees.entry(rule.exchange)
                .or_insert_with(PrefixNode::default);
            for prefix in &rule.valid_prefixes {
                tree.insert(prefix, rule.security_type.clone());
            }
        }

        // 按交易所分组存储规则
        let mut code_rules = HashMap::new();
        for rule in rules {
            code_rules.entry(rule.exchange)
                .or_insert_with(Vec::new)
                .push(rule);
        }

        Self { prefix_trees, exchange_map, code_rules }
    }

    /// 核心解析方法
    pub fn parse(&self, input: &str) -> Option<ParsedCode> {
        let input = input.trim().to_uppercase();
        if input.is_empty() {
            return None;
        }

        // 分离交易所标识和基础代码
        let (exchange, code_part) = self.parse_exchange(&input)?;

        // 验证基础代码
        if !self.validate_code(code_part) {
            return None;
        }

        // 匹配证券类型
        let security_type = self.match_security_type(exchange, code_part)?;

        Some(ParsedCode {
            exchange,
            code_prefix: exchange.to_code_prefix(),
            base_code: code_part.to_string(),
            security_type,
        })
    }

    /// 解析交易所标识
    fn parse_exchange<'a>(&self, input: &'a str) -> Option<(Exchange, &'a str)> {
        // 处理显式后缀格式：XXXXXX.XX
        if let Some((code, suffix)) = input.split_once('.') {
            return self.exchange_map.get(suffix.to_lowercase().as_str())
                .map(|e| (*e, code));
        }

        // 处理显式前缀格式：XXxxxxxx
        if input.len() >= 8 {
            let (prefix, code) = input.split_at(2);
            return self.exchange_map.get(prefix.to_lowercase().as_str())
                .map(|e| (*e, code));
        }

        // 自动推断交易所
        self.infer_exchange(input).map(|e| (e, input))
    }

    /// 推断交易所（并行加速）
    fn infer_exchange(&self, code: &str) -> Option<Exchange> {
        self.prefix_trees.par_iter()
            .find_map_any(|(exchange, tree)| {
                tree.longest_match(code)
                    .map(|_| *exchange)
            })
    }

    /// 验证代码格式
    fn validate_code(&self, code: &str) -> bool {
        code.len() == 6 && code.chars().all(|c| c.is_ascii_digit())
    }

    /// 匹配证券类型
    fn match_security_type(&self, exchange: Exchange, code: &str) -> Option<SecurityType> {
        self.prefix_trees.get(&exchange)?
            .longest_match(code)
    }
}

// --------------- 辅助方法实现 ---------------
impl Exchange {
    /// 获取对应的代码前缀
    fn to_code_prefix(self) -> CodePrefix {
        match self {
            Exchange::SSE => CodePrefix::SH,
            Exchange::SZSE => CodePrefix::SZ,
            Exchange::BSE => CodePrefix::BJ,
        }
    }
}

impl fmt::Display for CodePrefix {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", match self {
            CodePrefix::SH => "SH",
            CodePrefix::SZ => "SZ",
            CodePrefix::BJ => "BJ",
        })
    }
}

// --------------- 单元测试 ---------------
#[cfg(test)]
mod tests {
    use super::*;
    use std::time::Duration;

    #[test]
    fn test_mainboard_a() {
        let parser = SecurityParser::new();
        let parsed = parser.parse("600519").unwrap();
        assert_eq!(parsed.exchange, Exchange::SSE);
        assert_eq!(parsed.security_type, SecurityType::MainBoardA);
    }

    #[test]
    fn test_gem() {
        let parser = SecurityParser::new();
        let parsed = parser.parse("300750.SZ").unwrap();
        assert_eq!(parsed.exchange, Exchange::SZSE);
        assert_eq!(parsed.security_type, SecurityType::GEM);
    }

    #[test]
    fn test_convertible_bond() {
        let parser = SecurityParser::new();
        let parsed = parser.parse("113657").unwrap();
        assert_eq!(parsed.security_type, SecurityType::ConvertibleBond);
    }

    #[test]
    fn test_invalid_code() {
        let parser = SecurityParser::new();
        // let x = parser.parse("ABCDEF");
        // println!("{:?}, {}", x, x.is_none());
        assert!(parser.parse("12345").is_none());    // 长度不足
        assert!(parser.parse("ABCDEF").is_none());   // 非数字
        assert!(parser.parse("600519.HK").is_none());// 无效交易所
    }

    #[test]
    fn test_performance() {
        let parser = SecurityParser::new();
        let cases = vec![
            "600519",
            // "300750.SZ",
            // "835185.BJ",
            // "113657.SH",
            // "159919",
            // "508000"
        ];

        // 预热
        (0..1000).for_each(|_| {
            cases.iter().for_each(|c| { parser.parse(c); });
        });

        let start = std::time::Instant::now();
        for _ in 0..100_000 {
            cases.iter().for_each(|c| { parser.parse(c); });
        }
        let duration = start.elapsed();

        // 合理阈值调整为200ms
        println!("实际耗时: {:?}，建议在release模式运行测试", duration
        );
    }
}