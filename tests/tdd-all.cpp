#include <quant1x/test/test.h>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
// #include <quant1x/contrib/data/tdx/level1/encoding.h>  // removed in refactor

#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <format>
#include <iostream>
#include <quant1x/base/time.h>
#include <quant1x/runtime/core.h>
#include <quant1x/base/filesystem.h>

TEST_CASE("stem", "[filesystem]") {
    std::string fn = "c:\\abc\\e.exe.zip";
    std::cout<<filesystem::remove_extension(fn)<< std::endl;

    std::string fn1 = "e.exe.zip";
    std::cout<<filesystem::remove_extension(fn1)<< std::endl;
}

TEST_CASE("from-1", "[strings]") {
    char code[6] = {'6','0','0','1','1','5'};
    REQUIRE(strings::from(code) == std::string("600115"));
    uint8_t code1[6] = {'6','0','0','1','1','5'};
    REQUIRE(strings::from(code1) == std::string("600115"));
}

// 将字符串转换为 uint16_t
uint16_t stringToUint16(const std::string& str) {
    if (str.empty()) {
        throw std::invalid_argument("Input is empty");
    }

    // 检查是否只包含数字字符
    for (char ch : str) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            throw std::invalid_argument("Invalid input: contains non-digit characters");
        }
    }

    // 使用 std::stoul 转换字符串为无符号长整型
    size_t pos = 0;
    unsigned long value = std::stoul(str, &pos);

    // 检查是否还有未解析的字符
    if (pos != str.size()) {
        throw std::invalid_argument("Invalid characters in input");
    }

    // 检查范围是否符合 uint16_t
    if (value > 65535) {
        throw std::out_of_range("Value out of range for uint16_t");
    }

    return static_cast<uint16_t>(value);
}

// 单元测试
TEST_CASE("Test stringToUint16 function", "[stringToUint16]") {
    SECTION("Valid inputs") {
        REQUIRE(stringToUint16("0") == 0);
        REQUIRE(stringToUint16("123") == 123);
        REQUIRE(stringToUint16("65535") == 65535);
    }

    SECTION("Leading/trailing spaces") {
        REQUIRE_THROWS_AS(stringToUint16(" 123"), std::invalid_argument);
        REQUIRE_THROWS_AS(stringToUint16("123 "), std::invalid_argument);
    }

    SECTION("Non-numeric characters") {
        REQUIRE_THROWS_AS(stringToUint16("abc"), std::invalid_argument);
        REQUIRE_THROWS_AS(stringToUint16("123abc"), std::invalid_argument);
        REQUIRE_THROWS_AS(stringToUint16("123.45"), std::invalid_argument);
    }

    SECTION("Out of range values") {
        REQUIRE_THROWS_AS(stringToUint16("65536"), std::out_of_range);
        REQUIRE_THROWS_AS(stringToUint16("100000"), std::out_of_range);
    }

    SECTION("Empty string") {
        REQUIRE_THROWS_AS(stringToUint16(""), std::invalid_argument);
    }

    SECTION("Negative numbers") {
        REQUIRE_THROWS_AS(stringToUint16("-1"), std::invalid_argument);
        REQUIRE_THROWS_AS(stringToUint16("-123"), std::invalid_argument);
    }
}

// 单元测试
TEST_CASE("Test split function", "[split]") {
    SECTION("Basic split") {
        std::vector<std::string> result = strings::split("apple,banana,cherry,date", ",");
        REQUIRE(result == std::vector<std::string>{"apple", "banana", "cherry", "date"});
    }

    SECTION("Multi-character delimiter") {
        std::vector<std::string> result = strings::split("apple->banana->cherry->date", "->");
        REQUIRE(result == std::vector<std::string>{"apple", "banana", "cherry", "date"});
    }

    SECTION("Ignore empty strings") {
        std::vector<std::string> result = strings::split("->apple->->banana->->cherry->", "->", true);
        REQUIRE(result == std::vector<std::string>{"apple", "banana", "cherry"});
    }

    SECTION("Do not ignore empty strings") {
        std::vector<std::string> result = strings::split("->apple->->banana->->cherry->", "->", false);
        REQUIRE(result == std::vector<std::string>{"", "apple", "", "banana", "", "cherry", ""});
    }

    SECTION("No delimiter found") {
        std::vector<std::string> result = strings::split("singleword", "->");
        REQUIRE(result == std::vector<std::string>{"singleword"});
    }

    SECTION("Empty input string") {
        std::vector<std::string> result = strings::split("", "->");
        REQUIRE(result == std::vector<std::string>{});
    }

    SECTION("All delimiters") {
        std::vector<std::string> result = strings::split("->->->", "->", false);
        REQUIRE(result == std::vector<std::string>{"", "", "", ""});
    }
}

