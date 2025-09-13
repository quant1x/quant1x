#pragma once
#ifndef QUANT1X_ENCODING_YAML_H
#define QUANT1X_ENCODING_YAML_H 1

#include <yaml-cpp/yaml.h>
#include <boost/pfr.hpp>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>
#include <iostream>
#include <fstream>

namespace encoding {
    namespace safe_yaml {
        // 模式1：安全解析（带默认值）——保持原有逻辑
        template<typename T>
        inline void parse_field(const YAML::Node &node, const char *key, T &target, const T &default_val) {
            if (node[key]) {
                try {
                    target = node[key].as<T>();
                } catch (...) {
                    target = default_val;
                }
            } else {
                target = default_val;
            }
        }

        // 模式2：严格解析（无默认值）→ 返回是否成功
        template<typename T>
        inline bool try_parse_field(const YAML::Node &node, const char *key, T &target) {
            if (!node[key])
                return false;
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
            if (!node[key])
                return false;
            try {
                target = node[key].as<std::string>();
                return !target.empty(); // 额外检查空字符串
            } catch (...) {
                return false;
            }
        }
    } // namespace safe_yaml

    namespace yaml {
        // 类型特征
        template<typename T>
        struct is_vector : std::false_type {
        };

        template<typename T, typename A>
        struct is_vector<std::vector<T, A> > : std::true_type {
        };

        template<typename T>
        constexpr bool is_vector_v = is_vector<T>::value;

        // ========== 类型特征：检测是否为 std::map ==========
        template<typename T>
        struct is_map : std::false_type {
        };

        template<typename K, typename V, typename Compare, typename Alloc>
        struct is_map<std::map<K, V, Compare, Alloc> > : std::true_type {
        };

        template<typename T>
        constexpr bool is_map_v = is_map<T>::value;

        // ================================================
        // 对称的 YAML 序列化/反序列化器
        // ================================================

        // ========== 公共辅助函数 ==========
        template<typename T>
        inline std::string_view get_field_name(std::size_t idx) {
            return boost::pfr::names_as_array<T>()[idx];
        };

        // ========== 反序列化 ==========
        template<typename T>
        inline void deserialize_to_object(const YAML::Node &node, T &obj);
        template<typename T>
        inline void deserialize_vector(const YAML::Node &node, T &vec);
        template<typename T>
        inline void deserialize_map(const YAML::Node &node, T &map);

        template<typename T>
        inline void deserialize_field(const YAML::Node &node, T &field) {
            if constexpr (std::is_same_v<T, std::string>) {
                field = node.as<std::string>();
            } else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
                field = node.as<T>();
            } else if constexpr (is_vector_v<T>) {
                deserialize_vector(node, field);
            } else if constexpr (is_map_v<T>) {
                deserialize_map(node, field);
            } else {
                deserialize_to_object(node, field);
            }
        }

        template<typename T>
        inline void deserialize_to_object(const YAML::Node &node, T &obj) {
            boost::pfr::for_each_field(obj, [&](auto &field, std::size_t idx) {
                const auto field_name = get_field_name<T>(idx);
                if (node[field_name]) {
                    deserialize_field(node[field_name], field);
                }
            });
        }

        template<typename T>
        inline void deserialize_vector(const YAML::Node &node, T &vec) {
            using ValueType = typename T::value_type;
            vec.clear();
            for (const auto &item: node) {
                ValueType value{};
                deserialize_field(item, value);
                vec.push_back(value);
            }
        }

        template<typename T>
        inline void deserialize_map(const YAML::Node &node, T &map) {
            using KeyType = typename T::key_type;
            using ValueType = typename T::mapped_type;

            map.clear();
            for (const auto &pair: node) {
                // YAML map 节点的 key 是 pair.first, value 是 pair.second
                KeyType key{};
                ValueType value{};

                // 反序列化键
                deserialize_field(pair.first, key);
                // 反序列化值
                deserialize_field(pair.second, value);

                map.emplace(std::move(key), std::move(value));
            }
        }

        // ========== 序列化 ==========
        template<typename T>
        inline void serialize_field(YAML::Node &node, const T &field);
        template<typename T>
        inline void serialize_vector(YAML::Node &node, const T &vec);
        template<typename T>
        inline void serialize_map(YAML::Node &node, const T &map);

        template<typename T>
        inline YAML::Node serialize_to_node(const T &obj) {
            YAML::Node node;
            boost::pfr::for_each_field(obj, [&](const auto &field, std::size_t idx) {
                const auto field_name = get_field_name<T>(idx);
                auto &&target_node = node[field_name]; // ✅ 万能引用，兼容一切
                serialize_field(target_node, field);
            });
            return node;
        }

        template<typename T>
        inline void serialize_field(YAML::Node &node, const T &field) {
            if constexpr (std::is_same_v<T, std::string> || std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
                node = field;
            } else if constexpr (is_vector_v<T>) {
                serialize_vector(node, field);
            } else if constexpr (is_map_v<T>) {
                serialize_map(node, field);
            } else {
                node = serialize_to_node(field);
            }
        }

        template<typename T>
        inline void serialize_vector(YAML::Node &node, const T &vec) {
            for (const auto &item: vec) {
                YAML::Node item_node;
                serialize_field(item_node, item);
                node.push_back(item_node);
            }
        }

        template<typename T>
        inline void serialize_map(YAML::Node &node, const T &map) {
            using KeyType = typename T::key_type;
            using ValueType = typename T::mapped_type;

            node = YAML::Node(YAML::NodeType::Map); // 明确设为 Map 类型

            for (const auto &[key, value]: map) {
                YAML::Node key_node, value_node;
                serialize_field(key_node, key);
                serialize_field(value_node, value);

                // YAML::Node 支持直接赋值键值对
                node[key_node] = value_node;
            }
        }

        // 反序列化：YAML → struct
        template<typename T>
        inline T deserialize(const YAML::Node &node) {
            T obj{};
            deserialize_to_object(node, obj);
            return obj;
        }

        // 序列化：struct → YAML
        template<typename T>
        inline YAML::Node serialize(const T &obj) {
            return serialize_to_node(obj);
        }
    } // namespace yaml

    template<typename T>
    inline void save_yaml(const T &obj, const std::string &filename) {
        auto node = yaml::serialize(obj);
        std::ofstream file(filename, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
        file << node;
        file.close();
    }

    template<typename T>
    inline T load_yaml(const std::string &filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
        const YAML::Node node = YAML::Load(file);
        return yaml::deserialize<T>(node);
    }
} // namespace encoding

#endif  // QUANT1X_ENCODING_YAML_H
