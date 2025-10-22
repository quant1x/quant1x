#include <quant1x/test/test.h>
#include <quant1x/datasets/kline_raw.h>

#include <quant1x/datasets/kline.h>
#include <quant1x/datasets/kline_minute.h>

TEST_CASE("download-kline-raw", "[datasets]") {
    std::string code = "sz300773";
    exchange::timestamp now = exchange::last_trading_day();
    const auto adapter = std::make_unique<datasets::DataKLineRaw>();
    adapter->Update(code, now);
}

TEST_CASE("minute-kline", "[datasets]") {
    runtime::global_init();
    std::string code = "sz300773";
    exchange::timestamp now = exchange::last_trading_day();

    const auto adapter = std::make_unique<datasets::DataMinuteKLine>("1min");
    adapter->Update(code, now);
}