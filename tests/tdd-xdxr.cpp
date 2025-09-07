#include <quant1x/test/test.h>
#include <quant1x/datasets/xdxr.h>
#include <quant1x/datasets/xdxr_adjust_factor.h>

#include <ranges>
#include <quant1x/datasets/kline.h>
#include <quant1x/encoding/csv.h>
#include <quant1x/datasets/kline_raw.h>
#include <quant1x/exchange/security.h>
#include <quant1x/exchange.h>
#include <quant1x/factors/f10.h>
#include <quant1x/factors/history.h>
#include <quant1x/dataframe/dataframe.h>

TEST_CASE("xdxr-extract", "[xdxr]") {
    spdlog::set_level(spdlog::level::debug);
    std::string code = "sz300773";
    exchange::timestamp start("1990-12-19");
    exchange::timestamp end("2025-06-05");
    auto xdxrs = factors::get_xdxr_list(code);
    auto list = factors::combine_adjustments_in_period(xdxrs, start, end);
    std::cout << list << std::endl;
}

TEST_CASE("kline-extract", "[xdxr]") {
    spdlog::set_level(spdlog::level::debug);
    std::string code = "sz300773";
    exchange::timestamp end("2025-06-05");
    auto klines = factors::klines_forward_adjusted_to_date(code, end.only_date());
    DataFrame df = DataFrame::from_struct_vector(klines);
    std::cout << df.to_string() << std::endl;
}

TEST_CASE("klines-check", "[xdxr]") {
    spdlog::set_level(spdlog::level::debug);
    std::string code = "sz300773";
    auto xdxr_infos = factors::get_xdxr_list(code);

    std::string raw_cache_filename = config::get_kline_filename(code, false);
    auto raw_list = encoding::csv::csv_to_slices<datasets::KLineRaw>(raw_cache_filename);
    auto ipo_date = exchange::timestamp(raw_list[0].Date).pre_market_time();
    auto raw_view = raw_list | std::views::filter([](const datasets::KLineRaw& x){return x.Date>="1990-12-19";});
    (void)raw_view;
    auto start_date = exchange::timestamp(exchange::market_cn_first_listtime).pre_market_time();
    // 在指定开始日期和IPO日期之前选最大值, 部分上市公司在没上市之前的除权除息的记录也会有记录, 上市之前的除权除息数据不能用来复权
    // 在这里修正开始日期不能早于ipo日期
    start_date = std::max(ipo_date, start_date);
    auto end_date = exchange::timestamp(2025,6,5).pre_market_time();
    factors::apply_forward_adjustments(raw_list, xdxr_infos, start_date, end_date, true);
    //std::string cache_filename = config::get_kline_filename(code, true);
    //encoding::csv::slices_to_csv(raw_list, cache_filename);
    DataFrame df = DataFrame::from_struct_vector(raw_list);
    std::cout << df.to_string() << std::endl;
}

std::unordered_map<std::string, std::vector<factors::CumulativeAdjustment>> checkout_dividends_map(const exchange::timestamp &current) {
    std::unordered_map<std::string, std::vector<factors::CumulativeAdjustment>> result_map;
    auto all_codes = exchange::GetCodeList();
    //auto now = exchange::timestamp::now();
    for(auto const & security_code : all_codes) {
        // 1. 首先加载除权除息记录
        auto xdxr_infos = datasets::load_xdxr(security_code);
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
            auto ipo_from_xdxr = factors::ipo_date_from_xdxrs(xdxr_infos);
            if (ipo_from_xdxr.has_value()) {
                ipo_date = ipo_from_xdxr.value();
            } else {
                // 降级处理为1990-12-19
                ipo_date = exchange::market_cn_first_listtime;
            }
        }
        auto factors = factors::combine_adjustments_in_period(xdxr_infos, ipo_date, today);
        if(factors.empty()) {
            continue;
        }
//        auto last_factor = factors.back();
//        auto last_timestamp = exchange::timestamp(last_factor.timestamp).only_date();
//        if(last_timestamp != today) {
//            continue;
//        }
        result_map.emplace(security_code, factors);
        std::cout << security_code << ":" << factors.size() << std::endl;
    }
    return result_map;
}

TEST_CASE("today-adjust", "[xdxr]") {
    checkout_dividends_map(exchange::timestamp::now());
}

namespace {
    std::mutex g_factor_history_mutex{};
    tsl::robin_map<std::string, History> g_factor_history_map{};
    exchange::timestamp g_factor_history_date{};
}

void check_and_update(const exchange::timestamp& timestamp) {
    std::lock_guard<std::mutex> lock{g_factor_history_mutex};
    exchange::timestamp algin_date = timestamp.pre_market_time();
    algin_date = exchange::last_trading_day(algin_date);
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
                    v.adjust(factor.m, factor.a, factor.no);
                }
            }
            g_factor_history_map.emplace(v.Code, v);
        }
    }
}

#include <quant1x/proto/xdxr.pb.h>

void check_and_update_pb(const exchange::timestamp& timestamp) {
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
    check_and_update_pb(exchange::timestamp::now());
}
