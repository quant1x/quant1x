#include <quant1x/std/strings.h>

namespace strings {

    bool is_whitespace(char ch) {
        return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
    }

    // 使用位掩码优化, 性能好且易于扩展 [存在掩码误报的情况]
    bool v1_is_whitespace(char ch) {
        // 定义空白字符的掩码: 
        // 空格 ' ' (0x20), 制表符 '\t' (0x09), 换行 '\n' (0x0A), 回车 '\r' (0x0D)
        constexpr unsigned int whitespace_mask =
            (1 << (' '  & 0x1F)) |  // 空格
            (1 << ('\t' & 0x1F)) |  // 水平制表符
            (1 << ('\n' & 0x1F)) |  // 换行
            (1 << ('\r' & 0x1F));   // 回车

        // 检查字符是否在掩码中
        return (whitespace_mask >> (ch & 0x1F)) & 1;
    }

    // [存在掩码误报的情况]
    bool v2_is_whitespace(char ch) {
        constexpr uint32_t whitespace_mask =
            (1 << (' '  & 0x1F)) |
            (1 << ('\t' & 0x1F)) |
            (1 << ('\n' & 0x1F)) |
            (1 << ('\r' & 0x1F)) |
            (1 << ('\v' & 0x1F)) |
            (1 << ('\f' & 0x1F));

        return (whitespace_mask >> (static_cast<uint8_t>(ch) & 0x1F)) & 1;
    }

    std::string trim(const std::string& str) {
        size_t start = 0;
        size_t end = str.size();

        // 合并的空白字符检查循环
        while (start < end && is_whitespace(str[start])) ++start;
        while (start < end && is_whitespace(str[end - 1])) --end;

        return (start == 0 && end == str.size()) ? str : str.substr(start, end - start);
    }

    // string_view版本的trim
    std::string_view trim_view(std::string_view str) {
        size_t start = 0;
        size_t end = str.size();

        while (start < end && is_whitespace(str[start])) ++start;
        while (start < end && is_whitespace(str[end - 1])) --end;

        return str.substr(start, end - start);
    }

    std::vector<std::string> split(const std::string& str, char delimiter, bool ignoreEmpty) {
        std::vector<std::string> tokens;
        if (str.empty()) return tokens;

        // 预分配内存 (+1 是为了最后一个token)
        tokens.reserve(std::count(str.begin(), str.end(), delimiter) + 1);

        size_t start = 0;
        size_t end = 0;

        while (end != std::string::npos) {
            end = str.find(delimiter, start);

            // 获取当前token的视图
            std::string token = str.substr(start, end - start);
            // 去除前后空白
            token = trim(token);

            if (!ignoreEmpty || !token.empty()) {
                // 只在需要时才创建std::string
                tokens.emplace_back(token);
            }

            start = end + (end != std::string::npos);
        }

        return tokens;
    }

    std::vector<std::string> split(const std::string& str, const std::string& delimiter, bool ignoreEmpty)
    {
        std::vector<std::string> tokens;
        if (str.empty()) return tokens;

        const size_t delim_len = delimiter.length();
        if (delim_len == 0) {
            tokens.push_back(str);
            return tokens;
        }

        // 预分配内存 (基于分隔符出现次数的估计)
        tokens.reserve(str.length() / (delim_len + 1) + 1);

        size_t start = 0;
        size_t pos = 0;

        while ((pos = str.find(delimiter, start)) != std::string::npos) {
            std::string token(str.data() + start, pos - start);
            token = trim(token);
            if (!ignoreEmpty || !token.empty()) {
                // 只有在需要时才创建 std::string
                tokens.emplace_back(token);
            }

            start = pos + delim_len;
        }

        // 处理最后一部分
        std::string last_token(str.data() + start, str.length() - start);
        if (!ignoreEmpty || !last_token.empty()) {
            tokens.emplace_back(last_token);
        }

        return tokens;
    }

    std::vector<std::string> unique(std::vector<std::string> arr) {
        std::sort(arr.begin(), arr.end());
        auto last = std::unique(arr.begin(), arr.end());
        arr.erase(last, arr.end());
        return arr;
    }

