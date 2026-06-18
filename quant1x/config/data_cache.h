#pragma once
#ifndef QUANT1X_CONFIG_DATA_CACHE_H
#define QUANT1X_CONFIG_DATA_CACHE_H 1

#include <map>
#include <memory>
#include <ostream>
#include <variant>

namespace quant1x::config {
    using ConfigValue = std::variant<std::nullptr_t, // 对应 YAML 的 null
        bool, // 布尔类型
        int64_t, // 整数(推荐 int64_t, 兼容 long 和 int)
        double, // 浮点数
        std::string // 字符串
    >;

    // 分钟级K线的配置
    struct MinuteKLineConfig {
        std::string frequency = "1min"; // yaml文件的key, 默认1分钟k线
        int minutes = 1; // 分钟数, 默认分钟数是1
        bool enabled = false; // 是否生效, 默认不生效
    };

    // 缓存配置
    struct CacheParameter {
        std::map<std::string, bool> kline{}; // K线配置

        friend std::ostream &operator<<(std::ostream &os, const CacheParameter &obj) {
            os << "kline: {";
            bool first = true;
            for (const auto &[key, value]: obj.kline) {
                if (!first) {
                    os << ", ";
                }
                os << "\"" << key << "\": " << (value ? "true" : "false");
                first = false;
            }
            os << "}";
            return os;
        }
    };

    // 数据配置
    struct DataParameter {
        CacheParameter cache{};
        std::map<std::string, int> concurrency; // 并发参数

        friend std::ostream &operator<<(std::ostream &os, const DataParameter &obj) {
            os << "cache: " << obj.cache << ", concurrency: {";
            bool first = true;
            for (const auto &[key, value]: obj.concurrency) {
                if (!first) {
                    os << ", ";
                }
                os << "\"" << key << "\": " << value;
                first = false;
            }
            os << "}";
            return os;
        }
    };
} // namespace quant1x::config
#endif  // QUANT1X_CONFIG_DATA_CACHE_H
