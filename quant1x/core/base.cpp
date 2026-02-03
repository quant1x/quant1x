#include "base.h"
#include "lazy.h"
#include <quant1x/std/safe.h>
#include <quant1x/std/filesystem.h>
#include <mutex>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace quant1x {
namespace core {

namespace {
    const std::string DEFAULT_BASE_PATH = "~/.q1x";
    const std::string QUANT1X_CONFIG_FILENAME = "quant1x.yaml";

    // 使用Lazy模板实现线程安全的延迟初始化
    Lazy<std::string> base_path_lazy([]() -> std::string {
        std::string path = filepath::expand_user(DEFAULT_BASE_PATH);
        if (path.empty()) {
            return DEFAULT_BASE_PATH;
        } else {
            return path;
        }
    });

    // 全局配置懒加载
    Lazy<BaseConfig> global_config_lazy([]() -> BaseConfig {
        BaseConfig config;
        std::string cfg_filename = (std::filesystem::path(get_base_path()) / QUANT1X_CONFIG_FILENAME).string();
        if (!parse_yaml_config(cfg_filename, config)) {
            throw std::runtime_error("Failed to parse config file: " + cfg_filename);
        }
        return config;
    });
}  // namespace

std::string get_base_path() {
    return base_path_lazy.get_copy();
}

std::string get_meta_path() {
    std::filesystem::path base(get_base_path());
    return (base / "meta").string();
}

// 解析YAML配置，与Go版本的parseYamlConfig保持一致
bool parse_yaml_config(const std::string& filename, BaseConfig& config) {
    // 默认值
    config.debug = false;
    config.filename = filename;
    config.config_map.clear();

    // 若配置文件不存在：使用默认 BaseDir/LogDir，并保留空 map
    if (!std::filesystem::exists(filename)) {
        config.basedir = get_base_path();
        config.logdir = (std::filesystem::path(config.basedir) / "logs").string();
        return true;
    }

    try {
        YAML::Node root = YAML::LoadFile(filename);
        config.config_map = root.as<std::unordered_map<std::string, YAML::Node>>();

        // 解析到强类型配置
        if (root["debug"]) config.debug = root["debug"].as<bool>();
        if (root["basedir"]) config.basedir = root["basedir"].as<std::string>();
        if (root["logdir"]) config.logdir = root["logdir"].as<std::string>();

        // 处理basedir
        if (config.basedir.empty()) {
            config.basedir = get_base_path();
        } else {
            // 展开用户目录
            config.basedir = filepath::expand_user(config.basedir);
        }

        // 处理logdir
        if (config.logdir.empty()) {
            config.logdir = (std::filesystem::path(config.basedir) / "logs").string();
        } else {
            config.logdir = filepath::expand_user(config.logdir);
        }

        // 归一化后的值写回 map
        config.config_map["basedir"] = YAML::Node(config.basedir);
        config.config_map["logdir"] = YAML::Node(config.logdir);
        config.config_map["debug"] = YAML::Node(config.debug);

        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML parsing error: " << e.what() << std::endl;
        return false;
    }
}

// 获取全局配置实例
const BaseConfig& get_global_config() {
    return global_config_lazy.get();
}

// 全局函数接口，与Go版本保持一致的调用方式
const std::string& get_configfile_path() {
    return get_global_config().filename;
}

const std::string& get_logs_path() {
    return get_global_config().logdir;
}

std::string get_data_path() {
    return get_global_config().basedir;
}

std::unordered_map<std::string, YAML::Node> get_config_map() {
    return get_global_config().config_map;  // 返回拷贝，防止误修改
}

const std::unordered_map<std::string, YAML::Node>& get_config_map_ref() {
    return get_global_config().config_map;  // 返回引用，允许修改
}

} // namespace core
} // namespace quant1x