    // 将 string 的每个字节转为十六进制字符串
    std::string to_hex_string(const std::string& input) {
        std::stringstream ss;
        for (unsigned char c : input) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(c));
        }
        return ss.str();
    }

    // 将字节数组转换为十六进制字符串(默认大写)
    std::string bytesToHex(const std::vector<uint8_t>& bytes, bool uppercase) {
        std::string hex;
        hex.reserve(bytes.size() * 2); // 预分配内存, 避免多次扩容

        // 十六进制字符表(大写或小写)
        const char* hex_chars = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

        for (uint8_t byte : bytes) {
            // 提取高4位和低4位, 并映射到对应的字符
            hex.push_back(hex_chars[byte >> 4]);    // 高四位
            hex.push_back(hex_chars[byte & 0x0F]);  // 低四位
        }
        return hex;
    }

    std::vector<uint8_t> hexToBytes(const std::string& hex) {
        std::vector<uint8_t> bytes;
        // 检查字符串长度是否为偶数
        if (hex.length() % 2 != 0) {
            throw std::invalid_argument("Hex string must have even length");
        }

        for (size_t i = 0; i < hex.length(); i += 2) {
            char highChar = hex[i];
            char lowChar = hex[i + 1];

            // 验证是否为有效十六进制字符
            if (!isxdigit(highChar) || !isxdigit(lowChar)) {
                throw std::invalid_argument("Invalid hex character detected");
            }

            // 转换为大写统一处理
            highChar = toupper(highChar);
            lowChar = toupper(lowChar);

            // 计算高四位和低四位的值
            unsigned char high = (highChar >= 'A') ? (highChar - 'A' + 10) : (highChar - '0');
            unsigned char low = (lowChar >= 'A') ? (lowChar - 'A' + 10) : (lowChar - '0');

            // 合并为一个字节
            bytes.push_back((high << 4) | low);
        }

        return bytes;
    }

    std::string replace_all(std::string str, const std::string &from, const std::string &to) {
        size_t pos = 0;
        auto from_size = from.size();
        auto to_size = to.size();

        while ((pos = str.find(from, pos)) != std::string::npos) {
            str.replace(pos, from_size, to);
            pos += to_size; // 防止无限循环
        }
        return str;
    }

    // =============================================================================
    // Go 代码移植实现 - 从 api/strings*.go 转换而来
    // =============================================================================

    // 基础字符工具函数
    bool is_lower(char ch) {
        return ch >= 'a' && ch <= 'z';
    }

    bool is_upper(char ch) {
        return ch >= 'A' && ch <= 'Z';
    }

    char to_lower(char ch) {
        if (ch >= 'A' && ch <= 'Z') {
            return ch + 32;
        }
        return ch;
    }

    char to_upper(char ch) {
        if (ch >= 'a' && ch <= 'z') {
            return ch - 32;
        }
        return ch;
    }

    bool is_space(char ch) {
        return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
    }

    bool is_delimiter(char ch) {
        return ch == '-' || ch == '_' || is_space(ch);
    }

    // 字符串迭代器实现
    void string_iter(const std::string& s, const iter_func& callback) {
        if (s.empty()) return;
        
        char prev = 0;
        char curr = 0;
        
        for (size_t i = 0; i < s.length(); ++i) {
            char next = (i + 1 < s.length()) ? s[i + 1] : 0;
            
            if (curr == 0) {
                prev = curr;
                curr = s[i];
                continue;
            }
            
            callback(prev, curr, next);
            
            prev = curr;
            curr = s[i];
        }
        
        if (!s.empty()) {
            callback(prev, curr, 0);
        }
    }

    // ToString 系列函数实现
    std::string toString(int8_t value) {
        return std::to_string(static_cast<int>(value));
    }

    std::string toString(int16_t value) {
        return std::to_string(value);
    }

    std::string toString(int32_t value) {
        return std::to_string(value);
    }

    std::string toString(int64_t value) {
        return std::to_string(value);
    }

    std::string toString(uint8_t value) {
        return std::to_string(static_cast<unsigned>(value));
    }

    std::string toString(uint16_t value) {
        return std::to_string(value);
    }

    std::string toString(uint32_t value) {
        return std::to_string(value);
    }

    std::string toString(uint64_t value) {
        return std::to_string(value);
    }

    std::string toString(float value) {
        return std::to_string(value);
    }

    std::string toString(double value) {
        return std::to_string(value);
    }

    std::string toString(bool value) {
        return value ? "true" : "false";
    }

    // 内部实现命名空间
    namespace detail {
        std::string camel_case_impl(const std::string& str, bool upper_first) {
            std::string s = trim(str);
            if (s.empty()) return s;
            
            std::string result;
            result.reserve(s.length());
            
            string_iter(s, [&](char prev, char curr, char /*next*/) {
                if (!is_delimiter(curr)) {
                    if (is_delimiter(prev) || (upper_first && prev == 0)) {
                        result += to_upper(curr);
                    } else if (is_lower(prev)) {
                        result += curr;
                    } else {
                        result += to_lower(curr);
                    }
                }
            });
            
            return result;
        }

        std::string delimiter_case_impl(const std::string& str, char delimiter, bool upper_case) {
            std::string s = trim(str);
            if (s.empty()) return s;
            
            std::string result;
            result.reserve(s.length() + 3);
            
            auto adjust_case = [upper_case](char c) { return upper_case ? to_upper(c) : to_lower(c); };
            
            char prev = 0;
            char curr = 0;
            
            for (char next : s) {
                if (is_delimiter(curr)) {
                    if (!is_delimiter(prev) && prev != 0) {
                        result += delimiter;
                    }
                } else if (is_upper(curr)) {
                    if (is_lower(prev) || (is_upper(prev) && is_lower(next))) {
                        result += delimiter;
                    }
                    result += adjust_case(curr);
                } else if (curr != 0) {
                    result += adjust_case(curr);
                }
                prev = curr;
                curr = next;
            }
            
            // 处理最后一个字符
            if (!s.empty()) {
                if (is_upper(curr) && is_lower(prev) && prev != 0) {
                    result += delimiter;
                }
                if (!is_delimiter(curr)) {
                    result += adjust_case(curr);
                }
            }
            
            return result;
        }
    }

    // CamelCase 转换函数实现
    std::string to_camel_case(const std::string& str) {
        // 兼容原有的 ToCamelCase 逻辑
        std::string result;
        bool is_to_upper = false;
        
        for (char ch : str) {
            if (is_to_upper) {
                result += to_upper(ch);
                is_to_upper = false;
            } else {
                if (ch == '-' || ch == '_') {
                    is_to_upper = true;
                } else {
                    result += ch;
                }
            }
        }
        
        return result;
    }

    std::string upper_camel_case(const std::string& str) {
        return detail::camel_case_impl(str, true);
    }

    std::string lower_camel_case(const std::string& str) {
        return detail::camel_case_impl(str, false);
    }

    // SnakeCase 转换函数实现
    std::string snake_case(const std::string& str) {
        return detail::delimiter_case_impl(str, '_', false);
    }

    std::string upper_snake_case(const std::string& str) {
        return detail::delimiter_case_impl(str, '_', true);
    }

    // KebabCase 转换函数实现
    std::string kebab_case(const std::string& str) {
        return detail::delimiter_case_impl(str, '-', false);
    }

    std::string upper_kebab_case(const std::string& str) {
        return detail::delimiter_case_impl(str, '-', true);
    }

    // 字符串匹配和判断函数实现
    bool starts_with(const std::string& str, const std::vector<std::string>& prefixes) {
        if (str.empty() || prefixes.empty()) {
            return false;
        }
        
        for (const auto& prefix : prefixes) {
            if (str.length() >= prefix.length() && 
                str.substr(0, prefix.length()) == prefix) {
                return true;
            }
        }
        
        return false;
    }

    bool ends_with(const std::string& str, const std::vector<std::string>& suffixes) {
        if (str.empty() || suffixes.empty()) {
            return false;
        }
        
        for (const auto& suffix : suffixes) {
            if (str.length() >= suffix.length() && 
                str.substr(str.length() - suffix.length()) == suffix) {
                return true;
            }
        }
        
        return false;
    }

    bool is_empty(const std::string& str) {
        return trim(str).empty();
    }
}