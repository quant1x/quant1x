#include <quant1x/base/time.h>

#include <chrono>
#if __cpp_lib_chrono < 201907L
#define CXX_CHRONO_ZONE_USE_DATE 1
#else
#define CXX_CHRONO_ZONE_USE_DATE 0
static_assert(__cpp_lib_chrono >= 201907L, "C++20 <chrono> features not supported!");
#endif

#if CXX_CHRONO_ZONE_USE_DATE
#include <date/date.h>
#include <date/tz.h>
#include <fmt/chrono.h>  // fmt 对 chrono 的支持
#include <fmt/core.h>
#include <fmt/format.h>
#endif

#include <ctime>
#include <stdexcept>
#include <iostream>
#include <quant1x/base/except.h>
#include <quant1x/base/strings.h>
#include <quant1x/base/safe.h>

#if CXX_CHRONO_ZONE_USE_DATE
namespace fmt {
    template <>
    struct formatter<date::zoned_time<std::chrono::milliseconds>> {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const date::zoned_time<std::chrono::milliseconds>& zt, FormatContext& ctx) const
            -> decltype(ctx.out())
        {
            return format_to(ctx.out(), "{}", zt.get_sys_time());
        }
    };
}
#endif

namespace api {

    int64_t zone_offset_milliseconds() {
        std::time_t now = std::time(nullptr);
        std::tm     local = safe::localtime(now);
        std::tm utc = safe::gmtime(now);
        return (mktime(&local) - mktime(&utc)) * 1000LL;
    }

    // 解析日期
    int64_t parse_date(const std::string &str) {
        std::string str_datetime = strings::trim(str);
        if (str_datetime.empty()) {
            // 空字符串返回0,
            return 0;
        }
        std::chrono::sys_time<std::chrono::milliseconds> tp;
        std::istringstream iss(str_datetime);
        static const auto date_time_layout_supports = {
            "%Y-%m-%d %H:%M:%S",         // 2023-05-15 14:30:00
            "%Y-%m-%d",                  // 2023-05-15
            "%Y%m%d",                    // 20230515
            "%Y/%m/%d %H:%M:%S",         // 2023/05/15 14:30:00
            "%m/%d/%Y %H:%M:%S",         // 05/15/2023 14:30:00
            "%H:%M:%S %d-%m-%Y",         // 14:30:00 15-05-2023
            "%Y%m%d %H%M%S",             // 20230515 143000
            "%Y-%m-%dT%H:%M:%SZ",        // ISO 8601 UTC
            "%Y-%m-%dT%H:%M:%S%z",       // ISO 8601 with timezone
            "%a, %d %b %Y %H:%M:%S %Z",  // RFC 1123
            "%b %d %Y %H:%M:%S"          // May 15 2023 14:30:00
        };

        // 尝试多种格式
        for (const auto& fmt : date_time_layout_supports) {
            iss.clear();
            iss.seekg(0);
#if CXX_CHRONO_ZONE_USE_DATE
            date::from_stream(iss, fmt, tp);
#else
            iss >> std::chrono::parse(fmt, tp);
#endif

            if (!iss.fail()) {
                return tp.time_since_epoch().count();
            }
        }
        throw std::runtime_error("Failed to parse datetime string(" + str + ")");
    }

    // 解析时间
    int64_t parse_time(const std::string &str) {
        std::string str_time = strings::trim(str);
        if (str_time.empty()) {
            // 空字符串返回0,
            return 0;
        }
        std::istringstream iss(str_time);
        // 尝试多种格式
        static const auto only_time_layout_supports = {
            "%H:%M:%S",                  // 14:30:00
            "%Y-%m-%d %H:%M:%S",         // 2023-05-15 14:30:00
            "%Y-%m-%d",                  // 2023-05-15
            "%Y%m%d",                    // 20230515
            "%Y/%m/%d %H:%M:%S",         // 2023/05/15 14:30:00
            "%m/%d/%Y %H:%M:%S",         // 05/15/2023 14:30:00
            "%H:%M:%S %d-%m-%Y",         // 14:30:00 15-05-2023
            "%H%M%S",                    // 143000
            "%Y%m%d %H%M%S",             // 20230515 143000
            "%Y-%m-%dT%H:%M:%SZ",        // ISO 8601 UTC
            "%Y-%m-%dT%H:%M:%S%z",       // ISO 8601 with timezone
            "%a, %d %b %Y %H:%M:%S %Z",  // RFC 1123
            "%b %d %Y %H:%M:%S"          // May 15 2023 14:30:00
        };
        for (const auto& fmt : only_time_layout_supports) {
            iss.clear();
            iss.seekg(0);
            std::chrono::milliseconds parsedTime{};
#if CXX_CHRONO_ZONE_USE_DATE
            date::from_stream(iss, fmt, parsedTime);
#else
            std::chrono::from_stream(iss, fmt, parsedTime);
#endif
            if (!iss.fail()) {
                return parsedTime.count();
            }
        }
        throw std::runtime_error("Failed to parse only time string(" + str + ")");
    }

