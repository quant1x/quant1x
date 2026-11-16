// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <format>
#include <system_error>

#include "spec.hpp"

namespace argparser {

    namespace {

        /// 去空白。
        std::string trim(std::string_view text) {
            const auto begin = text.find_first_not_of(" \t\r\n");
            if (begin == std::string_view::npos) {
                return {};
            }
            const auto end = text.find_last_not_of(" \t\r\n");
            return std::string(text.substr(begin, end - begin + 1));
        }

        /// 小写化。
        std::string lower(std::string_view text) {
            std::string result(text);
            for (char &item : result) {
                item = static_cast<char>(std::tolower(static_cast<unsigned char>(item)));
            }
            return result;
        }

        /// 大写化。
        std::string upper(std::string_view text) {
            std::string result(text);
            for (char &item : result) {
                item = static_cast<char>(std::toupper(static_cast<unsigned char>(item)));
            }
            return result;
        }

        /// 整数上下界的开区间记法。
        std::string int_range_text(std::optional<std::int64_t> minimum, std::optional<std::int64_t> maximum) {
            if (!minimum && !maximum) {
                return {};
            }
            if (!minimum) {
                return "..=" + std::to_string(*maximum);
            }
            if (!maximum) {
                return std::to_string(*minimum) + "..";
            }
            return std::to_string(*minimum) + "..=" + std::to_string(*maximum);
        }

        /// 浮点上下界的开区间记法。
        std::string float_range_text(std::optional<double> minimum, std::optional<double> maximum) {
            if (!minimum && !maximum) {
                return {};
            }
            if (!minimum) {
                return "..=" + float_text(*maximum);
            }
            if (!maximum) {
                return float_text(*minimum) + "..";
            }
            return float_text(*minimum) + "..=" + float_text(*maximum);
        }

        /// 整数形态的十进制解析: 可选正负号 + ASCII 数字。
        bool parse_decimal(std::string_view text, std::int64_t &value) {
            if (text.empty()) {
                return false;
            }
            const auto result = std::from_chars(text.data(), text.data() + text.size(), value, 10);
            return result.ec == std::errc() && result.ptr == text.data() + text.size();
        }

    }  // namespace

    std::string_view action_name(Action action) {
        switch (action) {
            case Action::STORE:
                return "store";
            case Action::STORE_TRUE:
                return "store_true";
            case Action::STORE_FALSE:
                return "store_false";
            case Action::STORE_CONST:
                return "store_const";
            case Action::APPEND:
                return "append";
            case Action::APPEND_CONST:
                return "append_const";
            case Action::EXTEND:
                return "extend";
            case Action::COUNT:
                return "count";
            case Action::HELP:
                return "help";
            case Action::VERSION:
                return "version";
        }
        return "unknown";
    }

    bool action_takes_values(Action action) {
        return action == Action::STORE || action == Action::APPEND || action == Action::EXTEND;
    }

    Action coerce_action(std::string_view text) {
        std::string key = lower(trim(text));
        key.erase(std::remove(key.begin(), key.end(), '_'), key.end());
        key.erase(std::remove(key.begin(), key.end(), '-'), key.end());
        if (key == "store" || key == "set") {
            return Action::STORE;
        }
        if (key == "storetrue" || key == "settrue") {
            return Action::STORE_TRUE;
        }
        if (key == "storefalse" || key == "setfalse") {
            return Action::STORE_FALSE;
        }
        if (key == "storeconst") {
            return Action::STORE_CONST;
        }
        if (key == "append") {
            return Action::APPEND;
        }
        if (key == "appendconst") {
            return Action::APPEND_CONST;
        }
        if (key == "extend") {
            return Action::EXTEND;
        }
        if (key == "count") {
            return Action::COUNT;
        }
        if (key == "help") {
            return Action::HELP;
        }
        if (key == "version") {
            return Action::VERSION;
        }
        throw std::invalid_argument("无法识别的 action: " + std::string(text));
    }

    Nargs Nargs::one() {
        return {Kind::ONE, 0};
    }

    Nargs Nargs::fixed(std::int64_t count) {
        if (count < 0) {
            throw std::invalid_argument("nargs 不能为负: " + std::to_string(count));
        }
        return {Kind::FIXED, count};
    }

    Nargs Nargs::optional() {
        return {Kind::OPTIONAL, 0};
    }

    Nargs Nargs::zero_or_more() {
        return {Kind::ZERO_OR_MORE, 0};
    }

    Nargs Nargs::one_or_more() {
        return {Kind::ONE_OR_MORE, 0};
    }

    Nargs Nargs::remainder() {
        return {Kind::REMAINDER, 0};
    }

    std::string Nargs::kind_name() const {
        switch (kind_) {
            case Kind::FIXED:
                return std::to_string(count_);
            case Kind::OPTIONAL:
                return "?";
            case Kind::ZERO_OR_MORE:
                return "*";
            case Kind::ONE_OR_MORE:
                return "+";
            case Kind::REMAINDER:
                return "...";
            case Kind::ONE:
                break;
        }
        return {};
    }

