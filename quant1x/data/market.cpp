// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// market — 市场/证券代码识别与纠正, 与 Python data/market.py 对齐

#include "market.h"
#include "meta/ticker_rules/market_sse.h"
#include "meta/ticker_rules/market_szse.h"
#include "meta/ticker_rules/market_bse.h"
#include "meta/ticker_rules/market_hkex.h"
#include "meta/ticker_rules/market_usa.h"
#include <cctype>
#include <algorithm>

namespace data {

using meta::Exchange;
using meta::Instrument;
using meta::InstrumentType;
using meta::ticker_rules::match_rule;
using meta::ticker_rules::global_rules;

// ============================================================
// 常量: 前缀交易所标识集合
// ============================================================
static std::unordered_set<std::string> make_prefix_identifiers() {
    return {
        meta::exchange_identifier(Exchange::SSE),
        meta::exchange_identifier(Exchange::SZSE),
        meta::exchange_identifier(Exchange::BSE),
        meta::exchange_identifier(Exchange::HKEX),
    };
}

static std::unordered_set<std::string> make_all_identifiers() {
    return {
        meta::exchange_identifier(Exchange::SSE),
        meta::exchange_identifier(Exchange::SZSE),
        meta::exchange_identifier(Exchange::BSE),
        meta::exchange_identifier(Exchange::HKEX),
        meta::exchange_identifier(Exchange::HKFE),
        meta::exchange_identifier(Exchange::USA),
    };
}

static const std::unordered_set<std::string>& prefix_identifiers() {
    static std::unordered_set<std::string> s = make_prefix_identifiers();
    return s;
}

static const std::unordered_set<std::string>& all_identifiers() {
    static std::unordered_set<std::string> s = make_all_identifiers();
    return s;
}

// 辅助: 字符串转小写
static std::string to_lower(const std::string& s) {
    std::string r;
    for (char c : s) {
        r += static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    }
    return r;
}

// 辅助: 判断字符串是否全为ASCII字母
static bool is_all_alpha(const std::string& s) {
    for (char c : s) {
        if (!::isalpha(static_cast<unsigned char>(c))) return false;
    }
    return !s.empty();
}

// 辅助: 判断字符串是否全为ASCII数字
static bool is_all_digit(const std::string& s) {
    for (char c : s) {
        if (!::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return !s.empty();
}

// ============================================================
// detect_instrument_type_by_rule
// ============================================================
InstrumentType detect_instrument_type_by_rule(Exchange exchange, const std::string& code) {
    std::vector<meta::ticker_rules::CodeRule> rules;
    switch (exchange) {
        case Exchange::SSE:  rules = meta::ticker_rules::sse_rules(); break;
        case Exchange::SZSE: rules = meta::ticker_rules::szse_rules(); break;
        case Exchange::BSE:  rules = meta::ticker_rules::bse_rules(); break;
        case Exchange::HKEX: rules = meta::ticker_rules::hkex_rules(); break;
        case Exchange::USA:  rules = meta::ticker_rules::usa_rules(); break;
        default: return InstrumentType::Unknown;
    }
    auto cr = match_rule(code, rules);
    return cr.instrument_type;
}

// ============================================================
// detect_symbol
// ============================================================
Instrument detect_symbol(const std::string& input_str) {
    std::string s = input_str;
    // trim
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end   = s.find_last_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return Instrument{}; // Unknown
    }
    s = s.substr(start, end - start + 1);
    s = to_lower(s);

    std::string pure_code = s;
    std::string ticker;
    Exchange exchange = Exchange::UNKNOWN;
    InstrumentType typ = InstrumentType::Unknown;

    // 1. 判断前缀: sh600000
    if (pure_code.size() >= 2) {
        std::string prefix = pure_code.substr(0, 2);
        if (prefix_identifiers().count(prefix)) {
            ticker = pure_code.substr(2);
            try {
                exchange = meta::exchange_from_abbr(prefix);
            } catch (...) {
                exchange = Exchange::UNKNOWN;
            }
        }
    }

    // 2. 判断后缀: 600000.sh or AAPL.us
    if (exchange == Exchange::UNKNOWN && pure_code.size() >= 3) {
        size_t dot_pos = pure_code.size() - 3;
        if (pure_code[dot_pos] == '.') {
            std::string suffix = pure_code.substr(dot_pos + 1);
            if (all_identifiers().count(suffix)) {
                ticker = pure_code.substr(0, dot_pos);
                try {
                    exchange = meta::exchange_from_abbr(suffix);
                } catch (...) {
                    exchange = Exchange::UNKNOWN;
                }
            }
        }
    }

    // 3. 纯数字或者字母(无显式前缀/后缀)
    if (exchange == Exchange::UNKNOWN) {
        size_t code_len = pure_code.size();
        switch (code_len) {
            case 4:
                if (is_all_alpha(pure_code)) {
                    exchange = Exchange::USA;
                    typ = InstrumentType::Stock;
                    return Instrument{exchange, typ, pure_code, "", 100, 2, 0, 0, "", ""};
                }
                break;
            case 5:
                if (is_all_digit(pure_code)) {
                    exchange = Exchange::HKEX;
                    typ = InstrumentType::Stock;
                    return Instrument{exchange, typ, pure_code, "", 100, 2, 0, 0, "", ""};
                }
                break;
            case 6: {
                // 3.1 全局规则优先匹配
                auto cr = match_rule(pure_code, global_rules());
                if (cr.exchange != Exchange::UNKNOWN) {
                    return Instrument{cr.exchange, cr.instrument_type, pure_code, "", 100, 2, 0, 0, "", ""};
                }

                // 3.2 按市场匹配规则
                // 3.2.1 0, 159和3开头, 优先匹配深交所
                if (pure_code[0] == '0' || pure_code.substr(0, 3) == "159" || pure_code[0] == '3') {
                    cr = match_rule(pure_code, meta::ticker_rules::szse_rules());
                    if (cr.exchange != Exchange::UNKNOWN) {
                        return Instrument{cr.exchange, cr.instrument_type, pure_code, "", 100, 2, 0, 0, "", ""};
                    }
                }
                // 3.2.2 6和5开头, 优先匹配上交所
                if (pure_code[0] == '6' || pure_code[0] == '5') {
                    cr = match_rule(pure_code, meta::ticker_rules::sse_rules());
                    if (cr.exchange != Exchange::UNKNOWN) {
                        return Instrument{cr.exchange, cr.instrument_type, pure_code, "", 100, 2, 0, 0, "", ""};
                    }
                }
                // 3.2.3 匹配上交所
                cr = match_rule(pure_code, meta::ticker_rules::sse_rules());
                if (cr.exchange != Exchange::UNKNOWN) {
                    return Instrument{cr.exchange, cr.instrument_type, pure_code, "", 100, 2, 0, 0, "", ""};
                }
                // 3.2.4 匹配深交所
                cr = match_rule(pure_code, meta::ticker_rules::szse_rules());
                if (cr.exchange != Exchange::UNKNOWN) {
                    return Instrument{cr.exchange, cr.instrument_type, pure_code, "", 100, 2, 0, 0, "", ""};
                }
                // 3.2.5 匹配北交所
                cr = match_rule(pure_code, meta::ticker_rules::bse_rules());
                if (cr.exchange != Exchange::UNKNOWN) {
                    return Instrument{cr.exchange, cr.instrument_type, pure_code, "", 100, 2, 0, 0, "", ""};
                }
                break;
            }
            default:
                return Instrument{}; // Unknown
        }
    }

    // 4. 如果exchange是UNKNOWN, 则返回未知
    if (exchange == Exchange::UNKNOWN) {
        return Instrument{};
    }

    // 5. 如果typ是Unknown, 按市场规则匹配
    if (typ == InstrumentType::Unknown) {
        std::vector<meta::ticker_rules::CodeRule> rules;
        switch (exchange) {
            case Exchange::SSE:  rules = meta::ticker_rules::sse_rules(); break;
            case Exchange::SZSE: rules = meta::ticker_rules::szse_rules(); break;
            case Exchange::BSE:  rules = meta::ticker_rules::bse_rules(); break;
            case Exchange::HKEX: rules = meta::ticker_rules::hkex_rules(); break;
            case Exchange::USA:  rules = meta::ticker_rules::usa_rules(); break;
            default: return Instrument{};
        }

        auto cr = match_rule(ticker, rules);
        if (cr.instrument_type != InstrumentType::Unknown) {
            return Instrument{cr.exchange, cr.instrument_type, ticker, "", 100, 2, 0, 0, "", ""};
        } else {
            return Instrument{};
        }
    } else {
        return Instrument{exchange, typ, ticker, "", 100, 2, 0, 0, "", ""};
    }
}

// ============================================================
// correct_security_code
// ============================================================
std::string correct_security_code(const std::string& code) {
    if (code.empty()) {
        return "";
    }

    auto inst = detect_symbol(code);
    if (inst.can_construct_symbol()) {
        return inst.symbol();
    }

    // 回退: 简单的 strip + detect 逻辑
    std::string s = to_lower(code);
    // trim
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end   = s.find_last_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    s = s.substr(start, end - start + 1);

    static const char* market_flags[] = {"sh", "sz", "bj", "hk", "us"};

    // 前缀形式
    for (const char* flag : market_flags) {
        std::string f(flag);
        if (s.size() > f.size() && s.substr(0, f.size()) == f) {
            return f + s.substr(f.size());
        }
    }

    // 后缀形式
    for (const char* flag : market_flags) {
        std::string suffix = std::string(".") + flag;
        if (s.size() > suffix.size() && s.substr(s.size() - suffix.size()) == suffix) {
            return std::string(flag) + s.substr(0, s.size() - suffix.size());
        }
    }

    // 纯数字代码推断
    if (s.size() == 6 && is_all_digit(s)) {
        if (s[0] == '6' || s[0] == '5' || s[0] == '9') {
            return "sh" + s;
        } else {
            return "sz" + s;
        }
    }

    return code;
}

} // namespace data
