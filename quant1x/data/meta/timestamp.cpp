#include <quant1x/data/meta/timestamp.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace meta {

Timestamp::Timestamp(const std::string& s) {
    if (s.empty()) {
        ms = 0;
        return;
    }
    *this = parse(s);
}

Timestamp Timestamp::now() {
    auto now_tp = std::chrono::system_clock::now();
    auto ms_tp = std::chrono::duration_cast<std::chrono::milliseconds>(now_tp.time_since_epoch());
    // 转换为本地时间
    auto now_c = std::chrono::system_clock::to_time_t(now_tp);
    std::tm local_tm{};
#ifdef _WIN32
    localtime_s(&local_tm, &now_c);
#else
    localtime_r(&now_c, &local_tm);
#endif
    // 计算本地时区偏移
    std::time_t utc_mktime = std::mktime(&local_tm);
    auto utc_offset = static_cast<int64_t>(std::difftime(utc_mktime, now_c)) * MILLISECONDS_PER_SECOND;
    return Timestamp(ms_tp.count() + utc_offset);
}

Timestamp Timestamp::parse(const std::string& s) {
    if (s.empty()) return zero();
    std::tm tm{};
    [[maybe_unused]] int ms_part = 0;
    // 尝试多种格式
    const char* formats[] = {
        "%Y-%m-%d %H:%M:%S", "%Y-%m-%d", "%Y%m%d", "%Y/%m/%d %H:%M:%S",
        "%Y/%m/%d", "%Y%m%d %H%M%S", "%Y-%m-%dT%H:%M:%S"
    };
    for (auto fmt : formats) {
        std::istringstream ss(s);
        ss >> std::get_time(&tm, fmt);
        if (!ss.fail()) {
            std::time_t t = std::mktime(&tm);
            auto tp = std::chrono::system_clock::from_time_t(t);
            auto ms_tp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
            return Timestamp(ms_tp.count());
        }
    }
    // 最后尝试只有日期的紧凑格式
    if (s.length() == 8 && std::all_of(s.begin(), s.end(), ::isdigit)) {
        int year = std::stoi(s.substr(0, 4));
        int month = std::stoi(s.substr(4, 2));
        int day = std::stoi(s.substr(6, 2));
        return from_date(year, month, day);
    }
    return zero();
}

Timestamp Timestamp::parse_time(const std::string& s) {
    if (s.empty()) return zero();
    std::tm tm{};
    [[maybe_unused]] int ms_part = 0;
    // 先尝试纯时间格式
    const char* time_formats[] = {"%H:%M:%S", "%H:%M", "%H%M%S"};
    for (auto fmt : time_formats) {
        std::istringstream ss(s);
        ss >> std::get_time(&tm, fmt);
        if (!ss.fail()) {
            auto now_tp = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now_tp);
            std::tm local_tm{};
#ifdef _WIN32
            localtime_s(&local_tm, &now_c);
#else
            localtime_r(&now_c, &local_tm);
#endif
            local_tm.tm_hour = tm.tm_hour;
            local_tm.tm_min = tm.tm_min;
            local_tm.tm_sec = tm.tm_sec;
            std::time_t t = std::mktime(&local_tm);
            auto tp = std::chrono::system_clock::from_time_t(t);
            auto ms_tp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
            return Timestamp(ms_tp.count());
        }
    }
    // 回退到完整日期时间解析
    return parse(s);
}

Timestamp Timestamp::from_date(int year, int month, int day,
                                int hour, int minute, int second, int millisecond) {
    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    std::time_t t = std::mktime(&tm);
    auto tp = std::chrono::system_clock::from_time_t(t);
    auto ms_tp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    return Timestamp(ms_tp.count() + millisecond);
}

Timestamp Timestamp::pre_market_time() const {
    int year, month, day;
    extract(year, month, day);
    return from_date(year, month, day, PRE_MARKET_HOUR, PRE_MARKET_MINUTE, PRE_MARKET_SECOND, 0);
}

Timestamp Timestamp::pre_market_time(int year, int month, int day) {
    return from_date(year, month, day, PRE_MARKET_HOUR, PRE_MARKET_MINUTE, PRE_MARKET_SECOND, 0);
}

std::string Timestamp::only_date() const {
    return to_string("%Y-%m-%d");
}

std::string Timestamp::only_time() const {
    return to_string("%H:%M:%S");
}

std::string Timestamp::cache_date() const {
    return to_string("%Y%m%d");
}

int64_t Timestamp::yyyymmdd_int() const {
    int year, month, day;
    extract(year, month, day);
    return static_cast<int64_t>(year) * 10000 + month * 100 + day;
}

Timestamp Timestamp::from_yyyymmdd_int(int64_t date) {
    int year = static_cast<int>(date / 10000);
    int month = static_cast<int>((date % 10000) / 100);
    int day = static_cast<int>(date % 100);
    return from_date(year, month, day);
}

std::string Timestamp::to_string(const char* layout) const {
    std::time_t t = static_cast<std::time_t>(ms / MILLISECONDS_PER_SECOND);
    std::tm local_tm{};
#ifdef _WIN32
    localtime_s(&local_tm, &t);
#else
    localtime_r(&t, &local_tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&local_tm, layout);
    return oss.str();
}

bool Timestamp::is_same_date(const Timestamp& other) const {
    int y1, m1, d1, y2, m2, d2;
    extract(y1, m1, d1);
    other.extract(y2, m2, d2);
    return y1 == y2 && m1 == m2 && d1 == d2;
}

Timestamp Timestamp::offset(int hour, int minute, int second, int millisecond) const {
    int64_t offset_ms = static_cast<int64_t>(hour) * MILLISECONDS_PER_HOUR +
                        static_cast<int64_t>(minute) * MILLISECONDS_PER_MINUTE +
                        static_cast<int64_t>(second) * MILLISECONDS_PER_SECOND +
                        millisecond;
    return Timestamp(ms + offset_ms);
}

Timestamp Timestamp::start_of_day() const {
    return today_at(0, 0, 0, 0);
}

Timestamp Timestamp::today_at(int hour, int minute, int second, int millisecond) const {
    int year, month, day;
    extract(year, month, day);
    return from_date(year, month, day, hour, minute, second, millisecond);
}

void Timestamp::extract(int& year, int& month, int& day) const {
    std::time_t t = static_cast<std::time_t>(ms / MILLISECONDS_PER_SECOND);
    std::tm local_tm{};
#ifdef _WIN32
    localtime_s(&local_tm, &t);
#else
    localtime_r(&t, &local_tm);
#endif
    year = local_tm.tm_year + 1900;
    month = local_tm.tm_mon + 1;
    day = local_tm.tm_mday;
}

void Timestamp::extract_time(int& hour, int& minute, int& second, int& millisecond) const {
    std::time_t t = static_cast<std::time_t>(ms / MILLISECONDS_PER_SECOND);
    std::tm local_tm{};
#ifdef _WIN32
    localtime_s(&local_tm, &t);
#else
    localtime_r(&t, &local_tm);
#endif
    hour = local_tm.tm_hour;
    minute = local_tm.tm_min;
    second = local_tm.tm_sec;
    millisecond = static_cast<int>(ms % MILLISECONDS_PER_SECOND);
}

} // namespace meta
