#pragma once
#ifndef QUANT1X_ENCODING_JSON_H
#define QUANT1X_ENCODING_JSON_H 1

#include <spdlog/spdlog.h>

#include <array>
#include <boost/pfr.hpp>
#include <deque>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace encoding {

    namespace safe_json {

        template <typename T>
        T get_number(const nlohmann::json &j, const std::string &key, T default_val = T()) {
            if (j.contains(key) && !j[key].is_null()) {
                try {
                    return j.value<T>(key, default_val);
                    // return j.at(key).get<T>();
                } catch (...) {
                    spdlog::warn("JSON field type mismatch: {}", key);
                    return default_val;
                }
            }
            return default_val;
        }

        template <typename T>
        T get_string(const nlohmann::json &j, const std::string &key, const T &default_val = T()) {
            if (j.contains(key) && !j[key].is_null()) {
                try {
                    return j.value<T>(key, default_val);
                    // return j.at(key).get<T>();
                } catch (...) {
                    spdlog::warn("JSON field type mismatch: {}", key);
                    return default_val;
                }
            }
            return default_val;
        }

        template <typename T>
        T get_bool(const nlohmann::json &j, const std::string &key, T default_val = T()) {
            if (j.contains(key) && !j[key].is_null()) {
                try {
                    return j.value<T>(key, default_val);
                } catch (...) {
                    spdlog::warn("JSON field type mismatch: {}", key);
                    return default_val;
                }
            }
            return default_val;
        }

        template <typename T>
        T nested_get(const nlohmann::json &j, const std::vector<std::string> &keys, const T &default_val = T()) {
            const auto *current = &j;
            for (const auto &key : keys) {
                if (!current->contains(key))
                    return default_val;
                current = &(*current)[key];
            }
            if (current->is_null())
                return default_val;
            try {
                return current->get<T>();
            } catch (...) {
                spdlog::warn("nested JSON field type mismatch");
                return default_val;
            }
        }

    }  // namespace safe_json

    // 非安全json
    namespace unsafe_json {

        template <typename T>
        void get_number(const nlohmann::json &j, const char *key, T &target, T default_val) {
            if (j.contains(key)) {
                if (j[key].is_number()) {
                    target = j[key].get<T>();
                } else if (!j[key].is_null()) {
                    throw nlohmann::json::type_error::create(
                        302, std::string("Field '") + key + "' must be number", &j);
                } else {
                    target = default_val;
                }
            } else {
                target = default_val;
            }
        }

        void
        get_string(const nlohmann::json &j, const char *key, std::string &target, const std::string &default_val = "");
        void get_bool(const nlohmann::json &j, const char *key, bool &target, bool default_val = false);

    }  // namespace unsafe_json
}  // namespace encoding

namespace encoding {
    namespace json {
        using json_t = nlohmann::json;

        // Control whether std::optional without value should be serialized as JSON null
        inline thread_local bool g_serialize_optional_as_null = false;

        struct OptionalNullGuard {
            bool prev;
            explicit OptionalNullGuard(bool v) : prev(g_serialize_optional_as_null) { g_serialize_optional_as_null = v; }
            ~OptionalNullGuard() { g_serialize_optional_as_null = prev; }
        };

        // 类型特征
        template <typename T>
        struct is_vector : std::false_type {};
        template <typename T, typename A>
        struct is_vector<std::vector<T, A>> : std::true_type {};
        template <typename T>
        constexpr bool is_vector_v = is_vector<T>::value;

        template <typename T>
        struct is_map : std::false_type {};
        template <typename K, typename V, typename C, typename A>
        struct is_map<std::map<K, V, C, A>> : std::true_type {};
        template <typename T>
        constexpr bool is_map_v = is_map<T>::value;

        template <typename T>
        struct is_unordered_map : std::false_type {};
        template <typename K, typename V, typename H, typename E, typename A>
        struct is_unordered_map<std::unordered_map<K, V, H, E, A>> : std::true_type {};
        template <typename T>
        constexpr bool is_unordered_map_v = is_unordered_map<T>::value;

        template <typename T>
        struct is_deque : std::false_type {};
        template <typename T, typename A>
        struct is_deque<std::deque<T, A>> : std::true_type {};
        template <typename T>
        constexpr bool is_deque_v = is_deque<T>::value;

        template <typename T>
        struct is_array : std::false_type {};
        template <typename T, std::size_t N>
        struct is_array<std::array<T, N>> : std::true_type {};
        template <typename T>
        constexpr bool is_array_v = is_array<T>::value;

        template <typename T>
        struct is_optional : std::false_type {};
        template <typename T>
        struct is_optional<std::optional<T>> : std::true_type {};
        template <typename T>
        constexpr bool is_optional_v = is_optional<T>::value;

        template <typename T>
        struct is_enum : std::is_enum<T> {};
        template <typename T>
        constexpr bool is_enum_v = is_enum<T>::value;

        // 获取字段名(借用 yaml.h 风格)
        template <typename T>
        inline std::string_view get_field_name(std::size_t idx) {
            return boost::pfr::names_as_array<T>()[idx];
        }

