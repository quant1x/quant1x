// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// SZSE 深圳证券交易所规则 — 与 Python data/meta/ticker_rules/market_szse.py 对齐

use super::rule::{CodeRule, RulePrefix};
use super::super::exchange::Exchange;
use super::super::instrument::InstrumentType;

/// SZSE 深圳证券交易所规则
pub fn szse_rules() -> Vec<CodeRule> {
    vec![
        // 指数
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("395"), instrument_type: InstrumentType::INDEX, name: "成交量统计指数", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("399"), instrument_type: InstrumentType::INDEX, name: "深证指数", desc: "" },

        // 主板A股
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("000"), instrument_type: InstrumentType::STOCK, name: "主板A股", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("001"), instrument_type: InstrumentType::STOCK, name: "主板A股", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("002"), instrument_type: InstrumentType::STOCK, name: "主板A股", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("003"), instrument_type: InstrumentType::STOCK, name: "主板A股", desc: "" },

        // 权证
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("030"), instrument_type: InstrumentType::WARRANT, name: "权证", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("031"), instrument_type: InstrumentType::WARRANT, name: "权证", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("032"), instrument_type: InstrumentType::WARRANT, name: "权证", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("036"), instrument_type: InstrumentType::WARRANT, name: "创业板股权激励计划涉及的员工认股权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("0370"), instrument_type: InstrumentType::WARRANT, name: "主板A股股权激励计划涉及的员工认股权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("0371"), instrument_type: InstrumentType::WARRANT, name: "主板A股股权激励计划涉及的员工认股权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("0372"), instrument_type: InstrumentType::WARRANT, name: "创业板股权激励计划审计的员工认股权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("0373"), instrument_type: InstrumentType::WARRANT, name: "主板A股股权激励计划涉及的员工认股权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("0374"), instrument_type: InstrumentType::WARRANT, name: "主板A股股权激励计划涉及的员工认股权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("0375"), instrument_type: InstrumentType::WARRANT, name: "中小企业板股权激励计划涉及的员工认股权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("0376"), instrument_type: InstrumentType::WARRANT, name: "中小企业板股权激励计划涉及的员工认股权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("0377"), instrument_type: InstrumentType::WARRANT, name: "中小企业板股权激励计划涉及的员工认股权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("0378"), instrument_type: InstrumentType::WARRANT, name: "中小企业板股权激励计划涉及的员工认股权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("0379"), instrument_type: InstrumentType::WARRANT, name: "中小企业板股权激励计划涉及的员工认股权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("038"), instrument_type: InstrumentType::WARRANT, name: "主板A股及中小企业股票认沽权证", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("039"), instrument_type: InstrumentType::WARRANT, name: "主板A股及中小企业股票认沽权证", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("070"), instrument_type: InstrumentType::WARRANT, name: "主板A股增发/可转债申购", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("071"), instrument_type: InstrumentType::WARRANT, name: "主板A股增发/可转债申购", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("072"), instrument_type: InstrumentType::WARRANT, name: "中小企业板增发/可转债申购", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("073"), instrument_type: InstrumentType::WARRANT, name: "中小企业板增发/可转债申购", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("074"), instrument_type: InstrumentType::WARRANT, name: "中小企业板增发/可转债申购", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("080"), instrument_type: InstrumentType::WARRANT, name: "A股配股", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("0"), instrument_type: InstrumentType::STOCK, name: "股票", desc: "" },

        // 债券
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("10"), instrument_type: InstrumentType::BOND, name: "国债", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("11"), instrument_type: InstrumentType::BOND, name: "企业债", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("120"), instrument_type: InstrumentType::BOND, name: "企业债券", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("123"), instrument_type: InstrumentType::BOND, name: "可转债", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("127"), instrument_type: InstrumentType::BOND, name: "可转债", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("128"), instrument_type: InstrumentType::BOND, name: "可转债", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("13"), instrument_type: InstrumentType::BOND, name: "债券回购", desc: "" },

        // 基金
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("159"), instrument_type: InstrumentType::ETF, name: "深交所ETF", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("15"), instrument_type: InstrumentType::FUND, name: "ETF", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("16"), instrument_type: InstrumentType::FUND, name: "LOF", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("17"), instrument_type: InstrumentType::FUND, name: "传统投资基金", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("184"), instrument_type: InstrumentType::FUND, name: "封闭式基金", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("18"), instrument_type: InstrumentType::FUND, name: "封闭式基金", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("1"), instrument_type: InstrumentType::BOND, name: "债券", desc: "" },

        // B股
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("200"), instrument_type: InstrumentType::BSTOCK, name: "B股", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("238"), instrument_type: InstrumentType::OTHER, name: "B股现金选择权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("28"), instrument_type: InstrumentType::OTHER, name: "B股配股优先权", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("2"), instrument_type: InstrumentType::BSTOCK, name: "B股", desc: "" },

        // 创业板
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("300"), instrument_type: InstrumentType::STOCK, name: "创业板", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("301"), instrument_type: InstrumentType::STOCK, name: "创业板注册制", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("30"), instrument_type: InstrumentType::STOCK, name: "创业板", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("36"), instrument_type: InstrumentType::OTHER, name: "投票", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("37"), instrument_type: InstrumentType::OTHER, name: "增发/可转债申购", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("38"), instrument_type: InstrumentType::OTHER, name: "配股/可转债优先权", desc: "" },

        // 资产支持证券ABS
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("50"), instrument_type: InstrumentType::BOND, name: "资产支持证券ABS", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("56"), instrument_type: InstrumentType::BOND, name: "资产支持证券ABS", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("5"), instrument_type: InstrumentType::BOND, name: "资产支持证券ABS", desc: "" },

        // 其他
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("700"), instrument_type: InstrumentType::WARRANT, name: "B股增发", desc: "" },
        CodeRule { exchange: Exchange::SZSE, prefix: RulePrefix::Str("730"), instrument_type: InstrumentType::WARRANT, name: "跨市场申购", desc: "" },
    ]
}
