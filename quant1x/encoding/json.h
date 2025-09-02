#pragma once
#ifndef QUANT1X_ENCODING_JSON_H
#define QUANT1X_ENCODING_JSON_H 1

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <spdlog/spdlog.h>

namespace encoding {

    namespace safe_json {

        template<typename T>
        T get_number(const nlohmann::json &j, const std::string &key, T default_val = T()) {
            if (j.contains(key) && !j[key].is_null()) {
                try {
                    return j.value<T>(key, default_val);
                    //return j.at(key).get<T>();
                } catch (...) {
                    spdlog::warn("JSON field type mismatch: {}", key);
                    return default_val;
                }
            }
            return default_val;
        }

        template<typename T>
        T get_string(const nlohmann::json &j, const std::string &key, const T &default_val = T()) {
            if (j.contains(key) && !j[key].is_null()) {
                try {
                    return j.value<T>(key, default_val);
                    //return j.at(key).get<T>();
                } catch (...) {
                    spdlog::warn("JSON field type mismatch: {}", key);
                    return default_val;
                }
            }
            return default_val;
        }

        template<typename T>
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

        template<typename T>
        T nested_get(const nlohmann::json &j, const std::vector<std::string> &keys, const T &default_val = T()) {
            const auto *current = &j;
            for (const auto &key: keys) {
                if (!current->contains(key)) return default_val;
                current = &(*current)[key];
            }
            if (current->is_null()) return default_val;
            try {
                return current->get<T>();
            } catch (...) {
                spdlog::warn("nested JSON field type mismatch");
                return default_val;
            }
        }

    } // namespace safe_json

    // 非安全json
    namespace unsafe_json {

        template<typename T>
        void get_number(const nlohmann::json &j, const char *key, T &target, T default_val) {
            if (j.contains(key)) {
                if (j[key].is_number()) {
                    target = j[key].get<T>();
                } else if (!j[key].is_null()) {
                    throw nlohmann::json::type_error::create(302,
                                                             std::string("Field '") + key + "' must be number", &j);
                } else {
                    target = default_val;
                }
            } else {
                target = default_val;
            }
        }

        void get_string(const nlohmann::json &j, const char *key, std::string &target, const std::string &default_val = "");
        void get_bool(const nlohmann::json &j, const char *key, bool &target, bool default_val = false);

    } // namespace unsafe_json
} // namespace encoding

#endif //QUANT1X_ENCODING_JSON_H
