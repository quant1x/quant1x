#include "minute.h"
#include "client.h"
#include <quant1x/contrib/data/tdx/level1/minute_time.h>
#include <quant1x/config/base.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace tdx {

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
            auto conn = level1::get_std_conn();
            // 使用标准行情连接获取历史分时数据
            auto date_int = static_cast<uint32_t>(date.yyyymmdd_u32());
            level1::HistoryMinuteTime minute(code, date_int);
            level1::process(conn->socket(), minute);
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

} // namespace tdx
