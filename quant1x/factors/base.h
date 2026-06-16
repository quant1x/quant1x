#pragma once
#ifndef QUANT1X_FACTOR_BASE_H
#define QUANT1X_FACTOR_BASE_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/data/kline.h>
#include <quant1x/contrib/data/tdx/kline.h>

namespace factors {

    constexpr int  KLineMin    = 120;                       ///< K线最小记录数
    constexpr auto baseFeature = data::PluginMaskFeature;  // 特征类型基础编码

    // ==============================
    // 登记所有的特征数据
    // ==============================

    constexpr auto FeatureF10                       = baseFeature + 1;  // 特征数据-基本面
    constexpr auto FeatureHistory                   = baseFeature + 2;  // 特征数据-历史
    constexpr auto FeatureNo0                       = baseFeature + 3;  // 特征数据-0号策略
    constexpr auto FeatureMisc                      = baseFeature + 4;  // 特征数据-Misc
    constexpr auto FeatureBreaksThroughBox          = baseFeature + 5;  // 特征数据-box
    constexpr auto FeatureKLineShap                 = baseFeature + 6;  // 特征数据-K线形态等
    constexpr auto FeatureInvestmentSentimentMaster = baseFeature + 7;  // 狩猎者-情绪周期
    constexpr auto FeatureSecuritiesMarginTrading   = baseFeature + 8;  // 融资融券

    // ==============================
    // 以下函数已迁移到 tdx::kline.h/cpp，此处保留薄封装以兼容现有调用方
    // ==============================

    /**
     * @brief 捡出截至指定日期date的K线记录
     * @param code 证券代码
     * @param date 日期
     * @return 从上市第一天起到date的全部K线记录
     */
    std::vector<data::KLine> checkout_klines(const std::string &code, const std::string &date);

    /**
     * @brief 捡出截至指定日期date的K线记录, 并前复权
     * @param code 证券代码
     * @param date 日期
     * @return 从上市第一天起到date的全部K线记录
     */
    std::vector<data::KLine> klines_forward_adjusted_to_date(const std::string &code, const std::string &date);

}  // namespace factors

#endif  // QUANT1X_FACTOR_BASE_H
