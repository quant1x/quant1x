#include <test/test.h>
#include <q1x/datasets/kline_raw.h>

TEST_CASE("download-kline-raw", "[datasets]") {
    std::string code = "sz300773";
    exchange::timestamp now = exchange::last_trading_day();
    auto adapter = std::make_unique<datasets::DataKLineRaw>();
    adapter->Update(code, now);
}