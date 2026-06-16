#include <quant1x/test/test.h>
#include <quant1x/cache.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <iostream>
#include <chrono>
#include <format>

TEST_CASE("today", "[exchange]") {
    std::string today = api::today();
    std::cout << today << std::endl;
}

TEST_CASE("check-filelock", "[exchange]") {
    meta::Timestamp now = meta::Timestamp::now().since(15,10,0,0);
    auto check = cache::checkUpdateState("2025-05-29", now);
    std::cout << check << std::endl;
}

TEST_CASE("create-filelock", "[exchange]") {
    meta::Timestamp now = meta::Timestamp::now().since(15,10,0,0);
    cache::doneUpdate("2025-05-29", now);
}

TEST_CASE("update-all", "[exchange]") {
    cache::update_all();
}

TEST_CASE("cleanExpiredStateFiles", "[exchange]") {
    cache::cleanExpiredStateFiles();
}