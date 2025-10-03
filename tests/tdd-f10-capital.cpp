#include <quant1x/encoding/json.h>
#include <quant1x/test/test.h>

#include <quant1x/factors/f10.h>
#include <quant1x/factors/share-holder.h>

TEST_CASE("f10-share-holder", "[factors]") {
    auto list = dfcf::GetCacheShareHolder("sh600600", "2025-05-20");
    std::cout << list.size() << std::endl;
}

TEST_CASE("f10-struct", "[factors]") {
    F10 info;
    info.Code = "111";
    std::vector<F10> list;
    list.push_back(info);

    encoding::save_json(list, "output.csv");
    std::cout << "CSV file written successfully." << std::endl;
}

TEST_CASE("csv-header", "[csv]") {
    F10Feature f10{};
    std::cout<< f10.headers() << std::endl;
}