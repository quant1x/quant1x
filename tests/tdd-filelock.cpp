#include <quant1x/test/test.h>
#include <quant1x/cache.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <chrono>
#include <format>

namespace meta = quant1x::data::meta;

TEST_CASE("today", "[exchange]") {
    std::string today = api::today();
    std::cout << today << std::endl;
}

TEST_CASE("check-filelock", "[exchange]") {
    meta::Timestamp now = meta::Timestamp::now().since(15,10,0,0);
    auto check = cache::check_update_state("2025-05-29", now);
    std::cout << check << std::endl;
}

TEST_CASE("create-filelock", "[exchange]") {
    meta::Timestamp now = meta::Timestamp::now().since(15,10,0,0);
    cache::done_ipdate("2025-05-29", now);
}

TEST_CASE("update-all", "[exchange]") {
    cache::update_all();
}

TEST_CASE("clean_expired_state_files", "[exchange]") {
    cache::clean_expired_state_files();
}