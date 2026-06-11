#include <quant1x/test/test.h>
#include <gtest/gtest.h>
#include <quant1x/contrib/data/tdx/level1/security_quote.h>

using namespace level1;

TEST(SecurityQuoteImplicitSpread, TradePriceAndBidAskPresent) {
    SecurityQuote q{};
    q.price = 10.5;
    q.bid1 = 10.4;
    q.ask1 = 10.6;

    // midpoint = 10.5 -> implicit spread = 2 * |10.5 - 10.5| = 0
    EXPECT_DOUBLE_EQ(q.implicitSpread(), 0.0);
    EXPECT_DOUBLE_EQ(q.implicitSpreadPct(), 0.0);
}

TEST(SecurityQuoteImplicitSpread, TradePriceOffMid) {
    SecurityQuote q{};
    q.price = 10.55;
    q.bid1 = 10.4;
    q.ask1 = 10.6;

    // midpoint = 10.5 -> implicit spread = 2 * |10.55 - 10.5| = 0.1
    EXPECT_NEAR(q.implicitSpread(), 0.1, 1e-12);
    EXPECT_NEAR(q.implicitSpreadPct(), 0.1 / 10.5 * 100.0, 1e-12);
}

TEST(SecurityQuoteImplicitSpread, NoTradePriceUseOnbook) {
    SecurityQuote q{};
    q.price = 0.0; // invalid
    q.bid1 = 5.0;
    q.ask1 = 5.2;

    // fallback to on-book spread (use near comparison to avoid tiny FP rounding differences)
    EXPECT_NEAR(q.implicitSpread(), 0.2, 1e-12);
    EXPECT_NEAR(q.implicitSpreadPct(), 0.2 / 5.1 * 100.0, 1e-12);
}

TEST(SecurityQuoteImplicitSpread, NoBidAskNoPrice) {
    SecurityQuote q{};
    q.price = 0.0;
    q.bid1 = 0.0;
    q.ask1 = 0.0;
    q.lastClose = 0.0;

    EXPECT_DOUBLE_EQ(q.implicitSpread(), 0.0);
    EXPECT_DOUBLE_EQ(q.implicitSpreadPct(), 0.0);
}

TEST(SecurityQuoteImplicitSpread, FallbackToLastClosePercent) {
    SecurityQuote q{};
    q.price = 0.0;
    q.bid1 = 0.0;
    q.ask1 = 0.0;
    q.lastClose = 20.0;

    EXPECT_DOUBLE_EQ(q.implicitSpread(), 0.0);
    EXPECT_DOUBLE_EQ(q.implicitSpreadPct(), 0.0);
}

// Add some edge cases with NaN
#include <cmath>

TEST(SecurityQuoteImplicitSpread, NaNPrice) {
    SecurityQuote q{};
    q.price = std::numeric_limits<double>::quiet_NaN();
    q.bid1 = 3.0;
    q.ask1 = 3.5;

    EXPECT_DOUBLE_EQ(q.implicitSpread(), 0.5);
}

// main provided by test-gtest (gtest_main) in the test harness; no explicit main here.
