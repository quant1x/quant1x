#include <quant1x/test/test.h>
#include <yaml-cpp/yaml.h>
#include <filesystem>

// This test intentionally avoids depending on example-only types (Config etc.)
// and instead validates that the shipped `examples/quant1x.yaml` contains the
// expected top-level structure. Using an absolute path keeps the test robust
// across differing CMake/test working directories.

TEST_CASE("examples/quant1x.yaml - basic structure exists", "[encoding][yaml][smoke]") {
    const std::string yaml_path = "D:/projects/quant1x/quant1x/examples/quant1x.yaml";
    REQUIRE(std::filesystem::exists(yaml_path));

    YAML::Node root = YAML::LoadFile(yaml_path);

    REQUIRE(root.IsDefined());
    // trader.account_id should be present and non-empty
    REQUIRE(root["trader"].IsDefined());
    REQUIRE(root["trader"]["account_id"].IsDefined());
    REQUIRE(!root["trader"]["account_id"].as<std::string>().empty());

    // trader.strategies should be a sequence with 3 entries
    REQUIRE(root["trader"]["strategies"].IsDefined());
    REQUIRE(root["trader"]["strategies"].IsSequence());
    REQUIRE(root["trader"]["strategies"].size() == 3);
}
