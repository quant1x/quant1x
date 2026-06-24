#pragma once
#ifndef QUANT1X_DATA_META_TICKER_RULES_MARKET_SZSE_H
#define QUANT1X_DATA_META_TICKER_RULES_MARKET_SZSE_H 1

#include "rule.h"

namespace quant1x::data::meta {
namespace ticker_rules {

/// SZSE 深圳证券交易所规则
/// 对应 Python 的 szse_rules, Rust 的 szse_rules()
inline std::vector<CodeRule> szse_rules() {
    return {
        // 指数
        {Exchange::SZSE, RulePrefix("395"), InstrumentType::Index, "成交量统计指数", ""},
        {Exchange::SZSE, RulePrefix("399"), InstrumentType::Index, "深证指数", ""},

        // 主板A股
        {Exchange::SZSE, RulePrefix("000"), InstrumentType::Stock, "主板A股", ""},
        {Exchange::SZSE, RulePrefix("001"), InstrumentType::Stock, "主板A股", ""},
        {Exchange::SZSE, RulePrefix("002"), InstrumentType::Stock, "主板A股", ""},
        {Exchange::SZSE, RulePrefix("003"), InstrumentType::Stock, "主板A股", ""},

        // 权证
        {Exchange::SZSE, RulePrefix("030"), InstrumentType::Warrant, "权证", ""},
        {Exchange::SZSE, RulePrefix("031"), InstrumentType::Warrant, "权证", ""},
        {Exchange::SZSE, RulePrefix("032"), InstrumentType::Warrant, "权证", ""},
        {Exchange::SZSE, RulePrefix("036"), InstrumentType::Warrant, "创业板股权激励计划涉及的员工认股权", ""},
        {Exchange::SZSE, RulePrefix("0370"), InstrumentType::Warrant, "主板A股股权激励计划涉及的员工认股权", ""},
        {Exchange::SZSE, RulePrefix("0371"), InstrumentType::Warrant, "主板A股股权激励计划涉及的员工认股权", ""},
        {Exchange::SZSE, RulePrefix("0372"), InstrumentType::Warrant, "创业板股权激励计划审计的员工认股权", ""},
        {Exchange::SZSE, RulePrefix("0373"), InstrumentType::Warrant, "主板A股股权激励计划涉及的员工认股权", ""},
        {Exchange::SZSE, RulePrefix("0374"), InstrumentType::Warrant, "主板A股股权激励计划涉及的员工认股权", ""},
        {Exchange::SZSE, RulePrefix("0375"), InstrumentType::Warrant, "中小企业板股权激励计划涉及的员工认股权", ""},
        {Exchange::SZSE, RulePrefix("0376"), InstrumentType::Warrant, "中小企业板股权激励计划涉及的员工认股权", ""},
        {Exchange::SZSE, RulePrefix("0377"), InstrumentType::Warrant, "中小企业板股权激励计划涉及的员工认股权", ""},
        {Exchange::SZSE, RulePrefix("0378"), InstrumentType::Warrant, "中小企业板股权激励计划涉及的员工认股权", ""},
        {Exchange::SZSE, RulePrefix("0379"), InstrumentType::Warrant, "中小企业板股权激励计划涉及的员工认股权", ""},
        {Exchange::SZSE, RulePrefix("038"), InstrumentType::Warrant, "主板A股及中小企业股票认沽权证", ""},
        {Exchange::SZSE, RulePrefix("039"), InstrumentType::Warrant, "主板A股及中小企业股票认沽权证", ""},
        {Exchange::SZSE, RulePrefix("070"), InstrumentType::Warrant, "主板A股增发/可转债申购", ""},
        {Exchange::SZSE, RulePrefix("071"), InstrumentType::Warrant, "主板A股增发/可转债申购", ""},
        {Exchange::SZSE, RulePrefix("072"), InstrumentType::Warrant, "中小企业板增发/可转债申购", ""},
        {Exchange::SZSE, RulePrefix("073"), InstrumentType::Warrant, "中小企业板增发/可转债申购", ""},
        {Exchange::SZSE, RulePrefix("074"), InstrumentType::Warrant, "中小企业板增发/可转债申购", ""},
        {Exchange::SZSE, RulePrefix("080"), InstrumentType::Warrant, "A股配股", ""},
        {Exchange::SZSE, RulePrefix("0"), InstrumentType::Stock, "股票", ""},

        // 债券
        {Exchange::SZSE, RulePrefix("10"), InstrumentType::Bond, "国债", ""},
        {Exchange::SZSE, RulePrefix("11"), InstrumentType::Bond, "企业债", ""},
        {Exchange::SZSE, RulePrefix("120"), InstrumentType::Bond, "企业债券", ""},
        {Exchange::SZSE, RulePrefix("123"), InstrumentType::Bond, "可转债", ""},
        {Exchange::SZSE, RulePrefix("127"), InstrumentType::Bond, "可转债", ""},
        {Exchange::SZSE, RulePrefix("128"), InstrumentType::Bond, "可转债", ""},
        {Exchange::SZSE, RulePrefix("13"), InstrumentType::Bond, "债券回购", ""},

        // 基金
        {Exchange::SZSE, RulePrefix("159"), InstrumentType::ETF, "深交所ETF", ""},
        {Exchange::SZSE, RulePrefix("15"), InstrumentType::Fund, "ETF", ""},
        {Exchange::SZSE, RulePrefix("16"), InstrumentType::Fund, "LOF", ""},
        {Exchange::SZSE, RulePrefix("17"), InstrumentType::Fund, "传统投资基金", ""},
        {Exchange::SZSE, RulePrefix("184"), InstrumentType::Fund, "封闭式基金", ""},
        {Exchange::SZSE, RulePrefix("18"), InstrumentType::Fund, "封闭式基金", ""},
        {Exchange::SZSE, RulePrefix("1"), InstrumentType::Bond, "债券", ""},

        // B股
        {Exchange::SZSE, RulePrefix("200"), InstrumentType::BStock, "B股", ""},
        {Exchange::SZSE, RulePrefix("238"), InstrumentType::Other, "B股现金选择权", ""},
        {Exchange::SZSE, RulePrefix("28"), InstrumentType::Other, "B股配股优先权", ""},
        {Exchange::SZSE, RulePrefix("2"), InstrumentType::BStock, "B股", ""},

        // 创业板
        {Exchange::SZSE, RulePrefix("300"), InstrumentType::Stock, "创业板", ""},
        {Exchange::SZSE, RulePrefix("301"), InstrumentType::Stock, "创业板注册制", ""},
        {Exchange::SZSE, RulePrefix("30"), InstrumentType::Stock, "创业板", ""},
        {Exchange::SZSE, RulePrefix("36"), InstrumentType::Other, "投票", ""},
        {Exchange::SZSE, RulePrefix("37"), InstrumentType::Other, "增发/可转债申购", ""},
        {Exchange::SZSE, RulePrefix("38"), InstrumentType::Other, "配股/可转债优先权", ""},

        // 资产支持证券ABS
        {Exchange::SZSE, RulePrefix("50"), InstrumentType::Bond, "资产支持证券ABS", ""},
        {Exchange::SZSE, RulePrefix("56"), InstrumentType::Bond, "资产支持证券ABS", ""},
        {Exchange::SZSE, RulePrefix("5"), InstrumentType::Bond, "资产支持证券ABS", ""},

        // 其他
        {Exchange::SZSE, RulePrefix("700"), InstrumentType::Warrant, "B股增发", ""},
        {Exchange::SZSE, RulePrefix("730"), InstrumentType::Warrant, "跨市场申购", ""},
    };
}

} // namespace ticker_rules
} // namespace meta

#endif // QUANT1X_DATA_META_TICKER_RULES_MARKET_SZSE_H
