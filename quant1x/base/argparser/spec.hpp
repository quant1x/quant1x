// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace argparser {

    /// 参数出现时怎么取值、怎么落进命名空间, 对应 Python 的 `Action`。
    enum class Action {
        STORE,
        STORE_TRUE,
        STORE_FALSE,
        STORE_CONST,
        APPEND,
        APPEND_CONST,
        EXTEND,
        COUNT,
        HELP,
        VERSION,
    };

    /// 动作名, 与 Python 的 `Action.value` 同形 (`store` / `store_true` / ...)。
    std::string_view action_name(Action action);

    /// 该动作是否从命令行吃掉取值。
    bool action_takes_values(Action action);

    /// 把动作名归一成 `Action`, 兼容 clap 的 `SetTrue` 一类拼法; 无法识别抛 `std::invalid_argument`。
    Action coerce_action(std::string_view text);

    /// 取值形态; 零值是一个取值。
    class Nargs {
    public:
        /// 形态种类。
        enum class Kind {
            ONE,
            FIXED,
            OPTIONAL,
            ZERO_OR_MORE,
            ONE_OR_MORE,
            REMAINDER,
        };

        /// 一个取值。
        static Nargs one();
        /// 恰好 `count` 个取值。
        static Nargs fixed(std::int64_t count);
        /// `?`: 最多一个取值。
        static Nargs optional();
        /// `*`: 任意个取值。
        static Nargs zero_or_more();
        /// `+`: 至少一个取值。
        static Nargs one_or_more();
        /// 其余 token 全部收下, 对应 `argparse.REMAINDER`。
        static Nargs remainder();

        /// 形态种类。
        [[nodiscard]] Kind kind() const { return kind_; }
        /// 整数形态的取值个数; 其余形态为 0。
        [[nodiscard]] std::int64_t count() const { return count_; }
        /// 形态名, 与 Python 的 `nargs` 写法同形。
        [[nodiscard]] std::string kind_name() const;
        /// 最少吃几个取值。
        [[nodiscard]] std::int64_t min() const;
        /// 最多吃几个取值; 无上限为 `-1`。
        [[nodiscard]] std::int64_t max() const;
        /// 取值是否恒以列表形态落进命名空间。
        [[nodiscard]] bool collects() const;
        /// 是否 REMAINDER 形态。
        [[nodiscard]] bool is_remainder() const { return kind_ == Kind::REMAINDER; }

    private:
        Nargs(Kind kind, std::int64_t count) : kind_(kind), count_(count) {}

        Kind         kind_  = Kind::ONE;
        std::int64_t count_ = 0;
    };

    /// 解析结果里的一个取值, 对应 Python 侧的任意对象。
    /// 列表形态内嵌 `std::vector<Value>`, 值语义的拷贝与赋值天然成环。
    // NOLINTNEXTLINE(misc-no-recursion)
    class Value {
    public:
        /// 空值, 对应 Python 的 `None`。
        Value();
        Value(bool value);
        Value(std::int64_t value);
        Value(double value);
        Value(std::string value);
        Value(const char *value);

        /// 列表形态, 对应 `nargs` 的收集形态。
        static Value list(std::vector<Value> items);

        /// 是否空值。
        [[nodiscard]] bool is_null() const;
        /// 字符串形态; 不是字符串时为 `nullptr`。
        [[nodiscard]] const std::string *as_string() const;
        /// 整数形态; 不是整数时为 `nullptr`。
        [[nodiscard]] const std::int64_t *as_int() const;
        /// 浮点形态; 不是浮点时为 `nullptr`。
        [[nodiscard]] const double *as_double() const;
        /// 布尔形态; 不是布尔时为 `nullptr`。
        [[nodiscard]] const bool *as_bool() const;
        /// 列表形态; 不是列表时为 `nullptr`。
        [[nodiscard]] const std::vector<Value> *as_list() const;

        /// 文本形态: 浮点补 `.0`、`inf` / `nan` 小写、布尔 `true` / `false`、列表 `a, b`。
        [[nodiscard]] std::string text() const;

    private:
        /// 列表构造用的标签, 避免与字符串重载混淆。
        struct ListTag {};

        Value(std::vector<Value> items, ListTag) : storage_(std::move(items)) {}

        /// 标量形态的文本; 列表由 `text()` 逐层展开, 不走这一支。
        [[nodiscard]] std::string scalar_text() const;

        using Storage = std::variant<std::monostate, bool, std::int64_t, double, std::string, std::vector<Value>>;
        Storage storage_;
    };

    /// 浮点的文本形态, 与 Python `str(float)` 同形。
    std::string float_text(double value);

    /// 缺省值来源: 没给 / 不写进命名空间 / 具体取值。
    class DefaultValue {
    public:
        /// 没有显式给出默认值。
        static DefaultValue missing();
        /// 默认值取本标记表示不写进命名空间, 对应 `argparse.SUPPRESS`。
        static DefaultValue suppress();
        /// 显式给出一个默认值。
        static DefaultValue of(Value value);

        /// 是否没给默认值。
        [[nodiscard]] bool is_missing() const { return kind_ == Kind::MISSING; }
        /// 是否 `SUPPRESS`。
        [[nodiscard]] bool is_suppress() const { return kind_ == Kind::SUPPRESS; }
        /// 默认值本体; 没给或 `SUPPRESS` 时为 `nullptr`。
        [[nodiscard]] const Value *value() const;
        /// 文本形态。
        [[nodiscard]] std::string text() const;

    private:
        enum class Kind { MISSING, SUPPRESS, GIVEN };

        DefaultValue(Kind kind, std::optional<Value> value) : kind_(kind), value_(std::move(value)) {}

        Kind                 kind_ = Kind::MISSING;
        std::optional<Value> value_;
    };

    /// 一个可选值及其说明。
    struct Choice {
        Value       value;
        std::string help;
    };

    /// 取值转换器: 把命令行上的字符串翻成取值, 非法取值抛 `std::invalid_argument`。
    class ValueParser {
    public:
        /// 转换函数。
        std::function<Value(std::string_view)> convert;
        /// 类型名, 只用于文案。
        std::string type_name;

        /// 转换一个取值。
        Value operator()(std::string_view text) const;
    };

    /// 十进制整数转换器, 可带闭区间上下界。
    ValueParser parse_int(std::optional<std::int64_t> minimum = std::nullopt,
                          std::optional<std::int64_t> maximum = std::nullopt);

    /// 浮点转换器, 可带闭区间上下界。
    ValueParser parse_float(std::optional<double> minimum = std::nullopt, std::optional<double> maximum = std::nullopt);

    /// 布尔转换器: `true/false`、`yes/no`、`on/off`、`1/0`, 大小写不敏感。
    ValueParser parse_bool();

    /// 单个命令行参数的定义, 由 `Parser::add_argument` 登记。
    ///
    /// 位置参数没有 `names`; 选项至少有一个以 `-` 开头的名字。
    struct Argument {
        std::vector<std::string>           names;
        std::string                        dest;
        Action                             action = Action::STORE;
        Nargs                              nargs  = Nargs::one();
        std::optional<ValueParser>         type;
        std::optional<std::vector<Choice>> choices;
        DefaultValue                       default_value = DefaultValue::missing();
        DefaultValue                       const_value   = DefaultValue::missing();
        bool                               required      = false;
        std::optional<std::string>         metavar;
        std::optional<std::string>         help;
        std::optional<std::string>         long_help;
        std::optional<std::string>         env;
        std::vector<std::string>           alias;
        std::vector<std::string>           visible_alias;
        std::vector<std::string>           conflicts_with;
        /// 依赖的其它参数 (名字或 `dest`); C++ 侧避开关键字 `requires`。
        std::vector<std::string> requires_args;
        /// 与之互相覆盖的参数 (名字或 `dest`); 一对里命令行上后出现的留下。
        std::vector<std::string> overrides_with;
        /// 列出的参数里只要有一个出现过, 本参数就不算必填。
        std::vector<std::string>   required_unless_present;
        std::optional<std::string> value_delimiter;
        bool                       hide                = false;
        bool                       hide_default        = false;
        bool                       allow_hyphen_values = false;
        bool                       global              = false;
        std::optional<std::string> group;

        /// 位置参数: 没有选项名, 按命令行顺序吃取值。
        [[nodiscard]] bool is_positional() const { return names.empty(); }
        /// 是否吃取值; 开关类动作为假。
        [[nodiscard]] bool takes_values() const { return action_takes_values(action); }
        /// 取值是否恒以列表形态落进命名空间。
        [[nodiscard]] bool collects() const { return value_delimiter.has_value() || nargs.collects(); }
        /// 最少吃几个取值。
        [[nodiscard]] std::int64_t min_count() const { return nargs.min(); }
        /// 最多吃几个取值; 无上限为 `-1`。
        [[nodiscard]] std::int64_t max_count() const { return nargs.max(); }
        /// 取值的显示名: `metavar` 优先, 其次大写化的 `dest`。
        [[nodiscard]] std::string value_name() const;
        /// 取值占位, 如 `<FILE>`。
        [[nodiscard]] std::string value_spec() const { return "<" + value_name() + ">"; }
        /// 指代该参数时优先用长选项; 只有短选项时退回它。
        [[nodiscard]] std::string primary_name() const;
        /// 错误文案里的指代, 如 `--output <FILE>` / `<FILE>` / `-v`。
        [[nodiscard]] std::string error_name() const;
        /// 帮助左列。
        [[nodiscard]] std::string help_names() const;
        /// usage 行里的位置参数占位。
        [[nodiscard]] std::string usage_spec() const;
        /// 可选值集合; 没配 `choices` 时为空。
        [[nodiscard]] std::vector<Value> choice_values() const;
        /// 可选值的说明文字。
        [[nodiscard]] std::string choice_help(const Value &value) const;
    };

    /// 解析结果的容器, 对应 `argparse.Namespace`; 保持首次写入的顺序。
    class Namespace {
    public:
        /// 按名字读取; 未写入时为 `nullptr`。
        [[nodiscard]] const Value *find(std::string_view key) const;
        /// 该名字是否已写入。
        [[nodiscard]] bool contains(std::string_view key) const { return find(key) != nullptr; }
        /// 写入一个取值; 同名覆盖且位置不变。
        void set(std::string key, Value value);
        /// 抹掉一个已写入的取值; 没写过时什么也不做。
        void erase(std::string_view key);
        /// 按写入顺序遍历。
        [[nodiscard]] const std::vector<std::pair<std::string, Value>> &entries() const { return entries_; }
        /// 已写入的条目数。
        [[nodiscard]] std::size_t size() const { return entries_.size(); }
        /// 文本形态, 形如 `Namespace(k=v, ...)`。
        [[nodiscard]] std::string text() const;

    private:
        std::vector<std::pair<std::string, Value>> entries_;
    };

    /// 把取值拼成 `a, b, c` 形态。
    std::string format_items(const std::vector<Value> &values);

    /// 把名字拼成 `a, b, c` 形态。
    std::string format_names(const std::vector<std::string> &names);

}  // namespace argparser
