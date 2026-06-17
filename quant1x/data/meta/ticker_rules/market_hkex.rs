// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// HKEX 香港交易所规则 — 与 Python data/meta/ticker_rules/market_hkex.py 对齐

use super::rule::{CodeRule, RulePrefix};
use super::super::exchange::Exchange;
use super::super::instrument::InstrumentType;

/// HKEX 香港交易所规则
pub fn hkex_rules() -> Vec<CodeRule> {
    vec![
        // 指数
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Str("HSI"), instrument_type: InstrumentType::INDEX, name: "恒生指数", desc: "香港交易所" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Str("HSCEI"), instrument_type: InstrumentType::INDEX, name: "国企指数", desc: "香港交易所" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Str("HSCCI"), instrument_type: InstrumentType::INDEX, name: "红筹指数", desc: "香港交易所" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Str("HSTECH"), instrument_type: InstrumentType::INDEX, name: "恒生科技指数", desc: "香港交易所" },

        // 00001-09999, 主板及GEM上市证券
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "00001", end: "02799" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "02800", end: "02849" }, instrument_type: InstrumentType::FUND, name: "交易所买卖基金", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "02850", end: "02899" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "02900", end: "02999" }, instrument_type: InstrumentType::TEMPORARY_STOCK, name: "主板临时柜台", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "03000", end: "03199" }, instrument_type: InstrumentType::FUND, name: "交易所买卖基金", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "03200", end: "03399" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "03400", end: "03499" }, instrument_type: InstrumentType::FUND, name: "交易所买卖基金", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "03500", end: "03599" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "03600", end: "03999" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "04000", end: "04199" }, instrument_type: InstrumentType::BOND, name: "外汇基金债券", desc: "香港金融管理局" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "04200", end: "04299" }, instrument_type: InstrumentType::BOND, name: "政府债券", desc: "香港特别行政区" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "04300", end: "04329" }, instrument_type: InstrumentType::BOND, name: "债券证券", desc: "仅售予专业投资者" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "04330", end: "04399" }, instrument_type: InstrumentType::OTHER, name: "NASDQA-AMEX实验计划", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "04400", end: "04599" }, instrument_type: InstrumentType::BOND, name: "债券证券", desc: "仅售予专业投资者" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "04600", end: "04699" }, instrument_type: InstrumentType::STOCK, name: "优先股", desc: "仅售予专业投资者" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "04700", end: "04799" }, instrument_type: InstrumentType::BOND, name: "债务证券", desc: "售予公众" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "04800", end: "04999" }, instrument_type: InstrumentType::WARRANT, name: "权证", desc: "SPAC" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "05000", end: "06029" }, instrument_type: InstrumentType::BOND, name: "债券证券", desc: "仅售予专业投资者" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "06030", end: "06199" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "06200", end: "06299" }, instrument_type: InstrumentType::OTHER, name: "香港预讬证券", desc: "香港預託證券" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "06300", end: "06399" }, instrument_type: InstrumentType::OTHER, name: "证券/预讬证券", desc: "被美国联邦证券法界定为受限制(RS)证券" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "06400", end: "06599" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "06600", end: "06749" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "06750", end: "06799" }, instrument_type: InstrumentType::BOND, name: "财政部债券", desc: "中华人民共和国" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "06800", end: "06999" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "07000", end: "07199" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "07200", end: "07399" }, instrument_type: InstrumentType::OTHER, name: "杠杆及反向产品", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "07400", end: "07499" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "07500", end: "07599" }, instrument_type: InstrumentType::OTHER, name: "杠杆及反向产品", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "07600", end: "07699" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "07700", end: "07799" }, instrument_type: InstrumentType::OTHER, name: "杠杆及反向产品", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "07800", end: "07999" }, instrument_type: InstrumentType::OTHER, name: "股份", desc: "SPAC" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "08000", end: "08550" }, instrument_type: InstrumentType::GEM_MARKET, name: "GEM证券", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "08551", end: "08600" }, instrument_type: InstrumentType::TEMPORARY_STOCK, name: "GEM临时柜台", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "08601", end: "08999" }, instrument_type: InstrumentType::GEM_MARKET, name: "GEM证券", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "09000", end: "09199" }, instrument_type: InstrumentType::FUND, name: "交易所买卖基金", desc: "美元" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "09200", end: "09399" }, instrument_type: InstrumentType::OTHER, name: "杠杆及反向产品", desc: "美元" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "09400", end: "09499" }, instrument_type: InstrumentType::FUND, name: "交易所买卖基金", desc: "美元" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "09500", end: "09599" }, instrument_type: InstrumentType::OTHER, name: "杠杆及反向产品", desc: "美元" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "09600", end: "09699" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "09700", end: "09799" }, instrument_type: InstrumentType::OTHER, name: "杠杆及反向产品", desc: "美元" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "09800", end: "09849" }, instrument_type: InstrumentType::FUND, name: "交易所买卖基金", desc: "美元" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "09850", end: "09999" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "" },

        // 10000-29999, 衍生权证
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "10000", end: "10899" }, instrument_type: InstrumentType::WARRANT, name: "衍生权证", desc: "相关资产在香港以外地区上市的衍生权证, 一篮子权证及非标准型权证" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "10900", end: "10999" }, instrument_type: InstrumentType::WARRANT, name: "衍生权证", desc: "相关资产在香港以外地区上市的衍生权证(以美元买卖)" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "11000", end: "11999" }, instrument_type: InstrumentType::WARRANT, name: "衍生权证", desc: "相关资产在香港以外地区上市的衍生权证, 一篮子权证及非标准型权证" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "12000", end: "29999" }, instrument_type: InstrumentType::WARRANT, name: "衍生权证", desc: "" },

        // 30000-39999, 供沪深股通使用
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "30000", end: "39999" }, instrument_type: InstrumentType::OTHER, name: "沪深股通", desc: "" },

        // 40000-40999, 仅售于专业投资者的债务证券
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "40000", end: "40999" }, instrument_type: InstrumentType::BOND, name: "债务证券", desc: "仅售于专业投资者" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "41000", end: "46999" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "47000", end: "48999" }, instrument_type: InstrumentType::OTHER, name: "界内证", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "49000", end: "49499" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },

        // 49500-69999, 牛熊证
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "49500", end: "49999" }, instrument_type: InstrumentType::OPTION, name: "牛熊证", desc: "相关资产在香港以外地区上市" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "50000", end: "69999" }, instrument_type: InstrumentType::OPTION, name: "牛熊证", desc: "" },

        // 70000-79999, 供沪深股通使用
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "70000", end: "79999" }, instrument_type: InstrumentType::OTHER, name: "沪深股通", desc: "" },

        // 80000-89999, 以人民币买卖的产品
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "80000", end: "82799" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "82800", end: "82849" }, instrument_type: InstrumentType::FUND, name: "交易所买卖基金", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "82850", end: "82899" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "82900", end: "82999" }, instrument_type: InstrumentType::TEMPORARY_STOCK, name: "主板临时柜台", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "83000", end: "83199" }, instrument_type: InstrumentType::FUND, name: "交易所买卖基金", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "83200", end: "83399" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "83400", end: "83499" }, instrument_type: InstrumentType::FUND, name: "交易所买卖基金", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "83500", end: "83599" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "83600", end: "83999" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "84000", end: "84299" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "84300", end: "84329" }, instrument_type: InstrumentType::BOND, name: "债券证券", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "84330", end: "84399" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "84400", end: "84599" }, instrument_type: InstrumentType::BOND, name: "债务证券", desc: "仅售于专业投资者" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "84600", end: "84699" }, instrument_type: InstrumentType::STOCK, name: "优先股", desc: "仅售于专业投资者" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "84700", end: "84999" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "85000", end: "85743" }, instrument_type: InstrumentType::BOND, name: "债务证券", desc: "仅售于专业投资者" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "85744", end: "85900" }, instrument_type: InstrumentType::BOND, name: "债务证券", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "85901", end: "86029" }, instrument_type: InstrumentType::BOND, name: "债务证券", desc: "仅售于专业投资者" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "86030", end: "86199" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "86200", end: "86299" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "86600", end: "86799" }, instrument_type: InstrumentType::OTHER, name: "中华人民共和国财政部债券/主板证券", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "86800", end: "86999" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "87000", end: "87099" }, instrument_type: InstrumentType::FUND, name: "房地产投资信托基金及交易所买卖基金以外的单位信托/互惠基金", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "87100", end: "87199" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "87200", end: "87399" }, instrument_type: InstrumentType::OTHER, name: "杠杆及反向产品", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "87400", end: "87499" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "87500", end: "87599" }, instrument_type: InstrumentType::OTHER, name: "杠杆及反向产品", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "87600", end: "87699" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "87700", end: "87799" }, instrument_type: InstrumentType::OTHER, name: "杠杆及反向产品", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "87800", end: "88999" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "89000", end: "89099" }, instrument_type: InstrumentType::BOND, name: "中华人民共和国财政部债券", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "89100", end: "89199" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "89200", end: "89599" }, instrument_type: InstrumentType::WARRANT, name: "衍生权证", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "89600", end: "89699" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "以人民币买卖" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "89700", end: "89849" }, instrument_type: InstrumentType::OTHER, name: "供日后使用", desc: "保留" },
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Range { start: "89850", end: "89999" }, instrument_type: InstrumentType::STOCK, name: "主板", desc: "以人民币买卖" },

        // 90000-99999, 供沪深股通使用
        CodeRule { exchange: Exchange::HKEX, prefix: RulePrefix::Str("9"), instrument_type: InstrumentType::OTHER, name: "沪深股通", desc: "" },
    ]
}
