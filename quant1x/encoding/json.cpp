#include <quant1x/encoding/json.h>

namespace encoding {
    namespace unsafe_json {
        // 安全解析字符串(处理null和缺失字段)
        void get_string(const nlohmann::json &j, const char *key,
                               std::string &target,
                               const std::string& default_val) {
            if (j.contains(key)) {
                if (j[key].is_string()) {
                    target = j[key].get<std::string>();
                } else if (j[key].is_null()) {
                    target = default_val;
                } else {
                    throw nlohmann::json::type_error::create(302,
                                                             std::string("Field '") + key + "' must be string or null",
                                                             &j);
                }
            } else {
                target = default_val;
            }
        }

        void get_bool(const nlohmann::json &j, const char *key, bool &target, bool default_val) {
            if (j.contains(key)) {
                if (j[key].is_boolean()) {
                    target = j[key].get<bool>();
                } else if (j[key].is_number()) {
                    target = (j[key].get<int>() != 0);
                } else if (!j[key].is_null()) {
                    throw nlohmann::json::type_error::create(302,
                                                             std::string("Field '") + key + "' must be boolean", &j);
                } else {
                    target = default_val;
                }
            } else {
                target = default_val;
            }
        }
    }  // namespace unsafe_json
} // namespace encoding