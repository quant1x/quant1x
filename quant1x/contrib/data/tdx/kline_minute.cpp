#include <quant1x/contrib/data/tdx/kline_minute.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/level1/std/security_bars.h>
#include <quant1x/config/base.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace config = quant1x::config;
namespace meta = quant1x::data::meta;
using quant1x::contrib::data::tdx::KLineType;
using quant1x::contrib::data::tdx::SecurityBarsContext;

namespace quant1x::contrib::data::tdx {

    static std::string kline_minute_cache_filename(const meta::Instrument& inst) {
        return config::default_cache_path() + "/kline_minute/" + inst.cache_dir() + "/" + inst.symbol() + ".csv";
    }

    void DataMinuteKLine::Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void DataMinuteKLine::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto code = inst.symbol();
        try {
            auto conn = get_std_conn();
            // 使用 1分钟K线类型拉取
            SecurityBarsContext bars(inst, static_cast<u16>(KLineType::_1MIN), 0, security_bars_max);
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
