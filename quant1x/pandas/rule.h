#pragma once
#ifndef QUANT1X_PANDAS_RULE_H
#define QUANT1X_PANDAS_RULE_H 1
#include <chrono>

namespace pandas {
    // 解析频率
    std::tuple<int, std::string> parse_frequency(const std::string& freq);

    // 解析频率字符串并返回对应的duration
    std::chrono::duration<long long, std::nano> parse_time_rule(const std::string& freq);
} // namespace pandas

#endif //QUANT1X_PANDAS_RULE_H