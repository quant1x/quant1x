#include <quant1x/io/csv-reader.h>
#include <quant1x/io/csv-writer.h>
#include <quant1x/base/filesystem.h>
#include <quant1x/io/http.h>
#include <quant1x/runtime/cache1d.h>
#include <quant1x/config/config.h>
#include <quant1x/base/time.h>
#include <quant1x/data/cache.h>
#include <filesystem>
#include "sina/decoder.h"
#include "calendar.h"
#include "timestamp.h"

//============================================================
// exchange 交易日历相关                                      //
//============================================================

namespace quant1x::data::meta {
    namespace detail {

        // 获取交易日历的缓存文件名
        std::string get_calendar_filepath() {
            return quant1x::config::get_meta_path() + "/calendar";
        }

        // 获取交易日历标记文件名
        std::string get_calendar_marker_filepath() {
            return quant1x::config::get_meta_path() + "/calendar.updated";
        }

        // 预处理http接口返回的js文本, 去除赋值双引号等
        static std::string _preprocess_sina_text(const std::string &text) {
            std::string processed = text;
            size_t      eqPos     = processed.find('=');
            if (eqPos != std::string::npos) {
                processed = processed.substr(eqPos + 1);
            }
            size_t semiPos = processed.find(';');
            if (semiPos != std::string::npos) {
                processed = processed.substr(0, semiPos);
            }
            processed.erase(std::remove(processed.begin(), processed.end(), '"'), processed.end());
            return processed;
        }

        // js解码
        static std::vector<std::string> _decode_sina_text(const std::string &text) {
            std::string input = _preprocess_sina_text(text);
            sina::finance_decoder decoder(input);
            auto            dates   = decoder.decode();
            if (dates.empty()) {
                return {};
            }
            std::vector<std::string> result;
            result.reserve(dates.size());
            for (const auto &item : dates) {
                for (const auto &pair : item) {
                    if (pair.first == "date") {
                        result.emplace_back(pair.second);
                    }
                }
            }
            return result;
        }
    }  // namespace detail

    inline auto global_calendar_once = RollingOnce::create("meta-calendar", quant1x::config::GLOBAL_CRON_EXPR_DAILY_INIT);
    inline std::vector<std::string> global_calendars_string    = {};
    inline std::vector<Timestamp>   global_calendars_timestamp = {};

    static const char *const urlSinaRealstockCompanyKlcTdSh = "https://finance.sina.com.cn/realstock/company/klc_td_sh.txt";
    // static const char * const urlSinaRealstockCompanyKlcTdSz = "https://finance.sina.com.cn/realstock/company/klc_td_sz.txt";
    static const char *const calendarMissingDate = "1992-05-04";  // TODO:已知缺失的交易日期, 现在已经能自动甄别缺失的交易日期

    // 同步交易日历
    void update_calendar() {
        const auto cache_path = detail::get_calendar_filepath();
        const auto modified   = filesystem::last_modified_time(cache_path);
        auto [text, tm]       = http::request(urlSinaRealstockCompanyKlcTdSh, modified);
        if (!text.empty()) {
            auto list = detail::_decode_sina_text(text);
            auto it   = std::lower_bound(list.begin(), list.end(), calendarMissingDate);
            if (it == list.end() || *it != calendarMissingDate) {
                list.insert(it, calendarMissingDate);
            }
            {
                auto ec = filesystem::check_filepath(cache_path, true);
                ec.clear();
                io::CSVWriter writer(cache_path);
                writer.write_row("date", "source");
                for (auto const &v : list) {
                    writer.write_row(v, "sina");
                }
            }
            filesystem::last_modified_time(cache_path, tm);
        }
    }

