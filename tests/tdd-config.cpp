#include <quant1x/test/test.h>
#include <quant1x/runtime/config.h>

// 从YAML加载配置
config::TraderParameter LoadConfigFromYAML(const std::string& filename) {
    config::TraderParameter config;
    try {
        YAML::Node yaml = YAML::LoadFile(filename);
        config = yaml["trader"].as<config::TraderParameter>();
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML解析错误: " << e.what() << std::endl;
    }
    return config;
}

TEST_CASE("q2x-config-basic", "[config]") {

    const std::string& cfg_filename = config::config_filename();
    std::cout << cfg_filename << std::endl;
    config::TraderParameter traderParameter{};
    // 加载配置
    config::TraderParameter config = LoadConfigFromYAML(cfg_filename);
    traderParameter = config;
    // 打印配置
    std::cout << traderParameter << std::endl;
}

TEST_CASE("q2x-config-base", "[config]") {
    // 加载配置
    auto const & config = config::TraderConfig();

    // 打印配置
    std::cout << *config << std::endl;
}