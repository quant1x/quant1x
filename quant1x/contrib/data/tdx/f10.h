#pragma once
#ifndef QUANT1X_TDX_F10_ADAPTER_H
#define QUANT1X_TDX_F10_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/factors/f10.h>

namespace tdx {

    /// F10因子数据适配器 (对齐 Python DataF10)
    /// 使用 FeatureAdapter 接口, 支持 clone/headers/values 用于 CSV 聚合输出
    class DataF10 : public data::FeatureAdapter {
    private:
        F10 f10;
    public:
        DataF10() = default;
        DataF10(const DataF10&) = default;

        data::Kind Kind() const override;
        std::string Owner() override;
        std::string Key() const override;
        std::string Name() const override;
        std::string Usage() const override;

        std::vector<std::string> headers() const override;
        std::vector<std::string> values() const override;

        std::unique_ptr<data::FeatureAdapter> clone() const override;

        void init(const meta::Timestamp& timestamp) override;
        void Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates = {}) override;
        void Update(const meta::Instrument& inst, const meta::Timestamp& date = meta::Timestamp()) override;
    };

} // namespace tdx

#endif // QUANT1X_TDX_F10_ADAPTER_H
