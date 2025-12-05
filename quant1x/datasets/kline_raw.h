#pragma once
#ifndef QUANT1X_DATASETS_KLINE_RAW_H
#define QUANT1X_DATASETS_KLINE_RAW_H 1

#include <quant1x/datasets/xdxr.h>

namespace datasets {

    namespace detail {
        // 日线最小容错回溯(偏移)天数
        constexpr const size_t MAX_KLINE_LOOKBACK_DAYS = 1;
        constexpr const int    CN_DEFAULT_TOTALFZNUM   = 240;  // A股默认全天交易240分钟
        // 拉取数据
        std::vector<level1::SecurityBar> fetch_kline(const std::string &code,
                                                     u16                start,
                                                     u16                count,
                                                     level1::KLineType  kline_type = level1::KLineType::RI_K);

        /**
         * @brief 对K线数据应用前复权调整
         *
         * 根据除权除息信息对历史K线数据进行前复权处理，调整开盘价、收盘价、最高价、最低价和成交量。
         * 复权处理会跳过IPO上市日期之前的除权除息数据。
         *
         * @param klines 待复权的K线数据向量，会被原地修改
         * @param event_start_date 事件开始日期，用于过滤IPO上市前的除权除息数据
         * @param dividends 除权除息信息列表，包含日期和调整因子等信息
         *
         * @note 除权除息处理不包括除权除息当日的数据，只调整该日期之前的历史数据
         * @note 成交量复权采用"成交金额/复权后均价"的方式计算
         * @note 每条K线会记录调整次数(AdjustmentCount)
         */
        template <typename K>
        void apply_forward_adjustment_for_event(std::vector<K>                      &klines,
                                                const exchange::timestamp           &event_start_date,
                                                const std::vector<level1::XdxrInfo> &dividends) {
            if (klines.empty()) {
                return;
            }
            // 最后一根K线的日期
            auto const &last_day = klines[klines.size() - 1].Date;
            // 转成时间戳且对齐时间
            auto const &ts_last_day = exchange::timestamp::parse(last_day).pre_market_time();
            // 计算最后一根K线的下一个交易日的日期, 除权除息是不包括除权除息当日的,
            // 所以要计算下一个交易日与除权除息的列表去匹配 300773拉卡拉, 2025年6月6日除权, 数据公布于6月3日之前,
            // 那么在6月6日之前的6月4日收盘前是不能除权除息的，6月5日收盘可以除权
            auto const &last_day_next = exchange::next_trading_day(ts_last_day).only_date();
            auto        start_date    = event_start_date.only_date();
            auto        xdxr_infos    = dividends | std::views::filter([&last_day_next](const level1::XdxrInfo &x) {
                                  return last_day_next >= x.Date && x.Category == 1;
                              });
            // int times = 0; // 除权除息次数
            size_t count = std::ranges::distance(xdxr_infos);  // 除权除息总次数
            // 时间越早的记录除权除息次数越多, 第一条数据时时总的除权除息次数
            auto times = count;
            for (auto const &info : xdxr_infos) {
                if (info.Date <= start_date) {
                    // 除权除息数据在日线第一条数据之前, 也就是ipo上市日期之前的数据, 不能用作复权
                    // continue;
                } else {
                    auto [m, a]      = info.adjustFactor();
                    auto share_ratio = info.computeShareAdjustmentRatio();
                    auto klines_size = klines.size();
                    for (size_t i = 0; i < klines_size; ++i) {
                        auto kl = &(klines[i]);
                        if (kl->Date >= info.Date) {
                            break;
                        }
                        if (kl->Date < info.Date) {
                            kl->Open  = kl->Open * m + a;
                            kl->Close = kl->Close * m + a;
                            kl->High  = kl->High * m + a;
                            kl->Low   = kl->Low * m + a;
                            // 成交量前复权
                            // 1. 计算均价
                            auto ap = kl->Amount / kl->Volume;
                            // 2. 均价复权
                            auto ap_adjusted = ap * m + a;
                            // 3. 成交量复权
                            kl->Volume *= (1 + share_ratio);
                            // 4. 重新计算成交金额
                            kl->Amount = kl->Volume * ap_adjusted;
                            // kl->Amount = kl->Volume * ((kl->Amount / kl->Volume) * m + a);
                            //kl->Amount = kl->Amount * (m +a);
                            // 5. 更新除权除息次数
                            kl->AdjustmentCount += 1;
                        }
                    }
                }
                --times;
                (void)times;
            }
        }
    }  // namespace detail

    // 日K线 结构体
    struct KLineRaw {
        std::string Date;        // 日期
        double      Open   = 0;  // 开盘价
        double      Close  = 0;  // 收盘价
        double      High   = 0;  // 最高价
        double      Low    = 0;  // 最低价
        double      Volume = 0;  // 成交量(股)
        double      Amount = 0;  // 成交金额(元)
        int         Up     = 0;  // 上涨家数 / 外盘
        int         Down   = 0;  // 下跌家数 / 内盘
        std::string Datetime;    // 时间

        // // 复权
        // void adjust(const factors::CumulativeAdjustment &adj);

        static std::vector<std::string> headers() {
            return {"Date", "Open", "Close", "High", "Low", "Volume", "Amount", "Up", "Down", "Datetime"};
        }

        friend std::ostream &operator<<(std::ostream &os, const KLineRaw &line) {
            os << "Date: " << line.Date << " Open: " << line.Open << " Close: " << line.Close << " High: " << line.High
               << " Low: " << line.Low << " Volume: " << line.Volume << " Amount: " << line.Amount << " Up: " << line.Up
               << " Down: " << line.Down << " Datetime: " << line.Datetime;
            return os;
        }
    };

    // 加载原始K线
    std::vector<KLineRaw> load_kline_raw(const std::string &code);

    class DataKLineRaw : public cache::DataAdapter {
    public:
        cache::Kind Kind() const override { return RawKLine; }

        std::string Owner() override { return cache::DefaultDataProvider; }

        std::string Key() const override { return "day_raw"; }

        std::string Name() const override { return "日K线RAW"; }

        std::string Usage() const override { return "日K线RAW"; }

        void Print(const std::string &code, const std::vector<exchange::timestamp> &dates) override;

        void Update(const std::string &code, const exchange::timestamp &date) override;
    };

}  // namespace datasets

#endif  // QUANT1X_DATASETS_KLINE_RAW_H
