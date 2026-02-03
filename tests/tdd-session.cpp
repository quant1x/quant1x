#include <quant1x/test/test.h>
#include <quant1x/market/instruments.h>
#include <quant1x/exchange/session.h>
#include <quant1x/proto/data.h>

TEST_CASE("session-minutes", "[session]") {
    runtime::logger_set(true, true);
    exchange::timestamp now = exchange::timestamp::now().since(9,31,0);
    std::cout << exchange::ts_today_session.get().minutes(now) << std::endl;
    std::cout << exchange::ts_today_session.get().minutes() << std::endl;
}

TEST_CASE("check-realtime-status", "[session]") {
    runtime::logger_set(true, true);
    std::cout << exchange::ts_today_session<< std::endl;
    SECTION("盘前") {
        exchange::timestamp now = exchange::timestamp::now().since(9,14,59);
        auto ts = exchange::check_trading_timestamp(now);
        REQUIRE(ts.status == exchange::ExchangePreMarket);
        REQUIRE(exchange::IsOrderCancelable(ts.status) == false);
    }
    SECTION("早盘集合竞价") {
        exchange::timestamp now = exchange::timestamp::now().since(9,15,0);
        auto ts = exchange::check_trading_timestamp(now);
        REQUIRE(exchange::IsCallAuctionOpenPhase(ts.status) == true);
        REQUIRE(exchange::IsOrderCancelable(ts.status) == true);
    }
    SECTION("早盘集合竞价") {
        exchange::timestamp now = exchange::timestamp::now().since(9,20,0);
        auto ts = exchange::check_trading_timestamp(now);
        REQUIRE(exchange::IsCallAuctionOpenPhase(ts.status) == true);
        REQUIRE(exchange::IsOrderCancelable(ts.status) == true);
    }
    SECTION("盘后") {
        exchange::timestamp now = exchange::timestamp::now().since(15,1,0);
        auto ts = exchange::check_trading_timestamp(now);
        REQUIRE(exchange::IsTradingDisabled(ts.status) == true);
    }

    SECTION("收盘集合竞价") {
        exchange::timestamp now = exchange::timestamp::now().since(14,57,0);
        auto ts = exchange::check_trading_timestamp(now);
        REQUIRE(exchange::IsTradingDisabled(ts.status) == false);
    }
}

TEST_CASE("check-realtime-status-v2", "[session]") {
    runtime::logger_set(true, false);
    exchange::timestamp now = exchange::timestamp::now().since(9,14,59);
    auto ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = exchange::timestamp::now().since(9,15,1);
    ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = exchange::timestamp::now().since(9,25,1);
    ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = exchange::timestamp::now().since(9,30,1);
    ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = exchange::timestamp::now().since(11,30,1);
    ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = exchange::timestamp::now().since(12,59,1);
    ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = exchange::timestamp::now().since(13,0,1);
    ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = exchange::timestamp::now().since(14,56,1);
    ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = exchange::timestamp::now().since(14,57,1);
    ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = exchange::timestamp::now().since(14,58,1);
    ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = exchange::timestamp::now().since(14,59,59);
    ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = exchange::timestamp::now().since(15,0,1);
    ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
    now = exchange::timestamp::now().since(15,1,0);
    ts = exchange::check_trading_timestamp(now);
    spdlog::info("{}, realtime update: {}", now.toString(), ts.updateInRealTime);
}