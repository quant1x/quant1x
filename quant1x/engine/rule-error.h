#pragma once
#ifndef QUANT1X_RULE_ERROR_H
#define QUANT1X_RULE_ERROR_H 1

#include <string>

namespace engine {

    enum class RuleError {
        OK = 0,
        RULE_F10_BASE = 1000,
        RULE_BASE_BASE = 2000,

        // 示例：基础规则错误码
        INVALID_PRICE = RULE_BASE_BASE + 1,
        INVALID_VOLUME,
        UNKNOWN_RULE_KIND,
    };

    inline std::string to_string(RuleError err) {
        switch (err) {
            case RuleError::OK: return "OK";
            case RuleError::INVALID_PRICE: return "价格无效";
            case RuleError::INVALID_VOLUME: return "成交量无效";
            case RuleError::UNKNOWN_RULE_KIND: return "未知规则类型";
            default: return "未知错误";
        }
    }

} // namespace engine

#endif //QUANT1X_RULE_ERROR_H
