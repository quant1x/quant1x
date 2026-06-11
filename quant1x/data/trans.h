#pragma once
#ifndef QUANT1X_DATA_TRANS_H
#define QUANT1X_DATA_TRANS_H 1

#include <quant1x/data/base.h>
#include <quant1x/contrib/data/tdx/level1/transaction_data.h>


namespace data {
    constexpr const char *const HistoricalTransactionDataFirstTime        = "09:25"; // 第一个时间
    constexpr const char *const HistoricalTransactionDataStartTime        = "09:30"; // 开盘时间
    constexpr const char *const HistoricalTransactionDataFinalBiddingTime = "14:57"; // 尾盘集合竞价时间
    constexpr const char *const HistoricalTransactionDataLastTime         = "15:00"; // 最后一个时间

    // CheckoutTransactionData 获取指定日期的分笔成交记录
    std::vector<level1::TickTransaction> CheckoutTransactionData(const std::string &securityCode,
                                                                 const exchange::timestamp &featureDate,
                                                                 bool ignorePreviousData);

    /**
     * @brief 换手数据
     */
    struct TurnoverDataSummary {
        int64_t OuterVolume = 0;
        double OuterAmount = 0.0;
        int64_t InnerVolume = 0;
        double InnerAmount = 0.0;
        int64_t OpenVolume = 0;
        double OpenTurnZ = 0.0;
        int64_t CloseVolume = 0;
        double CloseTurnZ = 0.0;

        friend std::ostream &operator<<(std::ostream &os, const TurnoverDataSummary &summary) {
            os << "OuterVolume: " << summary.OuterVolume << " OuterAmount: " << summary.OuterAmount << " InnerVolume: "
               << summary.InnerVolume << " InnerAmount: " << summary.InnerAmount << " OpenVolume: "
               << summary.OpenVolume << " OpenTurnZ: " << summary.OpenTurnZ << " CloseVolume: " << summary.CloseVolume
               << " CloseTurnZ: " << summary.CloseTurnZ;
            return os;
        }
    };

    // 统计指定日期的内外盘
    TurnoverDataSummary CountInflow(const std::vector<level1::TickTransaction>& list,
                                    const std::string& securityCode,
                                    const exchange::timestamp& featureDate);

    class DataTrans : public data::DataAdapter {
    public:
        data::Kind Kind() const override { return BaseTransaction; }

        std::string Owner() override { return data::DefaultDataProvider; }
        std::string Key() const override { return "trans"; }

        std::string Name() const override { return "历史成交"; }

        std::string Usage() const override { return "历史成交"; }

        void Print(const std::string &code, const std::vector<exchange::timestamp> &dates) override;

        void Update(const std::string &code, const exchange::timestamp &date) override;
    };

} // namespace data

#endif //QUANT1X_DATA_TRANS_H
