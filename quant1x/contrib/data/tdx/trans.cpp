#include "trans.h"
#include "client.h"
#include <quant1x/contrib/data/tdx/level1/transaction_history.h>
#include <quant1x/config/base.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <algorithm>

namespace tdx {

    static std::string trans_cache_filename(const meta::Instrument& inst, const std::string& date_str) {
        auto clean_date = date_str;
        // 移除 '-' 和 '/' 分隔符
        clean_date.erase(std::remove(clean_date.begin(), clean_date.end(), '-'), clean_date.end());
        clean_date.erase(std::remove(clean_date.begin(), clean_date.end(), '/'), clean_date.end());
        std::string year = clean_date.length() >= 4 ? clean_date.substr(0, 4) : "0000";
        return config::default_cache_path() + "/trans/" + inst.cache_dir() + "/" + year + "/" + clean_date + "/" + inst.symbol() + ".csv";
    }

    void DataTrans::Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void DataTrans::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        auto code = inst.symbol();
        auto date_str = date.only_date();
        try {
            auto conn = level1::get_std_conn();
            // 使用标准行情连接获取历史分笔成交
            auto date_int = static_cast<uint32_t>(date.yyyymmdd_int());
            level1::HistoryTransaction trans(code, date_int, 0, 1000);
            level1::process(conn->socket(), trans);
            // 保存到 {cache}/trans/{cache_dir}/{year}/{YYYYMMDD}/{symbol}.csv (对齐 Rust/Python)
            auto filename = trans_cache_filename(inst, date_str);
            auto parent = std::filesystem::path(filename).parent_path().string();
            std::filesystem::create_directories(parent);
            std::ofstream out(filename);
            if (out) {
                out << "time,price,vol,num,amount,buyOrSell\n";
                for (auto const& t : trans.List) {
                    out << t.time << ","
                        << t.price << "," << t.vol << "," << t.num << ","
                        << t.amount << "," << t.buyOrSell << "\n";
                }
                out.close();
            }
            spdlog::info("[DataTrans] saved {} transactions for {} on {} to {}", trans.List.size(), code, date_str, filename);
        } catch (const std::exception& e) {
            spdlog::warn("[DataTrans] update failed for {}: {}", code, e.what());
        }
    }

} // namespace tdx
