#pragma once
#ifndef QUANT1X_FACTOR_HISTORY_H
#define QUANT1X_FACTOR_HISTORY_H 1

#include <quant1x/base/api.h>
#include <quant1x/data/adapter.h>
#include <quant1x/factors/base_compat.h>
#include <quant1x/contrib/data/tdx/bar.h>

struct History {
    std::string Date;                 // 日期, 数据落地的日期
    std::string Code;                 // 代码
    double      MA2;                  // 2日均价
    double      MA3;                  // 3日均价
    double      MV3;                  // 3日均量
    double      MA4;                  // 4日均价
    double      MA5;                  // 5日均价
    double      MV5;                  // 5日均量
    double      MA9;                  // 9日均价
    double      MV9;                  // 9日均量
    double      MA10;                 // 10日均价
    double      MV10;                 // 10日均量
    double      MA19;                 // 19日均价
    double      MV19;                 // 19日均量
    double      MA20;                 // 20日均价
    double      MV20;                 // 20日均量
    double      OPEN;                 // 昨日开盘
    double      CLOSE;                // 昨日收盘
    double      HIGH;                 // 昨日最高
    double      LOW;                  // 昨日最低
    double      VOL;                  // 昨日成交量
    double      AMOUNT;               // 昨日成交额
    double      AveragePrice;         // 昨日均价
    double      LastClose;            // 前日收盘
    int         BullN;                // 多头排列周期
    int         UpwardN;              // 向上跳空周期数
    int         NewHighN;             // 新高次数
    int         NewNoLowN;            // 低点提高次数
    int         NewLowN;              // 新低次数
    int64_t     OpenVolume;           // 开盘量
    int         AdjustmentCount;      // 新增: 除权除息次数
    std::string UpdateTime;           // 更新时间
    uint64_t    State;                // 样本状态

    /**
     * @brief 前复权
     * @param m 系数
     * @param a 偏移
     * @param number 第几次除权除息
     */
    void adjust(const factors::CumulativeAdjustment &adj);

    friend std::ostream &operator<<(std::ostream &os, const History &history);
};

class HistoryFeature : public quant1x::data::FeatureAdapter {
private:
    History history;

public:
    HistoryFeature()                       = default;
    HistoryFeature(const HistoryFeature &) = default;
    quant1x::data::Kind Kind() const override;

    std::string Owner() const override;

    std::string Key() const override;

    std::string Name() const override;

    std::string Usage() const override;

    void Print(const quant1x::data::meta::Instrument &inst, const quant1x::data::meta::Timestamp &date) override;

    void Update(const quant1x::data::meta::Instrument &inst, const quant1x::data::meta::Timestamp &date) override;

    void init(const quant1x::data::meta::Timestamp &timestamp) override;

    std::unique_ptr<FeatureAdapter> clone() const override;

    std::vector<std::string> headers() const override;

    std::vector<std::string> values() const override;
};

namespace factors {
    /// 获取指定日期的History数据
    std::optional<History> get_history(const std::string &code, const quant1x::data::meta::Timestamp &timestamp);
}  // namespace factors

#endif  // QUANT1X_FACTOR_HISTORY_H
