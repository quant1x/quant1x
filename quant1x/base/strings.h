#pragma once
#ifndef QUANT1X_BASE_STRINGS_H
#define QUANT1X_BASE_STRINGS_H 1

#include <quant1x/base/common.h>

namespace strings {

    // 判断是否空白字符
    bool is_whitespace(char ch);

    inline std::string from(const char *const start, size_t len) {
        size_t actual_len = strnlen(start, len); // 查找第一个 '\0' 或最大长度
        return std::string(start, actual_len); // 截断到第一个 '\0'
    }

    template <size_t N>
    inline std::string from(const char (&arr)[N]) {
        const char* start = reinterpret_cast<const char*>(arr);
        return from(start, N);
    }

    template <size_t N>
    inline std::string from(const uint8_t (&arr)[N]) {
        const char *const start = reinterpret_cast<const char*>(arr);
        return from(start, N);
    }

    // 将字符串转为 T 类型, 失败则返回默认值
    template<typename T>
    inline T from_string(const std::string& s, T default_val= T{}) {
        if (s.empty()) {
            return default_val;
        }

        std::istringstream iss(s);
        T val{};
        iss >> val;

        // 如果转换失败, 仍返回默认值
        if (iss.fail() || iss.bad()) {
            return default_val;
        }

        return val;
    }

    // 去掉字符串两端的空白字符
    std::string trim(const std::string& str);

    std::string_view trim_view(std::string_view str);

    std::vector<std::string> split(const std::string& str, char delimiter, bool ignoreEmpty = false);

    std::vector<std::string> split(const std::string& str, const std::string& delimiter, bool ignoreEmpty = false);

    // 将字符串容器通过分隔符连接成一个字符串
    inline std::string join(const std::vector<std::string>& tokens, const std::string& delimiter) {
        if (tokens.empty()) return {};

        // 计算总长度
        size_t totalLength = std::accumulate(tokens.begin(),
                                             tokens.end(),
                                             size_t(0),
                                             [](size_t sum, const std::string& s) { return sum + s.size(); }
                                             );
        totalLength += delimiter.size() * (tokens.size() - 1);

        std::string result;
        result.reserve(totalLength);

        if (!tokens.empty()) {
            result.append(tokens[0]);
            for (size_t i = 1; i < tokens.size(); ++i) {
                result.append(delimiter).append(tokens[i]);
            }
        }

        return result;
    }

    // 将字符串容器通过分隔符连接成一个字符串
    inline std::string join(const std::vector<std::string_view>& tokens, const std::string& delimiter) {
        if (tokens.empty()) return {};

        // 计算总长度
        size_t totalLength = std::accumulate(tokens.begin(),
                                             tokens.end(),
                                             size_t(0),
                                             [](size_t sum, const std::string_view & s) { return sum + s.size(); }
        );
        totalLength += delimiter.size() * (tokens.size() - 1);

        std::string result;
        result.reserve(totalLength);

        if (!tokens.empty()) {
            result.append(tokens[0]);
            for (size_t i = 1; i < tokens.size(); ++i) {
                result.append(delimiter).append(tokens[i]);
            }
        }

        return result;
    }

    // 将字符串容器通过分隔符连接成一个字符串
    inline std::string join(const std::vector<std::string>& tokens, char delimiter) {
        if (tokens.empty()) return {};

        // 计算总长度
        size_t totalLength = std::accumulate(tokens.begin(),
                                             tokens.end(),
                                             size_t(0),
                                             [](size_t sum, const std::string& s) { return sum + s.size(); }
                                             );
        totalLength += tokens.size() - 1;

        std::string result;
        result.reserve(totalLength);

        if (!tokens.empty()) {
            result.append(tokens[0]);
            for (size_t i = 1; i < tokens.size(); ++i) {
                result.push_back(delimiter);
                result.append(tokens[i]);
            }
        }

        return result;
    }

