#pragma once
#ifndef QUANT1X_DATA_META_TICKER_RULES_MARKET_BSE_H
#define QUANT1X_DATA_META_TICKER_RULES_MARKET_BSE_H 1

#include "rule.h"

namespace meta {
namespace ticker_rules {

/// BSE 北京证券交易所规则
/// 对应 Python 的 bse_rules, Rust 的 bse_rules()
inline std::vector<CodeRule> bse_rules() {
    return {
        // 指数
        {Exchange::BSE, RulePrefix("899"), InstrumentType::Index, "指数", "证券指数首三位代码为899"},

        // 股票
        {Exchange::BSE, RulePrefix("920"), InstrumentType::Stock, "北交所新上市", "2024-04-22 起新上市使用920号段; 已上市公司继续沿用原代码直到统一切换"},
        {Exchange::BSE, RulePrefix("92"), InstrumentType::Stock, "上市公司普通股", "首两位92: 上市公司普通股票; 920号段自2024-04-22起用于新上市公司"},
        {Exchange::BSE, RulePrefix("400"), InstrumentType::Stock, "两网/退市A股", "两网公司及退市公司A股首三位代码为400"},
        {Exchange::BSE, RulePrefix("420"), InstrumentType::BStock, "退市B股", "退市公司B股首三位代码为420"},

        // 债券/优先股
        {Exchange::BSE, RulePrefix("810"), InstrumentType::Bond, "可转换公司债", "向特定对象发行的可转换公司债券首三位代码为810"},
        {Exchange::BSE, RulePrefix("81"), InstrumentType::Bond, "优先股(极少)", "其他极少数代码"},
        {Exchange::BSE, RulePrefix("820"), InstrumentType::Bond, "优先股", "优先股票首三位代码为820"},
        {Exchange::BSE, RulePrefix("821"), InstrumentType::Bond, "优先股", "优先股票首三位代码为820"},
        {Exchange::BSE, RulePrefix("82"), InstrumentType::Bond, "优先股(极少)", "其他极少数代码"},
        {Exchange::BSE, RulePrefix("83"), InstrumentType::Stock, "挂牌公司普通股", "挂牌公司普通股票首两位为83"},
        {Exchange::BSE, RulePrefix("840"), InstrumentType::Other, "要约收购", "要约收购证券代码首三位代码为84"},
        {Exchange::BSE, RulePrefix("841"), InstrumentType::Other, "要约回购", "要约回购证券代码首三位代码为841"},
        {Exchange::BSE, RulePrefix("87"), InstrumentType::Stock, "挂牌公司普通股", "挂牌公司普通股票首两位为87"},
        {Exchange::BSE, RulePrefix("88"), InstrumentType::Stock, "挂牌公司普通股", "挂牌公司普通股票首两位为88"},
        {Exchange::BSE, RulePrefix("850"), InstrumentType::Option, "股权激励期权", "股权激励期权首三位代码为850"},
    };
}

} // namespace ticker_rules
} // namespace meta

#endif // QUANT1X_DATA_META_TICKER_RULES_MARKET_BSE_H
