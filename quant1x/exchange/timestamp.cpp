#include <quant1x/exchange/timestamp.h>
#include <quant1x/runtime/config.h>
#include <quant1x/std/time.h>

namespace exchange {

    constexpr auto only_date_layout = "{:%Y-%m-%d}";
    constexpr auto cache_date_layout = "{:%Y%m%d}";

    /**
     * @brief 本地当前时间
     * @return
     */
    int64_t timestamp::current() {
        auto utc_now = std::chrono::system_clock::now();
        auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(utc_now.time_since_epoch());
        auto ts = epoch.count();
        return api::ms_utc_to_local(ts);
    }

    timestamp::timestamp() : ms_(0) {}
    timestamp::timestamp(int64_t t) : ms_(t){}

    timestamp::timestamp(const std::chrono::system_clock::time_point &tp) :ms_(0) {
        auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
        auto ts = epoch.count();
        ms_ = api::ms_utc_to_local(ts);
    }

    timestamp::timestamp(const std::string &str) : ms_(0) {
        auto ts = parse(str).ms_;
        ms_ = ts;
    }

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
    timestamp::timestamp(int y, int m, int d, int hh, int mm, int ss, int sss) : ms_(0) {
        std::chrono::year ty{y};
        std::chrono::month tm{static_cast<unsigned int>(m)};
        std::chrono::day td{static_cast<unsigned int>(d)};
        auto ymd = ty / tm / td;
        std::chrono::sys_days sd = ymd;
        auto tp = sd
                  + std::chrono::hours(hh)
                  + std::chrono::minutes(mm)
                  + std::chrono::seconds(ss)
                  + std::chrono::milliseconds(sss);
        auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
        ms_ = epoch.count();
    }

    int64_t timestamp::value() const {
        return ms_;
    }

    /**
     * @brief 盘前初始化时间戳, 用年月日构造一个时间戳, 默认时间是9点整
     * @param year 年
     * @param month 月
     * @param day 日
     * @return
     */
    timestamp timestamp::pre_market_time(int year, int month, int day) {
        return {year, month, day, config::cn_pre_market_hour, config::cn_pre_market_minute, config::cn_pre_market_second, 0};
    }

    /**
     * @brief 当前时间戳
     * @return
     */
    timestamp timestamp::now() {
        int64_t ts = current();
        return timestamp{ts};
    }

    // 零值
    timestamp timestamp::zero() {
        return {0};
    }

    // 解析日期时间
    timestamp timestamp::parse(const std::string& str) {
        auto ts = api::parse_date(str);
        return timestamp{ts};
    }

    // 解析时间
    timestamp timestamp::parse_time(const std::string& str) {
        auto ts = api::parse_time(str);
        return timestamp{ts};
    }

    // 获取当前时间戳对应的当天零点（00:00:00.000）的时间戳 truncate
    timestamp timestamp::start_of_day() const {
        return timestamp{ms_ - (ms_ % milliseconds_per_day)};
    }

    // 当天零点整
    timestamp timestamp::midnight() {
        auto ts = current();
        return timestamp{ts - ts % milliseconds_per_day};
    }

    timestamp timestamp::today(int hour, int minute, int second, int millisecond) const {
        int64_t ts = start_of_day().value();
        ts += hour * milliseconds_per_hour;
        ts += minute * milliseconds_per_minute;
        ts += second * milliseconds_per_second;
        ts += millisecond;
        return timestamp{ts};
    }

    timestamp timestamp::since(int hour, int minute, int second, int millisecond) const {
        int64_t ts = start_of_day().value();
        ts += hour * milliseconds_per_hour;
        ts += minute * milliseconds_per_minute;
        ts += second * milliseconds_per_second;
        ts += millisecond;
        return timestamp{ts};
    }

    timestamp timestamp::offset(int hour, int minute, int second, int millisecond) const {
        int64_t ts = value();
        ts += hour * milliseconds_per_hour;
        ts += minute * milliseconds_per_minute;
        ts += second * milliseconds_per_second;
        //ts -= (ts % MillisecondsPerSecond); // 毫秒偏移, 去掉原毫秒数, 累加入参
        ts += millisecond;
        return timestamp{ts};
    }

    // truncate

    timestamp timestamp::pre_market_time() const {
        return since(config::cn_pre_market_hour, config::cn_pre_market_minute, config::cn_pre_market_second, 0);
    }

