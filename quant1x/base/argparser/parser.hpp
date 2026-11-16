// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "error.hpp"
#include "spec.hpp"

namespace argparser {

    class Parser;

    /// 着色的时机, 对应 Python 的 `color` 参数。
    enum class ColorChoice {
        AUTO,
        ALWAYS,
        NEVER,
    };

    /// 一条子命令的展示信息。
    struct Command {
        std::string              name;
        std::vector<std::string> aliases;
        Parser                  *parser = nullptr;
    };

    /// 帮助里的一个分组的标题与说明。
    struct GroupInfo {
        std::string                title;
        std::optional<std::string> description;
    };

    /// 互斥组: 组内参数最多出现一个; `required` 为真时至少出现一个。
    class MutexGroup {
    public:
        /// 是否要求至少出现一个。
        [[nodiscard]] bool required() const { return required_; }
        /// 组内参数, 按登记顺序。
        [[nodiscard]] const std::vector<Argument *> &arguments() const { return arguments_; }
        /// 登记一个参数并纳入本互斥组; 定义期错误抛 `std::invalid_argument`。
        Argument &add_argument(std::vector<std::string> names, Argument argument);

    private:
        friend class Parser;

        Parser                 *parser_   = nullptr;
        bool                    required_ = false;
        std::vector<Argument *> arguments_;
    };

    /// 帮助里的一个参数分组, 只影响渲染不影响解析。
    class ArgumentGroup {
    public:
        /// 分组标题。
        [[nodiscard]] const std::string &title() const { return title_; }
        /// 分组说明。
        [[nodiscard]] const std::optional<std::string> &description() const { return description_; }
        /// 登记一个参数并把它归到本组; 定义期错误抛 `std::invalid_argument`。
        Argument &add_argument(std::vector<std::string> names, Argument argument);

    private:
        friend class Parser;

        Parser                    *parser_ = nullptr;
        std::string                title_;
        std::optional<std::string> description_;
    };

    /// 多值组: 组内参数可以同时出现; `required` 为真时至少要出现一个。
    class ValueGroup {
    public:
        /// 组名, 缺必填时报在 `<名字>` 那一行。
        [[nodiscard]] const std::string &name() const { return name_; }
        /// 是否要求至少出现一个。
        [[nodiscard]] bool required() const { return required_; }
        /// 组内参数, 按登记顺序。
        [[nodiscard]] const std::vector<Argument *> &arguments() const { return arguments_; }
        /// 登记一个参数并纳入本组; 定义期错误抛 `std::invalid_argument`。
        Argument &add_argument(std::vector<std::string> names, Argument argument);

    private:
        friend class Parser;
        Parser                 *parser_ = nullptr;
        std::string             name_;
        bool                    required_ = false;
        std::vector<Argument *> arguments_;
    };

    /// 子命令集合: 名字 (含别名) 到命令的映射, 并记录帮助里的展示顺序。
    class SubParsers {
    public:
        /// 写进命名空间的子命令名字段。
        [[nodiscard]] const std::string &dest() const { return dest_; }
        /// 帮助章节标题。
        [[nodiscard]] const std::string &title() const { return title_; }
        /// 子命令是否必填。
        [[nodiscard]] bool required() const { return required_; }
        /// 子命令章节的说明文字。
        [[nodiscard]] const std::string &help() const { return help_; }
        /// 设置子命令章节的说明文字。
        void set_help(std::string help) { help_ = std::move(help); }

        /// 登记一条子命令; `aliases` 不出现在帮助里, `visible_aliases` 出现。
        Parser &add_parser(std::string              name,
                           std::string              about           = {},
                           std::vector<std::string> aliases         = {},
                           std::vector<std::string> visible_aliases = {});

