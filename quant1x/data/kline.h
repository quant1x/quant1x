#pragma once
#ifndef QUANT1X_DATA_KLINE_H
#define QUANT1X_DATA_KLINE_H 1

#include <quant1x/data/xdxr.h>

namespace data {

    // 日K线 结构体
    struct KLine {
        std::string date;                 // 日期
        double      open   = 0;           // 开盘价
        double      close  = 0;           // 收盘价
        double      high   = 0;           // 最高价
        double      low    = 0;           // 最低价
        double      volume = 0;           // 成交量(股)
        double      amount = 0;           // 成交金额(元)
        int         up     = 0;           // 上涨家数 / 外盘
        int         down   = 0;           // 下跌家数 / 内盘
        std::string datetime;             // 时间
        int         adjustment_count = 0; // 新增：除权除息次数

        void adjust(const factors::CumulativeAdjustment &adj);

        static std::vector<std::string> headers() {
            return {"date",
                    "open",
                    "close",
                    "high",
                    "low",
                    "volume",
                    "amount",
                    "up",
                    "down",
                    "datetime",
                    "adjustment_count"};
        }

        friend std::ostream &operator<<(std::ostream &os, const KLine &line) {
            os << "date: " << line.date << " open: " << line.open << " close: " << line.close << " high: " << line.high
               << " low: " << line.low << " volume: " << line.volume << " amount: " << line.amount << " up: " << line.up
               << " down: " << line.down << " datetime: " << line.datetime
               << " adjustment_count: " << line.adjustment_count;
            return os;
        }
    };

    std::vector<KLine> read_kline_from_csv(const std::string &filename);
    std::vector<KLine> load_kline(const std::string &code);

    class DataKLine : public data::DataAdapter {
    public:
        data::Kind Kind() const override { return BaseKLine; }

        std::string Owner() override { return data::DefaultDataProvider; }

        std::string Key() const override { return "day"; }

        std::string Name() const override { return "日K线"; }

        std::string Usage() const override { return "日K线"; }

        void Print(const std::string &code, const std::vector<exchange::timestamp> &dates) override;

        void Update(const std::string &code, const exchange::timestamp &date) override;
    };

}  // namespace data

#endif  // QUANT1X_DATA_KLINE_H
