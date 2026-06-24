#include <quant1x/test/test.h>
#include <quant1x/runtime/core.h>
#include <quant1x/trader/fee.h>



TEST_CASE("price-cage-default", "[trader]") {
    runtime::global_init();
    auto price = trader::calculate_price_cage(trader::Direction::BUY, 10.00);
    std::cout << price << std::endl;
}

TEST_CASE("price-cage-strategy", "[trader]") {
    runtime::global_init();
    auto price = trader::calculate_price_cage(1, trader::Direction::BUY, 10.00);
    std::cout << price << std::endl;
    price = trader::calculate_price_cage(1, trader::Direction::SELL, 10.00);
    std::cout << price << std::endl;
}