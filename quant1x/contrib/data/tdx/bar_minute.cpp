#include <quant1x/contrib/data/tdx/bar_minute.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/level1/std/security_bars.h>
#include <quant1x/config/base.h>
#include <quant1x/io/csv-reader.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <fstream>
#include <filesystem>

namespace config = quant1x::config;
namespace meta = quant1x::data::meta;
using quant1x::contrib::data::tdx::BarFreq;
using quant1x::contrib::data::tdx::SecurityBarsContext;

namespace quant1x::contrib::data::tdx {

    static std::string kline_minute_cache_filename(const meta::Instrument& inst) {
        return config::default_cache_path() + "/kline_minute/" + inst.cache_dir() + "/" + inst.symbol() + ".csv";
    }

    void DataMinuteKLine::Print(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto filename = kline_minute_cache_filename(inst);
        if (!std::filesystem::exists(filename)) {
            fmt::print("\n=== {}: {} ===\n  (no cache file)\n", Name(), inst.symbol());
            return;
        }
        io::CSVReader<9> reader(filename);
        reader.read_header(io::ignore_extra_column, "datetime", "open", "close", "high", "low", "volume", "amount", "up", "down");
        std::string dt;
        f64 open, close, high, low, volume, amount;
        int up, down;
        std::vector<std::tuple<std::string, f64, f64, f64, f64, f64, f64, int, int>> rows;
        while (reader.read_row(dt, open, close, high, low, volume, amount, up, down)) {
            rows.push_back({dt, open, close, high, low, volume, amount, up, down});
        }
        if (rows.empty()) {
            fmt::print("\n=== {}: {} ===\n  (no data)\n", Name(), inst.symbol());
            return;
        }
        fmt::print("\n=== {}: {} ({} rows) ===\n", Name(), inst.symbol(), rows.size());
        fmt::print("{:<20} {:>8} {:>8} {:>8} {:>8} {:>12} {:>14} {:>4} {:>4}\n",
                   "datetime", "open", "close", "high", "low", "volume", "amount", "up", "dn");
        fmt::print("{:-<94}\n", "");
        size_t head = std::min<size_t>(rows.size(), 10);
        for (size_t i = 0; i < head; ++i) {
            auto const& r = rows[i];
            fmt::print("{:<20} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>12.0f} {:>14.0f} {:>4} {:>4}\n",
                       std::get<0>(r), std::get<1>(r), std::get<2>(r), std::get<3>(r),
                       std::get<4>(r), std::get<5>(r), std::get<6>(r), std::get<7>(r), std::get<8>(r));
        }
        if (rows.size() > 20) {
            fmt::print("  ... {} rows omitted ...\n", rows.size() - 20);
            head = std::min<size_t>(10, rows.size());
            for (size_t i = rows.size() - head; i < rows.size(); ++i) {
                auto const& r = rows[i];
                fmt::print("{:<20} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>12.0f} {:>14.0f} {:>4} {:>4}\n",
                           std::get<0>(r), std::get<1>(r), std::get<2>(r), std::get<3>(r),
                           std::get<4>(r), std::get<5>(r), std::get<6>(r), std::get<7>(r), std::get<8>(r));
            }
        } else if (rows.size() > 10) {
            for (size_t i = 10; i < rows.size(); ++i) {
                auto const& r = rows[i];
                fmt::print("{:<20} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>12.0f} {:>14.0f} {:>4} {:>4}\n",
                           std::get<0>(r), std::get<1>(r), std::get<2>(r), std::get<3>(r),
                           std::get<4>(r), std::get<5>(r), std::get<6>(r), std::get<7>(r), std::get<8>(r));
            }
        }
    }

    void DataMinuteKLine::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto code = inst.symbol();
        try {
            auto conn = get_std_conn();
            // 使用 1分钟K线类型拉取
            SecurityBarsContext bars(inst, static_cast<u16>(BarFreq::Freq1Min), 0, security_bars_max);
            transact_message_sync(conn->socket(), bars);
            // 保存到 {cache}/kline_minute/{cache_dir}/{symbol}.csv
            auto filename = kline_minute_cache_filename(inst);
            auto parent = std::filesystem::path(filename).parent_path().string();
            std::filesystem::create_directories(parent);
            std::ofstream out(filename);
            if (out) {
                out << "datetime,open,close,high,low,volume,amount,up,down\n";
                for (auto const& bar : bars.List) {
                    out << bar.DateTime << ","
                        << bar.Open << "," << bar.Close << "," << bar.High << "," << bar.Low << ","
                        << bar.Vol << "," << bar.Amount << ","
                        << bar.UpCount << "," << bar.DownCount << "\n";
                }
                out.close();
            }
            spdlog::info("[DataMinuteKLine] saved {} bars for {} to {}", bars.List.size(), code, filename);
        } catch (const std::exception& e) {
            spdlog::warn("[DataMinuteKLine] update failed for {}: {}", code, e.what());
        }
    }

} // namespace quant1x::contrib::data::tdx
