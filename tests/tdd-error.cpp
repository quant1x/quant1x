#include "quant1x/test/test.h"
#include "quant1x/std/except.h"
#include <system_error>
#include <string>
#include <memory>

TEST_CASE("error-code", "[except]") {
    auto ec = q1x::make_error_code(1, "xxx");
    std::cout << ec.message() << std::endl;
}

TEST_CASE("error-code-2", "[except]") {
    q1x::error err(404, "File not found: /path/to/missing.txt");
    std::cout << err.message() << std::endl;
}