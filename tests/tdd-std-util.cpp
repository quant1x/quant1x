#include <quant1x/test/test.h>
#include <quant1x/std/filesystem.h>

TEST_CASE("getenv", "[std]") {
	auto homedir = filesystem::homedir();
    std::cout << homedir << std::endl;
    //REQUIRE(homedir == std::string(R"(C:\Users\wangfeng)"));
}
