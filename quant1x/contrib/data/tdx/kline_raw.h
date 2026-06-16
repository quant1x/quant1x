#pragma once
#ifndef QUANT1X_TDX_KLINE_RAW_ADAPTER_H
#define QUANT1X_TDX_KLINE_RAW_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/data/base.h>

namespace tdx {

    class DataKLineRaw : public data::DataAdapter {
    public:
        data::Kind Kind() const override { return data::BaseRawDailyKLine; }
        std::string Owner() override { return data::DefaultDataProvider; }
        std::string Key() const override { return "day_raw"; }
        std::string Name() const override { return "日K线RAW"; }
        std::string Usage() const override { return "日K线RAW数据适配器"; }

        void Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates = {}) override;
        void Update(const meta::Instrument& inst, const meta::Timestamp& date = meta::Timestamp()) override;
    };

} // namespace tdx

#endif // QUANT1X_TDX_KLINE_RAW_ADAPTER_H
