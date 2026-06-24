#pragma once
#ifndef QUANT1X_DATA_META_TICKER_RULES_MARKET_USA_H
#define QUANT1X_DATA_META_TICKER_RULES_MARKET_USA_H 1

#include "rule.h"
#include <unordered_map>

namespace quant1x::data::meta {
namespace ticker_rules {

/// USA 美国证券交易所规则
/// 对应 Python 的 usa_rules, Rust 的 usa_rules()
inline std::vector<CodeRule> usa_rules() {
    return {
        {Exchange::OFFSHORE, RulePrefix("IXIC"), InstrumentType::Index, "指数", "纳斯达克指数"},
        {Exchange::OFFSHORE, RulePrefix("DAX"), InstrumentType::Index, "指数", "德国DAX指数"},
        {Exchange::EXTENDED, RulePrefix("US"), InstrumentType::Sector, "指数", "美国板块指数"},
        {Exchange::USA, RulePrefix(""), InstrumentType::Stock, "挂牌公司普通股", ""},
    };
}

/// 美股 ticker -> 行情协议代码映射表
inline std::unordered_map<std::string, std::string> usa_ticker_code_map() {
    return {
        {"IXIC", "A_IXIC"}, // 纳斯达克指数
        {"DAX", "B_DAX"},   // 德国DAX指数
    };
}

/// 将美国股票代码转换为行情标准的代码
/// 对应 Python 的 usa_ticker_to_code
inline std::string usa_ticker_to_code(const std::string& ticker) {
    // 转大写
    std::string upper;
    for (char c : ticker) {
        upper += static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    }
    auto map = usa_ticker_code_map();
    auto it = map.find(upper);
    if (it != map.end()) {
        return it->second;
    }
    return upper;
}

/// 将美国股票协议代码转换为对应的股票代码
/// 对应 Python 的 usa_code_to_ticker
inline std::string usa_code_to_ticker(const std::string& code) {
    auto map = usa_ticker_code_map();
    for (const auto& entry : map) {
        if (entry.second == code) {
            return entry.first;
        }
    }
    return "";
}

} // namespace ticker_rules
} // namespace meta

#endif // QUANT1X_DATA_META_TICKER_RULES_MARKET_USA_H
