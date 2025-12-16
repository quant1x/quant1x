// C++ port of exchange/code_rule.go (rules + detection)
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <regex>
#include <tuple>
#include <quant1x/exchange/code.h>
#include <quant1x/std/strings.h>

namespace exchange {

// Rule tables (copied from Go code_rule.go)
struct CodeRule {
    const char* prefix;
    SecurityType type;
    const char* desc;
};

static const std::vector<CodeRule> globalRules = {
    {"880", SecurityType::Block, "板块指数(通达信)"},
    {"881", SecurityType::Block, "板块指数(通达信)"},
};

static const std::vector<CodeRule> sseRules = {
    {"000", SecurityType::Index, "上证指数"},
    {"51", SecurityType::ETF, "上交所ETF(510-519)"},
    {"588", SecurityType::ETF, "科创板ETF"},
    {"50", SecurityType::Fund, "LOF/封闭式基金"},
    {"52", SecurityType::Fund, "其他基金"},
    {"600", SecurityType::Stock, "主板A股"},
    {"601", SecurityType::Stock, "主板A股"},
    {"603", SecurityType::Stock, "主板A股"},
    {"605", SecurityType::Stock, "主板A股"},
    {"688", SecurityType::Stock, "科创板"},
    {"689", SecurityType::Stock, "科创板CDR"},
    {"900", SecurityType::BStock, "B股"},
    {"110", SecurityType::Bond, "债券"},
    {"113", SecurityType::Bond, "可转债"},
    {"118", SecurityType::Bond, "可交换债"},
    {"120", SecurityType::Bond, "公司债"},
    {"123", SecurityType::Bond, "可转债"},
    {"127", SecurityType::Bond, "可转债"},
    {"128", SecurityType::Bond, "可转债"},
    {"730", SecurityType::IPO, "新股申购"},
    {"780", SecurityType::IPO, "新股申购"},
};

static const std::vector<CodeRule> szseRules = {
    {"399", SecurityType::Index, "深证指数"},
    {"159", SecurityType::ETF, "深交所ETF"},
    {"150", SecurityType::Fund, "LOF"},
    {"160", SecurityType::Fund, "LOF"},
    {"161", SecurityType::Fund, "LOF"},
    {"162", SecurityType::Fund, "LOF"},
    {"163", SecurityType::Fund, "LOF"},
    {"164", SecurityType::Fund, "LOF"},
    {"167", SecurityType::Fund, "LOF"},
    {"168", SecurityType::Fund, "LOF"},
    {"169", SecurityType::Fund, "LOF"},
    {"184", SecurityType::Fund, "封闭式基金"},
    {"000", SecurityType::Stock, "主板A股"},
    {"001", SecurityType::Stock, "主板A股"},
    {"002", SecurityType::Stock, "主板A股"},
    {"003", SecurityType::Stock, "主板A股"},
    {"300", SecurityType::Stock, "创业板"},
    {"301", SecurityType::Stock, "创业板"},
    {"200", SecurityType::BStock, "B股"},
    {"110", SecurityType::Bond, "可转债"},
    {"111", SecurityType::Bond, "可转债"},
    {"118", SecurityType::Bond, "可交换债"},
    {"123", SecurityType::Bond, "可转债"},
    {"127", SecurityType::Bond, "可转债"},
    {"128", SecurityType::Bond, "可转债"},
};

static const std::vector<CodeRule> bjseRules = {
    {"899", SecurityType::Index, "北交所指数"},
    {"920", SecurityType::Stock, "北交所股票(2024年起新上市)"},
    {"83", SecurityType::Stock, "北交所股票(原精选层)"},
    {"87", SecurityType::Stock, "北交所股票(原精选层)"},
    {"88", SecurityType::Stock, "北交所股票(2022-2023年上市)"},
    {"82", SecurityType::Bond, "优先股"},
    {"89", SecurityType::Bond, "可转债"},
};

static const std::vector<CodeRule> hkseRules = {
    {"HSI", SecurityType::Index, "恒生指数"},
    {"HSCEI", SecurityType::Index, "国企指数"},
    {"HSCCI", SecurityType::Index, "红筹指数"},
    {"028", SecurityType::ETF, "ETF"},
    {"030", SecurityType::ETF, "ETF"},
    {"031", SecurityType::ETF, "ETF"},
    {"090", SecurityType::ETF, "ETF"},
    {"091", SecurityType::ETF, "ETF"},
    {"08", SecurityType::Stock, "港股(GEM)"},
    {"0", SecurityType::Stock, "港股"},
    {"1", SecurityType::Bond, "权证"},
    {"2", SecurityType::Bond, "权证"},
    {"4", SecurityType::Bond, "牛熊证"},
    {"5", SecurityType::Bond, "牛熊证"},
    {"6", SecurityType::Bond, "牛熊证"},
};

// matchRule: match longest prefix
static std::pair<SecurityType, std::string> matchRule(const std::string& code, const std::vector<CodeRule>& rules) {
    size_t best_len = 0;
    SecurityType matched = SecurityType::Unknown;
    std::string desc;
    for (const auto &r : rules) {
        const std::string pref(r.prefix);
        if (code.rfind(pref, 0) == 0) { // starts_with
            if (pref.size() > best_len) {
                best_len = pref.size();
                matched = r.type;
                desc = r.desc;
            }
        }
    }
    if (best_len > 0) return {matched, desc};
    return {SecurityType::Unknown, std::string()};
}


SecurityCode detect(const std::string &input) {
    // normalize inline: trim and tolower using strings utilities (match Go's Detect: trim then tolower)
    std::string s = strings::trim(input);
    if (s.empty()) return {ExchangeId::ShangHai, "", SecurityType::Unknown};
    s = strings::to_lower(s);

    // follow Go variable names: pureCode, symbol, exchangeCode, exchangeId, typ
    std::string pureCode = s;
    if (pureCode.empty()) return {ExchangeId::ShangHai, "", SecurityType::Unknown};

    std::string symbol = "";                    // 纯代码部分
    ExchangeCode exchangeCode = ExchangeUnknown; // 默认未知市场
    ExchangeId exchangeId = ExchangeId::Unknown; // 默认未知市场
    SecurityType typ = SecurityType::Unknown;   // 默认未知类型

    // 1. try explicit market flag (prefix or suffix)
    // Use strings utilities to check prefix/suffix exactly like Go's std.StartsWith / std.EndsWith
    if (strings::startsWith(pureCode, AllExchangeCodes)) {
        // prefix form: sh600000, hk00700, usappl
        symbol = pureCode.substr(2);
        exchangeCode = ExchangeCode(pureCode.substr(0, 2));
        exchangeId = exchangeCode.Id();
    } else if (strings::endsWith(pureCode, AllExchangeCodes) && pureCode.size() >= 3 && pureCode[pureCode.size()-3] == '.') {
        // suffix form: 600000.sh, 00700.hk, APPL.us
        size_t suffixLen = 3; // include dot
        symbol = pureCode.substr(0, pureCode.size() - suffixLen);
        exchangeCode = ExchangeCode(pureCode.substr(pureCode.size() - 2));
        exchangeId = exchangeCode.Id();
    }

    // 2. infer market if not set
    if (exchangeId == ExchangeId::Unknown) {
        if (std::regex_match(pureCode, std::regex("^\\d{6}$"))) {
            symbol = pureCode;
            if (pureCode.rfind("6", 0) == 0 || pureCode.rfind("5", 0) == 0 || pureCode.rfind("9", 0) == 0 || pureCode.rfind("7", 0) == 0 || pureCode.rfind("000", 0) == 0) {
                exchangeCode = ExchangeSSE;
                exchangeId = ExchangeId::ShangHai;
            } else if (pureCode.rfind("0", 0) == 0 || pureCode.rfind("3", 0) == 0 || pureCode.rfind("1", 0) == 0 || pureCode.rfind("2", 0) == 0) {
                exchangeCode = ExchangeSZSE;
                exchangeId = ExchangeId::ShenZhen;
            } else if (pureCode.rfind("8", 0) == 0 || pureCode.rfind("92", 0) == 0) {
                exchangeCode = ExchangeBJSE;
                exchangeId = ExchangeId::BeiJing;
            } else {
                return {ExchangeId::Unknown, "", SecurityType::Unknown};
            }
        } else if (std::regex_match(pureCode, std::regex("^\\d{5}$"))) {
            symbol = pureCode;
            exchangeCode = ExchangeHK;
            exchangeId = ExchangeId::HongKong;
        } else {
            symbol = pureCode;
        }
    } else if (symbol.empty()) {
        symbol = pureCode;
    }

    // 3. (no numeric-only validation here) -- follow Go: do not reject non-digit symbols here

    // Handle 4-letter all-alpha as US stock (pure form)
    if (exchangeId == ExchangeId::Unknown) {
        if (symbol.size() == 4 && std::all_of(symbol.begin(), symbol.end(), [](char c){ return std::isalpha(static_cast<unsigned char>(c)); })) {
            exchangeCode = ExchangeUS;
            exchangeId = ExchangeId::USA;
            typ = SecurityType::Stock;
        }
    }

    // 4. global rules priority
    if (std::regex_match(symbol, std::regex("^\\d{6}$"))) {
        if (auto [typ_, desc] = matchRule(symbol, globalRules); typ_ != SecurityType::Unknown) {
            // global rules belong to SSE
            ExchangeId exId = ExchangeId::ShangHai;
            try {
                exId = ExchangeCode(ExchangeSSE.String()).Id();
            } catch (...) { exId = ExchangeId::ShangHai; }
            return {exId, symbol, typ_};
        }
    }

    // 5. match market rules
    if (exchangeId == ExchangeId::Unknown) return {ExchangeId::Unknown, "", SecurityType::Unknown};

    if (typ == SecurityType::Unknown) {
        const std::vector<CodeRule>* rules = nullptr;
        switch (exchangeId) {
        case ExchangeId::ShangHai: rules = &sseRules; break;
        case ExchangeId::ShenZhen: rules = &szseRules; break;
        case ExchangeId::BeiJing: rules = &bjseRules; break;
        case ExchangeId::HongKong: rules = &hkseRules; break;
        case ExchangeId::USA: return {exchangeId, symbol, SecurityType::Stock};
        default: return {ExchangeId::Unknown, "", SecurityType::Unknown};
        }
        if (auto [typ_, desc] = matchRule(symbol, *rules); typ_ != SecurityType::Unknown) {
            return {exchangeId, symbol, typ_};
        } else {
            return {ExchangeId::Unknown, "", SecurityType::Unknown};
        }
    } else {
        return {exchangeId, symbol, typ};
    }
}

} // namespace exchange
