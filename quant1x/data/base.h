#pragma once
#ifndef QUANT1X_DATASETS_BASE_H
#define QUANT1X_DATASETS_BASE_H 1

#include "adapter.h"
#include <quant1x/data/meta/exchange.h>
#include <quant1x/data/market.h>

namespace data {
    constexpr data::Kind baseKind = 0;
    constexpr data::Kind BaseXdxr                = data::PluginMaskBaseData | (baseKind +  1); // 基础数据-除权除息
    constexpr data::Kind BaseRawDailyKLine       = data::PluginMaskBaseData | (baseKind +  2); // 基础数据-未复权K线
    constexpr data::Kind BaseKLine               = data::PluginMaskBaseData | (baseKind +  3); // 基础数据-前复权K线
    constexpr data::Kind BaseTransaction         = data::PluginMaskBaseData | (baseKind +  4); // 基础数据-历史成交
    constexpr data::Kind BaseMinutes             = data::PluginMaskBaseData | (baseKind +  5); // 基础数据-分时数据
    constexpr data::Kind BaseQuarterlyReports    = data::PluginMaskBaseData | (baseKind +  6); // 基础数据-季报
    constexpr data::Kind BaseSafetyScore         = data::PluginMaskBaseData | (baseKind +  7); // 基础数据-安全分
    constexpr data::Kind BaseWideKLine           = data::PluginMaskBaseData | (baseKind +  8); // 基础数据-宽表
    constexpr data::Kind BasePerformanceForecast = data::PluginMaskBaseData | (baseKind +  9); // 基础数据-业绩预告
    constexpr data::Kind BaseChipDistribution    = data::PluginMaskBaseData | (baseKind + 10); // 基础数据-筹码分布
    constexpr data::Kind BaseMinuteKLine         = data::PluginMaskBaseData | (baseKind + 11); // 基础数据-分钟级别K线

    // 市场开埠日期
    inline meta::Timestamp market_first_date = meta::Timestamp::parse(meta::MARKET_CN_FIRST_LISTTIME).pre_market_time();
}

#endif //QUANT1X_DATASETS_BASE_H
