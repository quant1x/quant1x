#include <gtest/gtest.h>
#include <quant1x/ta/ma.h>
#include <quant1x/ta/ema.h>
#include <quant1x/ta/sma.h>
#include <quant1x/ta/rsi.h>
#include <quant1x/ta/macd.h>

using namespace ta;

template <typename T>
class TechnicalIndicatorTest : public ::testing::Test {};

using MyTypes = ::testing::Types<double>;
TYPED_TEST_SUITE(TechnicalIndicatorTest, MyTypes);

TYPED_TEST(TechnicalIndicatorTest, MA_ShouldCalculateCorrectly) {
    using T = TypeParam;

    std::vector<T> data = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    Periods<T> periods(5);
    Rolling<T> rolling(data, periods);

    auto ma_func = [](T /*period*/, const T* block, size_t length) -> T {
        T sum = T(0);
        for (size_t i = 0; i < length; ++i)
            sum += block[i];
        return sum / static_cast<T>(length);
    };

    auto result = rolling.template apply<T>(ma_func);

    std::vector<T> expected = {
        T(10),
        T(10.5),
        T(11),
        T(11.5),
        T(12),
        T(13),
        T(13.8),
        T(15),
        T(16),
        T(17)
    };

    for (size_t i = 0; i < result.size(); ++i) {
        EXPECT_NEAR(result[i], expected[i], 1e-6);
    }
}

TYPED_TEST(TechnicalIndicatorTest, EMA_ShouldMatchPandas) {
    using T = TypeParam;

    std::vector<T> data = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    Periods<T> periods(7);
    Rolling<T> rolling(data, periods);

    auto ema_func = [](T period, const T* block, size_t length) -> T {
        T alpha = T(2) / (period + T(1));
        T prev_ema = block[0];

        for (size_t i = 1; i < length; ++i) {
            prev_ema = alpha * block[i] + (T(1) - alpha) * prev_ema;
        }

        return prev_ema;
    };

    auto result = rolling.template apply<T>(ema_func);
    std::vector<T> expected = {
        T(10.0),
        T(10.25),
        T(10.625),
        T(11.15625),
        T(11.7890625),
        T(12.5212890625),
        T(13.29041015625),
        T(14.0678076171875),
        T(14.8408547124023438),
        T(15.6056410341796875)
    };

    for (size_t i = 0; i < result.size(); ++i) {
        EXPECT_NEAR(result[i], expected[i], 1e-6);
    }
}

TYPED_TEST(TechnicalIndicatorTest, RSI_AllRise_ShouldReturn100AtFullWindow) {
    using T = TypeParam;

    std::vector<T> data = {10, 11, 12, 13, 14, 15, 16}; // 7 天上涨
    RSI<TypeParam> rsi(7);
    auto result = rsi.calculate(data);

    EXPECT_NEAR(result.back(), T(100), 1e-6);
}

TYPED_TEST(TechnicalIndicatorTest, MACD_ShouldHavePositiveValueWhenUpTrend) {
    using T = TypeParam;

    std::vector<T> data = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    MACD<TypeParam> macd(12, 26, 9);
    auto result = macd.calculate(data);

    EXPECT_GT(result.back(), T(0));
}