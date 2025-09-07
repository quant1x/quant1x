#include <quant1x/test/test.h>
#include <quant1x/std/util.h>

TEST_CASE("getenv", "[std]") {
	auto homedir = util::homedir();
    std::cout << homedir << std::endl;
    //REQUIRE(homedir == std::string(R"(C:\Users\wangfeng)"));
}
