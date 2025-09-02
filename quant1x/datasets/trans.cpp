#include <quant1x/datasets/trans.h>
#include <quant1x/level1/transaction_history.h>
#include <quant1x/cache.h>
#include <quant1x/encoding/csv.h>
#include <spdlog/spdlog.h>
#include <quant1x/factors/f10.h>
#include <filesystem>

namespace datasets {

    std::once_flag _historical_trading_data_once;
    std::mutex _historical_trading_data_mutex;
    exchange::timestamp _historical_trading_data_begin_date = exchange::timestamp(cache::trains_begin_date);

    static void lazyInitHistoricalTradingData() {
        exchange::timestamp date = exchange::timestamp(cache::trains_begin_date);
        _historical_trading_data_begin_date = date;
    }


    // 修改tick数据开始下载的日期
    static void UpdateBeginDateOfHistoricalTradingData(const std::string &date) {
        std::call_once(_historical_trading_data_once, lazyInitHistoricalTradingData);
        std::lock_guard<std::mutex> lock(_historical_trading_data_mutex);

        try {
            auto dt = exchange::timestamp(date);
            _historical_trading_data_begin_date = dt;
        } catch (...) {
            return;
        }
    }

    // 恢复默认的成交数据最早日期
    [[maybe_unused]] static void RestoreBeginDateOfHistoricalTradingData() {
        UpdateBeginDateOfHistoricalTradingData(cache::trains_begin_date);
    }

    // GetBeginDateOfHistoricalTradingData 获取系统默认的历史成交数据的最早日期
    static exchange::timestamp GetBeginDateOfHistoricalTradingData() {
        std::call_once(_historical_trading_data_once, lazyInitHistoricalTradingData);
        std::lock_guard<std::mutex> lock(_historical_trading_data_mutex);
        return _historical_trading_data_begin_date;
    }

    // 获取指定日期的分笔成交记录
    std::vector<level1::TickTransaction> CheckoutTransactionData(const std::string &securityCode,
                                                                 const exchange::timestamp &featureDate,
                                                                 bool ignorePreviousData) {
        std::vector<level1::TickTransaction> list;
        std::string correctedCode = exchange::CorrectSecurityCode(securityCode);
        // 对齐日期格式: YYYYMMDD
        u32 tradeDate = featureDate.yyyymmdd();

        if (ignorePreviousData) {
            // 在默认日期之前的数据直接返回空
            auto startDate = GetBeginDateOfHistoricalTradingData();
            if (tradeDate < startDate.yyyymmdd()) {
                spdlog::error("[dataset::trans] code={}, trade-date={}, start-date={}, 没有数据", correctedCode, tradeDate, startDate.toString());
                return list;
            }
        }

        std::string startTime = HistoricalTransactionDataFirstTime;
        std::string filename = config::get_historical_trade_filename(correctedCode, featureDate.only_date());

        if (std::filesystem::exists(filename)) {
            // 如果缓存存在
            list = encoding::csv::csv_to_slices<level1::TickTransaction>(filename);
            if (!list.empty()) {
                size_t cacheLength = list.size();
                if (cacheLength > 0) {
                    std::string lastTime = list.back().time;
                    if (lastTime == HistoricalTransactionDataLastTime) {
                        return list;
                    }

                    std::string firstTime;
                    size_t skipCount = 0;
                    for (size_t i = 1; i <= cacheLength; i++) {
                        std::string tm = list[cacheLength - i].time;
                        if (firstTime.empty()) {
                            firstTime = tm;
                            startTime = firstTime;
                            skipCount++;
                            continue;
                        }
                        if (tm < firstTime) {
                            startTime = firstTime;
                            break;
                        } else {
                            skipCount++;
                        }
                    }
                    // 截取 startTime之前的记录
                    list.resize(cacheLength - skipCount);
                }
            } else {
                spdlog::error("[dataset::trans] code={}, trade-date={}, 没有有效数据", correctedCode, tradeDate);
            }
        }
        auto today_is_last_trading_date = featureDate.is_same_date(exchange::last_trading_day());

        uint16_t offset = level1::tick_transaction_max;
        uint32_t u32Date = tradeDate;
        // 只求增量, 分笔成交数据是从后往前取数据, 缓存是从前到后顺序存取
        uint16_t start = 0;
        std::vector<level1::TickTransaction> history;
        std::vector<std::vector<level1::TickTransaction>> hs;
        auto [marketId, marketCode, pureCode] = exchange::DetectMarket(correctedCode);
        if(today_is_last_trading_date) {
            while (true) {
                try {
                    level1::TransactionRequest request(correctedCode, start, offset);
                    level1::TransactionResponse response(marketId, pureCode.c_str());
                    auto conn = level1::client();
                    level1::process(conn->socket(), request, response);

                    if (response.Count == 0 || response.List.empty()) {
                        break;
                    }

                    std::vector<level1::TickTransaction> tmp;
                    auto tmpList = response.List;
                    std::reverse(tmpList.begin(), tmpList.end());
                    for (const auto &td: tmpList) {
                        // 追加包含startTime之后的记录
                        if (td.time >= startTime) {
                            tmp.emplace_back(td);
                        }
                    }
                    std::reverse(tmp.begin(), tmp.end());
                    hs.emplace_back(tmp);

                    if (tmp.size() < offset) {
                        // 已经是最早的记录
                        // 需要排序
                        break;
                    }
                    start += offset;
                } catch (const std::exception &e) {
                    spdlog::error("[dataset::trans] code={}, tradeDate={}, error={}", correctedCode, tradeDate,
                                  e.what());
                    break;
                }
            }
        } else {
            while (true) {
                try {
                    level1::HistoryTransactionRequest request(correctedCode, u32Date, start, offset);
                    level1::HistoryTransactionResponse response(marketId, pureCode.c_str());
                    auto conn = level1::client();
                    level1::process(conn->socket(), request, response);

                    if (response.Count == 0 || response.List.empty()) {
                        break;
                    }

                    std::vector<level1::TickTransaction> tmp;
                    auto tmpList = response.List;
                    std::reverse(tmpList.begin(), tmpList.end());
                    for (const auto &td: tmpList) {
                        // 追加包含startTime之后的记录
                        if (td.time >= startTime) {
                            tmp.emplace_back(td);
                        }
                    }
                    std::reverse(tmp.begin(), tmp.end());
                    hs.emplace_back(tmp);

                    if (tmp.size() < offset) {
                        // 已经是最早的记录
                        // 需要排序
                        break;
                    }
                    start += offset;
                } catch (const std::exception &e) {
                    spdlog::error("[dataset::trans] code={}, tradeDate={}, error={}", correctedCode, tradeDate,
                                  e.what());
                    break;
                }
            }
        }

        // 这里需要反转一下
        std::reverse(hs.begin(), hs.end());
        for (const auto &v: hs) {
            history.insert(history.end(), v.begin(), v.end());
        }

        if (history.empty()) {
            return list;
        }

        list.insert(list.end(), history.begin(), history.end());
        encoding::csv::slices_to_csv(list, filename);
        return list;
    }

