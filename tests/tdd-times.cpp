#include <quant1x/test/test.h>

#include <quant1x/std/time.h>


TEST_CASE("time-now", "[times]") {
    auto today = api::today();
    std::cout << today << std::endl;

    auto timestamp = api::get_timestamp();
    std::cout << timestamp << std::endl;
}

TEST_CASE("parse-date", "[times]") {
    std::string date_str = "2025-06-19";
    auto ts = api::parse_date(date_str);
    std::cout << "Parsed date: " << ts << std::endl;
}
