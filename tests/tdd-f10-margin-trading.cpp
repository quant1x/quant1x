#include <quant1x/test/test.h>
#include <quant1x/data/market.h>
#include <quant1x/runtime/core.h>

TEST_CASE("check-target-is-margin-trading", "[f10]") {
    runtime::global_init();
    runtime::logger_set(true, true);
    std::string code = "600178";
    std::cout << "code:" << code<< ", IsMarginTradingTarget=" << std::boolalpha << data::is_margin_trading_target(code) << std::endl;
    code = "600600";
    std::cout  << "code:" << code<< ", IsMarginTradingTarget=" << std::boolalpha << data::is_margin_trading_target(code) << std::endl;
    spdlog::default_logger()->flush();
}