#include <quant1x/contrib/data/tdx/trans.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/level1/std/transaction.h>
#include <quant1x/config/base.h>
#include <quant1x/data/meta/calendar.h>
#include <quant1x/data/schema/trade.h>
#include <quant1x/io/csv-reader.h>
#include <quant1x/io/csv-writer.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <variant>

namespace config = quant1x::config;
namespace meta = quant1x::data::meta;
namespace schema = quant1x::data::schema;

namespace quant1x::contrib::data::tdx {

    // 与 Python 对齐的常量
    static const char* TransFirstTime = "09:25";
    static const char* TransLastTime  = "15:00";

    /// 生成缓存文件路径 (对齐 Python get_historical_trade_filename)
    static std::string trans_cache_filename(const meta::Instrument& inst, const std::string& date_str) {
        auto clean_date = date_str;
        clean_date.erase(std::remove(clean_date.begin(), clean_date.end(), '-'), clean_date.end());
        clean_date.erase(std::remove(clean_date.begin(), clean_date.end(), '/'), clean_date.end());
        std::string year = clean_date.length() >= 4 ? clean_date.substr(0, 4) : "0000";
        return config::default_cache_path() + "/trans/" + inst.cache_dir() + "/" + year + "/" + clean_date + "/" + inst.symbol() + ".csv";
    }

    /// 从缓存加载逐笔交易数据，并确定增量更新的起始时间
    /// 对齐 Python load_transaction_data_from_cache(inst, feature_date, ignore_previous_data=False)
    static std::tuple<std::vector<schema::Transaction>, std::string>
    load_transaction_data_from_cache(const meta::Instrument& inst, const std::string& date_str) {
        std::vector<schema::Transaction> list;
        std::string start_time = TransFirstTime;

        auto filename = trans_cache_filename(inst, date_str);
        if (!std::filesystem::exists(filename)) {
            return {list, start_time};
        }

        try {
            io::CSVReader<6, io::trim_chars<' ', '\t'>, io::double_quote_escape<',', '"'>> reader(filename);
            reader.read_header(io::ignore_extra_column | io::ignore_missing_column,
                               "time", "price", "volume", "num", "amount", "direction");

            std::string time;
            double      price;
            int         volume;
            int         num;
            double      amount;
            int         direction;

            while (reader.read_row(time, price, volume, num, amount, direction)) {
                list.push_back({time, price, volume, num, amount, direction});
            }
        } catch (const std::exception& e) {
            spdlog::warn("[DataTrans] failed to read cache {}: {}", filename, e.what());
            return {list, start_time};
        }

        if (!list.empty()) {
            // 如果数据已完整 (最后一条时间为 15:00), 直接返回
            if (list.back().time == TransLastTime) {
                return {list, start_time};
            }

            // 从尾部扫描, 确定 start_time 并去除尾部重复部分
            auto cache_length = list.size();
            std::string first_time;
            size_t skip_count = 0;
            for (size_t i = 1; i <= cache_length; i++) {
                auto& tm = list[cache_length - i].time;
                if (first_time.empty()) {
                    first_time = tm;
                    start_time = first_time;
                    skip_count++;
                    continue;
                }
                if (tm < first_time) {
                    start_time = first_time;
                    break;
                } else {
                    skip_count++;
                }
            }
            if (skip_count > 0) {
                list.resize(cache_length - skip_count);
            }
        }

        return {list, start_time};
    }

