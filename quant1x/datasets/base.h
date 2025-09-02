#pragma once
#ifndef QUANT1X_DATASETS_BASE_H
#define QUANT1X_DATASETS_BASE_H 1

#include <quant1x/engine/action.h>
#include <quant1x/exchange/code.h>

namespace datasets {
    constexpr cache::Kind baseKind = 0;
    constexpr cache::Kind BaseXdxr                = cache::PluginMaskBaseData | (baseKind +  1); // 基础数据-除权除息
    constexpr cache::Kind RawKLine                = cache::PluginMaskBaseData | (baseKind +  2); // 基础数据-未复权K线
    constexpr cache::Kind BaseKLine               = cache::PluginMaskBaseData | (baseKind +  3); // 基础数据-前复权K线
    constexpr cache::Kind BaseTransaction         = cache::PluginMaskBaseData | (baseKind +  4); // 基础数据-历史成交
    constexpr cache::Kind BaseMinutes             = cache::PluginMaskBaseData | (baseKind +  5); // 基础数据-分时数据
    constexpr cache::Kind BaseQuarterlyReports    = cache::PluginMaskBaseData | (baseKind +  6); // 基础数据-季报
    constexpr cache::Kind BaseSafetyScore         = cache::PluginMaskBaseData | (baseKind +  7); // 基础数据-安全分
    constexpr cache::Kind BaseWideKLine           = cache::PluginMaskBaseData | (baseKind +  8); // 基础数据-宽表
    constexpr cache::Kind BasePerformanceForecast = cache::PluginMaskBaseData | (baseKind +  9); // 基础数据-业绩预告
    constexpr cache::Kind BaseChipDistribution    = cache::PluginMaskBaseData | (baseKind + 10); // 基础数据-筹码分布

    // 市场开埠日期
    inline exchange::timestamp market_first_date = exchange::timestamp::parse(exchange::market_cn_first_listtime).pre_market_time();
}

#endif //QUANT1X_DATASETS_BASE_H
