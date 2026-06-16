#include "xdxr.h"
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/level1/xdxr_info.h>
#include <quant1x/config/base.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace tdx {

    static std::string xdxr_cache_filename(const meta::Instrument& inst) {
        return config::default_cache_path() + "/xdxr/" + inst.cache_dir() + "/" + inst.symbol() + ".csv";
    }

    void DataXdxr::Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void DataXdxr::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto code = inst.symbol();
        try {
            auto conn = level1::get_std_conn();
            // XdxrBatch 内部处理 market 检测
            level1::XdxrBatch batch({code});
            level1::process(conn->socket(), batch);
            // save to {cache}/xdxr/{cache_dir}/{symbol}.csv (对齐 Rust/Python)
            auto filename = xdxr_cache_filename(inst);
            auto parent = std::filesystem::path(filename).parent_path().string();
            std::filesystem::create_directories(parent);
            std::ofstream out(filename);
            if (out) {
                out << "date,category,name,fen_hong,pei_gu_jia,"
                    << "song_zhuan_gu,pei_gu,suo_gu,qian_liu_tong,hou_liu_tong,qian_zong_gu_ben,"
                    << "hou_zong_gu_ben,fen_shu,xing_quan_jia\n";
                for (auto const& entry : batch.entries) {
                    for (auto const& info : entry.list) {
                        out << info.Date << ","
                            << static_cast<int>(info.Category) << ","
                            << info.Name << ","
                            << info.FenHong << ","
                            << info.PeiGuJia << ","
                            << info.SongZhuanGu << ","
                            << info.PeiGu << ","
                            << info.SuoGu << ","
                            << info.QianLiuTong << ","
                            << info.HouLiuTong << ","
                            << info.QianZongGuBen << ","
                            << info.HouZongGuBen << ","
                            << info.FenShu << ","
                            << info.XingQuanJia << "\n";
                    }
                }
                out.close();
            }
        } catch (const std::exception& e) {
            spdlog::warn("[DataXdxr] update failed for {}: {}", code, e.what());
        }
    }

} // namespace tdx