    std::int64_t Nargs::min() const {
        switch (kind_) {
            case Kind::FIXED:
                return count_;
            case Kind::OPTIONAL:
            case Kind::ZERO_OR_MORE:
            case Kind::REMAINDER:
                return 0;
            case Kind::ONE:
            case Kind::ONE_OR_MORE:
                break;
        }
        return 1;
    }

    std::int64_t Nargs::max() const {
        switch (kind_) {
            case Kind::FIXED:
                return count_;
            case Kind::OPTIONAL:
                return 1;
            case Kind::ZERO_OR_MORE:
            case Kind::ONE_OR_MORE:
            case Kind::REMAINDER:
                return -1;
            case Kind::ONE:
                break;
        }
        return 1;
    }

    bool Nargs::collects() const {
        switch (kind_) {
            case Kind::FIXED:
            case Kind::ZERO_OR_MORE:
            case Kind::ONE_OR_MORE:
            case Kind::REMAINDER:
                return true;
            case Kind::ONE:
            case Kind::OPTIONAL:
                break;
        }
        return false;
    }

    Value::Value() : storage_(std::monostate{}) {
    }

    Value::Value(bool value) : storage_(value) {
    }

    Value::Value(std::int64_t value) : storage_(value) {
    }

    Value::Value(double value) : storage_(value) {
    }

    Value::Value(std::string value) : storage_(std::move(value)) {
    }

    Value::Value(const char *value) : storage_(std::string(value)) {
    }

    Value Value::list(std::vector<Value> items) {
        return Value(std::move(items), ListTag{});
    }

    bool Value::is_null() const {
        return std::holds_alternative<std::monostate>(storage_);
    }

    const std::string *Value::as_string() const {
        return std::get_if<std::string>(&storage_);
    }

    const std::int64_t *Value::as_int() const {
        return std::get_if<std::int64_t>(&storage_);
    }

    const double *Value::as_double() const {
        return std::get_if<double>(&storage_);
    }

    const bool *Value::as_bool() const {
        return std::get_if<bool>(&storage_);
    }

    const std::vector<Value> *Value::as_list() const {
        return std::get_if<std::vector<Value>>(&storage_);
    }

    std::string Value::text() const {
        std::string body;
        // 列表按元素顺序展开: 非首元素前补分隔符, 元素本身是列表时继续展开
        std::vector<std::pair<const Value *, bool>> pending{{this, false}};
        while (!pending.empty()) {
            const auto [value, separator] = pending.back();
            pending.pop_back();
            if (separator) {
                body += ", ";
            }
            const std::vector<Value> *items = value->as_list();
            if (items == nullptr) {
                body += value->scalar_text();
                continue;
            }
            for (std::size_t index = items->size(); index-- > 0;) {
                pending.emplace_back(&(*items)[index], index != 0);
            }
        }
        return body;
    }

    std::string Value::scalar_text() const {
        if (std::holds_alternative<std::monostate>(storage_)) {
            return "None";
        }
        if (const auto *flag = std::get_if<bool>(&storage_)) {
            return *flag ? "true" : "false";
        }
        if (const auto *number = std::get_if<std::int64_t>(&storage_)) {
            return std::to_string(*number);
        }
        if (const auto *number = std::get_if<double>(&storage_)) {
            return float_text(*number);
        }
        return std::get<std::string>(storage_);
    }

    std::string float_text(double value) {
        if (std::isnan(value)) {
            return "nan";
        }
        if (std::isinf(value)) {
            return value > 0.0 ? "inf" : "-inf";
        }
        std::string text = std::format("{}", value);
        if (text.find_first_of(".eE") == std::string::npos) {
            text += ".0";
        }
        return text;
    }

    DefaultValue DefaultValue::missing() {
        return {Kind::MISSING, std::nullopt};
    }

    DefaultValue DefaultValue::suppress() {
        return {Kind::SUPPRESS, std::nullopt};
    }

    DefaultValue DefaultValue::of(Value value) {
        return {Kind::GIVEN, std::move(value)};
    }

    const Value *DefaultValue::value() const {
        if (kind_ != Kind::GIVEN || !value_) {
            return nullptr;
        }
        return &*value_;
    }

    std::string DefaultValue::text() const {
        switch (kind_) {
            case Kind::MISSING:
                return "MISSING";
            case Kind::SUPPRESS:
                return "SUPPRESS";
            case Kind::GIVEN:
                break;
        }
        return value_ ? value_->text() : std::string("None");
    }

    Value ValueParser::operator()(std::string_view text) const {
        if (!convert) {
            throw std::invalid_argument("取值转换器未设置");
        }
        return convert(text);
    }

