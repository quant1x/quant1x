#pragma once
#ifndef QUANT1X_TDX_HISTORY_ADAPTER_H
#define QUANT1X_TDX_HISTORY_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/factors/history.h>

namespace tdx {

    /// 历史特征数据适配器 (对齐 Python — 暂未移植)
    class HistoryFeature : public data::FeatureAdapter {
    private:
        History history;
    public:
        HistoryFeature() = default;
        HistoryFeature(const HistoryFeature&) = default;

        data::Kind Kind() const override;
        std::string Owner() override;
        std::string Key() const override;
        std::string Name() const override;
        std::string Usage() const override;

        void Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates = {}) override;
        void Update(const meta::Instrument& inst, const meta::Timestamp& date = meta::Timestamp()) override;
        void init(const meta::Timestamp& timestamp) override;
        std::unique_ptr<data::FeatureAdapter> clone() const override;
        std::vector<std::string> headers() const override;
        std::vector<std::string> values() const override;
    };

} // namespace tdx

#endif // QUANT1X_TDX_HISTORY_ADAPTER_H
