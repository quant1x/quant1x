#include <quant1x/test/test.h>

#include <quant1x/level1/server.h>

TEST_CASE("detect", "[level1]") {
    auto list = level1::detect();
    for(auto const & v : list) {
        std::cout << v.Host << "," << v.CrossTime << std::endl;
    }
}