#pragma once
#ifndef QUANT1X_TDX_XDXR_ADAPTER_H
#define QUANT1X_TDX_XDXR_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/data/base.h>
#include <quant1x/data/meta/timestamp.h>
#include <vector>
#include <string>

namespace quant1x::contrib::data::tdx {

    class DataXdxr : public quant1x::data::DataAdapter {
    public:
        quant1x::data::Kind Kind() const override { return quant1x::data::BaseXdxr; }
        std::string Owner() override { return quant1x::data::DefaultDataProvider; }
        std::string Key() const override { return "xdxr"; }
        std::string Name() const override { return "除权除息"; }
        std::string Usage() const override { return ""; }

        void Print(const quant1x::data::meta::Instrument& inst, const std::vector<quant1x::data::meta::Timestamp>& dates = {}) override;
        void Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date = quant1x::data::meta::Timestamp()) override;
    };

} // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_TDX_XDXR_ADAPTER_H