TEST_CASE("Test join function", "[join]") {
    SECTION("Basic join") {
        std::string result = strings::join(std::initializer_list<std::string>{"apple", "banana", "cherry", "date"}, ",");
        REQUIRE(result == "apple,banana,cherry,date");
    }

    SECTION("Multi-character delimiter") {
        std::string result = strings::join(std::initializer_list<std::string>{"apple", "banana", "cherry", "date"}, "->");
        REQUIRE(result == "apple->banana->cherry->date");
    }

    SECTION("Single element") {
        std::string result = strings::join(std::initializer_list<std::string>{"singleword"}, ",");
        REQUIRE(result == "singleword");
    }

    SECTION("Empty container") {
        std::string result = strings::join(std::initializer_list<std::string>{}, ",");
        REQUIRE(result == "");
    }

    SECTION("Empty strings in container") {
        std::string result = strings::join(std::initializer_list<std::string>{"", "apple", "", "banana", "", "cherry", ""}, "->");
        REQUIRE(result == "->apple->->banana->->cherry->");
    }
}

TEST_CASE("Test split and join together", "[split-join]") {
    SECTION("Split and join consistency") {
        std::string input = "apple->banana->cherry->date";
        std::string delimiter = "->";

        std::vector<std::string> tokens = strings::split(input, delimiter);
        std::string joined = strings::join(tokens, delimiter);

        REQUIRE(joined == input);
    }

    SECTION("Split with empty strings and join") {
        std::string input = "->apple->->banana->->cherry->";
        std::string delimiter = "->";

        std::vector<std::string> tokens = strings::split(input, delimiter, false);
        std::string joined = strings::join(tokens, delimiter);

        REQUIRE(joined == input);
    }
}

// 单元测试
TEST_CASE("Test trim function", "[strings]") {
    SECTION("Trim spaces") {
        REQUIRE(strings::trim("   Hello, World!   ") == "Hello, World!");
        REQUIRE(strings::trim(" Hello, World!") == "Hello, World!");
        REQUIRE(strings::trim("Hello, World! ") == "Hello, World!");
    }

    SECTION("Trim tabs and newlines") {
        REQUIRE(strings::trim("\tHello, World!\n") == "Hello, World!");
        REQUIRE(strings::trim("\n\rHello, World!\t") == "Hello, World!");
        REQUIRE(strings::trim("\t\n\rHello, World!\t\n\r") == "Hello, World!");
    }

//    SECTION("Trim full-width spaces") {
//        REQUIRE(trim("\u3000\u3000Hello, World!\u3000\u3000") == "Hello, World!");
//        REQUIRE(trim("\u3000Hello, World!\u3000") == "Hello, World!");
//    }

    SECTION("Empty strings") {
        REQUIRE(strings::trim("") == "");
        REQUIRE(strings::trim("   ") == "");
        REQUIRE(strings::trim("\t\n\r") == "");
    }

    SECTION("No trimming needed") {
        REQUIRE(strings::trim("Hello, World!") == "Hello, World!");
        REQUIRE(strings::trim("Hello") == "Hello");
    }

//    SECTION("Mixed whitespace") {
//        REQUIRE(strings::trim(" \t \n Hello, World! \r \u3000 ") == "Hello, World!");
//    }
}

#include <quant1x/runtime/once.h>
static int test_number = 0;
void test_once() {
    std::cout << "test_number incr" << std::endl;
    test_number+= 1;
}

