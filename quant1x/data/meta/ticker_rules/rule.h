#pragma once
#ifndef QUANT1X_DATA_META_TICKER_RULES_RULE_H
#define QUANT1X_DATA_META_TICKER_RULES_RULE_H 1

#include "../exchange.h"
#include "../instrument.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

namespace meta {
namespace ticker_rules {

/// 规则前缀: 可以是字符串前缀或数字范围
/// 对应 Python 的 str | NumberRange, Rust 的 RulePrefix 枚举
struct RulePrefix {
    enum Type { Str, Range };
    Type type;
    std::string prefix_str;   // 当 type == Str 时使用
    std::string range_start;  // 当 type == Range 时使用
    std::string range_end;    // 当 type == Range 时使用

    // 构造函数: 字符串前缀
    RulePrefix(const char* s) : type(Str), prefix_str(s) {}
    RulePrefix(const std::string& s) : type(Str), prefix_str(s) {}

    // 构造函数: 数字范围
    RulePrefix(const char* start, const char* end)
        : type(Range), range_start(start), range_end(end) {}
    RulePrefix(const std::string& start, const std::string& end)
        : type(Range), range_start(start), range_end(end) {}

    /// 检查代码是否匹配此前缀
    bool matches(const std::string& code) const {
        if (type == Str) {
            if (prefix_str.empty()) {
                return true; // 空前缀匹配一切(如美股默认规则)
            }
            return code.size() >= prefix_str.size() &&
                   code.compare(0, prefix_str.size(), prefix_str) == 0;
        } else {
            // 对于数字范围, 按字符串比较(代码可能是前导零的数字字符串)
            return code >= range_start && code <= range_end;
        }
    }

    /// 返回前缀长度(用于最佳匹配排序)
    size_t match_length() const {
        if (type == Str) {
            return prefix_str.size();
        } else {
            return range_start.size();
        }
    }

    /// 返回范围的最大可能长度
    size_t max_value_length() const {
        if (type == Str) {
            return prefix_str.size();
        } else {
            return std::max(range_start.size(), range_end.size());
        }
    }
};

/// 证券代码规则
/// 对应 Python/Rust 的 CodeRule
struct CodeRule {
    Exchange       exchange;
    RulePrefix     prefix;
    InstrumentType instrument_type;
    std::string    name;
    std::string    desc;
};

/// 根据代码前缀匹配最优规则
/// 对应 Python 的 match_rule 函数, Rust 的 match_rule
inline CodeRule match_rule(const std::string& code, const std::vector<CodeRule>& rules) {
    // 转大写
    std::string upper;
    for (char c : code) {
        upper += static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    }
    // trim
    size_t start = upper.find_first_not_of(" \t\n\r");
    size_t end   = upper.find_last_not_of(" \t\n\r");
    std::string trimmed = (start == std::string::npos) ? "" : upper.substr(start, end - start + 1);

    const CodeRule* best_match = nullptr;
    size_t best_len = 0;

    for (const auto& entry : rules) {
        const RulePrefix& prefix = entry.prefix;
        if (prefix.matches(trimmed)) {
            size_t len = prefix.match_length();
            if (len > best_len) {
                best_len = len;
                best_match = &entry;
            } else if (best_len == 0 && len == 0) {
                // 空前缀在无其他匹配时使用
                best_match = &entry;
                break;
            }
        }
    }

    if (best_match != nullptr) {
        return *best_match;
    } else {
        return CodeRule{
            Exchange::UNKNOWN,
            RulePrefix(""),
            InstrumentType::Unknown,
            "",
            "未匹配到规则"
        };
    }
}

/// 全局规则(跨市场优先)
/// 对应 Python 的 global_rules, Rust 的 global_rules()
inline std::vector<CodeRule> global_rules() {
    return {
        {Exchange::SSE, RulePrefix("880"), InstrumentType::Sector, "板块指数", "通达信"},
        {Exchange::SSE, RulePrefix("881"), InstrumentType::Sector, "板块指数", "通达信"},
    };
}

} // namespace ticker_rules
} // namespace meta

#endif // QUANT1X_DATA_META_TICKER_RULES_RULE_H
