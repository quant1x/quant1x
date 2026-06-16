#include "kline.h"
#include "client.h"
#include <quant1x/contrib/data/tdx/level1/security_bars.h>
#include <quant1x/config/base.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace tdx {

    static std::string kline_cache_filename(const meta::Instrument& inst) {
        return config::default_cache_path() + "/day/" + inst.cache_dir() + "/" + inst.symbol() + ".csv";
    }

    void DataKLine::Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void DataKLine::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto code = inst.symbol();
        // 前复权K线: 先拉取 raw kline，前复权计算在 factors/base.cpp 中
        // 详细实现在 factors/base.cpp 的 klines_forward_adjusted_to_date 中
        try {
            auto conn = level1::get_std_conn();
            level1::SecurityBars bars(code, static_cast<u16>(level1::KLineType::DAILY), 0, level1::security_bars_max);
            level1::process(conn->socket(), bars);
            // 保存到 {cache}/day/{cache_dir}/{symbol}.csv (对齐 Rust/Python)
            auto filename = kline_cache_filename(inst);
            auto parent = std::filesystem::path(filename).parent_path().string();
            std::filesystem::create_directories(parent);
            std::ofstream out(filename);
            if (out) {
                out << "date,open,close,high,low,volume,amount,up,down,timestamp,adjustment_count\n";
                for (auto const& bar : bars.List) {
                    out << bar.DateTime.substr(0, 10) << ","
                        << bar.Open << "," << bar.Close << "," << bar.High << "," << bar.Low << ","
                        << bar.Vol << "," << bar.Amount << ","
                        << bar.UpCount << "," << bar.DownCount << ","
                        << bar.DateTime << ",0\n";
                }
                out.close();
            }
            spdlog::info("[DataKLine] saved {} bars for {} to {}", bars.List.size(), code, filename);
        } catch (const std::exception& e) {
            spdlog::warn("[DataKLine] update failed for {}: {}", code, e.what());
        }
    }

} // namespace tdx
