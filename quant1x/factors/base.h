#pragma once
#ifndef QUANT1X_FACTOR_BASE_H
#define QUANT1X_FACTOR_BASE_H 1

#include "quant1x/datasets/kline.h"
#include "quant1x/datasets/kline_raw.h"

namespace factors {

    // =============================
    // 复权因子模型: 仿射变换 P_adj = m * P + a
    // =============================

    // 累计复权因子
    struct CumulativeAdjustment {
        exchange::timestamp timestamp;  // 除权除息的毫秒数
        double              m;          // 系数, 比例因子（乘法）
        double              a;          // 偏移, 偏移因子（加法）
        int                 no;         // 第几次

        std::string to_string() const {
            return fmt::format("{{no={},timestamp={},m={},a={}}}", no, timestamp.only_date(), m, a);
        }

        // 将一个价格应用此次调整
        double apply(double price) const { return price * m + a; }

        // 返回此调整的逆变换（用于反向调整）
        double inverse(double adjusted_price) const { return (adjusted_price - a) / m; }
    };

    // 通过证券代码获取最新的除权除息列表
    std::span<const level1::XdxrInfo> get_xdxr_list(const std::string &);
    // 从除权除息的列表提取IPO日期
    std::optional<std::string> ipo_date_from_xdxrs(std::span<const level1::XdxrInfo> xdxrs);
    // 聚合给定一个时间范围内的复权因子
    std::vector<CumulativeAdjustment> combine_adjustments_in_period(std::span<const level1::XdxrInfo> xdxrs,
                                                                    const exchange::timestamp        &start_date,
                                                                    const exchange::timestamp        &end_date);

    /**
     * @brief 一次性复权, 只遍历一次
     * @tparam T 泛型类型
     * @param klines 需要复权的列表
     * @param xdxrs 全部除权除息的列表
     * @param start_date 开始时间戳
     * @param end_date 结束时间戳
     * @param should_truncate 是否应该截断 klines, 默认为true, 即截断
     */
    template <typename T>
    void apply_forward_adjustments(std::vector<T>                   &klines,
                                   std::span<const level1::XdxrInfo> xdxrs,
                                   const exchange::timestamp        &start_date,
                                   const exchange::timestamp        &end_date,
                                   bool                              should_truncate = true) {
        if (klines.empty()) {
            return;
        }
        // 强制统一为盘前时间
        auto ts_start = start_date.pre_market_time();
        auto ts_end   = end_date.pre_market_time();
        auto factors  = combine_adjustments_in_period(xdxrs, ts_start, ts_end);
        // 如果在时间范围内没有需要除权处理的记录, 则返回
        if (factors.empty()) {
            return;
        }
        size_t factors_count = factors.size();
        size_t i             = 0;  // 除权因子从第一个记录开始
        size_t rows          = 0;
        size_t klines_count  = klines.size();
        for (size_t idx = 0; idx < klines_count; ++idx) {
            auto kline        = &klines[idx];
            auto current_date = exchange::timestamp(kline->Date).pre_market_time();
            auto factor       = factors[i];
            if (current_date > ts_end) {
                break;
            }
            // 如果日线日期大于因子的日期, 因子索引+, 自动切换下一个因子
            // 考虑到可能存在长期停牌且停牌期间有除权除息记录的情况, 这种情况很少, 但还是可能会发生
            // 所以, 这里用了while循环, 直接找到最近的可以复权的记录
            while (i + 1 < factors_count && current_date >= factor.timestamp) {
                ++i;
                factor = factors[i];
            }
            if (current_date < factor.timestamp) {
                kline->adjust(factor.m, factor.a, factor.no);
            } else if (!should_truncate) {
                // 如果不截断数据, 那么, 对于已经没有需要复权的因子来说，后面的klines数据就没必要继续循环了
                break;
            }
            ++rows;
        }
        if (should_truncate) {
            klines.resize(rows);
        }
    }

    constexpr int  KLineMin    = 120;                       ///< K线最小记录数
    constexpr auto baseFeature = cache::PluginMaskFeature;  // 特征类型基础编码

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

    /**
     * @brief 捡出截至指定日期date的K线记录
     * @param code 证券代码
     * @param date 日期
     * @return 从上市第一天起到date的全部K线记录
     */
    std::vector<datasets::KLine> checkout_klines(const std::string &code, const std::string &date);

    /**
     * @brief 捡出截至指定日期date的K线记录, 并前复权
     * @param code 证券代码
     * @param date 日期
     * @return 从上市第一天起到date的全部K线记录
     */
    std::vector<datasets::KLine> klines_forward_adjusted_to_date(const std::string &code, const std::string &date);

}  // namespace factors

#endif  // QUANT1X_FACTOR_BASE_H