        /// 按名字或别名找子命令。
        [[nodiscard]] Parser *lookup(std::string_view name) const;
        /// 该名字 (含别名) 是否已被登记成子命令。
        [[nodiscard]] bool has_command(std::string_view name) const;
        /// 名字归一成注册时的正式名 (别名解析回正名)。
        [[nodiscard]] std::string canonical(std::string_view name) const;
        /// 按注册顺序给出子命令。
        [[nodiscard]] std::vector<Command> commands() const;

    private:
        friend class Parser;

        Parser                                         *parent_ = nullptr;
        std::string                                     dest_;
        std::string                                     title_ = "Commands";
        std::string                                     help_;
        bool                                            required_ = false;
        std::vector<std::unique_ptr<Parser>>            commands_;
        std::map<std::string, std::size_t>              lookup_;
        std::map<std::string, std::vector<std::string>> visible_;
    };

    /// 一条命令: 参数定义、解析入口与 usage 片段。子命令本身也是 `Parser`。
    ///
    /// 参数与子命令都按地址持有 (`unique_ptr`), 故 Parser 自身不搬家。
    class Parser {
    public:
        /// 起一条命令; 缺省带 `-h` / `--help`。
        explicit Parser(std::string prog);

        Parser(const Parser &)            = delete;
        Parser &operator=(const Parser &) = delete;
        Parser(Parser &&)                 = delete;
        Parser &operator=(Parser &&)      = delete;

        Parser &with_about(std::string about);
        Parser &with_long_about(std::string about);
        /// 设置版本号并登记 `-V` / `--version`; 子命令没自带时沿用父命令的。
        Parser &with_version(std::string version);
        Parser &with_epilog(std::string epilog);
        Parser &with_usage(std::string usage);
        /// 关掉长选项前缀匹配。
        Parser &without_abbrev();
        /// 不再自动登记 `-h` / `--help`。
        Parser &without_help();
        Parser &with_argument_default(DefaultValue value);
        Parser &with_color(ColorChoice color);
        Parser &with_max_width(std::uint32_t width);
        /// 子命令出现时不再要求本命令的必填参数。
        Parser &with_subcommand_negates_reqs(bool flag);
        /// 帮助宽度按 `COLUMNS` 取; 取不到或非法时仍用 `max_width`。
        Parser &with_auto_width();
        /// 帮助的生效宽度。
        [[nodiscard]] std::uint32_t width() const;
        /// 把 `@file` 形态的 token 展开成文件里的行; 传空串等于不展开。
        Parser &with_from_file_prefix_chars(std::string chars);

        /// 登记一个参数: 首个名字以 `-` 开头是选项, 否则是位置参数。
        Argument &add_argument(std::vector<std::string> names, Argument argument);
        /// 登记一个帮助分组。
        ArgumentGroup &add_group(std::string title, std::optional<std::string> description = std::nullopt);
        /// 登记一个互斥组。
        MutexGroup &add_mutually_exclusive_group(bool required = false);
        /// 登记一个多值组: 组内参数可同时出现, `required` 表示至少出现一个。
        ValueGroup &add_value_group(std::string name, bool required = false);
        /// 登记子命令集合; `dest` 给出时把命中 (含别名) 的原样写进命名空间。
        SubParsers &add_subparsers(std::string dest = {}, std::string title = "Commands", bool required = false);