        // 反序列化声明
        template <typename T>
        inline void deserialize_to_object(const json_t &node, T &obj, bool strict = false);
        template <typename T>
        inline void deserialize_vector(const json_t &node, T &vec, bool strict = false);
        template <typename T>
        inline void deserialize_map(const json_t &node, T &map, bool strict = false);
        template <typename T>
        inline void deserialize_unordered_map(const json_t &node, T &map, bool strict = false);
        template <typename T>
        inline void deserialize_deque(const json_t &node, T &dq, bool strict = false);
        template <typename T>
        inline void deserialize_array(const json_t &node, T &arr, bool strict = false);

        // 反序列化补全
        template <typename T>
        inline void deserialize_field(const json_t &node, T &field, bool strict = false) {
            if constexpr (is_optional_v<T>) {
                using ValueType = typename T::value_type;
                if (!node.is_null()) {
                    ValueType value{};
                    deserialize_field(node, value, strict);
                    field = value;
                } else {
                    field = std::nullopt;
                }
            } else if constexpr (std::is_same_v<T, std::string>) {
                field = node.get<std::string>();
            } else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
                field = node.get<T>();
            } else if constexpr (is_enum_v<T>) {
                field = static_cast<T>(node.get<int>());
            } else if constexpr (is_vector_v<T>) {
                deserialize_vector(node, field, strict);
            } else if constexpr (is_map_v<T>) {
                deserialize_map(node, field, strict);
            } else {
                deserialize_to_object(node, field, strict);
            }
        }

        template <typename T>
        inline void deserialize_to_object(const json_t &node, T &obj, bool strict) {
            boost::pfr::for_each_field(obj, [&](auto &field, std::size_t idx) {
                const auto field_name = get_field_name<T>(idx);
                if (node.contains(std::string(field_name))) {
                    deserialize_field(node.at(std::string(field_name)), field, strict);
                } else if (strict) {
                    throw std::runtime_error(std::string("Missing required field: ") + std::string(field_name));
                }
            });
        }

        template <typename T>
        inline void deserialize_vector(const json_t &node, T &vec, bool strict) {
            using ValueType = typename T::value_type;
            vec.clear();
            if (!node.is_array())
                return;
            for (const auto &item : node) {
                ValueType value{};
                deserialize_field(item, value, strict);
                vec.push_back(std::move(value));
            }
        }

        template <typename T>
        inline void deserialize_deque(const json_t &node, T &dq, bool strict) {
            using ValueType = typename T::value_type;
            dq.clear();
            if (!node.is_array())
                return;
            for (const auto &item : node) {
                ValueType value{};
                deserialize_field(item, value, strict);
                dq.push_back(std::move(value));
            }
        }

        template <typename T>
        inline void deserialize_array(const json_t &node, T &arr, bool strict) {
            using ValueType = typename T::value_type;
            if (!node.is_array())
                return;
            std::size_t idx = 0;
            for (const auto &item : node) {
                if (idx >= arr.size())
                    break;
                ValueType value{};
                deserialize_field(item, value, strict);
                arr[idx++] = std::move(value);
            }
        }

        template <typename T>
        inline void deserialize_map(const json_t &node, T &map, bool strict) {
            using KeyType   = typename T::key_type;
            using ValueType = typename T::mapped_type;
            map.clear();
            if (!node.is_object())
                return;
            for (auto it = node.begin(); it != node.end(); ++it) {
                KeyType   key{};
                ValueType value{};
                // parse key (from string) - rely on deserialize_field for conversion
                json_t key_node = it.key();
                deserialize_field(key_node, key);
                deserialize_field(it.value(), value, strict);
                map.emplace(std::move(key), std::move(value));
            }
        }

        template <typename T>
        inline void deserialize_unordered_map(const json_t &node, T &map, bool strict) {
            using KeyType   = typename T::key_type;
            using ValueType = typename T::mapped_type;
            map.clear();
            if (!node.is_object())
                return;
            for (auto it = node.begin(); it != node.end(); ++it) {
                KeyType   key{};
                ValueType value{};
                json_t    key_node = it.key();
                deserialize_field(key_node, key);
                deserialize_field(it.value(), value, strict);
                map.emplace(std::move(key), std::move(value));
            }
        }

        // 序列化
        template <typename T>
        inline json_t serialize_to_node(const T &obj);
        template <typename T>
        inline void serialize_vector(json_t &node, const T &vec);
        template <typename T>
        inline void serialize_map(json_t &node, const T &map);
        template <typename T>
        inline void serialize_unordered_map(json_t &node, const T &map);
        template <typename T>
        inline void serialize_deque(json_t &node, const T &dq);
        template <typename T>
        inline void serialize_array(json_t &node, const T &arr);

