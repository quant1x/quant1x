#include <quant1x/contrib/data/tdx/xdxr.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/kline.h>
#include <quant1x/contrib/data/tdx/level1/std/xdxr_info.h>
#include <quant1x/config/base.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace config = quant1x::config;
namespace meta = quant1x::data::meta;
using quant1x::contrib::data::tdx::XdxrBatch;

namespace quant1x::contrib::data::tdx {

    static std::string xdxr_cache_filename(const meta::Instrument& inst) {
        return config::default_cache_path() + "/xdxr/" + inst.cache_dir() + "/" + inst.symbol() + ".csv";
    }

    void DataXdxr::Print(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto xdxrs = get_xdxr_list(inst);
        if (xdxrs.empty()) {
            fmt::print("\n=== {}: {} ===\n  (no data)\n", Name(), inst.symbol());
            return;
        }
        fmt::print("\n=== {}: {} ({} rows) ===\n", Name(), inst.symbol(), xdxrs.size());
        fmt::print("{:<12} {:>4} {:>8} {:>10} {:>10} {:>8} {:>8} {:>8} {:>12} {:>12} {:>12} {:>12} {:>8} {:>10}\n",
                   "date", "cat", "name", "fen_hong", "pei_gu_jia", "songzg",
                   "pei_gu", "suo_gu", "qian_lt", "hou_lt", "qian_zgb", "hou_zgb", "fen_shu", "xing_quan");
        fmt::print("{:-<142}\n", "");
        size_t head = std::min<size_t>(xdxrs.size(), 20);
        for (size_t i = 0; i < head; ++i) {
            auto const& x = xdxrs[i];
            fmt::print("{:<12} {:>4} {:>8} {:>10.2f} {:>10.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>12.0f} {:>12.0f} {:>12.0f} {:>12.0f} {:>8.2f} {:>10.2f}\n",
                       x.Date, x.Category, x.Name,
                       x.FenHong, x.PeiGuJia, x.SongZhuanGu,
                       x.PeiGu, x.SuoGu, x.QianLiuTong, x.HouLiuTong,
                       x.QianZongGuBen, x.HouZongGuBen, x.FenShu, x.XingQuanJia);
        }
        if (xdxrs.size() > 20) {
            fmt::print("  ... {} rows omitted ...\n", xdxrs.size() - 20);
        }
    }

    void DataXdxr::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto code = inst.symbol();
        try {
            auto conn = get_std_conn();
            // XdxrBatch 内部处理 market 检测
            XdxrBatch batch({inst});
            transact_message_sync(conn->socket(), batch);
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

} // namespace quant1x::contrib::data::tdx