    /// 从 level1 拉取逐笔数据, 与现有缓存合并, 原子写入
    /// 对齐 Python update_transaction_data(inst, feature_date, start_time)
    static void update_transaction_data(const meta::Instrument& inst, const meta::Timestamp& date, const std::string& start_time) {
        auto code = inst.symbol();
        auto date_str = date.only_date();
        auto trade_date_int = static_cast<uint32_t>(date.yyyymmdd_u32());

        // 判断是否为最近交易日 — 如果是, 使用实时分笔协议; 否则用历史分笔协议
        bool today_is_last_trading_date = date.is_same_date(meta::last_trading_day());
        uint16_t offset = tick_transaction_per_request_max;
        int start = 0;

        std::vector<schema::Transaction> history;
        std::vector<std::vector<schema::Transaction>> hs;

        auto conn = get_std_conn();
        if (!conn) {
            spdlog::warn("[DataTrans] failed to get std connection for {}", code);
            return;
        }

        while (true) {
            try {
                std::variant<std::monostate, TransactionContext, HistoricalTransactionContext> msg;
                if (today_is_last_trading_date) {
                    msg.emplace<TransactionContext>(inst, static_cast<uint16_t>(start), offset);
                } else {
                    msg.emplace<HistoricalTransactionContext>(inst, trade_date_int, static_cast<uint16_t>(start), offset);
                }
                uint16_t count = 0;
                std::vector<TickTransaction> tmp_list;
                std::visit([&](auto& m) {
                    if constexpr (!std::is_same_v<std::decay_t<decltype(m)>, std::monostate>) {
                        transact_message_sync(conn->socket(), m);
                        count = m.Count;
                        tmp_list = m.List;
                    }
                }, msg);

                if (count == 0 || tmp_list.empty()) break;

                std::reverse(tmp_list.begin(), tmp_list.end());

                std::vector<schema::Transaction> tmp;
                for (auto const& td : tmp_list) {
                    if (td.time >= start_time) {
                        tmp.push_back({
                            td.time, td.price,
                            static_cast<int>(td.vol),
                            static_cast<int>(td.num),
                            td.amount,
                            static_cast<int>(td.buyOrSell)
                        });
                    }
                }
                std::reverse(tmp.begin(), tmp.end());
                hs.push_back(std::move(tmp));

                if (hs.back().size() < offset) break;

                start += offset;
            } catch (const std::exception& e) {
                spdlog::warn("[DataTrans] fetch failed for {}: {}", code, e.what());
                break;
            }
        }

        // bs: hs 此时按拉取顺序排列, hs[0] 最早, hs[-1] 最新
        // 反转后 hs[0] 最新, hs[-1] 最早, 展平后 history 整体最新→最旧
        // 对齐 Python: hs.reverse() then flatten
        std::reverse(hs.begin(), hs.end());
        for (auto& chunk : hs) {
            history.insert(history.end(), chunk.begin(), chunk.end());
        }

        if (history.empty()) {
            return;
        }

        // 与现有缓存合并
        auto [existing_list, _] = load_transaction_data_from_cache(inst, date_str);
        existing_list.insert(existing_list.end(), history.begin(), history.end());

        // 原子写入: 先写 .tmp, 再 rename
        auto filename = trans_cache_filename(inst, date_str);
        auto parent = std::filesystem::path(filename).parent_path().string();
        std::filesystem::create_directories(parent);
        auto tmp_filename = filename + ".tmp";

        try {
            io::CSVWriter writer(tmp_filename);
            writer.write_row("time", "price", "volume", "num", "amount", "direction");
            for (auto const& t : existing_list) {
                writer.write_row(t.time, t.price, t.volume, t.num, t.amount, t.direction);
            }
            writer.close();

            // 原子替换
            if (std::rename(tmp_filename.c_str(), filename.c_str()) != 0) {
                spdlog::error("[DataTrans] rename failed: {} -> {}", tmp_filename, filename);
                std::remove(tmp_filename.c_str());
                return;
            }
        } catch (const std::exception& e) {
            spdlog::error("[DataTrans] write failed for {}: {}", code, e.what());
            std::remove(tmp_filename.c_str());
            return;
        }

        spdlog::info("[DataTrans] saved {} transactions for {} on {} to {}", existing_list.size(), code, date_str, filename);
    }

    // ============================================================
    // DataTrans 公共接口
    // ============================================================

