#include <quant1x/test/test.h>
#include <quant1x/data/market/instruments.h>

TEST_CASE("parse-datetime", "[chrono]") {
    SECTION("date") {
        std::string test_date1 = "2025-05-29 15:00:01";
        auto ts1 = exchange::timestamp::parse(test_date1);
        REQUIRE(ts1.toString() == test_date1+".000");

        std::string test_date2 = "2025-05-29 15:00:01.123";
        auto ts2 = exchange::timestamp::parse(test_date2);
        REQUIRE(ts2.toString() == test_date2);

        std::string test_date3 = "2025-05-29";
        auto        ts3        = exchange::timestamp::parse(test_date3);
        REQUIRE(ts3.toString() == test_date3 + " 00:00:00.000");
    }

    SECTION("time") {
        std::string test_date1 = "2025-05-29 15:00:01";
        auto ts1 = exchange::timestamp::parse(test_date1);
        REQUIRE(ts1.only_time() == "15:00:01");

        std::string test_date2 = "2025-05-29 15:00:01.123";
        auto ts2 = exchange::timestamp::parse(test_date2);
        REQUIRE(ts2.only_time() == "15:00:01");
    }
}

TEST_CASE("parse-time-hhmmss", "[timestamp]") {
    exchange::timestamp ts;
    ts = exchange::timestamp::parse_time("15:00:01");
    std::cout << ts.only_time() << std::endl;
    ts = exchange::timestamp::parse_time("15:00:00.999");
    std::cout << ts.only_time() << std::endl;

    ts = exchange::timestamp::parse("2025-05-29 15:00:01");
    std::cout << ts.only_time() << std::endl;
}

TEST_CASE("parse-time-hhmmss.sss", "[timestamp]") {
    exchange::timestamp ts;
    ts = exchange::timestamp::parse_time("15:00:01");
    std::cout << ts.toString() << std::endl;
    ts = exchange::timestamp::parse_time("15:00:00.999");
    std::cout << ts.toString() << std::endl;

    ts = exchange::timestamp::parse("2025-05-29 15:00:01");
    std::cout << ts.toString() << std::endl;
}

//#define DATE_USE_FMT  // 必须放在 #include "date/date.h" 前面
#include <date/date.h>
#include <date/tz.h>

//// 本地时区: 上海
//static const auto _local_zone = date::locate_zone("Asia/ShangHai");
//static const auto _utc_now = std::chrono::system_clock::now();
//static const auto _zone_offset_seconds = _local_zone->get_info(_utc_now).offset;
//static const auto _zone_offset_milliseconds = std::chrono::milliseconds(_zone_offset_seconds).count();

constexpr auto date_time_layout_supports = {
    "%Y-%m-%d %H:%M:%S",        // 2023-05-15 14:30:00
    "%Y-%m-%d",                 // 2023-05-15
    "%Y%m%d",                   // 20230515
    "%Y/%m/%d %H:%M:%S",        // 2023/05/15 14:30:00
    "%m/%d/%Y %H:%M:%S",        // 05/15/2023 14:30:00
    "%H:%M:%S %d-%m-%Y",        // 14:30:00 15-05-2023
    "%Y%m%d %H%M%S",            // 20230515 143000
    "%Y-%m-%dT%H:%M:%SZ",       // ISO 8601 UTC
    "%Y-%m-%dT%H:%M:%S%z",      // ISO 8601 with timezone
    "%a, %d %b %Y %H:%M:%S %Z", // RFC 1123
    "%b %d %Y %H:%M:%S"         // May 15 2023 14:30:00
};

constexpr auto only_time_layout_supports = {
    "%Y-%m-%d %H:%M:%S",        // 2023-05-15 14:30:00
    "%Y-%m-%d",                 // 2023-05-15
    "%Y%m%d",                   // 20230515
    "%Y/%m/%d %H:%M:%S",        // 2023/05/15 14:30:00
    "%m/%d/%Y %H:%M:%S",        // 05/15/2023 14:30:00
    "%H:%M:%S %d-%m-%Y",        // 14:30:00 15-05-2023
    "%H:%M:%S",                 // 14:30:00
    "%H%M%S",                   // 143000
    "%Y%m%d %H%M%S",            // 20230515 143000
    "%Y-%m-%dT%H:%M:%SZ",       // ISO 8601 UTC
    "%Y-%m-%dT%H:%M:%S%z",      // ISO 8601 with timezone
    "%a, %d %b %Y %H:%M:%S %Z", // RFC 1123
    "%b %d %Y %H:%M:%S"         // May 15 2023 14:30:00
};

std::chrono::sys_time<std::chrono::milliseconds>
parse_datetime(const std::string& input) {
    std::istringstream iss(input);
    std::chrono::sys_time<std::chrono::milliseconds> tp;

    // 尝试您原始定义的所有格式
    for (const auto& fmt : date_time_layout_supports) {
        iss.clear();
        iss.seekg(0);
        date::from_stream(iss, fmt, tp);
        if (!iss.fail()) {
            return tp;
        }
    }

    throw std::runtime_error("无法解析时间字符串: " + input);
}

std::chrono::sys_time<std::chrono::milliseconds>
parse_time(const std::string& input) {
    std::istringstream iss(input);
    std::chrono::milliseconds parsedTime{};

    for (const auto& fmt : only_time_layout_supports) {
        iss.clear();
        iss.seekg(0);

        // 显式按 UTC 解析
        date::from_stream(iss, fmt, parsedTime);
        if (!iss.fail()) {
            return std::chrono::sys_time<std::chrono::milliseconds>{parsedTime};
        }
    }

    throw std::runtime_error("无法解析时间字符串: " + input);
}

//TEST_CASE("parse_datetime-v0", "[chrono]") {
//    std::time_t now = std::time(nullptr);
//    std::tm local_tm = *std::localtime(&now); // 本地时间 struct tm
//    std::tm utc_tm = *std::gmtime(&now);      // UTC 时间 struct tm
//
//    // 计算两个时间点之间的差值（秒）
//    std::time_t local = std::mktime(&local_tm);
//    std::time_t utc = std::mktime(&utc_tm);
//
//    auto offset_ms =  (local - utc) * 1000LL;  // 秒转毫秒
//
//    std::cout << "当前时区偏移（毫秒）: " << offset_ms << " ms\n";
//}

TEST_CASE("parse_datetime-v1", "[chrono]") {
    // 测试日期时间
    auto t1 = parse_datetime("2025-04-05 12:30:45");
    CHECK(t1.time_since_epoch().count() == 1743856245000);

    // 格式化回字符串（当作 UTC 时间输出）
    std::string output1 = date::format("%Y-%m-%d %H:%M:%S", t1);
    CHECK(output1 == "2025-04-05 12:30:45.000");

    // 测试日期时间
    auto t2 = parse_time("12:30:50.000");
    std::string output2 = date::format("%H:%M:%S", t2);
    CHECK(output2 == "12:30:50.000");

    // 测试纯日期
    auto t3 = parse_datetime("20250405");
    CHECK(t3.time_since_epoch().count() == 1743811200000);

    std::string output3 = date::format("%Y%m%d", t3);
    CHECK(output3 == "20250405");

    // 测试异常
    REQUIRE_THROWS(parse_datetime("invalid_date"));
}