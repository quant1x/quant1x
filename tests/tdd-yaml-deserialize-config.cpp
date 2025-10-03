#include <quant1x/test/test.h>
#include "../examples/config.h"
#include <quant1x/encoding/yaml.h>

TEST_CASE("deserialize Config from examples/quant1x.yaml", "[encoding][yaml][deserialize]") {
    const std::string yaml_path = "D:/projects/quant1x/quant1x/examples/quant1x.yaml";
    REQUIRE(std::filesystem::exists(yaml_path));
    YAML::Node root = YAML::LoadFile(yaml_path);

    Config cfg;
    REQUIRE_NOTHROW(cfg = encoding::yaml::deserialize<Config>(root));

    // basic sanity checks
    REQUIRE(!cfg.basedir.empty());
    REQUIRE(!cfg.trader.account_id.empty());
    REQUIRE(cfg.trader.strategies.size() == 3);
}
