#pragma once
#ifndef QUANT1X_CONIFG_CACHE_H
#define QUANT1X_CONIFG_CACHE_H 1

#include <quant1x/std/api.h>

namespace quant1x::config {

    // 获取交易日历的缓存文件名
    std::string get_calendar_filename();

    // 获取证券列表的缓存文件名
    std::string get_security_filename();

    // 获取板块列表的缓存文件名${~/.quant1x/meta/blocks.${YYYY-mm-dd} // namespace quant1x::config} // namespace quant1x::config
    std::string get_sector_filename(const std::string &date);

    // 历史成交记录
    // 目录结构${trans} // namespace quant1x::config/${YYYY} // namespace quant1x::config/${YYYYMMDD} // namespace quant1x::config/${SecurityCode} // namespace quant1x::config.csv
    std::string get_historical_trade_filename(const std::string &code, const std::string &cache_date);

    // 筹码分布
    // 目录结构${trans} // namespace quant1x::config/${YYYY} // namespace quant1x::config/${YYYYMMDD} // namespace quant1x::config/${SecurityCode} // namespace quant1x::config.cd
    std::string get_chip_distribution_filename(const std::string &code, const std::string &cache_date);

    // 板块数据文件路径
    std::string get_block_path();

    // 除权除息文件路径
    std::string get_xdxr_path();

    // 日K线文件路径
    std::string get_day_path();

    // 日K线文件路径
    std::string get_kline_path(const std::string &freq = "day");

    // 除权除息文件名
    std::string get_xdxr_filename(const std::string &code);

    /**
     * @brief 日K线文件名
     * @param code 证券代码
     * @param forward 是否前复权, 后复权不考虑
     * @return 前复权返回文件名后缀是csv, 不复权是raw
     */
    std::string get_kline_filename(const std::string &code, bool forward = true);

    // 通用K线文件名
    std::string get_kline_filename_ex(const std::string &code, const std::string &freq);

    // 分时数据文件名
    std::string get_minute_filename(const std::string &code, const std::string &cache_date);
    
    // top10_holders_filename 前十大流通股股东缓存文件名
    std::string top10_holders_filename(const std::string &code, const std::string &date);

    // 财报报告数据文件名
    std::string reports_filename(const std::string &date);

    // 获取qmt缓存路径
    std::string get_qmt_cache_path();
} // namespace quant1x::config
#endif // QUANT1X_CONIFG_CACHE_H