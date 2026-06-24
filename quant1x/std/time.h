#pragma once
#ifndef QUANT1X_STD_TIME_H
#define QUANT1X_STD_TIME_H 1

#include <quant1x/std/base.h>

namespace api {

    // 获取时区偏移的毫秒数
    int64_t zone_offset_milliseconds();
    // 解析日期
    int64_t parse_date(const std::string &str);
    // 解析时间
    int64_t parse_time(const std::string &str);

    int64_t ms_utc_to_local(const int64_t &milliseconds);

    int64_t ms_local_to_utc(const int64_t &milliseconds);

    std::chrono::system_clock::time_point from_local(const int64_t &milliseconds);
    int64_t from_time_point(const std::chrono::system_clock::time_point& tp);
    // std::tm 转 time_point (假设tm为本地时间)
    std::chrono::system_clock::time_point tm_to_time_point(const std::tm &tm);

    /// 获取当前日期的字符串
    std::string today();
    /// 获取当前时间戳的字符串
    std::string get_timestamp();
    /// utc时间 格式化成 本地时间字符串
    std::string to_string(const std::chrono::system_clock::time_point &tp);

    // 获得当前季度的初始和结束日期, months为偏移的月数
    std::pair<std::string, std::string> GetQuarterDay(int months = 0);

    // 通过给定的日期 获得日期所在财报的季度, 初始以及结束日期
    //
    // diff 季度偏移数, 大于0前移diff个季度, 小于0后移diff个季度, 默认为当前季度
    std::tuple<std::string, std::string, std::string> GetQuarterByDate(const std::string& date, int diff = 0);
}


#endif // QUANT1X_STD_TIME_H
