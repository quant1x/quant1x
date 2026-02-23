#include <quant1x/data/trans.h>
#include <quant1x/level1/transaction_history.h>
#include <quant1x/cache.h>
#include <quant1x/encoding/csv.h>
#include <spdlog/spdlog.h>
#include <quant1x/factors/f10.h>
#include <filesystem>
#include <quant1x/config/cache.h>
#include <quant1x/std/filesystem.h>

namespace data {

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

    // 从缓存加载分笔成交数据
    static std::pair<std::vector<level1::TickTransaction>, std::string> LoadTransactionDataFromCache(const std::string &correctedCode,
                                                                                                     const exchange::timestamp &featureDate,
                                                                                                     bool ignorePreviousData) {
        std::vector<level1::TickTransaction> list;
        u32 tradeDate = featureDate.yyyymmdd();

            if (ignorePreviousData) {
            // 在默认日期之前的数据直接返回空
            auto startDate = GetBeginDateOfHistoricalTradingData();
            if (tradeDate < startDate.yyyymmdd()) {
                    spdlog::error("[data::trans] code={}, trade-date={}, start-date={}, 没有数据", correctedCode, tradeDate, startDate.toString());
                return {list, HistoricalTransactionDataFirstTime};
            }
        }

        std::string startTime = HistoricalTransactionDataFirstTime;
        std::string filename = config::get_historical_trade_filename(correctedCode, featureDate.only_date());

        if (std::filesystem::exists(filename)) {
            // 如果缓存存在
            // list = encoding::csv::csv_to_slices<level1::TickTransaction>(filename);  // 屏蔽，使用io::CSVReader解析
            io::CSVReader<6> in(filename);
            in.read_header(io::ignore_extra_column, "time", "price", "volume", "number", "amount", "buy_or_sell");
            std::string time;
            f64 price;
            i64 vol, num;
            f64 amount;
            i64 buy_or_sell;
            while (in.read_row(time, price, vol, num, amount, buy_or_sell)) {
                list.emplace_back(time, price, vol, num, amount, buy_or_sell);
            }
            if (!list.empty()) {
                size_t cacheLength = list.size();
                if (cacheLength > 0) {
                    std::string lastTime = list.back().time;
                    if (lastTime == HistoricalTransactionDataLastTime) {
                        return {list, startTime};  // 数据完整，直接返回
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
                spdlog::error("[data::trans] code={}, trade-date={}, 没有有效数据", correctedCode, tradeDate);
            }
        }

        return {list, startTime};
    }

    // 更新分笔成交数据到缓存
    static void UpdateTransactionData(const std::string &correctedCode,
                                      const exchange::timestamp &featureDate,
                                      const std::string& startTime) {
        u32 tradeDate = featureDate.yyyymmdd();
        auto today_is_last_trading_date = featureDate.is_same_date(exchange::last_trading_day());

        uint16_t offset = level1::tick_transaction_per_request_max;
        uint32_t u32Date = tradeDate;
        // 只求增量, 分笔成交数据是从后往前取数据, 缓存是从前到后顺序存取
        uint16_t start = 0;
        std::vector<level1::TickTransaction> history;
        std::vector<std::vector<level1::TickTransaction>> hs;
        auto [marketId, marketCode, pureCode] = exchange::DetectMarket(correctedCode);

        if (today_is_last_trading_date) {
            while (true) {
                level1::TransactionRequest request(correctedCode, start, offset);
                level1::TransactionResponse response(static_cast<int>(marketId), pureCode.c_str());
                auto conn = level1::get_std_conn();
                auto err = level1::process(conn->socket(), request, response);
                if (err) {
                    spdlog::error("[data::trans] code={}, tradeDate={}, error={}", correctedCode, tradeDate, std::string(err.message()));
                    break;
                }

                if (response.Count == 0 || response.List.empty()) {
                    break;
                }

                std::vector<level1::TickTransaction> tmp{};
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
            }
        } else {
            while (true) {
                level1::HistoryTransactionRequest request(correctedCode, u32Date, start, offset);
                level1::HistoryTransactionResponse response(static_cast<int>(marketId), pureCode.c_str());
                auto conn = level1::get_std_conn();
                auto err = level1::process(conn->socket(), request, response);
                if (err) {
                    spdlog::error("[data::trans] code={}, tradeDate={}, error={}", correctedCode, tradeDate, std::string(err.message()));
                    break;
                }

                if (response.Count == 0 || response.List.empty()) {
                    break;
                }

                std::vector<level1::TickTransaction> tmp{};
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
            }
        }

        // 这里需要反转一下
        std::reverse(hs.begin(), hs.end());
        for (const auto &v: hs) {
            history.insert(history.end(), v.begin(), v.end());
        }

        if (history.empty()) {
            return;  // 无新数据
        }

        // 加载现有缓存数据并合并
        auto [existingList, _] = LoadTransactionDataFromCache(correctedCode, featureDate, false);
        existingList.insert(existingList.end(), history.begin(), history.end());

        // 手动写入CSV，避免packed结构体问题
        std::string filename = config::get_historical_trade_filename(correctedCode, featureDate.only_date());
        std::string tmp = filename + ".tmp";
        auto ec = filesystem::check_filepath(tmp, true);
        ec.clear();  // 忽略错误，已处理
        {
            io::CSVWriter writer(tmp);
            // 写入header，与reader对应
            writer.write_row("time", "price", "volume", "number", "amount", "buy_or_sell");
            // 写入数据
            for (const auto& rec : existingList) {
                writer.write_row(rec.time, rec.price, rec.vol, rec.num, rec.amount, rec.buyOrSell);
            }
        }
        // 原子重命名
            try {
            std::filesystem::rename(tmp, filename);
        } catch (const std::filesystem::filesystem_error& e) {
            spdlog::error("[data::trans] 重命名失败: {} -> {}: {}", tmp, filename, e.what());
            // 尝试删除临时文件
            std::filesystem::remove(tmp);
        }
    }

    // 确保分笔成交数据已更新到缓存
    static void EnsureTransactionDataUpdated(const std::string &correctedCode,
                                             const exchange::timestamp &featureDate,
                                             bool ignorePreviousData) {
        auto [list, startTime] = LoadTransactionDataFromCache(correctedCode, featureDate, ignorePreviousData);
        bool needsUpdate = list.empty() || (list.back().time != HistoricalTransactionDataLastTime);
        if (needsUpdate) {
            UpdateTransactionData(correctedCode, featureDate, startTime);
        }
    }

    // 获取指定日期的分笔成交记录
    std::vector<level1::TickTransaction> CheckoutTransactionData(const std::string &securityCode,
                                                                 const exchange::timestamp &featureDate,
                                                                 bool ignorePreviousData) {
        std::string correctedCode = exchange::CorrectSecurityCode(securityCode);
        EnsureTransactionDataUpdated(correctedCode, featureDate, ignorePreviousData);
        auto [list, _] = LoadTransactionDataFromCache(correctedCode, featureDate, ignorePreviousData);
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
        std::string correctedCode = exchange::CorrectSecurityCode(code);
        EnsureTransactionDataUpdated(correctedCode, date, false);
    }
} // namespace data