    // 本地时区: 上海
#if CXX_CHRONO_ZONE_USE_DATE
    static const auto _local_zone = date::locate_zone("Asia/Shanghai");
#else
    static const auto _local_zone = std::chrono::locate_zone("Asia/Shanghai");
#endif
    static const auto _utc_now = std::chrono::system_clock::now();
    static const auto _zone_offset_seconds = _local_zone->get_info(_utc_now).offset;
    static const auto _zone_offset_milliseconds = std::chrono::milliseconds(_zone_offset_seconds).count();
    constexpr const char * const default_chrono_format = "{:%Y-%m-%d %H:%M:%S}";
    //static const char * const default_chrono_parse = "%Y-%m-%d %H:%M:%S";
    constexpr const char * const layout_only_date = "%Y-%m-%d";
    //static const char * const layout_only_time = "%H:%M:%S";
    constexpr const char * const layout_date_time = "%Y-%m-%d %H:%M:%S";

    // 支持的日期时间格式列表
    constexpr const char* layout_supports[] = {
        "%Y-%m-%d %H:%M:%S",        // 2023-05-15 14:30:00
        "%Y-%m-%d",                 // 2023-05-15
        "%Y%m%d",                   // 20230515
        "%Y/%m/%d %H:%M:%S",        // 2023/05/15 14:30:00
        "%m/%d/%Y %H:%M:%S",        // 05/15/2023 14:30:00
        "%H:%M:%S %d-%m-%Y",        // 14:30:00 15-05-2023
        "%Y%m%d %H%M%S",            // 20230515 143000
        "%Y-%m-%dT%H:%M:%SZ",       // ISO 8601 UTC
        "%Y-%m-%dT%H:%M:%S%z",      // ISO 8601 with timezone
        "%a, %d %b %Y %H:%M:%S %Z", // RFC 1123
        "%b %d %Y %H:%M:%S"         // May 15 2023 14:30:00
    };

    int64_t ms_utc_to_local(const int64_t &milliseconds) {
        return milliseconds + _zone_offset_milliseconds;
    }

    int64_t ms_local_to_utc(const int64_t &milliseconds) {
        return milliseconds - _zone_offset_milliseconds;
    }

    std::chrono::system_clock::time_point from_local(const int64_t &milliseconds) {
        auto ms_ = ms_local_to_utc(milliseconds);
        return std::chrono::system_clock::time_point(std::chrono::milliseconds(ms_));
    }

    int64_t from_time_point(const std::chrono::system_clock::time_point& tp) {
        auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
        auto ts = epoch.count();
        return ms_utc_to_local(ts);
    }

    // utc时间 格式化成 本地时间字符串
    std::string to_string(const std::chrono::system_clock::time_point &tp) {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
        std::chrono::sys_time<std::chrono::milliseconds> milli{ms};
#if CXX_CHRONO_ZONE_USE_DATE
        // 转换为 system_clock 的时区时间(zoned_time)
        auto zt = date::zoned_time{_local_zone, milli};
        //std::string str = std::format(default_chrono_format, zt);
        // 格式化为字符串(包含毫秒)
        std::string str = date::format(default_chrono_format, zt);
#else
        // 转换为 system_clock 的时区时间(zoned_time)
        auto zt = std::chrono::zoned_time{_local_zone, milli};
        // 格式化为字符串(包含毫秒)
        std::string str = std::format(default_chrono_format, zt);
#endif
        return str;
    }

//    // 本地时间字符串 解析成 utc时间
//    std::chrono::system_clock::time_point from_string(const char * str) {
//        std::istringstream iss(str);
//        std::chrono::sys_time<std::chrono::milliseconds> tp; // 毫秒精度
//
//        if (iss >> parse(default_chrono_parse, tp)) {
//            //std::cout << "UTC time with ms: " << tp << "\n";
//        } else {
//            std::string msg(str);
//            THROW_EXCEPTION("解析<"+msg+">失败");
//        }
//        return tp-std::chrono::seconds(_zone_offset_seconds);
//    }

    static std::string to_string(std::time_t tm, const char *const format = layout_only_date) {
        std::tm local_time = safe::localtime(tm);
        std::array<char, 64> buf{};
        size_t               len = std::strftime(buf.data(), buf.size(), format, &local_time);

        if (len == 0) {
            throw std::runtime_error("时间格式化失败: 缓冲区不足或格式无效");
        }

        return std::string(buf.data(), len);
    }

//    std::time_t from_string(const std::string& tm, const char * const format = layout_only_date) {
//        std::tm tm_time = {};
//        // 解析字符串到 tm 结构(C++11 使用 get_time)
//        std::istringstream ss(tm);
//        ss >> std::get_time(&tm_time, format);
//        if (ss.fail()) {
//            return std::time_t(0);
//        }
//        // 转换为 time_t(假设输入为本地时间)
//        std::time_t time_parsed = std::mktime(&tm_time);
//        if (time_parsed == -1) {
//            return std::time_t(0);
//        }
//        return time_parsed;
//    }