        /// 命令全名。
        [[nodiscard]] const std::string &prog() const { return prog_; }
        /// 一段简介。
        [[nodiscard]] const std::optional<std::string> &about() const { return about_; }
        /// 长简介。
        [[nodiscard]] const std::optional<std::string> &long_about() const { return long_about_; }
        /// 版本号; 子命令没自带时沿用父命令的。
        [[nodiscard]] std::string version() const;
        /// 版本行的正文: `<prog> <version>`。
        [[nodiscard]] std::string version_text() const;
        /// 尾注。
        [[nodiscard]] const std::optional<std::string> &epilog() const { return epilog_; }
        /// 着色的时机。
        [[nodiscard]] ColorChoice color() const { return color_; }
        /// 帮助的折行宽度。
        [[nodiscard]] std::uint32_t max_width() const { return max_width_; }
        /// 本命令登记的参数 (含位置参数), 按注册顺序。
        [[nodiscard]] std::vector<const Argument *> arguments() const;
        /// 本命令的位置参数。
        [[nodiscard]] std::vector<const Argument *> positionals() const;
        /// 帮助分组, 按登记顺序。
        [[nodiscard]] const std::vector<GroupInfo> &groups() const { return groups_; }
        /// 互斥组, 按登记顺序。
        [[nodiscard]] const std::vector<std::unique_ptr<MutexGroup>> &mutex_groups() const { return mutex_; }
        /// 多值组, 按登记顺序。
        [[nodiscard]] const std::vector<std::unique_ptr<ValueGroup>> &value_groups() const { return value_groups_; }
        /// 子命令集合; 没登记则为 `nullptr`。
        SubParsers *subparsers() { return subparsers_.get(); }
        /// 子命令集合; 没登记则为 `nullptr`。
        [[nodiscard]] const SubParsers *subparsers() const { return subparsers_.get(); }
        /// 在父命令里的子命令名; 顶层命令为空。
        [[nodiscard]] const std::string &name() const { return name_; }
        /// 父命令; 顶层命令为 `nullptr`。
        [[nodiscard]] Parser *parent() const { return parent_; }
        /// 是否着色。
        [[nodiscard]] bool color_enabled() const;
        /// `-` / `--` 与签名数字以外的 `-` 开头 token 是选项。
        static bool is_option_token(std::string_view token);
        /// usage 行 (`Usage:` 之后) 的字符串形态。
        [[nodiscard]] std::string usage_spec() const;
        /// 整段帮助文本 (不含末尾换行); `long_help` 为真时用长简介与长说明。
        [[nodiscard]] std::string format_help(bool long_help = false) const;
        /// 单行用法文本 (`Usage:` 前缀在内)。
        [[nodiscard]] std::string format_usage() const;
        /// 把帮助写到流, 末尾补一个换行。
        void print_help(std::ostream &writer, bool long_help = false) const;
        /// 解析并落到进程出口: 帮助 / 版本写 `out` 退出码 0, 用法错误写 `err` 退出码 2。
        ///
        /// 返回 `int` 表示文本已经打印完毕, 调用方据此退出进程; 定义期错误照 Python 侧一样抛出
        /// `std::runtime_error`, 不退化成一次正常退出。
        std::variant<Namespace, int>
        parse_or_exit(std::vector<std::string> argv, std::ostream &out, std::ostream &err) const;

        /// `parse_known` 的结果: 命名空间与认不出的 token (按命令行顺序)。
        struct KnownOutcome {
            Namespace                namespace_value;
            std::vector<std::string> extras;
        };

        /// 解析命令行; 输入非法给出错误, 请求帮助 / 版本给出对应变体。
        [[nodiscard]] std::variant<Namespace, ParseError> parse(std::vector<std::string> argv) const;
        /// 同 `parse`, 但认不出的 token 收进返回值而不报错。
        [[nodiscard]] std::variant<KnownOutcome, ParseError> parse_known(std::vector<std::string> argv) const;

    private:
        friend class SubParsers;

        /// 一次解析的共享状态。
        struct Run;
        /// 解析途中的控制流: 携带要返回的错误。
        struct Failure {
            ParseError error;
        };

