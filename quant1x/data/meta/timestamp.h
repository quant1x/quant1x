#pragma once
#ifndef QUANT1X_DATA_META_TIMESTAMP_H
#define QUANT1X_DATA_META_TIMESTAMP_H 1

#include <chrono>
#include <ctime>
#include <iomanip>
#include <optional>
#include <ostream>
#include <sstream>
#include <tuple>

namespace meta {

    constexpr const int64_t seconds_per_minute      = 60;
    constexpr const int64_t seconds_per_hour        = 60 * seconds_per_minute;
    constexpr const int64_t seconds_per_day         = 24 * seconds_per_hour;
    constexpr const int64_t milliseconds_per_second = 1000;
    constexpr const int64_t milliseconds_per_minute = seconds_per_minute * milliseconds_per_second;
    constexpr const int64_t milliseconds_per_hour   = seconds_per_hour * milliseconds_per_second;
    constexpr const int64_t milliseconds_per_day    = seconds_per_day * milliseconds_per_second;

    /**
     * @brief 本地时间戳, 单位毫秒
     */
    class Timestamp {
    private:
        int64_t ms_;

        /**
         * @brief 本地当前时间
         * @return
         */
        static int64_t current();

    public:
        Timestamp();
        Timestamp(int64_t t);
        Timestamp(const std::chrono::system_clock::time_point &tp);
        // 默认接受日期时间
        Timestamp(const std::string &str);

        /**
         * @brief 通过本地时间构造时间戳
         * @param y 年
         * @param m 月
         * @param d 日
         * @param hh 时
         * @param mm 分
         * @param ss 秒
         * @param sss 毫秒
         */
        Timestamp(int y, int m, int d, int hh = 0, int mm = 0, int ss = 0, int sss = 0);

        /**
         * @brief 获取毫秒数
         * @return
         */
        int64_t value() const;

        /**
         * @brief 盘前初始化时间戳, 用年月日构造一个时间戳, 默认时间是9点整
         * @param year 年
         * @param month 月
         * @param day 日
         * @return
         */
        static Timestamp pre_market_time(int year, int month, int day);

        /**
         * @brief 当前时间戳
         * @return
         */
        static Timestamp now();

        // 零值
        static Timestamp zero();

        // 解析日期时间
        static Timestamp parse(const std::string &str);

        // 仅解析时间
        static Timestamp parse_time(const std::string &str);

        // 获取当前时间戳对应的当天零点（00:00:00.000）的时间戳 truncate
        Timestamp start_of_day() const;

        // 当天零点整
        static Timestamp midnight();

        // 今天几点几分几秒几毫秒
        Timestamp today(int hour = 0, int minute = 0, int second = 0, int millisecond = 0) const;
        // 今天自0点整开始的几点几分几秒几毫秒
        Timestamp since(int hour = 0, int minute = 0, int second = 0, int millisecond = 0) const;
        // 当前时间戳偏移几小时几分几秒几毫秒
        Timestamp offset(int hour = 0, int minute = 0, int second = 0, int millisecond = 0) const;

        // 将当前时间戳truncate到早盘时间
        Timestamp pre_market_time() const;

        // 调整时间戳到0秒0毫秒（用于 begin）
        Timestamp floor() const;

        // 调整时间戳到59秒999毫秒（用于 end）
        Timestamp ceil() const;

        // 提取年月日
        std::tuple<int, int, int> extract() const;
        // 以毫秒为单位格式化时间戳
        std::string to_string(const std::string &layout = "{:%Y-%m-%d %H:%M:%S}") const;
        // 截断毫秒数, 以秒为单位格式化时间, 默认输出时分秒
        std::string format_time_to_second(const std::string &layout = "{:%H:%M:%S}") const;
        // 返回日期
        std::string only_date() const;
        std::string cache_date() const;
        // 返回时间
        std::string only_time() const;
        // 返回整型日期
        uint32_t yyyymmdd_u32() const;
        // 是否为空, 即零值
        bool empty() const;

        friend std::ostream &operator<<(std::ostream &os, const Timestamp &ts);
        // 赋值操作符
        Timestamp &operator=(int64_t val) {
            ms_ = val;
            return *this;
        }
        // 类型转换操作符
        operator int64_t() const { return value(); }

        /**
         * @brief 检查两个时间戳是否代表同一天(本地时间), 基于本地时间比较，不考虑时区差异
         * @param other 要比较的另一个时间戳
         * @return 如果在同一天返回true，否则返回false
         */
        bool is_same_date(const Timestamp &other) const noexcept;

        bool operator==(const Timestamp &rhs) const;
        bool operator!=(const Timestamp &rhs) const;
        bool operator<(const Timestamp &rhs) const;
        bool operator>(const Timestamp &rhs) const;
        bool operator<=(const Timestamp &rhs) const;
        bool operator>=(const Timestamp &rhs) const;
    };
}  // namespace meta

#endif  // QUANT1X_DATA_META_TIMESTAMP_H