    // 调整时间戳到0秒0毫秒（用于 begin）
    timestamp timestamp::floor() const {
        int64_t ts = value();

        ts -= (ts % milliseconds_per_minute);

        return timestamp{ts};
    }

    // 调整时间戳到59秒999毫秒（用于 end）
    timestamp timestamp::ceil() const {
        int64_t ts = value();

        // 调整原时间戳中的秒部分，并加上 .999 毫秒
        ts = ts - (ts % milliseconds_per_minute) + (milliseconds_per_minute - 1);

        return timestamp{ts};
    }

    // 提取年月日
    std::tuple<int, int, int> timestamp::extract() const {
        // 将毫秒数转换为秒数（忽略毫秒部分）
        auto seconds = static_cast<std::time_t>(ms_ / milliseconds_per_second);

        // 使用 localtime 转换为本地时间结构体 tm
        std::tm local_time = {};
#ifdef _WIN32
        localtime_s(&local_time, &seconds); // Windows 平台
#else
        localtime_r(&seconds, &local_time); // Linux/macOS 平台
#endif

        // 提取年、月、日信息
        int year = local_time.tm_year + 1900; // tm_year 是从 1900 年开始的偏移量
        int month = local_time.tm_mon + 1;   // tm_mon 是从 0 开始的月份编号
        int day = local_time.tm_mday;
        return {year, month, day};
    }

    std::string timestamp::toString(const std::string& layout) const {
        // 1. 构造毫秒
        std::chrono::milliseconds ts{ms_};
        // 2. 转换为 system_clock 的时间点（sys_time）
        std::chrono::sys_time<std::chrono::milliseconds> tp{ts};
        //auto zt = std::chrono::zoned_time{_local_zone, tp}; // 本地时间, 这里不用本地时区转换
        // 3. 格式化为字符串（包含毫秒）
        std::string str = std::vformat(layout, std::make_format_args(tp));
        // 考虑更安全的解析时间字符串的方法
        return str;
    }

    std::string timestamp::toStringAsTimeInSeconds(const std::string& layout) const {
        //std::chrono::milliseconds ts{ms_};
        // 1. 构造 sys_time<milliseconds>
        auto ts = std::chrono::sys_time<std::chrono::milliseconds>(std::chrono::milliseconds(ms_));
        // 2. 转换为 sys_time<seconds>
        auto tp = std::chrono::time_point_cast<std::chrono::seconds>(ts);
        // 3. 格式化为字符串（包含毫秒）
        std::string str = std::vformat(layout, std::make_format_args(tp));
        // 考虑更安全的解析时间字符串的方法
        return str;
    }

    // 返回日期
    std::string timestamp::only_date() const {
        return toString(only_date_layout);
    }

    std::string timestamp::cache_date() const {
        return toString(cache_date_layout);
    }

    // 返回时间
    std::string timestamp::only_time() const {
        return toStringAsTimeInSeconds("{:%H:%M:%S}");
    }

    // 返回整型日期
    uint32_t timestamp::yyyymmdd() const {
        auto [y,m,d] = extract();
        return y*10000+m*100+d;
    }

    bool timestamp::empty() const {
        return value() == 0;
    }

    // 判断是否同一天
    bool timestamp::is_same_date(const timestamp &other) const noexcept {
        const int64_t day1 = ms_ / milliseconds_per_day;
        const int64_t day2 = other.ms_ / milliseconds_per_day;
        return day1 == day2;
    }

    std::ostream& operator<<(std::ostream &os, const timestamp &ts) {
        os << ts.toString();
        return os;
    }

    bool timestamp::operator==(const timestamp &rhs) const {
        return ms_ == rhs.ms_;
    }

    bool timestamp::operator!=(const timestamp &rhs) const {
        return ms_ != rhs.ms_;
    }

    bool timestamp::operator<(const timestamp &rhs) const {
        return ms_ < rhs.ms_;
    }

    bool timestamp::operator>(const timestamp &rhs) const {
        return ms_ > rhs.ms_;
    }

    bool timestamp::operator<=(const timestamp &rhs) const {
        return ms_ <= rhs.ms_;
    }

    bool timestamp::operator>=(const timestamp &rhs) const {
        return ms_ >= rhs.ms_;
    }
}