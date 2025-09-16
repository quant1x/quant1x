#include <quant1x/pandas/rule.h>
#include <quant1x/std/strings.h>

#include <vector>
#include <string>
#include <chrono>
#include <stdexcept>
#include <cctype>

namespace pandas {

    std::tuple<int, std::string> parse_frequency(const std::string& freq) {
        const auto frequency = strings::trim(freq);
        const auto d = ParseTimeRule(frequency);
        const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(d);
        return std::tuple<int, std::string>(minutes.count(), frequency);
    }

    // 解析频率字符串并返回对应的duration
    // frequencies, from pandas.tseries.frequencies import to_offset
    std::chrono::duration<long long, std::nano> ParseTimeRule(const std::string& freq) {
        const std::string frequency = strings::trim(freq);

        if (frequency.empty()) {
            throw std::runtime_error("empty freq string");
        }

        // 解析数字部分
        size_t i = 0;
        while (i < frequency.size() && std::isdigit(frequency[i])) {
            i++;
        }

        int n;
        if (i == 0) {
            n = 1; // 默认倍数为1
        } else {
            try {
                n = std::stoi(frequency.substr(0, i));
            } catch (const std::exception& e) {
                throw std::runtime_error("invalid number in freq: " + std::string(e.what()));
            }
        }

        const std::string unit = frequency.substr(i);
        if (unit.empty()) {
            throw std::runtime_error("missing unit in freq");
        }

        // 映射单位到duration
        if (unit == "N" || unit == "ns") {
            return std::chrono::nanoseconds(n);
        }
        if (unit == "U" || unit == "us" || unit == "µs") {
            return std::chrono::microseconds(n);
        }
        if (unit == "L" || unit == "ms") {
            return std::chrono::milliseconds(n);
        }
        if (unit == "S" || unit == "s") {
            return std::chrono::seconds(n);
        }
        if (unit == "T" || unit == "min") {
            return std::chrono::minutes(n);
        }
        if (unit == "H" || unit == "h") {
            return std::chrono::hours(n);
        }
        if (unit == "D" || unit == "d") {
            return std::chrono::hours(24 * n);
        }
        throw std::runtime_error("unsupported freq unit: " + unit);
    }

    enum class BinAlignment { Left, Right, Center };

    // 生成时间序列
    std::vector<std::chrono::system_clock::time_point> DateRange(
        const std::chrono::system_clock::time_point& start,
        const int periods,
        const std::string& freqStr)
    {
        const auto dur = ParseTimeRule(freqStr);
        
        std::vector<std::chrono::system_clock::time_point> result;
        std::chrono::system_clock::time_point t = start;
        for (int i = 0; i < periods; i++) {
            result.push_back(t);
            t += std::chrono::duration_cast<std::chrono::system_clock::duration>(dur);
        }
        return result;
    }
} // namespace pandas