    // 统计指定日期的内外盘
    TurnoverDataSummary CountInflow(const std::vector<level1::TickTransaction>& list,
                                    const std::string& securityCode,
                                    const exchange::timestamp& featureDate) {
        TurnoverDataSummary summary;

        if (list.empty()) {
            return summary;
        }

        std::string correctedCode = exchange::CorrectSecurityCode(securityCode);
        double lastPrice = 0.0;

        for (const auto& v : list) {
            std::string tm = v.time;
            int64_t direction = v.buyOrSell;
            double price = v.price;

            if (lastPrice == 0) {
                lastPrice = price;
            }

            int64_t vol = v.vol;

            if (direction != level1::tick_buy && direction != level1::tick_sell) {
                if (price > lastPrice) {
                    direction = level1::tick_buy;
                } else if (price < lastPrice) {
                    direction = level1::tick_sell;
                }
            }

            // 统计内外盘数据
            if (direction == level1::tick_buy) {
                // 买入
                summary.OuterVolume += vol;
                summary.OuterAmount += static_cast<double>(vol) * price;
            } else if (direction == level1::tick_sell) {
                // 卖出
                summary.InnerVolume += vol;
                summary.InnerAmount += static_cast<double>(vol) * price;
            } else {
                // 可能存在中性盘2, 最近又发现有类型是3, 暂时还是按照中性盘来处理
                int64_t vn = vol;
                int64_t buyOffset = vn / 2;
                int64_t sellOffset = vn - buyOffset;

                // 买入
                summary.OuterVolume += buyOffset;
                summary.OuterAmount += static_cast<double>(buyOffset) * price;
                // 卖出
                summary.InnerVolume += sellOffset;
                summary.InnerAmount += static_cast<double>(sellOffset) * price;
            }

            // 计算开盘竞价数据
            if (tm >= HistoricalTransactionDataFirstTime &&
                tm < HistoricalTransactionDataStartTime) {
                summary.OpenVolume += vol;
            }

            // 计算收盘竞价数据
            if (tm > HistoricalTransactionDataFinalBiddingTime &&
                tm <= HistoricalTransactionDataLastTime) {
                summary.CloseVolume += vol;
            }

            lastPrice = price;
        }

        auto f10 = factors::get_f10(correctedCode, featureDate);
        if (f10.has_value()) {
            summary.OpenTurnZ = f10->TurnZ(f64(summary.OpenVolume));
            summary.CloseTurnZ = f10->TurnZ(f64(summary.CloseVolume));
        }

        return summary;
    }

    void DataTrans::Print(const std::string &code, const std::vector<exchange::timestamp> &dates) {
        (void)code;
        (void)dates;
    }

    void DataTrans::Update(const std::string &code, const exchange::timestamp &date) {
        CheckoutTransactionData(code, date, false);
    }
} // namespace datasets