#pragma once
#ifndef QUANT1X_DATASETS_BASE_H
#define QUANT1X_DATASETS_BASE_H 1

#include "adapter.h"
#include <quant1x/data/meta/exchange.h>
#include <quant1x/data/market.h>

namespace quant1x::data {
    constexpr Kind baseKind = 0;
    constexpr Kind BaseXdxr                = PluginMaskBaseData | (baseKind +  1); // 基础数据-除权除息
    constexpr Kind BaseRawDailyKLine       = PluginMaskBaseData | (baseKind +  2); // 基础数据-未复权K线
    constexpr Kind BaseKLine               = PluginMaskBaseData | (baseKind +  3); // 基础数据-前复权K线
    constexpr Kind BaseTransaction         = PluginMaskBaseData | (baseKind +  4); // 基础数据-历史成交
    constexpr Kind BaseMinutes             = PluginMaskBaseData | (baseKind +  5); // 基础数据-分时数据
    constexpr Kind BaseQuarterlyReports    = PluginMaskBaseData | (baseKind +  6); // 基础数据-季报
    constexpr Kind BaseSafetyScore         = PluginMaskBaseData | (baseKind +  7); // 基础数据-安全分
    constexpr Kind BaseWideKLine           = PluginMaskBaseData | (baseKind +  8); // 基础数据-宽表
    constexpr Kind BasePerformanceForecast = PluginMaskBaseData | (baseKind +  9); // 基础数据-业绩预告
    constexpr Kind BaseChipDistribution    = PluginMaskBaseData | (baseKind + 10); // 基础数据-筹码分布
    constexpr Kind BaseMinuteKLine         = PluginMaskBaseData | (baseKind + 11); // 基础数据-分钟级别K线

    constexpr const char *const MARKET_CN_FIRST_LISTTIME = "1990-12-19"; // A股首个交易日
    constexpr const char *const GLOBAL_DEFAULT_START_DATE = "1900-01-01"; // 全球首个交易日
    // 市场开埠日期
    inline meta::Timestamp market_first_date = meta::Timestamp::parse(MARKET_CN_FIRST_LISTTIME).pre_market_time();
}

#endif // QUANT1X_DATASETS_BASE_H
