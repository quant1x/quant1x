#include <quant1x/pandas/rule.h>
#include <quant1x/std/strings.h>

#include <vector>
#include <string>
#include <chrono>
#include <stdexcept>
#include <cctype>

namespace pandas {
    // 解析频率字符串并返回对应的duration
    // frequencies, from pandas.tseries.frequencies import to_offset
    std::chrono::duration<long long, std::nano> ParseTimeRule(const std::string& freq) {
        std::string trimmed = strings::trim(freq);

        if (trimmed.empty()) {
            throw std::runtime_error("empty freq string");
        }

        // 解析数字部分
        size_t i = 0;
        while (i < trimmed.size() && std::isdigit(trimmed[i])) {
            i++;
        }

        int n;
        if (i == 0) {
            n = 1; // 默认倍数为1
        } else {
            try {
                n = std::stoi(trimmed.substr(0, i));
            } catch (const std::exception& e) {
                throw std::runtime_error("invalid number in freq: " + std::string(e.what()));
            }
        }

        std::string unit = trimmed.substr(i);
        if (unit.empty()) {
            throw std::runtime_error("missing unit in freq");
        }

        // 映射单位到duration
        if (unit == "N" || unit == "ns") {
            return std::chrono::nanoseconds(n);
        } else if (unit == "U" || unit == "us" || unit == "µs") {
            return std::chrono::microseconds(n);
        } else if (unit == "L" || unit == "ms") {
            return std::chrono::milliseconds(n);
        } else if (unit == "S") {
            return std::chrono::seconds(n);
        } else if (unit == "T" || unit == "min") {
            return std::chrono::minutes(n);
        } else if (unit == "H") {
            return std::chrono::hours(n);
        } else if (unit == "D") {
            return std::chrono::hours(24 * n);
        } else {
            throw std::runtime_error("unsupported freq unit: " + unit);
        }
    }

    enum class BinAlignment { Left, Right, Center };

    // 生成时间序列
    std::vector<std::chrono::system_clock::time_point> DateRange(
        const std::chrono::system_clock::time_point& start,
        int periods,
        const std::string& freqStr)
    {
        auto dur = ParseTimeRule(freqStr);

        std::vector<std::chrono::system_clock::time_point> result;
        std::chrono::system_clock::time_point t = start;
        for (int i = 0; i < periods; i++) {
            result.push_back(t);
            t += dur;
        }
        return result;
    }
} // namespace pandas