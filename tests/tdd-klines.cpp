#include <quant1x/test/test.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/data/kline_raw.h>
#include <quant1x/contrib/data/tdx/kline.h>
#include <quant1x/data/kline_minute.h>
#include <quant1x/pandas/dataframe.h>
#include <quant1x/factors/base.h>

TEST_CASE("download-kline-raw", "[data]") {
    std::string code = "sz300773";
    meta::Timestamp now = meta::last_trading_day();
    const auto adapter = std::make_unique<data::DataKLineRaw>();
    auto inst = data::detect_symbol(code);
    adapter->Update(inst, now);
}

TEST_CASE("daily-kline", "[data]") {
    runtime::global_init();
    std::string code = "sz002350";
    meta::Timestamp now = meta::last_trading_day();

    const auto adapter = std::make_unique<data::DataKLine>();
    auto inst = data::detect_symbol(code);
    adapter->Update(inst, now);
}

TEST_CASE("daily-kline-xdxr", "[data]") {
    runtime::global_init();
    std::string code = "sz300773";
    meta::Timestamp now = meta::Timestamp::pre_market_time(2025, 10, 24);

    const auto adapter = std::make_unique<data::DataKLine>();
    auto inst = data::detect_symbol(code);
    adapter->Update(inst, now);
}

TEST_CASE("minute-kline", "[data]") {
    runtime::global_init();
    std::string code = "sh510050";
    meta::Timestamp now = meta::last_trading_day();

    const auto adapter = std::make_unique<data::DataMinuteKLine>("5min");
    auto inst = data::detect_symbol(code);
    adapter->Update(inst, now);
}

TEST_CASE("checkout-klines", "[data]") {
    //using namespace formula;
    std::string code = "300773";
    std::string date = "2025-05-29";
    auto klines = factors::checkout_klines(code, date);
    std::cout << klines.size() << std::endl;
    DataFrame df = DataFrame::from_struct_vector(klines);
    std::cout << df.to_string() << std::endl;
}

TEST_CASE("klines_forward_adjusted_to_date", "[data]") {
    //using namespace formula;
    std::string code = "300773";
    std::string date = "2025-10-24";
    auto klines = factors::klines_forward_adjusted_to_date(code, date);
    std::cout << klines.size() << std::endl;
    DataFrame df = DataFrame::from_struct_vector(klines);
    std::cout << df.to_string() << std::endl;
}