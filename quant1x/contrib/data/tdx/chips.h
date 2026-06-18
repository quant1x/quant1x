#pragma once
#ifndef QUANT1X_TDX_CHIPS_ADAPTER_H
#define QUANT1X_TDX_CHIPS_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/data/base.h>

namespace quant1x::contrib::data::tdx {

    struct PriceLine {
        i32 price = 0;  // 价格, 单位厘
        f64 buy   = 0;  // 买入, 成交量, 单位股
        f64 sell  = 0;  // 卖出, 成交量, 单位股
    };

    class DataChips : public quant1x::data::DataAdapter {
    public:
        quant1x::data::Kind Kind() const override { return quant1x::data::BaseChipDistribution; }
        std::string Owner() override { return quant1x::data::DefaultDataProvider; }
        std::string Key() const override { return "chips"; }
        std::string Name() const override { return "筹码分布"; }
        std::string Usage() const override { return "筹码分布"; }

        void Print(const quant1x::data::meta::Instrument& inst, const std::vector<quant1x::data::meta::Timestamp>& dates = {}) override;
        void Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date = quant1x::data::meta::Timestamp()) override;
    };

} // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_TDX_CHIPS_ADAPTER_H
