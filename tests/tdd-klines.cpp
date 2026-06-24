#include <quant1x/test/test.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/data/meta/calendar.h>
#include <quant1x/contrib/data/tdx/bar_raw.h>
#include <quant1x/contrib/data/tdx/bar.h>
#include <quant1x/contrib/data/tdx/bar_minute.h>
#include <quant1x/factors/base_compat.h>
#include <quant1x/runtime/core.h>

// Note: ::runtime is a global namespace, not under quant1x::
namespace data = quant1x::data;
namespace meta = quant1x::data::meta;
namespace tdx = quant1x::contrib::data::tdx;

TEST_CASE("download-kline-raw", "[data]") {
    std::string code = "sz300773";
    meta::Timestamp now = meta::last_trading_day();
    const auto adapter = std::make_unique<tdx::DataKLineRaw>();
    auto inst = data::detect_symbol(code);
    adapter->Update(inst, now);
}

TEST_CASE("daily-kline", "[data]") {
    runtime::global_init();
    std::string code = "sz002350";
    meta::Timestamp now = meta::last_trading_day();

    const auto adapter = std::make_unique<tdx::DataKLine>();
    auto inst = data::detect_symbol(code);
    adapter->Update(inst, now);
}

TEST_CASE("daily-kline-xdxr", "[data]") {
    runtime::global_init();
    std::string code = "sz300773";
    meta::Timestamp now = meta::Timestamp::pre_market_time(2025, 10, 24);

    const auto adapter = std::make_unique<tdx::DataKLine>();
    auto inst = data::detect_symbol(code);
    adapter->Update(inst, now);
}

#include <quant1x/contrib/data/tdx/bar_minute.h>

TEST_CASE("minute-kline", "[data]") {
    runtime::global_init();
    std::string code = "sh510050";
    meta::Timestamp now = meta::last_trading_day();

    const auto adapter = std::make_unique<tdx::DataMinuteKLine>();  // constructor changed: no arg (was "5min" in old API)
    auto inst = data::detect_symbol(code);
    adapter->Update(inst, now);
}

#if 0
// DataFrame class (pandas/dataframe.h) removed in refactoring
TEST_CASE("checkout-klines", "[data]") {
    std::string code = "300773";
    std::string date = "2025-05-29";
    auto klines = tdx::checkout_klines(code, date);
    std::cout << klines.size() << std::endl;
    DataFrame df = DataFrame::from_struct_vector(klines);
    std::cout << df.to_string() << std::endl;
}

TEST_CASE("klines_forward_adjusted_to_date", "[data]") {
    std::string code = "300773";
    std::string date = "2025-10-24";
    auto klines = tdx::klines_forward_adjusted_to_date(code, date);
    std::cout << klines.size() << std::endl;
    DataFrame df = DataFrame::from_struct_vector(klines);
    std::cout << df.to_string() << std::endl;
}
#endif