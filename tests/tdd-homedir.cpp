#include <quant1x/test/test.h>
#include <quant1x/std/util.h>

TEST_CASE("homedir", "[io]") {
    auto homedir = util::homedir();
    std::cout << homedir << std::endl;
}