    void DataTrans::Print(const meta::Instrument& inst, const meta::Timestamp& date) {
        std::string date_str;
        if (!date.empty()) {
            date_str = date.only_date();
        } else {
            date_str = meta::Timestamp::now().only_date();
        }
        auto filename = trans_cache_filename(inst, date_str);
        if (!std::filesystem::exists(filename)) {
            fmt::print("\n=== {}: {} @ {} ===\n  (no cache file)\n", Name(), inst.symbol(), date_str);
            return;
        }
        try {
            io::CSVReader<6, io::trim_chars<' ', '\t'>, io::double_quote_escape<',', '"'>> reader(filename);
            reader.read_header(io::ignore_extra_column | io::ignore_missing_column,
                               "time", "price", "volume", "num", "amount", "direction");
            std::vector<schema::Transaction> rows;
            std::string time;
            double price;
            int volume, num, direction;
            double amount;
            while (reader.read_row(time, price, volume, num, amount, direction)) {
                rows.push_back({time, price, volume, num, amount, direction});
            }
            if (rows.empty()) {
                fmt::print("\n=== {}: {} @ {} ===\n  (no data)\n", Name(), inst.symbol(), date_str);
                return;
            }
            fmt::print("\n=== {}: {} @ {} ({} rows) ===\n", Name(), inst.symbol(), date_str, rows.size());
            fmt::print("{:<10} {:>8} {:>10} {:>6} {:>14} {:>10}\n",
                       "time", "price", "volume", "num", "amount", "dir");
            fmt::print("{:-<64}\n", "");
            size_t head = std::min<size_t>(rows.size(), 20);
            for (size_t i = 0; i < head; ++i) {
                auto const& t = rows[i];
                const char* dir = t.direction == 0 ? "BUY" : (t.direction == 1 ? "SELL" : "MID");
                fmt::print("{:<10} {:>8.2f} {:>10} {:>6} {:>14.0f} {:>10}\n",
                           t.time, t.price, t.volume, t.num, t.amount, dir);
            }
            if (rows.size() > 40) {
                fmt::print("  ... {} rows omitted ...\n", rows.size() - 40);
                head = std::min<size_t>(20, rows.size());
                for (size_t i = rows.size() - head; i < rows.size(); ++i) {
                    auto const& t = rows[i];
                    const char* dir = t.direction == 0 ? "BUY" : (t.direction == 1 ? "SELL" : "MID");
                    fmt::print("{:<10} {:>8.2f} {:>10} {:>6} {:>14.0f} {:>10}\n",
                               t.time, t.price, t.volume, t.num, t.amount, dir);
                }
            } else if (rows.size() > 20) {
                for (size_t i = 20; i < rows.size(); ++i) {
                    auto const& t = rows[i];
                    const char* dir = t.direction == 0 ? "BUY" : (t.direction == 1 ? "SELL" : "MID");
                    fmt::print("{:<10} {:>8.2f} {:>10} {:>6} {:>14.0f} {:>10}\n",
                               t.time, t.price, t.volume, t.num, t.amount, dir);
                }
            }
        } catch (const std::exception& e) {
            fmt::print("\n=== {}: {} @ {} ===\n  read error: {}\n", Name(), inst.symbol(), date_str, e.what());
        }
    }

    void DataTrans::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        auto date_str = date.only_date();

        // 先加载缓存, 判断是否需要更新
        auto [existing_list, start_time] = load_transaction_data_from_cache(inst, date_str);

        // 数据为空 或 最后时间不为 15:00 则需要更新
        bool needs_update = existing_list.empty() || (existing_list.back().time != TransLastTime);

        if (needs_update) {
            update_transaction_data(inst, date, start_time);
        } else {
            spdlog::info("[DataTrans] {} on {} already complete, skip", inst.symbol(), date_str);
        }
    }

    // =============================
    // 交易分析函数
    // =============================

    std::vector<quant1x::data::schema::Transaction> CheckoutTransactionData(
        const std::string& code, const quant1x::data::meta::Timestamp& date, bool /*ignorePreviousData*/) {
        auto inst = quant1x::data::detect_symbol(code);
        auto [list, _] = load_transaction_data_from_cache(inst, date.only_date());
        return list;
    }

    TurnoverDataSummary CountInflow(
        const std::vector<quant1x::data::schema::Transaction>& list,
        const std::string& /*securityCode*/,
        const quant1x::data::meta::Timestamp& /*featureDate*/) {
        TurnoverDataSummary summary{};
        if (list.empty()) return summary;

        // 统计内外盘成交
        double last_price = list[0].price;
        for (const auto& v : list) {
            int direction = v.direction;
            double price = v.price;
            int vol = v.volume;
            double amount = v.amount;

            if (direction == 0) { // 外盘/买盘
                summary.OuterVolume += vol;
                summary.OuterAmount += amount;
            } else if (direction == 1) { // 内盘/卖盘
                summary.InnerVolume += vol;
                summary.InnerAmount += amount;
            } else { // 中性盘, 按价格变化判断
                if (price > last_price) {
                    summary.OuterVolume += vol;
                    summary.OuterAmount += amount;
                } else if (price < last_price) {
                    summary.InnerVolume += vol;
                    summary.InnerAmount += amount;
                }
                // 价格不变: 不归类
            }
            if (last_price == 0) last_price = price;
        }

        // 开盘量 (前10笔)
        size_t open_count = std::min(size_t(10), list.size());
        for (size_t i = 0; i < open_count; ++i) {
            summary.OpenVolume += list[i].volume;
        }

        // 收盘量 (后10笔)
        size_t close_count = std::min(size_t(10), list.size());
        for (size_t i = list.size() - close_count; i < list.size(); ++i) {
            summary.CloseVolume += list[i].volume;
        }

        return summary;
    }

} // namespace quant1x::contrib::data::tdx
