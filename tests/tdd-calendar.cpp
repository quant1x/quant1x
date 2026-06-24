#include <quant1x/test/test.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/data/meta/calendar.h>

namespace meta = quant1x::data::meta;

TEST_CASE("date-range-1", "[calendar]") {
    meta::Timestamp begin("2025-07-17");
    meta::Timestamp end("2025-07-18");
    begin = begin.pre_market_time();
    end = end.pre_market_time();
    auto list = meta::date_range(begin, end);
    std::cout<< list << std::endl;
}

TEST_CASE("date-range-2", "[calendar]") {
    meta::Timestamp begin("2025-07-20");
    meta::Timestamp end("2025-07-20");
    begin = begin.pre_market_time();
    end = end.pre_market_time();
    if (begin > end) {
        std::cout << "begin > end" << std::endl;
    }
    auto list = meta::date_range(begin, end);
    std::cout<< list << std::endl;
}

TEST_CASE("date-range-3", "[calendar]") {
    meta::Timestamp begin("2025-07-21");
    meta::Timestamp end("2025-07-21");
    begin = begin.pre_market_time();
    end = end.pre_market_time();
    if (begin > end) {
        std::cout << "begin > end" << std::endl;
    }
    auto list = meta::date_range(begin, end);
    std::cout<< list << std::endl;
}