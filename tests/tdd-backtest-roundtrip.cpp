#include <quant1x/test/test.h>
#include <quant1x/backtest/backtest.h>

using namespace backtest;

TEST_CASE("roundtrip-full-and-partial", "[backtest][roundtrip]") {
    // 构造 BacktestEngine with minimal dummy strategy pointer (nullptr ok for this unit test)
    BacktestConfig cfg;
    cfg.initial_capital = 100000.0;
    // Use default times
    std::shared_ptr<StrategyBase> dummy = nullptr;
    BacktestEngine engine(cfg, dummy);

    // Scenario 1: full open then full close -> should count 1 closed round-trip and be profitable
    // Open: buy 100 @10, Close: sell 100 @12
    Trade t1;
    t1.trade_id = "t1";
    t1.order_id = "o1";
    t1.symbol = "ABC";
    t1.direction = TradeDirection::LONG;
    t1.price = 10.0;
    t1.quantity = 100;

    Trade t2;
    t2.trade_id = "t2";
    t2.order_id = "o2";
    t2.symbol = "ABC";
    t2.direction = TradeDirection::SHORT;
    t2.price = 12.0;
    t2.quantity = 100;

    std::vector<Trade> trades1{t1, t2};
    engine.setTradesForTest(trades1);
    engine.computeRoundTripStats();
    const auto &res1 = engine.getBacktestData().result;
    REQUIRE(res1.closed_trades == 1);
    REQUIRE(res1.winning_trades == 1);
    // realized profit per winning trade should be 200
    REQUIRE(res1.avg_profit == Catch::Approx(200.0));

    // Scenario 2: partial fills -> open 100 (in two fills 40 + 60), then close 100 -> still one closed roundtrip
    // Scenario 2: partial fills -> open 100 (40 + 60), then close 100 @12 -> two completed open-lots
    Trade a1{ "t3","o3","ABC",TradeDirection::LONG, 10.0, 40, 0.0,"" };
    Trade a2{ "t4","o4","ABC",TradeDirection::LONG, 10.0, 60, 0.0,"" };
    Trade a3{ "t5","o5","ABC",TradeDirection::SHORT,12.0,100,0.0,"" };
    std::vector<Trade> trades2{a1, a2, a3};
    engine.setTradesForTest(trades2);
    engine.computeRoundTripStats();
    const auto &res2 = engine.getBacktestData().result;
    REQUIRE(res2.closed_trades == 2);
    REQUIRE(res2.winning_trades == 2);
    // engine stores avg_profit as average per winning round-trip: total 200 / 2 = 100
    REQUIRE(res2.avg_profit == Catch::Approx(100.0));
}
