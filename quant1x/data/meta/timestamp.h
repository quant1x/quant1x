#pragma once
#ifndef QUANT1X_DATA_META_TIMESTAMP_H
#define QUANT1X_DATA_META_TIMESTAMP_H 1

#include <cstdint>
#include <string>
#include <chrono>
#include <stdexcept>
#include <vector>

namespace meta {

/// 时间常量
constexpr int64_t SECONDS_PER_MINUTE = 60;
constexpr int64_t SECONDS_PER_HOUR = 60 * SECONDS_PER_MINUTE;
constexpr int64_t SECONDS_PER_DAY = 24 * SECONDS_PER_HOUR;
constexpr int64_t MILLISECONDS_PER_SECOND = 1000;
constexpr int64_t MILLISECONDS_PER_MINUTE = SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr int64_t MILLISECONDS_PER_HOUR = SECONDS_PER_HOUR * MILLISECONDS_PER_SECOND;
constexpr int64_t MILLISECONDS_PER_DAY = SECONDS_PER_DAY * MILLISECONDS_PER_SECOND;

/// 盘前时间配置
constexpr int PRE_MARKET_HOUR   = 9;
constexpr int PRE_MARKET_MINUTE = 0;
constexpr int PRE_MARKET_SECOND = 0;

/// 本地时间戳，单位毫秒
/// 与 Python/Go/Rust 的 Timestamp 对齐
class Timestamp {
public:
    int64_t ms = 0;

    Timestamp() : ms(0) {}
    explicit Timestamp(int64_t ms_val) : ms(ms_val) {}
    explicit Timestamp(const std::string& s);

    int64_t value() const { return ms; }

    /// 静态工厂方法
    static Timestamp now();
    static Timestamp zero() { return Timestamp(0); }
    static Timestamp parse(const std::string& s);
    static Timestamp parse_time(const std::string& s);
    static Timestamp from_date(int year, int month, int day,
                               int hour = 0, int minute = 0,
                               int second = 0, int millisecond = 0);

    /// 盘前时间戳
    Timestamp pre_market_time() const;
    static Timestamp pre_market_time(int year, int month, int day);

    /// 格式化
    std::string only_date() const;           ///< "YYYY-MM-DD"
    std::string only_time() const;           ///< "HH:MM:SS"
    std::string cache_date() const;          ///< "YYYYMMDD"
    int64_t yyyymmdd_int() const;              ///< YYYYMMDD 整数
    static Timestamp from_yyyymmdd_int(int64_t date);
    std::string to_string(const char* layout = "%Y-%m-%d %H:%M:%S") const;

    /// 比较
    bool is_same_date(const Timestamp& other) const;
    bool empty() const { return ms == 0; }

    /// 偏移
    Timestamp offset(int hour, int minute, int second, int millisecond) const;
    Timestamp start_of_day() const;
    Timestamp today_at(int hour, int minute, int second, int millisecond) const;

    // 比较运算符
    bool operator<(const Timestamp& other) const { return ms < other.ms; }
    bool operator<=(const Timestamp& other) const { return ms <= other.ms; }
    bool operator>(const Timestamp& other) const { return ms > other.ms; }
    bool operator>=(const Timestamp& other) const { return ms >= other.ms; }
    bool operator==(const Timestamp& other) const { return ms == other.ms; }
    bool operator!=(const Timestamp& other) const { return ms != other.ms; }

    /// 提取年月日
    void extract(int& year, int& month, int& day) const;
    void extract_time(int& hour, int& minute, int& second, int& millisecond) const;
};

/// A股市场首个上市日 (上海证券交易所开业日)
constexpr const char* MARKET_CN_FIRST_LISTTIME = "1990-12-19";

/// 最近一个交易日 (简化实现，待接入真实日历)
inline Timestamp last_trading_day(const Timestamp& date = Timestamp::now()) {
    return date.start_of_day();
}

/// 下一个交易日 (简化实现)
inline Timestamp next_trading_day(const Timestamp& date = Timestamp::now()) {
    return date.start_of_day().offset(0, 0, 0, MILLISECONDS_PER_DAY);
}

/// 上一个交易日 (简化实现)
inline Timestamp prev_trading_day(const Timestamp& date = Timestamp::now()) {
    return date.start_of_day().offset(0, 0, 0, -MILLISECONDS_PER_DAY);
}

/// 检查时间戳是否为交易日
inline bool check_trading_timestamp(const Timestamp& ts) {
    (void)ts;
    return true; // 简化实现，待接入真实日历
}

/// 生成交易日期范围
inline std::vector<Timestamp> date_range(const Timestamp& start, const Timestamp& end) {
    std::vector<Timestamp> dates;
    auto current = start.start_of_day();
    auto last = end.start_of_day();
    while (current <= last) {
        dates.push_back(current);
        current = current.offset(0, 0, 0, MILLISECONDS_PER_DAY);
    }
    return dates;
}

/// 两个日期之间的交易日数
inline int trading_days_between(const Timestamp& start, const Timestamp& end) {
    return static_cast<int>(date_range(start, end).size());
}

} // namespace meta

#endif // QUANT1X_DATA_META_TIMESTAMP_H
