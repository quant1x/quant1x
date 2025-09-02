#pragma once
#ifndef QUANT1X_ENCODING_YAML_H
#define QUANT1X_ENCODING_YAML_H 1

#include <yaml-cpp/yaml.h>

namespace encoding {

    namespace safe_yaml {
        // 模式1：安全解析（带默认值）——保持原有逻辑
        template<typename T>
        inline void parse_field(const YAML::Node &node, const char *key, T &target, const T &default_val) {
            if (node[key]) {
                try { target = node[key].as<T>(); }
                catch (...) { target = default_val; }
            } else {
                target = default_val;
            }
        }

        // 模式2：严格解析（无默认值）→ 返回是否成功
        template<typename T>
        inline bool try_parse_field(const YAML::Node &node, const char *key, T &target) {
            if (!node[key]) return false;
            try {
                target = node[key].as<T>();
                return true;
            } catch (...) {
                return false;
            }
        }

        // 字符串特化版本
        template<>
        inline bool try_parse_field<std::string>(const YAML::Node &node, const char *key, std::string &target) {
            if (!node[key]) return false;
            try {
                target = node[key].as<std::string>();
                return !target.empty(); // 额外检查空字符串
            } catch (...) {
                return false;
            }
        }
    } // namespace yaml_safe
} // namespace encoding

#endif //QUANT1X_ENCODING_YAML_H
