#pragma once
#ifndef QUANT1X_CONTRIB_DATA_TDX_HISTORY_ADAPTER_H
#define QUANT1X_CONTRIB_DATA_TDX_HISTORY_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/factors/history.h>

namespace quant1x::contrib::data::tdx {

    /// 历史特征数据适配器 (对齐 Python — 暂未移植)
    class HistoryFeature : public quant1x::data::FeatureAdapter {
    private:
        History history;
    public:
        HistoryFeature() = default;
        HistoryFeature(const HistoryFeature&) = default;

        quant1x::data::Kind Kind() const override;
        std::string Owner() override;
        std::string Key() const override;
        std::string Name() const override;
        std::string Usage() const override;

        void Print(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) override;
        void Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) override;
        void init(const quant1x::data::meta::Timestamp& timestamp) override;
        std::unique_ptr<quant1x::data::FeatureAdapter> clone() const override;
        std::vector<std::string> headers() const override;
        std::vector<std::string> values() const override;
    };

} // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_CONTRIB_DATA_TDX_HISTORY_ADAPTER_H