        template <typename T>
        inline void serialize_field(json_t &node, const T &field) {
            if constexpr (is_optional_v<T>) {
                if (field.has_value()) {
                    serialize_field(node, field.value());
                } else {
                    if (g_serialize_optional_as_null) node = nullptr;
                    // otherwise omit the field (leave node untouched)
                }
            } else if constexpr (std::is_same_v<T, std::string> || std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
                node = field;
            } else if constexpr (is_enum_v<T>) {
                node = static_cast<int>(field);
            } else if constexpr (is_vector_v<T>) {
                serialize_vector(node, field);
            } else if constexpr (is_deque_v<T>) {
                serialize_deque(node, field);
            } else if constexpr (is_array_v<T>) {
                serialize_array(node, field);
            } else if constexpr (is_map_v<T>) {
                serialize_map(node, field);
            } else if constexpr (is_unordered_map_v<T>) {
                serialize_unordered_map(node, field);
            } else {
                node = serialize_to_node(field);
            }
        }

        template <typename T>
        inline json_t serialize_to_node(const T &obj) {
            json_t node = json_t::object();
            boost::pfr::for_each_field(obj, [&](const auto &field, std::size_t idx) {
                const auto field_name = get_field_name<T>(idx);
                json_t    &target     = node[std::string(field_name)];
                serialize_field(target, field);
            });
            return node;
        }

        template <typename T>
        inline void serialize_vector(json_t &node, const T &vec) {
            node = json_t::array();
            for (const auto &item : vec) {
                json_t item_node;
                serialize_field(item_node, item);
                node.push_back(item_node);
            }
        }

        template <typename T>
        inline void serialize_map(json_t &node, const T &map) {
            node = json_t::object();
            for (const auto &p : map) {
                json_t key_node;
                serialize_field(key_node, p.first);
                // key_node should be a primitive convertible to string
                std::string key_str = key_node.is_string() ? key_node.get<std::string>() : key_node.dump();
                json_t      value_node;
                serialize_field(value_node, p.second);
                node[key_str] = value_node;
            }
        }

        template <typename T>
        inline void serialize_unordered_map(json_t &node, const T &map) {
            node = json_t::object();
            for (const auto &p : map) {
                json_t key_node;
                serialize_field(key_node, p.first);
                std::string key_str = key_node.is_string() ? key_node.get<std::string>() : key_node.dump();
                json_t      value_node;
                serialize_field(value_node, p.second);
                node[key_str] = value_node;
            }
        }

        template <typename T>
        inline void serialize_deque(json_t &node, const T &dq) {
            node = json_t::array();
            for (const auto &item : dq) {
                json_t item_node;
                serialize_field(item_node, item);
                node.push_back(item_node);
            }
        }

        template <typename T>
        inline void serialize_array(json_t &node, const T &arr) {
            node = json_t::array();
            for (const auto &item : arr) {
                json_t item_node;
                serialize_field(item_node, item);
                node.push_back(item_node);
            }
        }

        template <typename T>
        inline T deserialize(const json_t &node, bool strict = false) {
            if constexpr (is_vector_v<T> || is_deque_v<T>) {
                T obj{};
                if constexpr (is_vector_v<T>)
                    deserialize_vector(node, obj, strict);
                else
                    deserialize_deque(node, obj, strict);
                return obj;
            } else if constexpr (is_map_v<T> || is_unordered_map_v<T>) {
                T obj{};
                if constexpr (is_map_v<T>)
                    deserialize_map(node, obj, strict);
                else
                    deserialize_unordered_map(node, obj, strict);
                return obj;
            } else if constexpr (is_array_v<T>) {
                T obj{};
                deserialize_array(node, obj, strict);
                return obj;
            } else if constexpr (std::is_same_v<T, std::string> || std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
                return node.get<T>();
            } else {
                T obj{};
                deserialize_to_object(node, obj, strict);
                return obj;
            }
        }

        template <typename T>
        inline json_t serialize(const T &obj) {
            json_t node;
            if constexpr (is_vector_v<T> || is_deque_v<T> || is_array_v<T>) {
                if constexpr (is_vector_v<T>)
                    serialize_vector(node, obj);
                else if constexpr (is_deque_v<T>)
                    serialize_deque(node, obj);
                else
                    serialize_array(node, obj);
                return node;
            } else if constexpr (is_map_v<T> || is_unordered_map_v<T>) {
                if constexpr (is_map_v<T>)
                    serialize_map(node, obj);
                else
                    serialize_unordered_map(node, obj);
                return node;
            } else if constexpr (std::is_same_v<T, std::string> || std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
                node = obj;
                return node;
            } else {
                return serialize_to_node(obj);
            }
        }

        template <typename T>
        inline json_t serialize_with_optional_null(const T &obj) {
            OptionalNullGuard g(true);
            return serialize(obj);
        }
    }  // namespace json

    // convenience wrappers
    template <typename T>
    inline void save_json(const T &obj, const std::string &filename) {
        auto          node = json::serialize(obj);
        std::ofstream file(filename, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!file.is_open())
            throw std::runtime_error("Cannot open file: " + filename);
        file << node.dump(2);
        file.close();
    }

    template <typename T>
    inline T load_json(const std::string &filename) {
        std::ifstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("Cannot open file: " + filename);
        json::json_t node;
        file >> node;
        return json::deserialize<T>(node);
    }
}  // namespace encoding

#endif  // QUANT1X_ENCODING_JSON_H