    // 交易日历
    void lazy_load_calendar() {
        spdlog::debug("加载交易日历...");

        auto load_from_file = [](const std::string &cache_path) -> bool {
            if (!std::filesystem::exists(cache_path)) {
                return false;
            }
            try {
                global_calendars_string.clear();
                global_calendars_timestamp.clear();

                io::CSVReader<1> in(cache_path);
                in.read_header(io::ignore_extra_column, "date");
                std::string date;
                while (in.read_row(date)) {
                    global_calendars_string.emplace_back(date);
                    Timestamp ts = date;
                    ts           = ts.pre_market_time();
                    global_calendars_timestamp.emplace_back(ts);
                }
                return !global_calendars_timestamp.empty();
            } catch (const std::exception &e) {
                spdlog::error("加载交易日历缓存文件失败: {}", e.what());
                return false;
            }
        };

        // 1. 检查标记文件是否过期, 决定是否需要更新
        std::string marker      = detail::get_calendar_marker_filepath();
        Timestamp   now_time    = Timestamp::now();
        Timestamp   mod_time    = quant1x::data::get_filename_modified_time(marker);
        Timestamp   today_init  = now_time.pre_market_time();

        if (now_time > today_init && mod_time < today_init) {
            spdlog::debug("交易日历缓存文件过期, 执行更新");
            try {
                update_calendar();
                // 更新标记文件的修改时间 (对齐 Python fs.update_file_mtime)
                if (!filesystem::write_file(marker, "", 0)) {
                    spdlog::warn("写入交易日历标记文件失败: {}", marker);
                }
                filesystem::last_modified_time(marker, now_time.value());
            } catch (const std::exception &e) {
                spdlog::debug("交易日历更新失败: {}", e.what());
            }
        } else {
            spdlog::debug("交易日历缓存文件未过期, 跳过更新");
        }

        // 2. 加载交易日历缓存文件到内存 (对齐 Python: 文件不存在则跳过)
        spdlog::debug("加载交易日历缓存文件到内存");
        auto cache_path = detail::get_calendar_filepath();
        // 确保父目录存在
        filesystem::check_filepath(cache_path, true);
        bool loaded = load_from_file(cache_path);
        if (!loaded) {
            spdlog::warn("交易日历缓存为空或缺失, 触发一次强制更新");
            try {
                update_calendar();
                if (!filesystem::write_file(marker, "", 0)) {
                    spdlog::warn("写入交易日历标记文件失败: {}", marker);
                }
                filesystem::last_modified_time(marker, now_time.value());
            } catch (const std::exception &e) {
                spdlog::error("交易日历强制更新失败: {}", e.what());
            }
            loaded = load_from_file(cache_path);
        }
        if (loaded) {
            if (!std::filesystem::exists(marker)) {
                // 日历加载成功但标记缺失时补建, 保证后续过期判断稳定。
                if (!filesystem::write_file(marker, "", 0)) {
                    spdlog::warn("写入交易日历标记文件失败: {}", marker);
                }
                filesystem::last_modified_time(marker, now_time.value());
            }
            spdlog::debug("交易日历加载完成, 共 {} 个交易日", global_calendars_string.size());
        } else {
            spdlog::warn("交易日历最终仍为空");
        }
    }

    // 这里简单的封装一层, 以后扩展动态更新加载
    std::vector<std::string> get_calendar_list() {
        global_calendar_once->Do(lazy_load_calendar);
        if (global_calendars_string.empty()) {
            throw std::runtime_error("exchange calendar is empty");
        }
        return global_calendars_string;
    }

    // 获取最近一个交易日
    //[[deprecated("获取最后一个交易日的函数, 自0.1.0版本起废弃. 使用 last_trade_day() 代替")]]
    std::string get_last_trading_day(const std::string &date, const std::string &debug_timestamp) {
        spdlog::debug("[exchange::calendar] date={}, debug_timestamp={}", date, debug_timestamp);
        auto tradeDates = get_calendar_list();
        auto it         = std::upper_bound(tradeDates.begin(), tradeDates.end(), date);
        if (it != tradeDates.begin()) {
            --it;
        }
        // 判断是否盘前
        std::string last_timestamp    = to_timestamp(*it);
        std::string current_timestamp = debug_timestamp.empty() || debug_timestamp == "1970-01-01"
                                            ? api::get_timestamp()
                                            : debug_timestamp;
        if (current_timestamp < last_timestamp && it != tradeDates.begin()) {
            --it;
        }
        return *it;
    }

    Timestamp last_trading_day(const Timestamp &date, const Timestamp &debug_timestamp) {
        global_calendar_once->Do(lazy_load_calendar);
        spdlog::debug("[exchange::calendar] date={}, debug_timestamp={}", date.to_string(), debug_timestamp.to_string());
        const std::vector<Timestamp> &trade_dates = global_calendars_timestamp;
        if (trade_dates.empty()) {
            spdlog::warn("[exchange::calendar] trade calendar is empty, fallback to input date");
            const Timestamp fallback = date.empty() ? Timestamp::now().pre_market_time() : date;
            return fallback;
        }
        auto                          it          = std::upper_bound(trade_dates.begin(), trade_dates.end(), date);
        if (it != trade_dates.begin()) {
            --it;
        }
        // 判断是否盘前
        const Timestamp &last_timestamp    = *it;
        const Timestamp &current_timestamp = debug_timestamp.empty() ? Timestamp::now() : debug_timestamp;
        if (current_timestamp < last_timestamp && it != trade_dates.begin()) {
            --it;
        }
        auto ts = *it;
        spdlog::debug("[exchange::calendar] last_trading_day={}", ts.to_string());
        return ts;
    }

    // 获取上一个交易日
    Timestamp prev_trading_day(const Timestamp &date, const Timestamp &debug_timestamp) {
        global_calendar_once->Do(lazy_load_calendar);
        spdlog::debug("[exchange::calendar] date={}, debug_timestamp={}", date.to_string(), debug_timestamp.to_string());
        const std::vector<Timestamp> &trade_dates = global_calendars_timestamp;
        if (trade_dates.empty()) {
            spdlog::warn("[exchange::calendar] trade calendar is empty, fallback to input date");
            const Timestamp fallback = date.empty() ? Timestamp::now().pre_market_time() : date;
            return fallback;
        }
        auto                          it          = std::lower_bound(trade_dates.begin(), trade_dates.end(), date);
        if (it != trade_dates.begin()) {
            --it;
        }
        // 判断是否盘前
        const Timestamp &last_timestamp    = *it;
        const Timestamp &current_timestamp = debug_timestamp.empty() ? Timestamp::now() : debug_timestamp;
        if (current_timestamp < last_timestamp && it != trade_dates.begin()) {
            --it;
        }
        auto ts = *it;
        spdlog::debug("[exchange::calendar] prev_trading_day={}", ts.to_string());
        return ts;
    }