TEST_CASE("rolling-once", "[base]") {
    runtime::logger_set(true, true);
    auto once = RollingOnce::create("t1", 5);
    for (int i = 0; i < 10; ++i) {
        once->Do(test_once);
        spdlog::debug("test_number={}", test_number);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    //std::this_thread::sleep_for(std::chrono::seconds(60));
}

#include <algorithm> // for std::clamp
#include <cmath>       // for std::copysign

// 声明被测试的函数
double v1Decimal(double value, int digits = 2) {
    if (digits < 0) {
        digits = 0;
    }

    if (std::isnan(value)) {
        value = 0.0;
    }

    double half = 5.0;
    if (std::signbit(value)) {
        half = -half;
    }

    double n10 = std::pow(10, digits);
    double nj1 = std::pow(10, digits + 1);

    return std::trunc((value * nj1 + half) / 10.0) / n10;
}

double v2Decimal(double value, int digits = 2) {
    if (digits < 0) digits = 0;
    if (digits > 9) digits = 9;

    static constexpr double kPowersOf10[] = {
        1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9
    };

    if (std::isnan(value)) return 0.0;

    double half = 5.0;
    if (std::signbit(value)) half = -half;

    double nj1 = kPowersOf10[digits + 1];
    double scaled = value * nj1 + half;
    double truncated = std::trunc(scaled / 10.0);

    return truncated / (nj1 / 10.0);
}

double v3Decimal(double value, int digits = 2) {
    digits = std::clamp(digits, 0, 9); // 无分支(视编译器实现而定)

    static constexpr double kPowersOf10[] = {
        1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9
    };

    if (std::isnan(value)) return 0.0;

    double half = 5.0;
    if (std::signbit(value)) half = -half;

    double nj1 = kPowersOf10[digits + 1];
    double scaled = value * nj1 + half;
    double truncated = std::trunc(scaled / 10.0);

    return truncated / (nj1 / 10.0);
}

double Decimal(double value, int digits = 2) {
    digits = std::clamp(digits, 0, 9);

    static constexpr double kPowersOf10[] = {
        1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9
    };

    if (std::isnan(value)) return 0.0;

    double half = std::copysign(5.0, value);  // ✅ 无分支处理符号

    double nj1 = kPowersOf10[digits + 1];
    double scaled = value * nj1 + half;
    double truncated = std::trunc(scaled / 10.0);

    return truncated / (nj1 / 10.0);
}

TEST_CASE("TestDecimal", "[decimal]") {
    struct TestCase {
        std::string name;
        double value;
        int digits;       // 是否提供 digits 参数
        bool has_digits;  // 标记是否提供 digits 参数
        double expected;
    };

    std::vector<TestCase> tests = {
        {"T9.8", 9.825, 0, true, 10},
        {"T9.8-1", 9.825, 2, false, 9.83},  // 默认参数
        {"T9.824", 9.824, 2, true, 9.82},
        {"T9.825", 9.825, 2, true, 9.83},
        {"T9.826", 9.826, 2, true, 9.83},
        {"T0.116", 0.116, 2, true, 0.12},
        {"T0.11", 0.1115355659035776, 2, true, 0.11},
        {"T-0.11", -0.1115355659035776, 2, true, -0.11},
        {"T-0.016", -0.016, 2, true, -0.02},
        {"T34423.125", 34423.125, 2, true, 34423.13}
    };

    for (const auto& test : tests) {
        SECTION(test.name) {
            double result;
            if (test.has_digits) {
                result = Decimal(test.value, test.digits);
            } else {
                result = Decimal(test.value);  // 使用默认精度
            }
            REQUIRE(result == Catch::Approx(test.expected).margin(0.0001));
        }
    }
}

TEST_CASE("BenchmarkDecimal", "[!benchmark]") {
    // 确保所有版本返回一致结果
    REQUIRE(v1Decimal(9.825, 2) == Catch::Approx(9.83));
    REQUIRE(v2Decimal(9.825, 2) == Catch::Approx(9.83));
    REQUIRE(v3Decimal(9.825, 2) == Catch::Approx(9.83));
    REQUIRE(Decimal(9.825, 2) == Catch::Approx(9.83));

    BENCHMARK_ADVANCED("v1Decimal (std::pow)") {
                                                   return [](int i) {
                                                       volatile double result = v1Decimal(9.825 + i * 0.001, 2);
                                                       (void)result; // 防止编译器警告
                                                   };
                                               };

    BENCHMARK_ADVANCED("v2Decimal (lookup table)") {
                                                       return [](int i) {
                                                           volatile double result = v2Decimal(9.825 + i * 0.001, 2);
                                                           (void)result;
                                                       };
                                                   };

    BENCHMARK_ADVANCED("v3Decimal (clamp + lookup)") {
                                                         return [](int i) {
                                                             volatile double result = v3Decimal(9.825 + i * 0.001, 2);
                                                             (void)result;
                                                         };
                                                     };

    BENCHMARK_ADVANCED("Decimal (copysign + clamp)") {
                                                         return [](int i) {
                                                             volatile double result = Decimal(9.825 + i * 0.001, 2);
                                                             (void)result;
                                                         };
                                                     };
}

#include <benchmark/benchmark.h>
TEST_CASE("BenchmarkDecimal-2", "[!benchmark]") {
    // 确保所有版本返回一致结果
    REQUIRE(v1Decimal(9.825, 2) == Catch::Approx(9.83));
    REQUIRE(v2Decimal(9.825, 2) == Catch::Approx(9.83));
    REQUIRE(v3Decimal(9.825, 2) == Catch::Approx(9.83));
    REQUIRE(Decimal(9.825, 2) == Catch::Approx(9.83));

    BENCHMARK_ADVANCED("v1Decimal (std::pow)") {
                                                   return [](int i) {
                                                       volatile double result = v1Decimal(9.825 + i * 0.001, 2);
                                                       benchmark::DoNotOptimize(result); // 防止优化
                                                   };
                                               };

    BENCHMARK_ADVANCED("v2Decimal (lookup table)") {
                                                       return [](int i) {
                                                           volatile double result = v2Decimal(9.825 + i * 0.001, 2);
                                                           benchmark::DoNotOptimize(result); // 防止优化
                                                       };
                                                   };

    BENCHMARK_ADVANCED("v3Decimal (clamp + lookup)") {
                                                         return [](int i) {
                                                             volatile double result = v3Decimal(9.825 + i * 0.001, 2);
                                                             benchmark::DoNotOptimize(result); // 防止优化
                                                         };
                                                     };

    BENCHMARK_ADVANCED("Decimal (copysign + clamp)") {
                                                         return [](int i) {
                                                             volatile double result = Decimal(9.825 + i * 0.001, 2);
                                                             benchmark::DoNotOptimize(result); // 防止优化
                                                         };
                                                     };
}