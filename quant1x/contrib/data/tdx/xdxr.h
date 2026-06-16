#pragma once
#ifndef QUANT1X_TDX_XDXR_ADAPTER_H
#define QUANT1X_TDX_XDXR_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/data/base.h>
#include <quant1x/data/meta/timestamp.h>
#include <vector>
#include <string>

namespace tdx {

    class DataXdxr : public data::DataAdapter {
    public:
        data::Kind Kind() const override { return data::BaseXdxr; }
        std::string Owner() override { return data::DefaultDataProvider; }
        std::string Key() const override { return "xdxr"; }
        std::string Name() const override { return "除权除息"; }
        std::string Usage() const override { return ""; }

        void Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates = {}) override;
        void Update(const meta::Instrument& inst, const meta::Timestamp& date = meta::Timestamp()) override;
    };

} // namespace tdx

#endif // QUANT1X_TDX_XDXR_ADAPTER_H
