#pragma once
#ifndef QUANT1X_DATA_META_CALENDAR_H
#define QUANT1X_DATA_META_CALENDAR_H 1

#include <quant1x/config/config.h>
#include <quant1x/runtime/cache1d.h>
#include <quant1x/base/time.h>
#include <quant1x/base/filesystem.h>
#include <quant1x/io/http.h>
#include <quant1x/io/csv-reader.h>
#include <quant1x/io/csv-writer.h>
#include "timestamp.h"

//============================================================
// exchange 最基础的部分功能                                   //
//============================================================

namespace quant1x::data::meta {

    // 获取当前日期, 这里再封装一层, 尽量不暴露内部函数
    inline std::string init_current_day() {
        return api::today();
    }

    // 当前日期, 过0点转换
    inline auto current_day = runtime::cache1d<std::string>("calendar_current_day", init_current_day);

    inline Timestamp init_timestamp() {
        auto now = Timestamp::now();
        return now.pre_market_time();
    }

    inline auto ts_today_init = runtime::cache1d<Timestamp>("calendar_ts_today_init", init_timestamp);
}

//============================================================
// exchange 交易时段相关                                      //
//============================================================

namespace quant1x::data::meta {
    

    namespace {

        static const char * const begin_hhmmss = "09:15:00";
        static const char * const end_hhmmss = "15:00:59";

        static inline std::string to_timestamp(const std::string &date) {
            std::string timestamp = date.substr(0, 10) + " " + begin_hhmmss;
            return timestamp;
        }
    }
}

//============================================================
// exchange 交易日历相关                                      //
//============================================================

namespace quant1x::data::meta {

    /// 同步交易日历
    void update_calendar();

    // 交易日历
    void lazy_load_calendar();

    // 这里简单的封装一层, 以后扩展动态更新加载
    std::vector<std::string> get_calendar_list();

    // 获取最近一个交易日
    //[[deprecated("获取最后一个交易日的函数, 自0.1.0版本起废弃. 使用 last_trade_day() 代替")]]
    std::string get_last_trading_day(const std::string &date = current_day, const std::string & debug_timestamp = "");

    // 获取最近一个交易日
    Timestamp last_trading_day(const Timestamp& date = ts_today_init, const Timestamp& debug_timestamp = Timestamp::zero());
    // 获取上一个交易日
    Timestamp prev_trading_day(const Timestamp& date = ts_today_init, const Timestamp& debug_timestamp = Timestamp::zero());
    // 获取下一个交易日
    Timestamp next_trading_day(const Timestamp& date = ts_today_init, const Timestamp& debug_timestamp = Timestamp::zero());

    // 获取日期范围
    std::vector<std::string> get_date_range(const std::string &begin, const std::string &end, bool skipToday = false);

    // 获取日期范围
    std::vector<Timestamp> date_range(const Timestamp &begin, const Timestamp &end = Timestamp::now(), bool skipToday = false);

    /// 获取当前时间戳
    std::string get_current_timestamp();
}

#endif //QUANT1X_DATA_META_CALENDAR_H
