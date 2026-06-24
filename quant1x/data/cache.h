#pragma once
#ifndef QUANT1X_DATA_CACHE_H
#define QUANT1X_DATA_CACHE_H 1

#include <quant1x/data/meta/timestamp.h>
#include <string>

namespace quant1x::data {

    /**
     * @brief 获取今天初始化时间, 即当前时间的盘前时间戳
     *
     * 对应 Python cache.get_today_initialized_time
     * @return meta::Timestamp 盘前初始化时间戳
     */
    meta::Timestamp get_today_initialized_time();

    /**
     * @brief 获取文件最后修改时间
     *
     * 对应 Python cache.get_filename_modified_time
     * - 文件不存在 → Timestamp::zero()
     * - OS 错误(权限, 竞争条件等) → Timestamp::zero()
     *
     * @param fname 文件路径
     * @return meta::Timestamp 文件最后修改时间, 失败返回零值
     */
    meta::Timestamp get_filename_modified_time(const std::string &fname);

    /**
     * @brief 增量更新缓存清理的最大天数
     *
     * 对应 Python MaxCachedDaysToDropOnIncrementalUpdate
     * 该机制确保在 A 股除权除息日等场景下, 当日数据能被正确覆盖.
     * 由于 A 股的复权处理以交易日为单位, 且同一天内可能多次更新数据,
     * 因此需先删除缓存中已有的当日记录, 再插入最新增量数据.
     */
    constexpr int MaxCachedDaysToDropOnIncrementalUpdate = 1;

    /**
     * @brief 根据周期标识返回中文名称
     *
     * 对应 Python cache.get_period_name
     * @param period 周期标识 ('D', 'W', 'M', 'Q', 'Y')
     * @return std::string 中文周期名称
     */
    std::string get_period_name(const std::string &period);

    /**
     * @brief 默认K线周期
     */
    constexpr const char *const default_bar_period = "D";

    /**
     * @brief 日期格式化, 自动识别常见日期格式并输出为目标格式
     *
     * 对应 Python cache.date_format
     * 支持的输入格式: YYYY-MM-DD, YYYY/MM/DD, YYYY.MM.DD, YYYYMMDD
     * @param date 输入日期字符串
     * @param layout 输出格式, 默认 "%Y-%m-%d"
     * @return std::string 格式化后的日期字符串, 解析失败时返回原字符串
     */
    std::string date_format(const std::string &date, const std::string &layout = "%Y-%m-%d");

}  // namespace quant1x::data

#endif  // QUANT1X_DATA_CACHE_H
