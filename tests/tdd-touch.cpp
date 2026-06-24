#include <quant1x/test/test.h>

#include <quant1x/std/filesystem.h>
#include <quant1x/data/meta/timestamp.h>

namespace meta = quant1x::data::meta;

TEST_CASE("touch-empty", "[io]") {
    std::string filename = "123.txt";
    filesystem::write_file(filename);
}

TEST_CASE("touch-read-ftime", "[io]") {
    std::string filename = "123.txt";
    int64_t ms = filesystem::last_modified_time(filename);
    meta::Timestamp ts = ms;
    std::cout<< ts << std::endl;
}

TEST_CASE("touch-write-ftime", "[io]") {
    std::string filename = "123.txt";
    auto mtime =  meta::Timestamp::now().pre_market_time();
    filesystem::last_modified_time(filename, mtime);
}