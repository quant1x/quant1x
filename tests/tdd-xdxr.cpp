#include <quant1x/test/test.h>
#include <quant1x/contrib/data/tdx/xdxr.h>
#include <quant1x/contrib/data/tdx/bar.h>

#include <ranges>
#include <quant1x/encoding/csv.h>
#include <quant1x/contrib/data/tdx/instruments.h>
#include <quant1x/factors/f10.h>
#include <quant1x/factors/history.h>
#include <quant1x/data/schema/bar.h>

namespace meta = quant1x::data::meta;
namespace data = quant1x::data;
namespace tdx = quant1x::contrib::data::tdx;
using namespace encoding;

TEST_CASE("xdxr-extract", "[xdxr]") {
    spdlog::set_level(spdlog::level::debug);
    std::string code = "sz300773";
    meta::Timestamp start("1990-12-19");
    meta::Timestamp end("2025-06-05");
    auto xdxrs = tdx::get_xdxr_list(code);
    auto list = tdx::combine_adjustments_in_period(xdxrs, start, end);
    spdlog::info("xdxr list size: {}", list.size());
}

TEST_CASE("bar-extract", "[xdxr]") {
    spdlog::set_level(spdlog::level::debug);
    std::string code = "sz300773";
    meta::Timestamp end("2025-06-05");
    auto bars = tdx::bars_forward_adjusted_to_date(code, end.only_date());
    std::cout << "bars count: " << bars.size() << std::endl;
    for (const auto& bar : bars) {
        std::cout << bar << std::endl;
    }
}

// TEST_CASE("klines-check", "[xdxr]") {  // KLineRaw removed in C++ refactor, see bar_raw.rs
//     spdlog::set_level(spdlog::level::debug);
//     std::string code = "sz300773";
//     auto xdxr_infos = tdx::get_xdxr_list(code);
//
//     std::string raw_cache_filename = config::get_bar_filename(code, false);
//     auto raw_list = encoding::csv::csv_to_slices<datasets::KLineRaw>(raw_cache_filename);
//     auto ipo_date = meta::Timestamp(raw_list[0].Date).pre_market_time();
//     auto raw_view = raw_list | std::views::filter([](const datasets::KLineRaw& x){return x.Date>="1990-12-19";});
//     (void)raw_view;
//     auto start_date = meta::Timestamp(meta::MARKET_CN_FIRST_LISTTIME).pre_market_time();
//     start_date = std::max(ipo_date, start_date);
//     auto end_date = meta::Timestamp(2025,6,5).pre_market_time();
//     DataFrame df = DataFrame::from_struct_vector(raw_list);
//     std::cout << df.to_string() << std::endl;
// }

std::unordered_map<std::string, std::vector<factors::CumulativeAdjustment>> checkout_dividends_map(const meta::Timestamp &current) {
    std::unordered_map<std::string, std::vector<factors::CumulativeAdjustment>> result_map;
    auto all_codes = tdx::instruments::get_code_list();
    //auto now = meta::Timestamp::now();
    for(auto const & security_code : all_codes) {
        // 1. 首先加载除权除息记录
        auto xdxr_infos = tdx::get_xdxr_list(security_code);
        // 2. 确定当前日期
        std::string today = current.only_date();
        // 3. 获取IPO日期
        // 3.1 先给一个最早的日期, 作为初始化
        std::string ipo_date;
        // 3.2 通过f10数据获取ipo日期
        auto opt_f10 = factors::get_f10(security_code, current);
        if(opt_f10.has_value()) {
            // f10数据存在
            ipo_date = opt_f10->IpoDate;
        } else {
            // f10数据不存在, 则从除权除息的第一条股本变化的记录中获取
            auto ipo_from_xdxr = tdx::ipo_date_from_xdxrs(xdxr_infos);
            if (ipo_from_xdxr.has_value()) {
                ipo_date = ipo_from_xdxr.value();
            } else {
                // 降级处理为1990-12-19
                ipo_date = data::MARKET_CN_FIRST_LISTTIME;
            }
        }
        auto factors = tdx::combine_adjustments_in_period(xdxr_infos, ipo_date, today);
        if(factors.empty()) {
            continue;
        }
//        auto last_factor = factors.back();
//        auto last_timestamp = meta::Timestamp(last_factor.timestamp).only_date();
//        if(last_timestamp != today) {
//            continue;
//        }
        result_map.emplace(security_code, factors);
        std::cout << security_code << ":" << factors.size() << std::endl;
    }
    return result_map;
}

TEST_CASE("today-adjust", "[xdxr]") {
    checkout_dividends_map(meta::Timestamp::now());
}

namespace {
    std::mutex g_factor_history_mutex{};
    tsl::robin_map<std::string, History> g_factor_history_map{};
    meta::Timestamp g_factor_history_date{};
}

void check_and_update(const meta::Timestamp& timestamp) {
    std::lock_guard<std::mutex> lock{g_factor_history_mutex};
    meta::Timestamp algin_date = timestamp.pre_market_time();
    algin_date = meta::last_trading_day(algin_date);
    if(g_factor_history_map.empty() || g_factor_history_date != algin_date) {
        g_factor_history_date = algin_date;
        auto adapter = HistoryFeature();
        auto cache_filename = adapter.Filename(g_factor_history_date);
        if(!std::filesystem::exists(cache_filename)) {
            spdlog::error("[history] cache file[{}], not found", cache_filename);
            return;
        }
        std::unordered_map<std::string, std::vector<factors::CumulativeAdjustment>> xdxr_map = checkout_dividends_map(timestamp);
        std::vector<History> list = encoding::csv::csv_to_slices<History>(cache_filename);
        for(auto &v : list) {
            auto it = xdxr_map.find(v.Code);
            if(it != xdxr_map.end()) {
                auto tmp_list = it->second;
                // 只处理最后一条复权因子
                auto factor = tmp_list.back();
                if(factor.timestamp < timestamp.pre_market_time()) {
                    v.adjust(factor);
                }
            }
            g_factor_history_map.emplace(v.Code, v);
        }
    }
}

#include <quant1x/proto/xdxr.pb.h>

void check_and_update_pb(const meta::Timestamp& timestamp) {
    std::unordered_map<std::string, std::vector<factors::CumulativeAdjustment>> xdxr_map = checkout_dividends_map(timestamp);
    std::cout<< xdxr_map.size() << std::endl;
    xdxr::adjust_map msg{};
    for(auto & row : xdxr_map) {
        auto adjust_list = row.second;
        auto list = (*msg.mutable_complex_map())[row.first].mutable_values();
        for(auto & v : adjust_list) {
            auto factor = list->Add();
            factor->set_timestamp(v.timestamp);
            factor->set_m(v.m);
            factor->set_a(v.a);
            factor->set_number(v.no);
        }
    }
    std::ofstream out("xdxr.pb", std::ios::binary|std::ios::trunc);
    msg.SerializePartialToOstream(&out);
    out.flush();
    out.close();
}


TEST_CASE("xdxr-protobuf", "[xdxr]") {
    check_and_update_pb(meta::Timestamp::now());
}
