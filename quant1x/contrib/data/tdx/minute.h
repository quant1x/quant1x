#pragma once
#ifndef QUANT1X_TDX_MINUTE_ADAPTER_H
#define QUANT1X_TDX_MINUTE_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/data/base.h>

namespace quant1x::contrib::data::tdx {

    class DataMinute : public quant1x::data::DataAdapter {
    public:
        quant1x::data::Kind Kind() const override { return quant1x::data::BaseMinutes; }
        std::string Owner() const override { return quant1x::data::DefaultDataProvider; }
        std::string Key() const override { return "minute"; }
        std::string Name() const override { return "分时数据"; }
        std::string Usage() const override { return "分时数据"; }

        void Print(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) override;
        void Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) override;
    };

} // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_TDX_MINUTE_ADAPTER_H
