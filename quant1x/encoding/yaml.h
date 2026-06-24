#pragma once
#ifndef QUANT1X_ENCODING_YAML_H
#define QUANT1X_ENCODING_YAML_H 1

#include <yaml-cpp/yaml.h>

#include <boost/pfr.hpp>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace encoding {
    namespace safe_yaml {
        // 模式1: 安全解析(带默认值)——保持原有逻辑
        template <typename T>
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

        // 模式2: 严格解析(无默认值)→ 返回是否成功
        template <typename T>
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
        template <>
        inline bool try_parse_field<std::string>(const YAML::Node &node, const char *key, std::string &target) {
            if (!node[key])
                return false;
            try {
                target = node[key].as<std::string>();
                return !target.empty();  // 额外检查空字符串
            } catch (...) {
                return false;
            }
        }
    }  // namespace safe_yaml

    namespace yaml {

        // (No temporary tracing here) -- production code should be silent.

        // 类型特征
        template <typename T>
        struct is_vector : std::false_type {};

        // ========== 类型特征: enum ==========
        template <typename T>
        struct is_enum : std::is_enum<T> {};
        template <typename T>
        constexpr bool is_enum_v = is_enum<T>::value;

        template <typename T, typename A>
        struct is_vector<std::vector<T, A>> : std::true_type {};

        template <typename T>
        constexpr bool is_vector_v = is_vector<T>::value;

        // ========== 类型特征: 检测是否为 std::map ==========
        template <typename T>
        struct is_map : std::false_type {};

        template <typename K, typename V, typename Compare, typename Alloc>
        struct is_map<std::map<K, V, Compare, Alloc>> : std::true_type {};

        template <typename T>
        constexpr bool is_map_v = is_map<T>::value;

        // ========== 类型特征: optional ==========
        template <typename T>
        struct is_optional : std::false_type {};
        template <typename T>
        struct is_optional<std::optional<T>> : std::true_type {};
        template <typename T>
        constexpr bool is_optional_v = is_optional<T>::value;

        // ================================================
        // 对称的 YAML 序列化/反序列化器
        // ================================================

        // ========== 公共辅助函数 ==========
        // Field aliases hook: users may specialize yaml_field_aliases<T> to provide
        // an explicit array/list of YAML keys for type T's fields (in declaration order).
        template <typename T>
        struct yaml_field_aliases;  // optional user specialization

        template <typename T, typename = void>
        struct has_yaml_field_aliases : std::false_type {};
        template <typename T>
        struct has_yaml_field_aliases<T, std::void_t<decltype(yaml_field_aliases<T>::names)>> : std::true_type {};

        template <typename T>
        inline std::string get_field_name(std::size_t idx) {
            // Prefer user-provided aliases if present
            if constexpr (has_yaml_field_aliases<T>::value) {
                try {
                    return std::string(yaml_field_aliases<T>::names.at(idx));
                } catch (...) {
                    return std::string();
                }
            } else {
                // boost::pfr::names_as_array<T>() returns an array of const char*
                // convert to std::string for convenience
                try {
                    return std::string(boost::pfr::names_as_array<T>()[idx]);
                } catch (...) {
                    return std::string();
                }
            }
        };

        // Map C++ member name to a likely YAML key name.
        // Rules applied:
        //  - if member ends with '_' -> strip trailing underscore (convenience for C++ identifiers)
        //  - try replacing '_' with '-' as fallback
        inline std::string member_to_yaml_key(const std::string &member) {
            if (!member.empty() && member.back() == '_') {
                return member.substr(0, member.size() - 1);
            }
            return member;
        }

        inline YAML::Node find_child_node(const YAML::Node &node, const std::string &member_name) {
            // Simplified and safer lookup: direct key, member_name, underscore->dash, is_ fallback.
            if (!node || node.IsNull())
                return YAML::Node();
            const std::string key = member_to_yaml_key(member_name);
            // direct lookup
            try {
                if (node[key])
                    return node[key];
            } catch (...) {
            }

            // if member_name itself matches
            try {
                if (node[member_name])
                    return node[member_name];
            } catch (...) {
            }

            // try underscore -> dash replacement
            std::string dash_name = key;
            for (auto &c : dash_name)
                if (c == '_')
                    c = '-';
            if (dash_name != key) {
                try {
                    if (node[dash_name])
                        return node[dash_name];
                } catch (...) {
                }
            }

            // try is_ prefix fallback: if member is named is_foo, try 'foo' and dash form
            if (key.rfind("is_", 0) == 0 && key.size() > 3) {
                std::string without_is = key.substr(3);
                try {
                    if (node[without_is])
                        return node[without_is];
                } catch (...) {
                }
                std::string dash_without_is = without_is;
                for (auto &c : dash_without_is)
                    if (c == '_')
                        c = '-';
                if (dash_without_is != without_is) {
                    try {
                        if (node[dash_without_is])
                            return node[dash_without_is];
                    } catch (...) {
                    }
                }
            }

            return YAML::Node();
        }

        // ========== 反序列化 ==========
        template <typename T>
        inline void deserialize_to_object(const YAML::Node &node, T &obj);
        template <typename T>
        inline void deserialize_vector(const YAML::Node &node, T &vec);
        template <typename T>
        inline void deserialize_map(const YAML::Node &node, T &map);

        // ========== 反序列化补全 ==========
        template <typename T>
        inline void deserialize_field(const YAML::Node &node, T &field) {
            // silent in normal runs; errors are propagated
            try {
                if constexpr (is_optional_v<T>) {
                    using ValueType = typename T::value_type;
                    if (node && !node.IsNull()) {
                        ValueType value{};
                        deserialize_field(node, value);
                        field = value;
                    } else {
                        field = std::nullopt;
                    }
                } else if constexpr (std::is_same_v<T, std::string>) {
                    try {
                        if (node && !node.IsNull())
                            field = node.as<std::string>();
                        else
                            field = std::string();
                    } catch (...) {
                        field = std::string();
                    }
                } else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
                    try {
                        if (node && !node.IsNull())
                            field = node.as<T>();
                        else
                            field = T{};
                    } catch (...) {
                        field = T{};
                    }
                } else if constexpr (is_enum_v<T>) {
                    try {
                        if (node && !node.IsNull())
                            field = static_cast<T>(node.as<int>());
                        else
                            field = static_cast<T>(0);
                    } catch (...) {
                        field = static_cast<T>(0);
                    }
                } else if constexpr (is_vector_v<T>) {
                    deserialize_vector(node, field);
                } else if constexpr (is_map_v<T>) {
                    // Deserialize maps using the safer deserialize_map helper which first
                    // converts to std::map<std::string, YAML::Node> and then decodes each
                    // key/value via deserialize_field. This avoids requiring yaml-cpp
                    // conversions for user-defined mapped types.
                    deserialize_map(node, field);
                } else {
                    deserialize_to_object(node, field);
                }
            } catch (...) {
                throw;
            }
        }

        template <typename T>
        inline void deserialize_to_object(const YAML::Node &node, T &obj) {
            try {
                // minimal debug: only fatal errors will be printed during normal runs
                boost::pfr::for_each_field(obj, [&](auto &field, std::size_t idx) {
                    const auto field_name = get_field_name<T>(idx);
                    if (field_name.empty())
                        return;  // skip if we couldn't get a name
                    const YAML::Node child = find_child_node(node, field_name);
                    if (child) {
                        deserialize_field(child, field);
                    }
                });
                (void)obj;  // no extra debug
            } catch (...) {
                // rethrow, keep minimal stderr output to help in CI if required
                throw;
            }
        }

        template <typename T>
        inline void deserialize_vector(const YAML::Node &node, T &vec) {
            using ValueType = typename T::value_type;
            vec.clear();
            if (!node.IsDefined() || node.IsNull())
                return;  // nothing to do for null/undefined
            // Fast path for simple element types: let yaml-cpp convert the whole sequence
            if constexpr (std::is_same_v<ValueType, std::string> || std::is_arithmetic_v<ValueType> ||
                          yaml::is_enum<ValueType>::value) {
                try {
                    vec = node.as<T>();
                    return;
                } catch (...) {
                    // fall back to element-wise parsing when as<> fails
                }
            }

            if (!node.IsSequence())
                return;  // avoid iterating non-sequence nodes

            // Safer iteration: convert YAML sequence to std::vector<YAML::Node> first
            try {
                auto tmp_seq = node.as<std::vector<YAML::Node>>();
                for (std::size_t i = 0; i < tmp_seq.size(); ++i) {
                    const auto &item = tmp_seq[i];
                    ValueType value{};
                    deserialize_field(item, value);
                    vec.push_back(value);
                }
                return;
            } catch (...) {
                // fallback to iterating node directly if as<> fails
            }

            // Fallback: avoid using node's iterator (which triggered crashes);
            // iterate by numeric index instead which uses operator[] accessors.
            try {
                for (std::size_t i = 0; i < node.size(); ++i) {
                    YAML::Node item = node[i];
                    ValueType value{};
                    deserialize_field(item, value);
                    vec.push_back(value);
                }
            } catch (...) {
                throw;
            }
        }

        template <typename T>
        inline void deserialize_map(const YAML::Node &node, T &map) {
            using KeyType   = typename T::key_type;
            using ValueType = typename T::mapped_type;
            map.clear();
            if (!node.IsDefined() || node.IsNull())
                return;  // nothing to do
            if (!node.IsMap())
                return;  // avoid iterating non-map nodes
            // Safer iteration: convert the YAML map to a std::map<string, YAML::Node>
            // then iterate that. Some yaml-cpp iterator uses caused crashes in practice
            // (observed as SIGSEGV in YAML::Node::begin). Using as<> conversion is
            // more robust here; if it fails we'll leave the map empty and return.
            try {
                auto tmp = node.as<std::map<std::string, YAML::Node>>();
                for (auto &kv : tmp) {
                    KeyType     key{};
                    ValueType   value{};
                    YAML::Node  key_node = YAML::Node(kv.first);
                    YAML::Node &val_node = kv.second;
                    deserialize_field(key_node, key);
                    deserialize_field(val_node, value);
                    map.emplace(std::move(key), std::move(value));
                }
            } catch (...) {
                // fallback: skip map deserialization to avoid crashing
            }
        }

        // ========== 序列化 ==========

        template <typename T>
        inline void serialize_field(YAML::Node &node, const T &field);
        template <typename T>
        inline void serialize_vector(YAML::Node &node, const T &vec);
        template <typename T>
        inline void serialize_map(YAML::Node &node, const T &map);

        template <typename T>
        inline YAML::Node serialize_to_node(const T &obj) {
            YAML::Node node;
            boost::pfr::for_each_field(obj, [&](const auto &field, std::size_t idx) {
                const auto field_name = get_field_name<T>(idx);
                if (field_name.empty())
                    return;
                const std::string key         = member_to_yaml_key(field_name);
                auto            &&target_node = node[key];
                serialize_field(target_node, field);
            });
            return node;
        }

        // ========== 序列化补全 ==========
        template <typename T>
        inline void serialize_field(YAML::Node &node, const T &field) {
            if constexpr (is_optional_v<T>) {
                if (field.has_value()) {
                    serialize_field(node, field.value());
                }
            } else if constexpr (std::is_same_v<T, std::string> || std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
                node = field;
            } else if constexpr (is_enum_v<T>) {
                node = static_cast<int>(field);
            } else if constexpr (is_vector_v<T>) {
                serialize_vector(node, field);
            } else if constexpr (is_map_v<T>) {
                serialize_map(node, field);
            } else {
                node = serialize_to_node(field);
            }
        }

        template <typename T>
        inline void serialize_vector(YAML::Node &node, const T &vec) {
            for (const auto &item : vec) {
                YAML::Node item_node;
                serialize_field(item_node, item);
                node.push_back(item_node);
            }
        }

        template <typename T>
        inline void serialize_map(YAML::Node &node, const T &map) {
            node = YAML::Node(YAML::NodeType::Map);  // 明确设为 Map 类型

            for (const auto &pair : map) {
                YAML::Node key_node, value_node;
                serialize_field(key_node, pair.first);
                serialize_field(value_node, pair.second);

                // YAML::Node 支持直接赋值键值对
                node[key_node] = value_node;
            }
        }

        // 反序列化: YAML → struct
        template <typename T>
        inline T deserialize(const YAML::Node &node) {
            T obj{};
            deserialize_to_object(node, obj);
            return obj;
        }

        // 序列化: struct → YAML
        template <typename T>
        inline YAML::Node serialize(const T &obj) {
            return serialize_to_node(obj);
        }
    }  // namespace yaml

    template <typename T>
    inline void save_yaml(const T &obj, const std::string &filename) {
        auto          node = yaml::serialize(obj);
        std::ofstream file(filename, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
        file << node;
        file.close();
    }

    template <typename T>
    inline T load_yaml(const std::string &filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
        const YAML::Node node = YAML::Load(file);
        return yaml::deserialize<T>(node);
    }
}  // namespace encoding

#endif  // QUANT1X_ENCODING_YAML_H
