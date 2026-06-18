#include <quant1x/contrib/data/tdx/minute.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/level1/std/minute_time.h>
#include <quant1x/config/base.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace config = ::config;
namespace meta = quant1x::data::meta;
using quant1x::contrib::data::tdx::HistoryMinuteTime;

namespace quant1x::contrib::data::tdx {

    static std::string minute_cache_filename(const meta::Instrument& inst) {
        return config::default_cache_path() + "/minute/" + inst.cache_dir() + "/" + inst.symbol() + ".csv";
    }

    void DataMinute::Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void DataMinute::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        auto code = inst.symbol();
        try {
            auto conn = get_std_conn();
            // 使用标准行情连接获取历史分时数据
            auto date_int = static_cast<uint32_t>(date.yyyymmdd_u32());
            HistoryMinuteTime minute(inst, date_int);
            process_message(conn->socket(), minute);
            // 保存到 {cache}/minute/{cache_dir}/{symbol}.csv
            auto filename = minute_cache_filename(inst);
            auto parent = std::filesystem::path(filename).parent_path().string();
            std::filesystem::create_directories(parent);
            std::ofstream out(filename);
            if (out) {
                out << "price,volume\n";
                for (auto const& m : minute.List) {
                    out << m.Price << "," << m.Vol << "\n";
                }
                out.close();
            }
            spdlog::info("[DataMinute] saved {} bars for {} to {}", minute.List.size(), code, filename);
        } catch (const std::exception& e) {
            spdlog::warn("[DataMinute] update failed for {}: {}", code, e.what());
        }
    }

} // namespace quant1x::contrib::data::tdx
