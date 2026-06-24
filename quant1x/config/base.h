#pragma once
#ifndef QUANT1X_CONFIG_BASE_H
#define QUANT1X_CONFIG_BASE_H 1

#include <quant1x/std/api.h>
#include <quant1x/config/data_cache.h>
#include <quant1x/config/trader_parameter.h>
#include <mutex>

// 全部的配置信息
namespace quant1x::config {
    constexpr const int cn_pre_market_hour   = 9;  ///< 盘前9点
    constexpr const int cn_pre_market_minute = 0;  ///< 盘点9点0分
    constexpr const int cn_pre_market_second = 0;  ///< 盘点9点0分0秒
    // 每天9点整
    inline std::string GLOBAL_CRON_EXPR_DAILY_INIT = std::format("{} {} {} * * *", cn_pre_market_second, cn_pre_market_minute, cn_pre_market_hour);
    
    constexpr const char *const cache_filename_date_layout = "{:%Y%m%d}";  ///< 缓存路径的日期格式

    constexpr const int64_t TenThousand = 10000;              // 万
    constexpr const int64_t Million     = 100 * TenThousand;  // 百万
    constexpr const int64_t Billion     = 100 * Million;      // 1亿

    struct BaseConfig {
        std::string          filename;
        std::string          homeDir;
        std::string          cacheDir;
        std::string          logsDir;
        bool                 running_in_debug = false;
        DataParameter        data{};
        friend std::ostream &operator<<(std::ostream &os, const BaseConfig &obj) {
            return os << "filename: " << obj.filename << " homeDir: " << obj.homeDir << " cacheDir: " << obj.cacheDir
                      << " logsDir: " << obj.logsDir << " running_in_debug: " << obj.running_in_debug
                      << " data: " << obj.data;
        }
    };

    extern std::once_flag global_cache_once;
    // extern BaseConfig global_quant1x_config;

    BaseConfig &global_config();

    // 配置文件路径
    std::string config_filename();
    // 获取交易配置参数
    std::shared_ptr<TraderParameter> TraderConfig();
    // 是否调试模式
    bool is_debug() noexcept;

    // 获取用户主路径
    std::string default_home_path();

    // 获取默认缓存路径
    std::string default_cache_path();

    // 获取元数据路径
    std::string get_meta_path();

    // 获取日志路径
    std::string get_logs_path();                                                              
}  // namespace quant1x::config

#endif // QUANT1X_CONFIG_BASE_H