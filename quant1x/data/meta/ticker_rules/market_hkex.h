#pragma once
#ifndef QUANT1X_DATA_META_TICKER_RULES_MARKET_HKEX_H
#define QUANT1X_DATA_META_TICKER_RULES_MARKET_HKEX_H 1

#include "rule.h"

namespace meta {
namespace ticker_rules {

/// HKEX 香港交易所规则
/// 对应 Python 的 hkex_rules, Rust 的 hkex_rules()
inline std::vector<CodeRule> hkex_rules() {
    return {
        // 指数
        {Exchange::HKEX, RulePrefix("HSI"), InstrumentType::Index, "恒生指数", "香港交易所"},
        {Exchange::HKEX, RulePrefix("HSCEI"), InstrumentType::Index, "国企指数", "香港交易所"},
        {Exchange::HKEX, RulePrefix("HSCCI"), InstrumentType::Index, "红筹指数", "香港交易所"},
        {Exchange::HKEX, RulePrefix("HSTECH"), InstrumentType::Index, "恒生科技指数", "香港交易所"},

        // 00001-09999, 主板及GEM上市证券
        {Exchange::HKEX, RulePrefix("00001", "02799"), InstrumentType::Stock, "主板", ""},
        {Exchange::HKEX, RulePrefix("02800", "02849"), InstrumentType::Fund, "交易所买卖基金", ""},
        {Exchange::HKEX, RulePrefix("02850", "02899"), InstrumentType::Stock, "主板", ""},
        {Exchange::HKEX, RulePrefix("02900", "02999"), InstrumentType::TemporaryStock, "主板临时柜台", ""},
        {Exchange::HKEX, RulePrefix("03000", "03199"), InstrumentType::Fund, "交易所买卖基金", ""},
        {Exchange::HKEX, RulePrefix("03200", "03399"), InstrumentType::Stock, "主板", ""},
        {Exchange::HKEX, RulePrefix("03400", "03499"), InstrumentType::Fund, "交易所买卖基金", ""},
        {Exchange::HKEX, RulePrefix("03500", "03599"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("03600", "03999"), InstrumentType::Stock, "主板", ""},
        {Exchange::HKEX, RulePrefix("04000", "04199"), InstrumentType::Bond, "外汇基金债券", "香港金融管理局"},
        {Exchange::HKEX, RulePrefix("04200", "04299"), InstrumentType::Bond, "政府债券", "香港特别行政区"},
        {Exchange::HKEX, RulePrefix("04300", "04329"), InstrumentType::Bond, "债券证券", "仅售予专业投资者"},
        {Exchange::HKEX, RulePrefix("04330", "04399"), InstrumentType::Other, "NASDQA-AMEX实验计划", ""},
        {Exchange::HKEX, RulePrefix("04400", "04599"), InstrumentType::Bond, "债券证券", "仅售予专业投资者"},
        {Exchange::HKEX, RulePrefix("04600", "04699"), InstrumentType::Stock, "优先股", "仅售予专业投资者"},
        {Exchange::HKEX, RulePrefix("04700", "04799"), InstrumentType::Bond, "债务证券", "售予公众"},
        {Exchange::HKEX, RulePrefix("04800", "04999"), InstrumentType::Warrant, "权证", "SPAC"},
        {Exchange::HKEX, RulePrefix("05000", "06029"), InstrumentType::Bond, "债券证券", "仅售予专业投资者"},
        {Exchange::HKEX, RulePrefix("06030", "06199"), InstrumentType::Stock, "主板", ""},
        {Exchange::HKEX, RulePrefix("06200", "06299"), InstrumentType::Other, "香港预讬证券", "香港預託證券"},
        {Exchange::HKEX, RulePrefix("06300", "06399"), InstrumentType::Other, "证券/预讬证券", "被美国联邦证券法界定为受限制(RS)证券"},
        {Exchange::HKEX, RulePrefix("06400", "06599"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("06600", "06749"), InstrumentType::Stock, "主板", ""},
        {Exchange::HKEX, RulePrefix("06750", "06799"), InstrumentType::Bond, "财政部债券", "中华人民共和国"},
        {Exchange::HKEX, RulePrefix("06800", "06999"), InstrumentType::Stock, "主板", ""},
        {Exchange::HKEX, RulePrefix("07000", "07199"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("07200", "07399"), InstrumentType::Other, "杠杆及反向产品", ""},
        {Exchange::HKEX, RulePrefix("07400", "07499"), InstrumentType::Stock, "主板", ""},
        {Exchange::HKEX, RulePrefix("07500", "07599"), InstrumentType::Other, "杠杆及反向产品", ""},
        {Exchange::HKEX, RulePrefix("07600", "07699"), InstrumentType::Stock, "主板", ""},
        {Exchange::HKEX, RulePrefix("07700", "07799"), InstrumentType::Other, "杠杆及反向产品", ""},
        {Exchange::HKEX, RulePrefix("07800", "07999"), InstrumentType::Other, "股份", "SPAC"},
        {Exchange::HKEX, RulePrefix("08000", "08550"), InstrumentType::GemMarket, "GEM证券", ""},
        {Exchange::HKEX, RulePrefix("08551", "08600"), InstrumentType::TemporaryStock, "GEM临时柜台", ""},
        {Exchange::HKEX, RulePrefix("08601", "08999"), InstrumentType::GemMarket, "GEM证券", ""},
        {Exchange::HKEX, RulePrefix("09000", "09199"), InstrumentType::Fund, "交易所买卖基金", "美元"},
        {Exchange::HKEX, RulePrefix("09200", "09399"), InstrumentType::Other, "杠杆及反向产品", "美元"},
        {Exchange::HKEX, RulePrefix("09400", "09499"), InstrumentType::Fund, "交易所买卖基金", "美元"},
        {Exchange::HKEX, RulePrefix("09500", "09599"), InstrumentType::Other, "杠杆及反向产品", "美元"},
        {Exchange::HKEX, RulePrefix("09600", "09699"), InstrumentType::Stock, "主板", ""},
        {Exchange::HKEX, RulePrefix("09700", "09799"), InstrumentType::Other, "杠杆及反向产品", "美元"},
        {Exchange::HKEX, RulePrefix("09800", "09849"), InstrumentType::Fund, "交易所买卖基金", "美元"},
        {Exchange::HKEX, RulePrefix("09850", "09999"), InstrumentType::Stock, "主板", ""},

        // 10000-29999, 衍生权证
        {Exchange::HKEX, RulePrefix("10000", "10899"), InstrumentType::Warrant, "衍生权证", "相关资产在香港以外地区上市的衍生权证、一篮子权证及非标准型权证"},
        {Exchange::HKEX, RulePrefix("10900", "10999"), InstrumentType::Warrant, "衍生权证", "相关资产在香港以外地区上市的衍生权证(以美元买卖)"},
        {Exchange::HKEX, RulePrefix("11000", "11999"), InstrumentType::Warrant, "衍生权证", "相关资产在香港以外地区上市的衍生权证、一篮子权证及非标准型权证"},
        {Exchange::HKEX, RulePrefix("12000", "29999"), InstrumentType::Warrant, "衍生权证", ""},

        // 30000-39999, 供沪深股通使用
        {Exchange::HKEX, RulePrefix("30000", "39999"), InstrumentType::Other, "沪深股通", ""},

        // 40000-40999, 仅售于专业投资者的债务证券
        {Exchange::HKEX, RulePrefix("40000", "40999"), InstrumentType::Bond, "债务证券", "仅售于专业投资者"},
        {Exchange::HKEX, RulePrefix("41000", "46999"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("47000", "48999"), InstrumentType::Other, "界内证", "保留"},
        {Exchange::HKEX, RulePrefix("49000", "49499"), InstrumentType::Other, "供日后使用", "保留"},

        // 49500-69999, 牛熊证
        {Exchange::HKEX, RulePrefix("49500", "49999"), InstrumentType::Option, "牛熊证", "相关资产在香港以外地区上市"},
        {Exchange::HKEX, RulePrefix("50000", "69999"), InstrumentType::Option, "牛熊证", ""},

        // 70000-79999, 供沪深股通使用
        {Exchange::HKEX, RulePrefix("70000", "79999"), InstrumentType::Other, "沪深股通", ""},

        // 80000-89999, 以人民币买卖的产品
        {Exchange::HKEX, RulePrefix("80000", "82799"), InstrumentType::Stock, "主板", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("82800", "82849"), InstrumentType::Fund, "交易所买卖基金", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("82850", "82899"), InstrumentType::Stock, "主板", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("82900", "82999"), InstrumentType::TemporaryStock, "主板临时柜台", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("83000", "83199"), InstrumentType::Fund, "交易所买卖基金", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("83200", "83399"), InstrumentType::Stock, "主板", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("83400", "83499"), InstrumentType::Fund, "交易所买卖基金", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("83500", "83599"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("83600", "83999"), InstrumentType::Stock, "主板", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("84000", "84299"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("84300", "84329"), InstrumentType::Bond, "债券证券", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("84330", "84399"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("84400", "84599"), InstrumentType::Bond, "债务证券", "仅售于专业投资者"},
        {Exchange::HKEX, RulePrefix("84600", "84699"), InstrumentType::Stock, "优先股", "仅售于专业投资者"},
        {Exchange::HKEX, RulePrefix("84700", "84999"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("85000", "85743"), InstrumentType::Bond, "债务证券", "仅售于专业投资者"},
        {Exchange::HKEX, RulePrefix("85744", "85900"), InstrumentType::Bond, "债务证券", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("85901", "86029"), InstrumentType::Bond, "债务证券", "仅售于专业投资者"},
        {Exchange::HKEX, RulePrefix("86030", "86199"), InstrumentType::Stock, "主板", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("86200", "86299"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("86600", "86799"), InstrumentType::Other, "中华人民共和国财政部债券/主板证券", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("86800", "86999"), InstrumentType::Stock, "主板", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("87000", "87099"), InstrumentType::Fund, "房地产投资信托基金及交易所买卖基金以外的单位信托/互惠基金", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("87100", "87199"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("87200", "87399"), InstrumentType::Other, "杠杆及反向产品", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("87400", "87499"), InstrumentType::Stock, "主板", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("87500", "87599"), InstrumentType::Other, "杠杆及反向产品", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("87600", "87699"), InstrumentType::Stock, "主板", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("87700", "87799"), InstrumentType::Other, "杠杆及反向产品", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("87800", "88999"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("89000", "89099"), InstrumentType::Bond, "中华人民共和国财政部债券", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("89100", "89199"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("89200", "89599"), InstrumentType::Warrant, "衍生权证", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("89600", "89699"), InstrumentType::Stock, "主板", "以人民币买卖"},
        {Exchange::HKEX, RulePrefix("89700", "89849"), InstrumentType::Other, "供日后使用", "保留"},
        {Exchange::HKEX, RulePrefix("89850", "89999"), InstrumentType::Stock, "主板", "以人民币买卖"},

        // 90000-99999, 供沪深股通使用
        {Exchange::HKEX, RulePrefix("9"), InstrumentType::Other, "沪深股通", ""},
    };
}

} // namespace ticker_rules
} // namespace meta

#endif // QUANT1X_DATA_META_TICKER_RULES_MARKET_HKEX_H