    /// 获取当前时间
    std::time_t current_time() {
        return std::time(nullptr);
    }

    /// 获取当前日期的字符串
    std::string today() {
        std::time_t now = current_time();
        return to_string(now);
    }

    /// 获取当前时间戳的字符串
    std::string get_timestamp() {
        std::time_t now = current_time();
        return to_string(now, layout_date_time);
    }

    std::pair<std::string, std::string> GetQuarterDay(int months) {
        // 获取当前时间点, 并减去若干个月
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = safe::localtime(now_time_t);

        // 减去 months 个月
        tm.tm_mon -= months;
        mktime(&tm);  // Normalize 时间

        int year = tm.tm_year + 1900;  // 年份
        int month = tm.tm_mon + 1;     // 月份 (从 0 开始)

        std::string firstOfQuarter;
        std::string lastOfQuarter;

        if (month >= 1 && month <= 3) {
            firstOfQuarter = std::to_string(year) + "-01-01 00:00:00";
            lastOfQuarter = std::to_string(year) + "-03-31 23:59:59";
        } else if (month >= 4 && month <= 6) {
            firstOfQuarter = std::to_string(year) + "-04-01 00:00:00";
            lastOfQuarter = std::to_string(year) + "-06-30 23:59:59";
        } else if (month >= 7 && month <= 9) {
            firstOfQuarter = std::to_string(year) + "-07-01 00:00:00";
            lastOfQuarter = std::to_string(year) + "-09-30 23:59:59";
        } else {
            firstOfQuarter = std::to_string(year) + "-10-01 00:00:00";
            lastOfQuarter = std::to_string(year) + "-12-31 23:59:59";
        }

        return {firstOfQuarter, lastOfQuarter};
    }

    // 尝试用指定格式解析日期时间字符串
    std::optional<tm> tryParse(const std::string& str, const char* fmt) {
        tm tm = {};
        std::istringstream ss(str);
        ss >> std::get_time(&tm, fmt);
        if (ss.fail() || ss.rdbuf()->in_avail() != 0) {
            return std::nullopt;
        }
        return tm;
    }

    // 改进后的 parseTime 函数, 支持多种日期格式
    tm parseTime(const std::string& date) {
        // 尝试所有支持的格式
        for (const auto& fmt : layout_supports) {
            auto result = tryParse(date, fmt);
            if (result) {
                // 确保年份是完整的(如将23转换为2023)
                if (result->tm_year < 100) {
                    result->tm_year += (result->tm_year < 70) ? 2000 : 1900;
                }
                return *result;
            }
        }

        // 所有格式都失败时返回当前时间
        time_t now = time(nullptr);
        return safe::localtime(now);
    }

    std::tuple<std::string, std::string, std::string> GetQuarterByDate(const std::string& date, int diffQuarters) {
        tm now = parseTime(date);

        // Apply quarter offset
        now.tm_mon -= 3 * diffQuarters;

        // Normalize the time (handle month overflow)
        mktime(&now);

        int year = 1900 + now.tm_year;
        int month = now.tm_mon + 1; // tm_mon is 0-11

        std::string quarter;
        std::string firstOfQuarter;
        std::string lastOfQuarter;

        if (month >= 1 && month <= 3) {
            // 1月1号
            firstOfQuarter = std::to_string(year) + "-01-01 00:00:00";
            lastOfQuarter = std::to_string(year) + "-03-31 23:59:59";
            quarter = std::to_string(year) + "Q1";
        }
        else if (month >= 4 && month <= 6) {
            firstOfQuarter = std::to_string(year) + "-04-01 00:00:00";
            lastOfQuarter = std::to_string(year) + "-06-30 23:59:59";
            quarter = std::to_string(year) + "Q2";
        }
        else if (month >= 7 && month <= 9) {
            firstOfQuarter = std::to_string(year) + "-07-01 00:00:00";
            lastOfQuarter = std::to_string(year) + "-09-30 23:59:59";
            quarter = std::to_string(year) + "Q3";
        }
        else {
            firstOfQuarter = std::to_string(year) + "-10-01 00:00:00";
            lastOfQuarter = std::to_string(year) + "-12-31 23:59:59";
            quarter = std::to_string(year) + "Q4";
        }

        return {quarter, firstOfQuarter, lastOfQuarter};
    }

    // std::tm 转 time_point (假设tm为本地时间)
    std::chrono::system_clock::time_point tm_to_time_point(const std::tm& tm)
    {
        std::tm tmp = tm;
        const std::time_t t = mktime(&tmp);
        if(t == -1) {
            throw std::runtime_error("Invalid tm structure");
        }
        return std::chrono::system_clock::from_time_t(t);
    }
}