    // 将字符串容器通过分隔符连接成一个字符串
    inline std::string join(const std::vector<std::string_view >& tokens, char delimiter) {
        if (tokens.empty()) return {};

        // 计算总长度
        size_t totalLength = std::accumulate(tokens.begin(),
                                             tokens.end(),
                                             size_t(0),
                                             [](size_t sum, const std::string_view& s) { return sum + s.size(); }
        );
        totalLength += tokens.size() - 1;

        std::string result;
        result.reserve(totalLength);

        if (!tokens.empty()) {
            result.append(tokens[0]);
            for (size_t i = 1; i < tokens.size(); ++i) {
                result.push_back(delimiter);
                result.append(tokens[i]);
            }
        }

        return result;
    }

//    // 约束 StringT 只能是 std::string 或 std::string_view
//    template <typename StringT>
//    using EnableIfStringLike = std::enable_if_t<
//        std::is_same_v<StringT, std::string> ||
//        std::is_same_v<StringT, std::string_view>
//    >;
//
//    // 约束 DelimiterT 只能是 char, std::string 或 std::string_view
//    template <typename DelimiterT>
//    using EnableIfDelimiterLike = std::enable_if_t<
//        std::is_same_v<DelimiterT, char> ||
//        std::is_same_v<DelimiterT, const char*> ||
//        std::is_same_v<DelimiterT, std::string> ||
//        std::is_same_v<DelimiterT, std::string_view>
//    >;
//
//    // 字符串类容器拼接成字符串
//    template <typename StringT, typename DelimiterT,
//        typename = EnableIfStringLike<StringT>,
//        typename = EnableIfDelimiterLike<DelimiterT>>
//    std::string join(const std::vector<StringT>& tokens, DelimiterT delimiter) {
//        if (tokens.empty()) return "";
//
//        // 计算总长度
//        size_t total_length = std::accumulate(
//            tokens.begin(), tokens.end(), 0,
//            [](size_t sum, const StringT& s) { return sum + s.size(); }
//        );
//
//        // 添加分隔符的长度
//        if constexpr (std::is_same_v<DelimiterT, char>) {
//            total_length += tokens.size() - 1;
//        } else if constexpr (std::is_same_v<DelimiterT, std::string> ||
//                             std::is_same_v<DelimiterT, std::string_view>) {
//            total_length += (tokens.size() - 1) * delimiter.size();
//        } else {
//            // 处理const char*的情况
//            total_length += (tokens.size() - 1) * std::char_traits<char>::length(delimiter);
//        }
//
//        std::string result;
//        result.reserve(total_length); // 预分配内存
//
//        if (!tokens.empty()) {
//            result.append(tokens[0]);
//            for (size_t i = 1; i < tokens.size(); ++i) {
//                if constexpr (std::is_same_v<DelimiterT, char>) {
//                    result.push_back(delimiter);
//                } else {
//                    result.append(delimiter);
//                }
//                result.append(tokens[i]);
//            }
//        }
//
//        return result;
//    }

    // to_lower_inplace, 原地转小写
    inline char* strtolc_inplace_branchless(char *str) {
        if (!str) return NULL;
        for (unsigned char *p = (unsigned char*)str; *p; p++) {
            // 判断当前字符是否为大写字母
            const unsigned char mask = (*p >= 'A') & (*p <= 'Z');
            // 只有在 mask 为 1 时才设置第 5 位
            *p |= ((mask << 5) & 0x20);
        }
        return str;
    }

    inline char* strtouc_inplace_branchless(char *str) {
        if (!str) return NULL;
        for (unsigned char *p = (unsigned char*)str; *p; p++) {
            // 判断当前字符是否为小写字母
            const unsigned char mask = (*p >= 'a') & (*p <= 'z');
            // 只有在 mask 为 1 时才清除第 5 位
            *p &= ~((mask << 5) & 0x20);
        }
        return str;
    }

    // 安全小写转换(仅修改A-Z)
    inline char* strtolc(char *str) {
        if (!str) return NULL; // 处理NULL输入

        for (unsigned char *p = (unsigned char*)str; *p; p++) {
            // 位运算+范围判断: 仅修改A-Z, 避免影响其他字符
            *p = ( *p >= 'A' && *p <= 'Z' ) ? (*p | 0x20) : *p;
        }
        return str;
    }

    // 安全大写转换(仅修改a-z)
    inline char* strtouc(char *str) {
        if (!str) return NULL;

        for (unsigned char *p = (unsigned char*)str; *p; p++) {
            // 位运算+范围判断: 仅修改a-z, 避免影响其他字符
            *p = ( *p >= 'a' && *p <= 'z' ) ? (*p & ~0x20) : *p;
        }
        return str;
    }

