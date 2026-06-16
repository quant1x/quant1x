#include "kline_raw.h"
#include "client.h"
#include <quant1x/contrib/data/tdx/level1/security_bars.h>
#include <quant1x/data/kline_raw.h>
#include <quant1x/config/base.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace tdx {

    void DataKLineRaw::Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void DataKLineRaw::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto code = inst.symbol();
        try {
            auto conn = level1::get_std_conn();
            // Fetch raw kline data via SecurityBars protocol
            level1::SecurityBars bars(code, static_cast<u16>(level1::KLineType::DAILY), 0, level1::security_bars_max);
            level1::process(conn->socket(), bars);
            // Save to CSV
            std::string dir = config::get_data_path() + "/day_raw/" + code;
            std::filesystem::create_directories(dir);
            std::string filename = dir + "/" + code + ".csv";
            std::ofstream out(filename);
            if (out) {
                out << "date,open,close,high,low,volume,amount,up,down,datetime\n";
                for (auto const& bar : bars.List) {
                    out << bar.DateTime.substr(0, 10) << ","
                        << bar.Open << "," << bar.Close << "," << bar.High << "," << bar.Low << ","
                        << bar.Vol << "," << bar.Amount << ","
                        << bar.UpCount << "," << bar.DownCount << ","
                        << bar.DateTime << "\n";
                }
                out.close();
            }
        } catch (const std::exception& e) {
            spdlog::warn("[DataKLineRaw] update failed for {}: {}", code, e.what());
        }
    }

} // namespace tdx
