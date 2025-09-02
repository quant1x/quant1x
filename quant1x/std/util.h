#pragma once
#ifndef QUANT1X_STD_UTIL_H
#define QUANT1X_STD_UTIL_H 1

#include <quant1x/std/base.h>
#include <thread>

namespace util {

    /// 私有静态变量
    namespace {
        struct duration_unit {
            const char *name;
            double threshold; // 单位阈值（秒）
            double divisor;   // 转换除数
        };

        static const duration_unit units[] = {
            {"h",   3600, 3600},      // 小时
            {"min", 60,   60},        // 分钟
            {"s",   1,    1},         // 秒
            {"ms",  1e-3, 1e-3},     // 毫秒
            {"us",  1e-6, 1e-6},     // 微秒
            {"ns",  1e-9, 1e-9}      // 纳秒
        };
    }

    /// 核心单位推测函数
    template<typename Rep, typename Period>
    std::pair<double, std::string> auto_unit_infer(const std::chrono::duration<Rep, Period> &duration) {
        auto sec = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
        for (const auto &unit: units) {
            if (sec >= unit.threshold) {
                return {sec / unit.divisor, unit.name};
            }
        }
        // 默认返回纳秒（处理小于1ns的情况）
        return {sec / 1e-9, "ns"};
    }

    /// 智能格式化函数
    template<typename Rep, typename Period>
    std::string format_duration_auto(const std::chrono::duration<Rep, Period> &duration,
                                     const char *format = "%.3f%s") {
        auto [value, unit] = auto_unit_infer(duration);
        char buf[128] = {0x00};
        std::snprintf(buf, sizeof(buf), format, value, unit.c_str());
        return buf;
    }

    std::string homedir();
    std::string expand_homedir(const std::string &filename);
    std::error_code mkdirs(const std::string &path, bool notExistToCreate = true);
    std::error_code check_filepath(const std::string &filename, bool notExistToCreate = false);

    std::chrono::system_clock::time_point tm_to_time_point(const std::tm& tm);

    uint64_t get_thread_id(const std::thread::id &tid);
}

#endif //QUANT1X_STD_UTIL_H
