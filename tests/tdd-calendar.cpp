#include <quant1x/test/test.h>
#include <quant1x/exchange/calendar.h>

TEST_CASE("date-range-1", "[calendar]") {
    exchange::timestamp begin("2025-07-17");
    exchange::timestamp end("2025-07-18");
    begin = begin.pre_market_time();
    end = end.pre_market_time();
    auto list = exchange::date_range(begin, end);
    std::cout<< list << std::endl;
}

TEST_CASE("date-range-2", "[calendar]") {
    exchange::timestamp begin("2025-07-20");
    exchange::timestamp end("2025-07-20");
    begin = begin.pre_market_time();
    end = end.pre_market_time();
    if (begin > end) {
        std::cout << "begin > end" << std::endl;
    }
    auto list = exchange::date_range(begin, end);
    std::cout<< list << std::endl;
}

TEST_CASE("date-range-3", "[calendar]") {
    exchange::timestamp begin("2025-07-21");
    exchange::timestamp end("2025-07-21");
    begin = begin.pre_market_time();
    end = end.pre_market_time();
    if (begin > end) {
        std::cout << "begin > end" << std::endl;
    }
    auto list = exchange::date_range(begin, end);
    std::cout<< list << std::endl;
}