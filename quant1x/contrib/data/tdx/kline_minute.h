#pragma once
#ifndef QUANT1X_TDX_KLINE_MINUTE_ADAPTER_H
#define QUANT1X_TDX_KLINE_MINUTE_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/data/base.h>

namespace quant1x::contrib::data::tdx {

    class DataMinuteKLine : public quant1x::data::DataAdapter {
    public:
        quant1x::data::Kind Kind() const override { return quant1x::data::BaseMinuteKLine; }
        std::string Owner() override { return quant1x::data::DefaultDataProvider; }
        std::string Key() const override { return "kline_minute"; }
        std::string Name() const override { return "分钟K线"; }
        std::string Usage() const override { return "分钟K线"; }

        void Print(const quant1x::data::meta::Instrument& inst, const std::vector<quant1x::data::meta::Timestamp>& dates = {}) override;
        void Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date = quant1x::data::meta::Timestamp()) override;
    };

} // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_TDX_KLINE_MINUTE_ADAPTER_H
