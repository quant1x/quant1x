#include <quant1x/test/test.h>
#include <quant1x/backtest/backtest.h>

using namespace backtest;

TEST_CASE("backtest result compatibility fields", "[backtest][compat]") {
    BacktestConfig cfg;
    cfg.initial_capital = 100000.0;
    BacktestEngine engine(cfg, nullptr);

    // reuse the same makeTrade helper pattern used by other tests to ensure
    // full member initialization (compiler treats missing fields as errors).
    auto makeTrade = [](const std::string &tid, const std::string &sym, TradeDirection dir, double price, double qty) {
        Trade t;
        t.trade_id = tid;
        t.order_id = "";
        t.symbol = sym;
        t.direction = dir;
        t.price = price;
        t.quantity = qty;
        t.fee = 0.0;
        t.trade_time = "";
        return t;
    };

    std::vector<Trade> trades;
    trades.push_back(makeTrade("t1", "AAA", TradeDirection::LONG, 10.0, 100));
    trades.push_back(makeTrade("t2", "AAA", TradeDirection::SHORT, 12.0, 100));

    engine.setTradesForTest(trades);
    engine.computeRoundTripStats();
    const auto &res = engine.getBacktestData().result;

    SECTION("trade_events_count equals trades.size") {
        REQUIRE(res.trade_events_count == trades.size());
    }

    SECTION("closed_roundtrips equals closed_trades and total_trades") {
        REQUIRE(res.closed_roundtrips == res.closed_trades);
        REQUIRE(res.closed_roundtrips == res.total_trades);
    }
}
