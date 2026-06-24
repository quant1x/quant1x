#pragma once
#ifndef QUANT1X_FEATURES_NO0_H
#define QUANT1X_FEATURES_NO0_H 1

#include <quant1x/std/base.h>
#include <quant1x/data/adapter.h>
#include <quant1x/factors/base_compat.h>

namespace data = quant1x::data;
namespace meta = quant1x::data::meta;

/// 0号策略特征工程结构体
/// 0号作为演示策略, 仅供学习和参考, 不作为投资建议, 请勿直接使用
/// 0号策略: 5日均线向上突破, 且5日均线在10日均线上方, 则买入, 否则卖出
struct No0 {
    std::string Date;        // 日期, 数据落地的日期
    std::string Code;        // 代码
    f64         ma4;         // 4日均线
    f64         ma5;         // 5日均线
    f64         ma9;         // 9日均线
    f64         ma10;        // 10日均线
    std::string UpdateTime;  // 更新时间
    uint64_t    State;       // 样本状态
};

class DataNo0 : public data::FeatureAdapter {
private:
    No0 feature;

public:
    DataNo0()                   = default;
    DataNo0(const DataNo0 &) = default;
    data::Kind Kind() const override { return factors::FeatureNo0; }

    std::string Owner() const override { return data::DefaultDataProvider; }

    std::string Key() const override { return "no0"; }

    std::string Name() const override { return "0号策略"; }

    std::string Usage() const override { return "no0"; }

    void Print(const meta::Instrument &inst, const meta::Timestamp &date) override;

    void Update(const meta::Instrument &inst, const meta::Timestamp &date) override;

    void init(const meta::Timestamp &timestamp) override;

    std::unique_ptr<FeatureAdapter> clone() const override;

    std::vector<std::string> headers() const override;

    std::vector<std::string> values() const override;
};

namespace factors {
    /// 获取指定日期的No0数据
    std::optional<No0> get_no0(const std::string &code, const meta::Timestamp &timestamp);
}  // namespace factors

#endif  // QUANT1X_FEATURES_NO0_H