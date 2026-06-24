#include <quant1x/contrib/data/tdx/minute.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/level1/std/minute_time.h>
#include <quant1x/config/base.h>
#include <quant1x/io/csv-writer.h>
#include <quant1x/io/csv-reader.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <filesystem>

namespace config = quant1x::config;
namespace meta = quant1x::data::meta;
using quant1x::contrib::data::tdx::HistoricalMinuteTimeContext;

namespace quant1x::contrib::data::tdx {

    static std::string minute_cache_filename(const meta::Instrument& inst) {
        return config::default_cache_path() + "/minute/" + inst.cache_dir() + "/" + inst.symbol() + ".csv";
    }

    void DataMinute::Print(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto filename = minute_cache_filename(inst);
        if (!std::filesystem::exists(filename)) {
            fmt::print("\n=== {}: {} ===\n  (no cache file)\n", Name(), inst.symbol());
            return;
        }
        io::CSVReader<2> reader(filename);
        reader.read_header(io::ignore_extra_column, "price", "volume");
        std::vector<std::pair<f64, i64>> rows;
        f64 price;
        i64 vol;
        while (reader.read_row(price, vol)) {
            rows.push_back({price, vol});
        }
        if (rows.empty()) {
            fmt::print("\n=== {}: {} ===\n  (no data)\n", Name(), inst.symbol());
            return;
        }
        fmt::print("\n=== {}: {} ({} rows) ===\n", Name(), inst.symbol(), rows.size());
        fmt::print("{:>10} {:>14}\n", "price", "volume");
        fmt::print("{:-<26}\n", "");
        size_t head = std::min<size_t>(rows.size(), 20);
        for (size_t i = 0; i < head; ++i) {
            fmt::print("{:>10.2f} {:>14}\n", rows[i].first, rows[i].second);
        }
        if (rows.size() > 40) {
            fmt::print("  ... {} rows omitted ...\n", rows.size() - 40);
            head = std::min<size_t>(20, rows.size());
            for (size_t i = rows.size() - head; i < rows.size(); ++i) {
                fmt::print("{:>10.2f} {:>14}\n", rows[i].first, rows[i].second);
            }
        } else if (rows.size() > 20) {
            for (size_t i = 20; i < rows.size(); ++i) {
                fmt::print("{:>10.2f} {:>14}\n", rows[i].first, rows[i].second);
            }
        }
    }

    void DataMinute::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        auto code = inst.symbol();
        try {
            auto conn = get_std_conn();
            // 使用标准行情连接获取历史分时数据
            auto date_int = static_cast<uint32_t>(date.yyyymmdd_u32());
            HistoricalMinuteTimeContext minute(inst, date_int);
            transact_message_sync(conn->socket(), minute);
            // 保存到 {cache}/minute/{cache_dir}/{symbol}.csv
            auto filename = minute_cache_filename(inst);
            auto parent = std::filesystem::path(filename).parent_path().string();
            std::filesystem::create_directories(parent);
            io::CSVWriter writer(filename);
            writer.write_row("price", "volume");
            for (auto const& m : minute.List) {
                writer.write_row(m.Price, m.Vol);
            }
            writer.close();
            spdlog::info("[DataMinute] saved {} bars for {} to {}", minute.List.size(), code, filename);
        } catch (const std::exception& e) {
            spdlog::warn("[DataMinute] update failed for {}: {}", code, e.what());
        }
    }

} // namespace quant1x::contrib::data::tdx