        void push(Argument argument);
        void register_version();
        /// 按命令全名定位到命令本身, 供帮助渲染找到请求帮助的那条子命令。
        [[nodiscard]] const Parser                          *parser_by_prog(std::string_view prog) const;
        [[nodiscard]] std::variant<KnownOutcome, ParseError> parse_run(std::vector<std::string> argv,
                                                                       bool                     tolerant) const;
        void                                                 parse_tokens(std::vector<std::string>    tokens,
                                                                          Run                        &run,
                                                                          std::vector<const Parser *> chain,
                                                                          std::size_t                 base) const;
        [[nodiscard]] std::optional<std::vector<std::string>>
        expand_cluster(std::string_view token, const std::vector<const Parser *> &chain) const;
        [[nodiscard]] std::size_t     positional_minimum() const;
        [[nodiscard]] const Parser   *subcommand_of(std::string_view token, std::size_t pending) const;
        [[nodiscard]] bool            help_subcommand_of(std::string_view token, std::size_t pending) const;
        [[nodiscard]] const Parser   *help_target(const std::vector<std::string> &rest) const;
        [[nodiscard]] bool            remainder_ready(std::size_t pending) const;
        [[nodiscard]] const Argument *lookup(std::string_view name, const std::vector<const Parser *> &chain) const;
        [[nodiscard]] const Argument *abbreviate(std::string_view name) const;
        [[noreturn]] void throw_unknown_argument(std::string_view name, const std::vector<const Parser *> &chain) const;
        std::size_t       consume_option(std::vector<std::string>          &tokens,
                                         std::size_t                        index,
                                         Run                               &run,
                                         const std::vector<const Parser *> &chain,
                                         std::size_t                        base) const;
        [[nodiscard]] std::optional<std::pair<const Argument *, std::string>>
        attached_short(std::string_view name, const std::vector<const Parser *> &chain) const;
        [[nodiscard]] std::pair<std::size_t, std::vector<std::string>>
        take_values(const Argument &argument, const std::vector<std::string> &tokens, std::size_t start) const;
        [[noreturn]] void throw_missing_value(const Argument &argument, std::int64_t needed, std::int64_t given) const;
        [[nodiscard]] std::vector<Value> convert_texts(const Argument                 &argument,
                                                       const std::vector<std::string> &raw) const;
        [[nodiscard]] Value              convert(const Argument &argument, std::string_view text) const;
        void                             apply(const Argument &argument, std::vector<Value> values, Run &run) const;
        static void                      append_value(Run &run, const std::string &dest, Value item);
        void        assign_positionals(const std::vector<std::pair<std::size_t, std::string>> &pending, Run &run) const;
        void        store_positional(const Argument &argument, const std::vector<std::string> &chunk, Run &run) const;
        static void finish(Run &run);
        static void apply_overrides(Run &run);
        static void apply_env(Run &run);
        static void validate(Run &run);
        static void apply_defaults(Run &run);
        [[nodiscard]] UsageError               make_error(std::string message, ErrorKind kind) const;
        [[nodiscard]] std::vector<std::string> expand_files(const std::vector<std::string> &tokens) const;

        std::string                prog_;
        std::optional<std::string> about_;
        std::optional<std::string> long_about_;
        std::optional<std::string> version_;
        std::optional<std::string> epilog_;
        std::optional<std::string> usage_;
        bool                       allow_abbrev_            = true;
        DefaultValue               argument_default_        = DefaultValue::missing();
        ColorChoice                color_                   = ColorChoice::AUTO;
        std::uint32_t              max_width_               = 100;
        bool                       subcommand_negates_reqs_ = false;
        /// 是否按 `COLUMNS` 取帮助宽度。
        bool                                        auto_width_ = false;
        std::optional<std::string>                  fromfile_prefix_chars_;
        std::vector<std::unique_ptr<Argument>>      arguments_;
        std::map<std::string, std::size_t>          by_name_;
        std::vector<std::string>                    name_order_;
        std::map<std::string, std::size_t>          dests_;
        std::vector<Argument *>                     positionals_;
        std::vector<GroupInfo>                      groups_;
        std::vector<std::unique_ptr<ArgumentGroup>> group_objects_;
        std::vector<std::unique_ptr<MutexGroup>>    mutex_;
        std::vector<std::unique_ptr<ValueGroup>>    value_groups_;
        std::unique_ptr<SubParsers>                 subparsers_;
        Parser                                     *parent_ = nullptr;
        std::string                                 name_;
    };

}  // namespace argparser
