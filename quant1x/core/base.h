#pragma once
#ifndef QUANT1X_CORE_BASE_H
#define QUANT1X_CORE_BASE_H

#include <string>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include "defaults.h"

namespace quant1x {
namespace core {

// 返回默认的基础路径, 如果无法展开用户目录则返回默认路径
std::string get_base_path();

/**
 * @brief 获取元数据存储路径
 *
 * 基于基础路径构建元数据子目录的完整路径. 
 *
 * @return 元数据目录的完整路径字符串
 */
std::string get_meta_path();


// BaseConfig 基础配置结构体, 继承Defaultable以支持默认值应用
struct BaseConfig : public Defaultable<BaseConfig> {
    bool debug = false; // 是否处于调试模式
    std::string basedir; // 基础目录
    std::string logdir;  // 日志目录
    std::string filename; // 配置文件路径
    std::unordered_map<std::string, YAML::Node> config_map; // 配置数据映射

    BaseConfig() = default;

    // 实现do_apply_defaults
    void do_apply_defaults() override {
        if (basedir.empty()) {
            basedir = get_base_path();
        }
        if (logdir.empty()) {
            logdir = basedir + "/logs";
        }
        // 其他默认值逻辑
    }
};

// 解析YAML配置, 与Go版本的parseYamlConfig保持一致
bool parse_yaml_config(const std::string& filename, BaseConfig& config);

// 获取全局配置实例
const BaseConfig& get_global_config();

// 全局函数接口, 与Go版本保持一致的调用方式
const std::string& get_configfile_path();
const std::string& get_logs_path();
std::string get_data_path();
std::unordered_map<std::string, YAML::Node> get_config_map();
const std::unordered_map<std::string, YAML::Node>& get_config_map_ref();

} // namespace core
} // namespace quant1x

#endif // QUANT1X_CORE_BASE_H
