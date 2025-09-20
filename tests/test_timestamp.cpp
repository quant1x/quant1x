#include <gtest/gtest.h>
#include "timestamp.h"

using namespace quant1x;

class TimestampTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test basic construction
TEST_F(TimestampTest, Construction) {
    timestamp ts1(1640995200000);
    EXPECT_EQ(ts1.value(), 1640995200000);

    timestamp ts2;
    EXPECT_TRUE(ts2.empty());

    auto ts_now = timestamp::now();
    EXPECT_FALSE(ts_now.empty());
    EXPECT_GT(ts_now.value(), 0);

    auto ts_zero = timestamp::zero();
    EXPECT_TRUE(ts_zero.empty());
    EXPECT_EQ(ts_zero.value(), 0);
}

// Test construction from date/time
TEST_F(TimestampTest, DateTimeConstruction) {
    timestamp ts(2022, 6, 15, 14, 30, 45, 123);
    EXPECT_FALSE(ts.empty());
    
    auto [year, month, day] = ts.extract();
    EXPECT_EQ(year, 2022);
    EXPECT_EQ(month, 6);
    EXPECT_EQ(day, 15);
}

// Test string parsing
TEST_F(TimestampTest, StringParsing) {
    auto ts1 = timestamp::parse("2022-06-15 14:30:45");
    EXPECT_FALSE(ts1.empty());
    
    auto ts2 = timestamp::parse("2022-06-15");
    EXPECT_FALSE(ts2.empty());
    
    auto [year, month, day] = ts1.extract();
    EXPECT_EQ(year, 2022);
    EXPECT_EQ(month, 6);
    EXPECT_EQ(day, 15);
}

// Test string formatting
TEST_F(TimestampTest, StringFormatting) {
    timestamp ts(2022, 6, 15, 14, 30, 45, 123);
    
    std::string str = ts.toString();
    EXPECT_FALSE(str.empty());
    
    std::string date_only = ts.only_date();
    EXPECT_EQ(date_only, "2022-06-15");
    
    uint32_t yyyymmdd = ts.yyyymmdd();
    EXPECT_EQ(yyyymmdd, 20220615);
}

// Test time operations
TEST_F(TimestampTest, TimeOperations) {
    timestamp ts(2022, 6, 15, 14, 30, 45, 123);
    
    auto start_day = ts.start_of_day();
    auto [year, month, day] = start_day.extract();
    EXPECT_EQ(year, 2022);
    EXPECT_EQ(month, 6);
    EXPECT_EQ(day, 15);
    
    auto today_9am = ts.today(9, 0, 0, 0);
    EXPECT_FALSE(today_9am.empty());
    
    auto offset_ts = ts.offset(2, 30, 0, 0);  // +2.5 hours
    EXPECT_GT(offset_ts.value(), ts.value());
}

// Test comparisons
TEST_F(TimestampTest, Comparisons) {
    timestamp ts1(1640995200000);
    timestamp ts2(1640995260000);  // 1 minute later
    timestamp ts3(1640995200000);  // same as ts1
    
    EXPECT_TRUE(ts1 < ts2);
    EXPECT_FALSE(ts2 < ts1);
    EXPECT_TRUE(ts1 == ts3);
    EXPECT_FALSE(ts1 == ts2);
    EXPECT_TRUE(ts2 > ts1);
    
    // Same date test
    timestamp same_day1(2022, 6, 15, 9, 0, 0, 0);
    timestamp same_day2(2022, 6, 15, 18, 0, 0, 0);
    timestamp diff_day(2022, 6, 16, 9, 0, 0, 0);
    
    EXPECT_TRUE(same_day1.is_same_date(same_day2));
    EXPECT_FALSE(same_day1.is_same_date(diff_day));
}

// Test pre-market time
TEST_F(TimestampTest, PreMarketTime) {
    auto pre_market = timestamp::pre_market_time(2022, 6, 15);
    EXPECT_FALSE(pre_market.empty());
    
    auto [year, month, day] = pre_market.extract();
    EXPECT_EQ(year, 2022);
    EXPECT_EQ(month, 6);
    EXPECT_EQ(day, 15);
}

// Test edge cases
TEST_F(TimestampTest, EdgeCases) {
    // Test invalid parsing (should throw or return empty)
    EXPECT_THROW(timestamp::parse("invalid-date"), std::exception);
    
    // Test zero timestamp
    timestamp zero_ts;
    EXPECT_TRUE(zero_ts.empty());
    EXPECT_EQ(zero_ts.value(), 0);
    
    // Test large values
    timestamp large_ts(LLONG_MAX / 2);  // Avoid overflow
    EXPECT_FALSE(large_ts.empty());
    EXPECT_GT(large_ts.value(), 0);
}

// Test floor and ceil operations
TEST_F(TimestampTest, FloorCeilOperations) {
    timestamp ts(2022, 6, 15, 14, 30, 45, 123);
    
    auto floor_ts = ts.floor();
    EXPECT_LE(floor_ts.value(), ts.value());
    
    auto ceil_ts = ts.ceil();
    EXPECT_GE(ceil_ts.value(), ts.value());
}

// Performance test
TEST_F(TimestampTest, Performance) {
    const int iterations = 10000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        timestamp ts(1640995200000 + i);
        auto str = ts.toString();
        EXPECT_FALSE(str.empty());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete reasonably fast (less than 1 second for 10k operations)
    EXPECT_LT(duration.count(), 1000);
}