    ValueParser parse_int(std::optional<std::int64_t> minimum, std::optional<std::int64_t> maximum) {
        return ValueParser{
            [minimum, maximum](std::string_view text) -> Value {
                std::int64_t value = 0;
                if (!parse_decimal(text, value)) {
                    throw std::invalid_argument("invalid digit found in string");
                }
                const std::string range = int_range_text(minimum, maximum);
                if (minimum && value < *minimum) {
                    throw std::invalid_argument(std::to_string(value) + " is not in " + range);
                }
                if (maximum && value > *maximum) {
                    throw std::invalid_argument(std::to_string(value) + " is not in " + range);
                }
                return {value};
            },
            "INTEGER",
        };
    }

    ValueParser parse_float(std::optional<double> minimum, std::optional<double> maximum) {
        return ValueParser{
            [minimum, maximum](std::string_view text) -> Value {
                double     value  = 0.0;
                const auto result = std::from_chars(
                    text.data(), text.data() + text.size(), value, std::chars_format::general);
                if (result.ec != std::errc() || result.ptr != text.data() + text.size()) {
                    throw std::invalid_argument("invalid float literal");
                }
                const std::string range = float_range_text(minimum, maximum);
                if (minimum && value < *minimum) {
                    throw std::invalid_argument(float_text(value) + " is not in " + range);
                }
                if (maximum && value > *maximum) {
                    throw std::invalid_argument(float_text(value) + " is not in " + range);
                }
                return {value};
            },
            "FLOAT",
        };
    }

    ValueParser parse_bool() {
        return ValueParser{
            [](std::string_view text) -> Value {
                const std::string key = lower(trim(text));
                if (key == "true" || key == "yes" || key == "on" || key == "1") {
                    return {true};
                }
                if (key == "false" || key == "no" || key == "off" || key == "0") {
                    return {false};
                }
                throw std::invalid_argument("provided string was not one of true/false/yes/no/on/off/1/0");
            },
            "BOOL",
        };
    }

    std::string Argument::value_name() const {
        if (metavar) {
            return *metavar;
        }
        return upper(dest);
    }

    std::string Argument::primary_name() const {
        for (const std::string &name : names) {
            if (name.rfind("--", 0) == 0) {
                return name;
            }
        }
        return names.empty() ? dest : names.front();
    }

    std::string Argument::error_name() const {
        if (is_positional()) {
            return value_spec();
        }
        const std::string name = primary_name();
        return takes_values() ? name + " " + value_spec() : name;
    }

    std::string Argument::help_names() const {
        if (is_positional()) {
            return usage_spec();
        }
        std::vector<std::string> all = names;
        all.insert(all.end(), visible_alias.begin(), visible_alias.end());
        std::string head = format_names(all);
        if (action == Action::COUNT) {
            head += "...";
        }
        return takes_values() ? head + " " + value_spec() : head;
    }

    std::string Argument::usage_spec() const {
        const std::string name = value_name();
        switch (nargs.kind()) {
            case Nargs::Kind::OPTIONAL:
                return "[" + name + "]";
            case Nargs::Kind::ZERO_OR_MORE:
            case Nargs::Kind::REMAINDER:
                return "[" + name + "]...";
            case Nargs::Kind::ONE_OR_MORE:
            case Nargs::Kind::FIXED:
                return "<" + name + ">...";
            case Nargs::Kind::ONE:
                break;
        }
        return "<" + name + ">";
    }

    std::vector<Value> Argument::choice_values() const {
        std::vector<Value> values;
        if (!choices) {
            return values;
        }
        values.reserve(choices->size());
        for (const Choice &choice : *choices) {
            values.push_back(choice.value);
        }
        return values;
    }

    std::string Argument::choice_help(const Value &value) const {
        if (!choices) {
            return {};
        }
        for (const Choice &choice : *choices) {
            if (choice.value.text() == value.text()) {
                return choice.help;
            }
        }
        return {};
    }

    const Value *Namespace::find(std::string_view key) const {
        for (const auto &[name, value] : entries_) {
            if (name == key) {
                return &value;
            }
        }
        return nullptr;
    }

    void Namespace::set(std::string key, Value value) {
        for (auto &[name, slot] : entries_) {
            if (name == key) {
                slot = std::move(value);
                return;
            }
        }
        entries_.emplace_back(std::move(key), std::move(value));
    }

    void Namespace::erase(std::string_view key) {
        entries_.erase(
            std::remove_if(entries_.begin(), entries_.end(), [key](const auto &entry) { return entry.first == key; }),
            entries_.end());
    }

    std::string Namespace::text() const {
        std::string body;
        for (std::size_t index = 0; index < entries_.size(); ++index) {
            if (index != 0) {
                body += ", ";
            }
            body += entries_[index].first + "=" + entries_[index].second.text();
        }
        return "Namespace(" + body + ")";
    }

    std::string format_items(const std::vector<Value> &values) {
        std::string body;
        for (std::size_t index = 0; index < values.size(); ++index) {
            if (index != 0) {
                body += ", ";
            }
            body += values[index].text();
        }
        return body;
    }

    std::string format_names(const std::vector<std::string> &names) {
        std::string body;
        for (std::size_t index = 0; index < names.size(); ++index) {
            if (index != 0) {
                body += ", ";
            }
            body += names[index];
        }
        return body;
    }

}  // namespace argparser
