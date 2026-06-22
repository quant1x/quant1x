#pragma once
#ifndef QUANT1X_TDX_TRANS_ADAPTER_H
#define QUANT1X_TDX_TRANS_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/data/base.h>
#include <quant1x/data/schema/trade.h>
#include <quant1x/data/meta/timestamp.h>
#include <cstdint>
#include <string>
#include <vector>

namespace quant1x::contrib::data::tdx {

    class DataTrans : public quant1x::data::DataAdapter {
    public:
        quant1x::data::Kind Kind() const override { return quant1x::data::BaseTransaction; }
        std::string Owner() const override { return quant1x::data::DefaultDataProvider; }
        std::string Key() const override { return "trans"; }
        std::string Name() const override { return "历史成交"; }
        std::string Usage() const override { return "历史成交"; }

        void Print(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) override;
        void Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) override;
    };

    // 成交统计概要 (对应 Go TurnoverDataSummary)
    struct TurnoverDataSummary {
        int64_t OuterVolume = 0;
        double  OuterAmount = 0.0;
        int64_t InnerVolume = 0;
        double  InnerAmount = 0.0;
        int64_t OpenVolume  = 0;
        double  OpenTurnZ   = 0.0;
        int64_t CloseVolume = 0;
        double  CloseTurnZ  = 0.0;

        friend std::ostream& operator<<(std::ostream& os, const TurnoverDataSummary& s) {
            os << "otv:" << s.OuterVolume << " ota:" << s.OuterAmount
               << " inv:" << s.InnerVolume << " ina:" << s.InnerAmount
               << " opnv:" << s.OpenVolume << " opnz:" << s.OpenTurnZ
               << " clsv:" << s.CloseVolume << " clsz:" << s.CloseTurnZ;
            return os;
        }
    };

    // 检出指定日期的逐笔成交数据
    std::vector<quant1x::data::schema::Transaction> CheckoutTransactionData(
        const std::string& code, const quant1x::data::meta::Timestamp& date, bool ignorePreviousData);

    // 计算成交额/成交量汇总
    TurnoverDataSummary CountInflow(
        const std::vector<quant1x::data::schema::Transaction>& list,
        const std::string& securityCode,
        const quant1x::data::meta::Timestamp& featureDate);

} // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_TDX_TRANS_ADAPTER_H
