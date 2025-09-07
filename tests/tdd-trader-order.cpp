#include <test/test.h>
#include <q1x/trader/order_state.h>
#include <users/no1.h>
#include <q1x/trader/trader.h>
#include <q1x/trader/account.h>

TEST_CASE("trader-account", "[trader]") {
    auto info = trader::QueryAccount();
    std::cout << info.value() << std::endl;

    uint64_t strategyId = 1;
    auto const & traderParameter = config::TraderConfig();
    auto opt_strategy = traderParameter->GetStrategyParameterByCode(strategyId);
    auto    &strategyParameter          = opt_strategy.value();
    int quotaForTheNumberOfTargets = strategyParameter.Total;
    auto singleFundsAvailable = trader::CalculateAvailableFundsForSingleTarget(
        quotaForTheNumberOfTargets,
        strategyParameter.Weight,
        strategyParameter.FeeMax,
        strategyParameter.FeeMin);
    std::cout << "singleFundsAvailable = " << singleFundsAvailable << std::endl;
}

TEST_CASE("trader-order-place", "[trader]") {
    auto result = trader::PlaceOrder(trader::Direction::BUY, "S1", "tail", "sh600010", PriceType::LATEST_PRICE, 1.82, 100);
    std::cout << "order result=" << result << std::endl;
}

TEST_CASE("trader-order-query", "") {
    int64_t order_id = 1098907807;
    auto result = trader::QueryOrders(order_id);
    std::cout << result << std::endl;
}

TEST_CASE("trader-position-query", "") {
    auto result = trader::QueryHolding();
    std::cout << result << std::endl;
}

TEST_CASE("order-stat-file", "[trader]") {
    auto date = "2025-06-10";
    auto direction  = trader::Direction::BUY;
    auto strategyInfo = std::make_unique<HousNo1Strategy>();
    auto security_code = "sh600600";
    std::string state_filename = trader::order_state_filename(date, *strategyInfo, direction, security_code);
    std::cout << state_filename << std::endl;
    auto state = trader::CheckOrderState(date, *strategyInfo, security_code, direction);
    std::cout << "check state:" << state << std::endl;
    auto num = trader::CountStrategyOrders(date, *strategyInfo, direction);
    std::cout << "check state num:" << num << std::endl;
    if(!state) {
        state = trader::PushOrderState(date, *strategyInfo, security_code, direction);
        std::cout << "push state:" << state << std::endl;
    }
    auto list = trader::FetchListForFirstPurchase(date, "S2", trader::Direction::BUY);
    std::cout << list<< std::endl;
}