#include <quant1x/test/test.h>
#include <quant1x/backtest/backtest.h>

using namespace backtest;

TEST_CASE("round-trip extra cases", "[backtest][roundtrip]") {
    BacktestConfig cfg;
    cfg.initial_capital = 100000;
    BacktestEngine engine(cfg, nullptr);

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

    SECTION("short-first then cover") {
        std::vector<Trade> trades;
        // Sell 100 at 15 (short open) then buy 100 at 10 (cover) => profit
        trades.push_back(makeTrade("t1", "AAA", TradeDirection::SHORT, 15.0, 100));
        trades.push_back(makeTrade("t2", "AAA", TradeDirection::LONG, 10.0, 100));
        engine.setTradesForTest(trades);
        engine.computeRoundTripStats();
        const auto &res = engine.getBacktestData().result;
        REQUIRE(res.closed_roundtrips == 1);
        REQUIRE(res.avg_profit > 0);
    }

    SECTION("over-close (closing more than open) - should only count matched lots") {
        std::vector<Trade> trades;
        // Buy 50 @10, then sell 100 @12 => closes 50, remaining sell 50 unmatched (ignored)
    trades.push_back(makeTrade("t1", "BBB", TradeDirection::LONG, 10.0, 50));
    trades.push_back(makeTrade("t2", "BBB", TradeDirection::SHORT, 12.0, 100));
        engine.setTradesForTest(trades);
        engine.computeRoundTripStats();
        const auto &res = engine.getBacktestData().result;
            REQUIRE(res.closed_roundtrips == 1);
            REQUIRE(res.total_trades == 1);
    }

    SECTION("multi-symbol independence") {
        std::vector<Trade> trades;
    trades.push_back(makeTrade("t1", "X", TradeDirection::LONG, 5.0, 100));
    trades.push_back(makeTrade("t2", "Y", TradeDirection::LONG, 20.0, 100));
    trades.push_back(makeTrade("t3", "X", TradeDirection::SHORT, 7.0, 100));
    trades.push_back(makeTrade("t4", "Y", TradeDirection::SHORT, 18.0, 100));
        engine.setTradesForTest(trades);
        engine.computeRoundTripStats();
        const auto &res = engine.getBacktestData().result;
        REQUIRE(res.closed_trades == 2);
        REQUIRE(res.winning_trades == 1); // X: +200, Y: -200 -> one win
    }

    SECTION("zero-pnl roundtrip") {
        std::vector<Trade> trades;
    trades.push_back(makeTrade("t1", "Z", TradeDirection::LONG, 11.0, 100));
    trades.push_back(makeTrade("t2", "Z", TradeDirection::SHORT, 11.0, 100));
        engine.setTradesForTest(trades);
        engine.computeRoundTripStats();
        const auto &res = engine.getBacktestData().result;
        REQUIRE(res.closed_trades == 1);
        REQUIRE(res.winning_trades == 0);
        REQUIRE(res.avg_profit == 0.0);
        REQUIRE(res.avg_loss == 0.0);
    }
}