    // 获取下一个交易日
    Timestamp next_trading_day(const Timestamp &date, const Timestamp &debug_timestamp) {
        global_calendar_once->Do(lazy_load_calendar);
        spdlog::debug("[exchange::calendar] date={}, debug_timestamp={}", date.to_string(), debug_timestamp.to_string());

        const auto &trade_dates = global_calendars_timestamp;
        // 获取当前时间(用于判断是否“已过今日”)
        const Timestamp current_time = debug_timestamp.empty() ? Timestamp::now() : debug_timestamp;

        // 找到第一个大于 date.pre_market_time() 的交易日
        auto it = std::lower_bound(trade_dates.begin(), trade_dates.end(), date);

        // 如果没有比 date 更大的交易日, 就返回最后一个交易日
        if (it == trade_dates.end()) {
            if (!trade_dates.empty()) {
                spdlog::debug("[exchange::calendar] 已达交易日历尾部, 返回最后一个交易日");
                return trade_dates.back();
            } else {
                // 没有交易日数据, 返回空
                return Timestamp{};
            }
        }

        const Timestamp &candidate_day = *it;
        spdlog::debug("[exchange::calendar] candidate_day={}, current_time={}",
                      candidate_day.to_string(),
                      current_time.to_string());
        // 如果当前时间已经过了候选交易日的盘前时间, 则取下一个
        if (current_time >= candidate_day && it != trade_dates.end()) {
            ++it;
            if (it == trade_dates.end()) {
                spdlog::debug("[exchange::calendar] 已达交易日历尾部, 返回最后一个交易日");
                return trade_dates.back();
            }
            return *it;
        }

        // 否则返回当前找到的交易日
        spdlog::debug("[exchange::calendar] next_trading_day={}", candidate_day.to_string());
        return candidate_day;
    }

    // 获取日期范围
    std::vector<std::string> get_date_range(const std::string &begin, const std::string &end, bool skipToday) {
        if (begin > end) {
            return {};  // 起始日期不能大于结束日期
        }

        auto tradeDates = get_calendar_list();
        if (tradeDates.empty()) {
            return {};  // 交易日历为空
        }

        // 查找起始索引
        auto itStart = std::lower_bound(tradeDates.begin(), tradeDates.end(), begin);
        auto is      = static_cast<int>(std::distance(tradeDates.begin(), itStart));

        // 查找结束索引
        auto itEnd = std::lower_bound(tradeDates.begin(), tradeDates.end(), end);
        auto ie    = static_cast<int>(std::distance(tradeDates.begin(), itEnd));

        // 调整结束索引
        if (skipToday) {
            if (static_cast<size_t>(ie) < tradeDates.size()) {  // 确保索引有效
                std::string        today_  = current_day;
                const std::string &lastDay = tradeDates[ie];
                if (lastDay > today_ || lastDay > end) {
                    --ie;  // 如果最后一天大于今天或结束日期, 则向前调整
                }
            }
        } else {
            // 确保ie在有效范围内并调整到<= end的最大日期
            while (ie >= 0 && static_cast<size_t>(ie) < tradeDates.size() && tradeDates[ie] > end) {
                --ie;  // 向前调整索引
            }
        }

        // 检查索引有效性
        if (is < 0 || ie < 0 || is > ie || static_cast<size_t>(ie) >= tradeDates.size()) {
            return {};  // 索引无效时返回空结果
        }

        // 返回日期范围
        return {tradeDates.begin() + is, tradeDates.begin() + ie + 1};
    }

    // 获取日期范围
    std::vector<Timestamp> date_range(const Timestamp &begin, const Timestamp &end, bool skipToday) {
        // 1. 检查无效输入
        if (begin > end) {
            return {};
        }

        // 2. 确保日历数据已加载
        global_calendar_once->Do(lazy_load_calendar);
        const std::vector<Timestamp> &trade_dates = global_calendars_timestamp;
        if (trade_dates.empty()) {
            return {};
        }

        // 3. 使用更清晰的变量名
        const auto first = trade_dates.begin();
        const auto last  = trade_dates.end();

        // 4. 查找范围边界
        auto lower = std::lower_bound(first, last, begin);
        auto upper = std::upper_bound(first, last, end);

        // 5. 处理skipToday逻辑
        if (skipToday && upper != last) {
            const Timestamp today = ts_today_init;
            if (*upper > today || *upper > end) {
                --upper;
            }
        } else {
            // 调整upper到最后一个<=end的日期
            while (upper != first && *(upper - 1) > end) {
                --upper;
            }
        }

        // 6. 检查有效范围
        if (lower >= upper || lower == last || upper == first) {
            return {};
        }

        // 7. 返回结果
        return {lower, upper};
    }

    /// 获取当前时间戳
    std::string get_current_timestamp() {
        return api::get_timestamp();
    }
}  // namespace exchange
