#include <quant1x/test/test.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/data/meta/session.h>
#include <quant1x/proto/data.h>

TEST_CASE("session-minutes", "[session]") {
    runtime::logger_set(true, true);
    meta::Timestamp now = meta::Timestamp::now().since(9,31,0);
    std::cout << meta::TradingSession.get().minutes(now) << std::endl;
    std::cout << meta::TradingSession.get().minutes() << std::endl;
}

TEST_CASE("check-realtime-status", "[session]") {
    runtime::logger_set(true, true);
    std::cout << meta::TradingSession<< std::endl;
    SECTION("盘前") {
        meta::Timestamp now = meta::Timestamp::now().since(9,14,59);
        auto ts = meta::check_trading_timestamp(now);
        REQUIRE(ts.status == meta::ExchangePreMarket);
        REQUIRE(meta::perm_can_cancel(ts.status) == false);
    }
    SECTION("早盘集合竞价") {
        meta::Timestamp now = meta::Timestamp::now().since(9,15,0);
        auto ts = meta::check_trading_timestamp(now);
        REQUIRE(meta::ts_is_trading_disabled(ts.status) == true);
        REQUIRE(meta::perm_can_cancel(ts.status) == true);
    }
    SECTION("早盘集合竞价") {
        meta::Timestamp now = meta::Timestamp::now().since(9,20,0);
        auto ts = meta::check_trading_timestamp(now);
        REQUIRE(meta::ts_is_trading_disabled(ts.status) == true);
        REQUIRE(meta::perm_can_cancel(ts.status) == true);
    }
    SECTION("盘后") {
        meta::Timestamp now = meta::Timestamp::now().since(15,1,0);
        auto ts = meta::check_trading_timestamp(now);
        REQUIRE(meta::ts_is_trading_disabled(ts.status) == true);
    }

    SECTION("收盘集合竞价") {
        meta::Timestamp now = meta::Timestamp::now().since(14,57,0);
        auto ts = meta::check_trading_timestamp(now);
        REQUIRE(meta::ts_is_trading_disabled(ts.status) == false);
    }
}

TEST_CASE("check-realtime-status-v2", "[session]") {
    runtime::logger_set(true, false);
    meta::Timestamp now = meta::Timestamp::now().since(9,14,59);
    auto ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = meta::Timestamp::now().since(9,15,1);
    ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = meta::Timestamp::now().since(9,25,1);
    ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = meta::Timestamp::now().since(9,30,1);
    ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = meta::Timestamp::now().since(11,30,1);
    ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = meta::Timestamp::now().since(12,59,1);
    ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = meta::Timestamp::now().since(13,0,1);
    ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = meta::Timestamp::now().since(14,56,1);
    ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = meta::Timestamp::now().since(14,57,1);
    ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = meta::Timestamp::now().since(14,58,1);
    ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = meta::Timestamp::now().since(14,59,59);
    ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = meta::Timestamp::now().since(15,0,1);
    ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = meta::Timestamp::now().since(15,1,0);
    ts = meta::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
}