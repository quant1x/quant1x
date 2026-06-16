#pragma once
#ifndef QUANT1X_TDX_KLINE_MINUTE_ADAPTER_H
#define QUANT1X_TDX_KLINE_MINUTE_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/data/base.h>

namespace tdx {

    class DataMinuteKLine : public data::DataAdapter {
    public:
        data::Kind Kind() const override { return data::BaseMinuteKLine; }
        std::string Owner() override { return data::DefaultDataProvider; }
        std::string Key() const override { return "kline_minute"; }
        std::string Name() const override { return "分钟K线"; }
        std::string Usage() const override { return "分钟K线"; }

        void Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates = {}) override;
        void Update(const meta::Instrument& inst, const meta::Timestamp& date = meta::Timestamp()) override;
    };

} // namespace tdx

#endif // QUANT1X_TDX_KLINE_MINUTE_ADAPTER_H