    // ==============================
    // 字符串(std::string) 大小写转换
    // ==============================

//    // 使用模板 + SFINAE 禁止非 std::string 类型(可用于通用库)
//    template <typename T>
//    std::enable_if_t<std::is_same_v<T, std::string>, std::string>
//    to_lower(const T& origin) {
//        std::string str(origin);
//        strtolc_inplace_branchless(str.data());
//        return str;
//    }
//
//    // 使用模板 + SFINAE 禁止非 std::string 类型(可用于通用库)
//    template <typename T>
//    std::enable_if_t<std::is_same_v<T, std::string>, std::string>
//    to_upper(const T& origin) {
//        std::string str(origin);
//        strtouc_inplace_branchless(str.data());
//        return str;
//    }

    template <typename T>
    std::enable_if_t<std::is_convertible_v<T, std::string_view>, std::string>
    to_lower(const T& input) {
        std::string result = input;
        strtolc_inplace_branchless(result.data());
        return result;
    }

    template <typename T>
    std::enable_if_t<std::is_convertible_v<T, std::string_view>, std::string>
    to_upper(const T& input) {
        std::string result = input;
        strtouc_inplace_branchless(result.data());
        return result;
    }

    // 标准库用法: 将字符串转换为小写
    [[deprecated("Use to_lower() instead")]]
    inline std::string toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    // 标准库用法: 将字符串转换为大写
    [[deprecated("Use to_upper() instead")]]
    inline std::string toUpper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }

    /**
     * @brief 检查字符串是否以指定前缀列表中的任一前缀开头
     * @tparam Container 容器类型(支持vector, initializer_list等)
     * @param str 目标字符串
     * @param prefixes 前缀容器
     * @return 是否存在匹配的前缀
     */
    inline bool startsWith(const std::string &str, const std::vector<std::string> &prefixes) {
        for (const auto &prefix: prefixes) {
            if (str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief 检查字符串是否以指定前缀列表中任一前缀开头
     * @param str 容器字符串
     * @param prefixes 前缀容器
     * @return 是否存在匹配的前缀
     */
    inline bool startsWith(const std::string &str, std::initializer_list<std::string> prefixes) {
        return startsWith(str, std::vector<std::string>(prefixes));
    }

    /**
     * @brief 检查字符串是否以指定后缀列表中的任一后缀结尾
     * @param str 目标字符串
     * @param suffixes 后缀容器
     * @return 是否存在匹配的后缀
     */
    inline bool endsWith(const std::string &str, const std::vector<std::string> &suffixes) {
        for (const auto &suffix: suffixes) {
            if (str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0) {
                return true;
            }
        }
        return false;
    }

    // 工具函数: 去除双引号
    inline std::string remove_quotes(const std::string& s) {
        std::string processed = trim(s);
        if (processed.size() >= 2 && processed.front() == '"' && processed.back() == '"') {
            return processed.substr(1, processed.size() - 2);
        }
        return processed;
    }

    // ==============================
    // 字符串(std::string) → 类型 T
    // ==============================

//    /**
//     * @brief  string转换其它数据类型 主模板(拦截未支持的类型)
//     * @tparam T
//     * @param str
//     * @param out_value
//     * @return
//     */
//    template<typename T>
//    inline bool try_parse(const std::string& str, T& out_value) {
//        static_assert(std::is_arithmetic_v<T>, "Unsupported type");
//        return false;
//    }

    //  数值类型的通用实现(int, double, float 等)
    template<typename T>
    //std::enable_if_t<std::is_arithmetic_v<T>, bool>
    inline bool try_parse(const std::string& str, T& out_value) {
        std::string processed = remove_quotes(str);
        std::istringstream iss(processed);
        T                  value{};
        if ((iss >> value) && iss.eof()) {
            out_value = value;
            return true;
        }
        return false;
    }

//    // 主模板 - 使用 SFINAE
//    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
//    inline bool try_parse(const std::string& str, T& out_value) {
//        std::string processed = remove_quotes(str);
//        std::istringstream iss(processed);
//        return (iss >> out_value) && iss.eof();
//    }

    /**
     * @brief 特化: bool 类型
     * @param str
     * @param out_value
     * @return
     */
    template<>
    inline bool try_parse<bool>(const std::string& str, bool& out_value) {
        std::string processed = remove_quotes(str);
        std::string lower = strtolc_inplace_branchless(processed.data());

        if (lower == "true" || lower == "yes" || lower == "on" || lower == "1") {
            out_value = true;
            return true;
        } else if (lower == "false" || lower == "no" || lower == "off" || lower == "0") {
            out_value = false;
            return true;
        }

        std::istringstream iss(processed);
        int num = 0;
        if (iss >> num) {
            out_value = num != 0;
            return true;
        }

        return false;
    }

    // 特化: std::string
    template<>
    inline bool try_parse<std::string>(const std::string& str, std::string& out_value) {
        out_value = str;
        return true;
    }

//    // 特化: std::vector<T>
//    template<typename T>
//    inline bool try_parse(const std::string& str, std::vector<T>& out_value) {
//        std::string processed = remove_quotes(str);
//        std::istringstream iss(processed);
//        std::string token;
//        out_value.clear();
//
//        while (std::getline(iss, token, ',')) {
//            T val;
//            if (!try_parse(token, val)) {
//                return false;
//            }
//            out_value.push_back(val);
//        }
//
//        return true;
//    }

    // ==============================
    // 类型 T → 字符串(std::string)
    // ==============================

//    // 类型 T → 字符串 主模板声明(未定义)
//    template <typename T>
//    std::string to_string(const T& value);

    // 特化: int, double, float 等数值类型
    template <typename T>
    //std::enable_if_t<std::is_arithmetic_v<T>, std::string>
    inline std::string to_string(const T& value) {
        return std::to_string(value);
    }

    // 特化: bool 类型
    template <>
    inline std::string to_string<bool>(const bool& value) {
        return value ? "true" : "false";
    }

    // 特化: std::string
    template <>
    inline std::string to_string<std::string>(const std::string& value) {
        return value;
    }

    // 特化: const char*
    template <>
    inline std::string to_string<const char*>(const char* const& value) {
        return value;
    }

    // 特化: char*
    template <>
    inline std::string to_string<char*>(char* const& value) {
        return value;
    }

    // std::string_view
    template <>
    inline std::string to_string<std::string_view>(const std::string_view& value) {
        return std::string(value);
    }

    // 特化: std::vector<T>
    template <typename T>
    inline std::string to_string(const std::vector<T>& vec) {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            oss << to_string(vec[i]);
            if (i != vec.size() - 1) {
                oss << ", ";
            }
        }
        oss << "]";
        return oss.str();
    }

    std::vector<std::string> unique(std::vector<std::string> arr);
    // 将 string 的每个字节转为十六进制字符串
    std::string to_hex_string(const std::string& input);

    // 将字节数组转换为十六进制字符串(默认大写)
    std::string bytesToHex(const std::vector<uint8_t>& bytes, bool uppercase = true);
    // 将16进制字符串传承uint8_t数组
    std::vector<uint8_t> hexToBytes(const std::string& hex);
    // 全部替换
    std::string replace_all(std::string str, const std::string &from, const std::string &to);

    // =============================================================================
    // Go 代码移植函数 - 转换为 C++ 实现
    // =============================================================================
    
    // 基础字符工具函数
    bool is_lower(char ch);
    bool is_upper(char ch);
    char to_lower(char ch);
    char to_upper(char ch);
    bool is_space(char ch);
    bool is_delimiter(char ch);
    
    // 字符串迭代器回调函数类型
    using iter_func = std::function<void(char prev, char curr, char next)>;
    void string_iter(const std::string& s, const iter_func& callback);
    
    // ToString 系列函数 - 将各种类型转换为字符串
    std::string toString(int8_t value);
    std::string toString(int16_t value);
    std::string toString(int32_t value);
    std::string toString(int64_t value);
    std::string toString(uint8_t value);
    std::string toString(uint16_t value);
    std::string toString(uint32_t value);
    std::string toString(uint64_t value);
    std::string toString(float value);
    std::string toString(double value);
    std::string toString(bool value);
    
    // 通用 ToString 模板
    template<typename T>
    std::string toString(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
    
    // CamelCase 转换函数
    std::string to_camel_case(const std::string& str);
    std::string upper_camel_case(const std::string& str);
    std::string lower_camel_case(const std::string& str);
    
    // SnakeCase 转换函数  
    std::string snake_case(const std::string& str);
    std::string upper_snake_case(const std::string& str);
    
    // KebabCase 转换函数
    std::string kebab_case(const std::string& str);
    std::string upper_kebab_case(const std::string& str);
    
    // 字符串匹配和判断函数
    bool starts_with(const std::string& str, const std::vector<std::string>& prefixes);
    bool ends_with(const std::string& str, const std::vector<std::string>& suffixes);
    bool is_empty(const std::string& str);
    
    // // 内部辅助函数
    // namespace detail {
    //     std::string camel_case_impl(const std::string& str, bool upper_first);
    //     std::string delimiter_case_impl(const std::string& str, char delimiter, bool upper_case);
    // }
}

#endif //QUANT1X_BASE_STRINGS_H
