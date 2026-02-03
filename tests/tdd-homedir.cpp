#include <quant1x/test/test.h>
#include <quant1x/std/filesystem.h>

TEST_CASE("homedir", "[io]") {
    auto homedir = filepath::homedir();
    std::cout << homedir << std::endl;
}