#pragma once
#ifndef QUANT1X_TDX_F10_ADAPTER_H
#define QUANT1X_TDX_F10_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/factors/f10.h>

namespace quant1x::contrib::data::tdx {

    /// F10因子数据适配器 (对齐 Python DataF10)
    /// 使用 FeatureAdapter 接口, 支持 clone/headers/values 用于 CSV 聚合输出
    class DataF10 : public quant1x::data::FeatureAdapter {
    private:
        F10 f10;
    public:
        DataF10() = default;
        DataF10(const DataF10&) = default;

        quant1x::data::Kind Kind() const override;
        std::string Owner() override;
        std::string Key() const override;
        std::string Name() const override;
        std::string Usage() const override;

        std::vector<std::string> headers() const override;
        std::vector<std::string> values() const override;

        std::unique_ptr<quant1x::data::FeatureAdapter> clone() const override;

        void init(const quant1x::data::meta::Timestamp& timestamp) override;
        void Print(const quant1x::data::meta::Instrument& inst, const std::vector<quant1x::data::meta::Timestamp>& dates = {}) override;
        void Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date = quant1x::data::meta::Timestamp()) override;
    };

} // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_TDX_F10_ADAPTER_H
