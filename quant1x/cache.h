#pragma once
#ifndef QUANT1X_CACHE_H
#define QUANT1X_CACHE_H 1

#include <quant1x/std/api.h>
#include <quant1x/proto/data.h>
#include <quant1x/data/adapter.h>
#include <quant1x/config/config.h>
#include <quant1x/data/market.h>
#include "data/meta/calendar.h"

namespace cache {

    namespace meta = quant1x::data::meta;
    namespace data = quant1x::data;

    constexpr const char *const trains_begin_date = "2024-10-01";

    /// 检查状态文件是否存在, 存在返回false, 不存在返回true
    bool check_update_state(const std::string &date, const meta::Timestamp &timestamp);
    /// 设置一个时间段为已更新状态
    void done_ipdate(const std::string &date, const meta::Timestamp &timestamp);
    /// 清除所有的过期状态文件
    bool clean_expired_state_files();
    /// 更新所有数据
    void update_all();
    /// 完成数据处理的适配器数
    int update_with_adapters(const std::vector<data::DataAdapter*> &adapters, const meta::Timestamp& feature_date = meta::last_trading_day());

    //============================================================
    // 历史成交记录                                                //
    //============================================================

    /// 交易类型枚举
    enum TradeDirection : int {
        TICK_BUY     = 0,  // 买入
        TICK_SELL    = 1,  // 卖出
        TICK_NEUTRAL = 2,  // 中性盘
        TICK_UNKNOWN = 3   // 未知类型(09:27分历史数据, 暂定为中性盘)
    };

    struct HistoricalTrade {
        std::string Time;                                      // 时间 hh:mm
        f64         Price     = 0;                             // 价格
        int         Vol       = 0;                             // 成交量, 股数
        int         Num       = 0;                             // 历史成交数据中无该字段, 但仍保留
        f64         Amount    = 0;                             // 金额
        int         BuyOrSell = TradeDirection::TICK_UNKNOWN;  // 交易方向
    };

    struct PriceLine {
        i32 price = 0;  // 价格, 单位分
        f64 buy   = 0;  // 买入, 成交量, 单位股
        f64 sell  = 0;  // 卖出, 成交量, 单位股
    };

}  // namespace cache

#endif  // QUANT1X_CACHE_H
