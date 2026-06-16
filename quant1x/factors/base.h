#pragma once
#ifndef QUANT1X_FACTOR_BASE_H
#define QUANT1X_FACTOR_BASE_H 1

#include <quant1x/contrib/data/tdx/kline.h>
#include <quant1x/data/kline_raw.h>

namespace factors {

    // =============================
    // 复权因子模型: 仿射变换 P_adj = m * P + a
    // =============================

    // 累计复权因子
    struct CumulativeAdjustment {
        meta::Timestamp timestamp;             // 除权除息的毫秒数
        double              m;                     // 系数, 比例因子（乘法）
        double              a;                     // 偏移, 偏移因子（加法）
        double              monetaryAdjustment;    // 货币调整（例如每10股的货币调整）
        double              shareAdjustmentRatio;  // 股本调整比率（例如新增股/基数）
        int                 no;                    // 第几次
        // 为避免多个相邻元素在同一缓存行导致的伪共享，补齐到 64 字节（典型 cache-line 大小）
        // 重新排列字段后需要更多填充以保持 sizeof == 64
        // 保持结构体总大小为 64 字节
        char _cacheline_pad[20] = {0};

        std::string to_string() const {
            return fmt::format("{{no={},timestamp={},m={},a={},monetaryAdjustment={},shareAdjRatio={}}}",
                               no,
                               timestamp.only_date(),
                               m,
                               a,
                               monetaryAdjustment,
                               shareAdjustmentRatio);
        }

        // 将一个价格应用此次调整
        double apply(double price) const { return price * m + a; }

        // 返回此调整的逆变换（用于反向调整）
        double inverse(double adjusted_price) const { return (adjusted_price - a) / m; }
    };

    static_assert(sizeof(CumulativeAdjustment) == 64, "CumulativeAdjustment must be 64 bytes (cache-line aligned)");

    // 通过证券代码获取最新的除权除息列表
    std::span<const level1::XdxrInfo> get_xdxr_list(const std::string &security_code);
    // 从除权除息的列表提取IPO日期
    std::optional<std::string> ipo_date_from_xdxrs(std::span<const level1::XdxrInfo> xdxrs);
    // 聚合给定一个时间范围内的复权因子
    std::vector<CumulativeAdjustment> combine_adjustments_in_period(std::span<const level1::XdxrInfo> xdxrs,
                                                                    const meta::Timestamp        &start_date,
                                                                    const meta::Timestamp        &end_date);

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
    void apply_forward_adjustments_once(std::vector<T>                   &klines,
                                        std::span<const level1::XdxrInfo> xdxrs,
                                        const meta::Timestamp        &start_date,
                                        const meta::Timestamp        &end_date,
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
            auto current_date = meta::Timestamp(kline->date).pre_market_time();
            auto factor       = factors[i];
            if (current_date > ts_end) {
                break;
            }
            // 如果日线日期大于因子的日期, 因子索引+, 自动切换下一个因子
            // 考虑到可能存在长期停牌且停牌期间有除权除息记录的情况, 这种情况很少, 但还是可能会发生
            // 所以, 这里用了while循环, 直接找到最近的可以复权的记录
            // 注意：除权/除息事件在 "当日" 不应修改当日的 K 线，而是将之前的历史数据向前调整。
            // 因此当 current_date >= factor.timestamp 时我们会推进因子索引，使得当天的 K 线不使用当天发生的因子。
            // 换言之，事件生效于当日之前的行情（即历史数据被修改），当天数据保持不变。
            while (i + 1 < factors_count && current_date >= factor.timestamp) {
                ++i;
                factor = factors[i];
            }
            if (current_date < factor.timestamp) {
                kline->adjust(factor);
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

    /**
     * @brief 对K线数据进行前复权计算
     *
     * 根据提供的分红送配信息，对K线数据进行前复权处理，确保历史价格数据可比性。
     *
     * @param klines 需要复权的K线数据向量，会被直接修改
     * @param dividends 分红送配信息向量，包含除权除息数据
     *
     * @note 如果输入K线数据为空，函数将直接返回不做任何处理
     * @note 使用factors::apply_forward_adjustments_once实现实际复权计算
     */
    template <typename T>
    void calculate_pre_adjust(std::vector<T> &klines, const std::vector<level1::XdxrInfo> &dividends) {
        if (klines.empty()) {
            return;
        }
        // 使用apply_forward_adjustments_once进行前复权
        auto start_ts = meta::Timestamp(klines[0].Date).pre_market_time();
        auto end_ts   = meta::Timestamp(klines.back().Date).pre_market_time();
        apply_forward_adjustments_once(klines, dividends, start_ts, end_ts, true);
    }

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
