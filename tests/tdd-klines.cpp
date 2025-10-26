#include <quant1x/test/test.h>
#include <quant1x/exchange/timestamp.h>
#include <quant1x/datasets/kline_raw.h>
#include <quant1x/datasets/kline.h>
#include <quant1x/datasets/kline_minute.h>
#include <quant1x/pandas/dataframe.h>
#include <quant1x/factors/base.h>

TEST_CASE("download-kline-raw", "[datasets]") {
    std::string code = "sz300773";
    exchange::timestamp now = exchange::last_trading_day();
    const auto adapter = std::make_unique<datasets::DataKLineRaw>();
    adapter->Update(code, now);
}

TEST_CASE("daily-kline", "[datasets]") {
    runtime::global_init();
    std::string code = "sz002350";
    exchange::timestamp now = exchange::last_trading_day();

    const auto adapter = std::make_unique<datasets::DataKLine>();
    adapter->Update(code, now);
}

TEST_CASE("daily-kline-xdxr", "[datasets]") {
    runtime::global_init();
    std::string code = "sz300773";
    exchange::timestamp now = exchange::timestamp::pre_market_time(2025, 10, 24);

    const auto adapter = std::make_unique<datasets::DataKLine>();
    adapter->Update(code, now);
}

TEST_CASE("minute-kline", "[datasets]") {
    runtime::global_init();
    std::string code = "sz300773";
    exchange::timestamp now = exchange::last_trading_day();

    const auto adapter = std::make_unique<datasets::DataMinuteKLine>("5min");
    adapter->Update(code, now);
}

TEST_CASE("checkout-klines", "[datasets]") {
    //using namespace formula;
    std::string code = "300773";
    std::string date = "2025-05-29";
    auto klines = factors::checkout_klines(code, date);
    std::cout << klines.size() << std::endl;
    DataFrame df = DataFrame::from_struct_vector(klines);
    std::cout << df.to_string() << std::endl;
}

TEST_CASE("klines_forward_adjusted_to_date", "[datasets]") {
    //using namespace formula;
    std::string code = "300773";
    std::string date = "2025-10-24";
    auto klines = factors::klines_forward_adjusted_to_date(code, date);
    std::cout << klines.size() << std::endl;
    DataFrame df = DataFrame::from_struct_vector(klines);
    std::cout << df.to_string() << std::endl;
}