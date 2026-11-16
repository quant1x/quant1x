// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ranges>
#include <set>
#include <stdexcept>

#include "parser.hpp"

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

#include "help.hpp"

namespace argparser {

    namespace {

        /// 是否以 `prefix` 开头。
        bool has_prefix(std::string_view text, std::string_view prefix) {
            return text.size() >= prefix.size() && text.substr(0, prefix.size()) == prefix;
        }

        /// `@file` 展开的嵌套层数上限; 只用来挡住自引用, 正常用法远到不了。
        constexpr std::size_t kMaxIncludeDepth = 16;

        /// 按 LF 拆行, 行尾的 CR 去掉, 末尾那个空元素不算一行。
        std::vector<std::string> split_lines(const std::string &body) {
            std::vector<std::string> lines;
            std::string              current;
            for (const char item : body) {
                if (item == '\n') {
                    if (!current.empty() && current.back() == '\r') {
                        current.pop_back();
                    }
                    lines.push_back(std::move(current));
                    current.clear();
                    continue;
                }
                current.push_back(item);
            }
            if (!current.empty() && current.back() == '\r') {
                current.pop_back();
            }
            if (!current.empty()) {
                lines.push_back(std::move(current));
            }
            return lines;
        }

        /// 把 `--key=value` / `-k=value` 拆成 (名字, 内联取值)。
        std::pair<std::string, std::optional<std::string>> split_option(std::string_view token) {
            const auto position = token.find('=');
            if (position == std::string_view::npos) {
                return {std::string(token), std::nullopt};
            }
            return {std::string(token.substr(0, position)), std::string(token.substr(position + 1))};
        }

        /// 是否全部是十进制数字; 空串视为全部满足。
        bool all_digits(std::string_view text) {
            return std::all_of(text.begin(), text.end(), [](char item) { return item >= '0' && item <= '9'; });
        }

        /// 签名数字: 可选的 `+` / `-` 号加整数或小数, 整个 token 当一个取值参数 (`-1`、`-.5`、`+3`、`-1.5`)。
        bool is_signed_number(std::string_view token) {
            if (token.size() < 2 || (token[0] != '-' && token[0] != '+')) {
                return false;
            }
            const std::string_view body = token.substr(1);
            const auto             dot  = body.find('.');
            if (dot == std::string_view::npos) {
                return all_digits(body);
            }
            if (!all_digits(body.substr(0, dot))) {
                return false;
            }
            const std::string_view fraction = body.substr(dot + 1);
            return !fraction.empty() && all_digits(fraction);
        }

        /// 读环境变量; 未设置时为空。
        std::optional<std::string> get_env(const std::string &name) {
#if defined(_WIN32)
            char       *buffer = nullptr;
            std::size_t size   = 0;
            if (_dupenv_s(&buffer, &size, name.c_str()) != 0 || buffer == nullptr) {
                return std::nullopt;
            }
            std::string value(buffer);
            std::free(buffer);
            return value;
#else
            const char *raw = std::getenv(name.c_str());
            if (raw == nullptr) {
                return std::nullopt;
            }
            return std::string(raw);
#endif
        }

        /// 由选项名推 dest: 优先最长形态的长选项, 中划线转下划线。
        std::string dest_of_option(const std::vector<std::string> &names) {
            for (const std::string &name : names) {
                if (has_prefix(name, "--")) {
                    std::string body = name.substr(2);
                    std::replace(body.begin(), body.end(), '-', '_');
                    return body;
                }
            }
            if (names.empty()) {
                return {};
            }
            std::string body  = names.front();
            const auto  first = body.find_first_not_of('-');
            body              = first == std::string::npos ? std::string() : body.substr(first);
            std::replace(body.begin(), body.end(), '-', '_');
            return body;
        }

        /// 没显式给默认值时的落值。
        Value implicit_default(const Argument &argument) {
            switch (argument.action) {
                case Action::STORE_TRUE:
                    return {false};
                case Action::STORE_FALSE:
                    return {true};
                case Action::COUNT:
                    return Value(std::int64_t{0});
                default:
                    break;
            }
            if (argument.is_positional() &&
                (argument.nargs.kind() == Nargs::Kind::ZERO_OR_MORE || argument.nargs.is_remainder())) {
                return Value::list({});
            }
            return {};
        }

        /// 按默认值的类型推断取值转换器。
        std::optional<ValueParser> infer_parser(const DefaultValue &value) {
            const Value *resolved = value.value();
            if (resolved == nullptr) {
                return std::nullopt;
            }
            if (resolved->as_bool() != nullptr) {
                return parse_bool();
            }
            if (resolved->as_int() != nullptr) {
                return parse_int();
            }
            if (resolved->as_double() != nullptr) {
                return parse_float();
            }
            return std::nullopt;
        }

        /// 把默认值当成列表初值。
        std::vector<Value> default_list(const DefaultValue &value) {
            if (const Value *resolved = value.value()) {
                if (const auto *items = resolved->as_list()) {
                    return *items;
                }
            }
            return {};
        }

        /// 保持顺序去重后取错误文案里的指代。
        std::vector<std::string> unique_labels(const std::vector<const Argument *> &arguments) {
            std::set<std::string>    seen;
            std::vector<std::string> labels;
            for (const Argument *argument : arguments) {
                const std::string label = argument->error_name();
                if (!seen.insert(label).second) {
                    continue;
                }
                labels.push_back(label);
            }
            return labels;
        }

    }  // namespace

    /// 一次解析的共享状态: 命名空间、出现记录、参与的命令链与待报缺口。
    struct Parser::Run {
        explicit Run(bool tolerant_flag) : tolerant(tolerant_flag) {}

        bool                  tolerant = false;
        Namespace             namespace_value;
        std::set<std::string> present;
        /// 命令行上出现的 dest, 按出现顺序 (覆盖判定用)。
        std::vector<std::string>                         seen;
        std::vector<const Parser *>                      parsers;
        std::vector<const Argument *>                    missing;
        std::vector<std::pair<std::size_t, std::string>> extras;
        std::string                                      subcommand;
    };

    Argument &MutexGroup::add_argument(std::vector<std::string> names, Argument argument) {
        Argument &registered = parser_->add_argument(std::move(names), std::move(argument));
        arguments_.push_back(&registered);
        return registered;
    }

    Argument &ValueGroup::add_argument(std::vector<std::string> names, Argument argument) {
        Argument &registered = parser_->add_argument(std::move(names), std::move(argument));
        arguments_.push_back(&registered);
        return registered;
    }

    Argument &ArgumentGroup::add_argument(std::vector<std::string> names, Argument argument) {
        Argument &registered = parser_->add_argument(std::move(names), std::move(argument));
        registered.group     = title_;
        return registered;
    }

    Parser &SubParsers::add_parser(std::string              name,
                                   std::string              about,
                                   std::vector<std::string> aliases,
                                   std::vector<std::string> visible_aliases) {
        auto child   = std::make_unique<Parser>(parent_->prog_ + " " + name);
        child->name_ = name;
        if (!about.empty()) {
            child->about_ = std::move(about);
        }
        const std::string inherited = parent_->version();
        if (!inherited.empty()) {
            child->version_ = inherited;
            child->register_version();
        }
        child->allow_abbrev_ = parent_->allow_abbrev_;
        child->color_        = parent_->color_;
        child->max_width_    = parent_->max_width_;
        child->parent_       = parent_;
        std::vector<std::string> keys{name};
        keys.insert(keys.end(), aliases.begin(), aliases.end());
        for (const std::string &alias : visible_aliases) {
            keys.push_back(alias);
        }
        for (const std::string &key : keys) {
            if (lookup_.count(key) != 0) {
                throw std::invalid_argument("子命令名重复登记: " + key);
            }
        }
        const std::size_t index = commands_.size();
        for (const std::string &key : keys) {
            lookup_[key] = index;
        }
        visible_[name] = visible_aliases;
        commands_.push_back(std::move(child));
        return *commands_.back();
    }

    Parser *SubParsers::lookup(std::string_view name) const {
        const auto found = lookup_.find(std::string(name));
        if (found == lookup_.end()) {
            return nullptr;
        }
        return commands_[found->second].get();
    }

    bool SubParsers::has_command(std::string_view name) const {
        return lookup_.count(std::string(name)) != 0;
    }

    std::string SubParsers::canonical(std::string_view name) const {
        if (Parser *command = lookup(name)) {
            if (!command->name_.empty()) {
                return command->name_;
            }
        }
        return std::string(name);
    }

    std::vector<Command> SubParsers::commands() const {
        std::vector<Command> listed;
        listed.reserve(commands_.size());
        for (const auto &command : commands_) {
            Command item;
            item.name          = command->name_;
            const auto aliases = visible_.find(command->name_);
            if (aliases != visible_.end()) {
                item.aliases = aliases->second;
            }
            item.parser = command.get();
            listed.push_back(std::move(item));
        }
        return listed;
    }

    Parser::Parser(std::string prog) : prog_(std::move(prog)) {
        Argument help_argument;
        help_argument.names         = {"-h", "--help"};
        help_argument.dest          = "help";
        help_argument.action        = Action::HELP;
        help_argument.nargs         = Nargs::fixed(0);
        help_argument.default_value = DefaultValue::suppress();
        help_argument.help          = "Print help";
        push(std::move(help_argument));
    }

    Parser &Parser::with_about(std::string about) {
        about_ = std::move(about);
        return *this;
    }

    Parser &Parser::with_long_about(std::string about) {
        long_about_ = std::move(about);
        return *this;
    }

    Parser &Parser::with_version(std::string version) {
        version_ = std::move(version);
        register_version();
        return *this;
    }

    Parser &Parser::with_epilog(std::string epilog) {
        epilog_ = std::move(epilog);
        return *this;
    }

    Parser &Parser::with_usage(std::string usage) {
        usage_ = std::move(usage);
        return *this;
    }

    Parser &Parser::without_abbrev() {
        allow_abbrev_ = false;
        return *this;
    }

    Parser &Parser::without_help() {
        std::size_t index = arguments_.size();
        for (std::size_t offset = 0; offset < arguments_.size(); ++offset) {
            if (arguments_[offset]->action == Action::HELP) {
                index = offset;
                break;
            }
        }
        if (index == arguments_.size()) {
            return *this;
        }
        const Argument *victim = arguments_[index].get();
        dests_.erase(victim->dest);
        for (const std::string &name : victim->names) {
            by_name_.erase(name);
            name_order_.erase(std::remove(name_order_.begin(), name_order_.end(), name), name_order_.end());
        }
        arguments_.erase(arguments_.begin() + static_cast<std::ptrdiff_t>(index));
        for (auto &entry : by_name_) {
            if (entry.second > index) {
                --entry.second;
            }
        }
        for (auto &entry : dests_) {
            if (entry.second > index) {
                --entry.second;
            }
        }
        return *this;
    }

    Parser &Parser::with_argument_default(DefaultValue value) {
        argument_default_ = std::move(value);
        return *this;
    }

    Parser &Parser::with_color(ColorChoice color) {
        color_ = color;
        return *this;
    }

    Parser &Parser::with_max_width(std::uint32_t width) {
        max_width_ = width;
        return *this;
    }

    Parser &Parser::with_subcommand_negates_reqs(bool flag) {
        subcommand_negates_reqs_ = flag;
        return *this;
    }

    Parser &Parser::with_auto_width() {
        auto_width_ = true;
        return *this;
    }

    std::uint32_t Parser::width() const {
        if (auto_width_) {
            if (const std::optional<std::string> raw = get_env("COLUMNS")) {
                std::string       text  = *raw;
                const std::size_t begin = text.find_first_not_of(" \t");
                const std::size_t end   = text.find_last_not_of(" \t");
                text              = begin == std::string::npos ? std::string() : text.substr(begin, end - begin + 1);
                const bool digits = !text.empty() && std::all_of(text.begin(), text.end(), [](char item) {
                    return item >= '0' && item <= '9';
                });
                if (digits) {
                    const unsigned long parsed = std::stoul(text);
                    if (parsed > 0 && parsed <= 0xFFFFFFFFul) {
                        return static_cast<std::uint32_t>(parsed);
                    }
                }
            }
        }
        return max_width_;
    }

    Parser &Parser::with_from_file_prefix_chars(std::string chars) {
        fromfile_prefix_chars_ = chars.empty() ? std::nullopt : std::optional<std::string>(std::move(chars));
        return *this;
    }

    void Parser::register_version() {
        if (!version_ || dests_.count("version") != 0) {
            return;
        }
        Argument version_argument;
        version_argument.names         = {"-V", "--version"};
        version_argument.dest          = "version";
        version_argument.action        = Action::VERSION;
        version_argument.nargs         = Nargs::fixed(0);
        version_argument.default_value = DefaultValue::suppress();
        version_argument.help          = "Print version";
        push(std::move(version_argument));
    }

    void Parser::push(Argument argument) {
        auto      storage    = std::make_unique<Argument>(std::move(argument));
        Argument *registered = storage.get();
        arguments_.push_back(std::move(storage));
        const std::size_t index      = arguments_.size() - 1;
        dests_[registered->dest]     = index;
        std::vector<std::string> all = registered->names;
        all.insert(all.end(), registered->alias.begin(), registered->alias.end());
        all.insert(all.end(), registered->visible_alias.begin(), registered->visible_alias.end());
        for (const std::string &name : all) {
            by_name_[name] = index;
            name_order_.push_back(name);
        }
        if (registered->is_positional()) {
            positionals_.push_back(registered);
        }
    }

    Argument &Parser::add_argument(std::vector<std::string> names, Argument argument) {
        if (names.empty()) {
            throw std::invalid_argument("至少需要一个参数名");
        }
        const bool positional = !has_prefix(names.front(), "-");
        if (positional) {
            if (names.size() > 1) {
                throw std::invalid_argument("位置参数只能有一个名字");
            }
            for (const Argument *existing : positionals_) {
                if (existing->nargs.is_remainder()) {
                    throw std::invalid_argument("REMAINDER 位置参数必须是最后一个, 其后不能再加位置参数");
                }
            }
            if (argument.required) {
                throw std::invalid_argument("位置参数的必填性由 nargs 决定, 不接受 required");
            }
        }
        std::vector<std::string> option_names;
        if (!positional) {
            option_names = names;
        }
        std::vector<std::string> all = option_names;
        all.insert(all.end(), argument.alias.begin(), argument.alias.end());
        all.insert(all.end(), argument.visible_alias.begin(), argument.visible_alias.end());
        for (const std::string &name : all) {
            if (!has_prefix(name, "-")) {
                throw std::invalid_argument("选项名必须以 '-' 开头: " + name);
            }
            if (is_signed_number(name)) {
                throw std::invalid_argument("签名数字不能作选项名, 会被整体当成取值: " + name);
            }
        }

        std::string dest = argument.dest;
        if (dest.empty()) {
            if (positional) {
                dest = names.front();
                std::replace(dest.begin(), dest.end(), '-', '_');
            } else {
                dest = dest_of_option(option_names);
            }
        }
        Action       action        = argument.action;
        Nargs        nargs         = argument.nargs;
        DefaultValue const_value   = argument.const_value;
        DefaultValue default_value = argument.default_value;
        if (!action_takes_values(action)) {
            const bool zero_or_one = nargs.kind() == Nargs::Kind::ONE ||
                                     (nargs.kind() == Nargs::Kind::FIXED && nargs.count() == 0);
            if (!zero_or_one) {
                throw std::invalid_argument("action=" + std::string(action_name(action)) +
                                            " 不吃取值, 不能配 nargs=" + nargs.kind_name());
            }
            nargs = Nargs::fixed(0);
            switch (action) {
                case Action::STORE_TRUE:
                case Action::STORE_FALSE: {
                    if (const_value.is_missing()) {
                        const_value = DefaultValue::of(Value(action == Action::STORE_TRUE));
                    }
                    if (default_value.is_missing()) {
                        const bool *flag = const_value.value()->as_bool();
                        default_value    = DefaultValue::of(Value(!(flag != nullptr && *flag)));
                    }
                    break;
                }
                case Action::COUNT: {
                    if (default_value.is_missing()) {
                        default_value = DefaultValue::of(Value(std::int64_t{0}));
                    }
                    break;
                }
                case Action::STORE_CONST:
                case Action::APPEND_CONST: {
                    if (const_value.is_missing()) {
                        throw std::invalid_argument("action=" + std::string(action_name(action)) + " 必须给 const");
                    }
                    break;
                }
                default:
                    break;
            }
        } else if (argument.value_delimiter && nargs.kind() == Nargs::Kind::ONE) {
            nargs = Nargs::fixed(1);
        }

        if (!argument.type && action_takes_values(action) && !default_value.is_missing()) {
            argument.type = infer_parser(default_value);
        }
        for (const std::string &name : all) {
            if (by_name_.count(name) != 0) {
                throw std::invalid_argument("选项名重复登记: " + name);
            }
        }
        if (dests_.count(dest) != 0) {
            throw std::invalid_argument("dest 重复登记: " + dest);
        }

        argument.names         = option_names;
        argument.dest          = dest;
        argument.action        = action;
        argument.nargs         = nargs;
        argument.default_value = default_value;
        argument.const_value   = const_value;
        push(std::move(argument));
        return *arguments_.back();
    }

    ArgumentGroup &Parser::add_group(std::string title, std::optional<std::string> description) {
        groups_.push_back(GroupInfo{title, description});
        auto group          = std::make_unique<ArgumentGroup>();
        group->parser_      = this;
        group->title_       = std::move(title);
        group->description_ = std::move(description);
        group_objects_.push_back(std::move(group));
        return *group_objects_.back();
    }

    MutexGroup &Parser::add_mutually_exclusive_group(bool required) {
        auto group       = std::make_unique<MutexGroup>();
        group->parser_   = this;
        group->required_ = required;
        mutex_.push_back(std::move(group));
        return *mutex_.back();
    }

    ValueGroup &Parser::add_value_group(std::string name, bool required) {
        auto group       = std::make_unique<ValueGroup>();
        group->parser_   = this;
        group->name_     = std::move(name);
        group->required_ = required;
        value_groups_.push_back(std::move(group));
        return *value_groups_.back();
    }

    SubParsers &Parser::add_subparsers(std::string dest, std::string title, bool required) {
        if (subparsers_) {
            throw std::invalid_argument("子命令集合只能登记一次");
        }
        auto subparsers     = std::make_unique<SubParsers>();
        subparsers->parent_ = this;
        subparsers->dest_   = std::move(dest);
        if (!title.empty()) {
            subparsers->title_ = std::move(title);
        }
        subparsers->required_ = required;
        subparsers_           = std::move(subparsers);
        return *subparsers_;
    }

    std::string Parser::version() const {
        for (const Parser *command = this; command != nullptr; command = command->parent_) {
            if (command->version_) {
                return *command->version_;
            }
        }
        return {};
    }

    std::string Parser::version_text() const {
        const std::string inherited = version();
        return inherited.empty() ? prog_ : prog_ + " " + inherited;
    }

    std::vector<const Argument *> Parser::arguments() const {
        std::vector<const Argument *> listed;
        listed.reserve(arguments_.size());
        for (const auto &argument : arguments_) {
            listed.push_back(argument.get());
        }
        return listed;
    }

    std::vector<const Argument *> Parser::positionals() const {
        std::vector<const Argument *> listed;
        listed.reserve(positionals_.size());
        for (const Argument *argument : positionals_) {
            listed.push_back(argument);
        }
        return listed;
    }

    bool Parser::color_enabled() const {
        switch (color_) {
            case ColorChoice::ALWAYS:
                return true;
            case ColorChoice::NEVER:
                return false;
            case ColorChoice::AUTO:
                break;
        }
        if (get_env("NO_COLOR")) {
            return false;
        }
#if defined(_WIN32)
        return _isatty(_fileno(stdout)) != 0;
#else
        return isatty(STDOUT_FILENO) != 0;
#endif
    }

    bool Parser::is_option_token(std::string_view token) {
        if (token == "--") {
            return true;
        }
        if (token.empty() || token.front() != '-' || token == "-") {
            return false;
        }
        return !is_signed_number(token);
    }

    std::string Parser::format_help(bool long_help) const {
        return render_help(*this, long_help);
    }

    std::string Parser::format_usage() const {
        return render_usage(*this);
    }

    void Parser::print_help(std::ostream &writer, bool long_help) const {
        writer << format_help(long_help) << "\n";
    }

    const Parser *Parser::parser_by_prog(std::string_view prog) const {
        std::vector<const Parser *> pending{this};
        while (!pending.empty()) {
            const Parser *command = pending.back();
            pending.pop_back();
            if (command->prog_ == prog) {
                return command;
            }
            if (!command->subparsers_) {
                continue;
            }
            const auto &children = command->subparsers_->commands_;
            // 反序压栈, 弹出时按登记顺序下沉
            for (const auto &child : std::views::reverse(children)) {
                pending.push_back(child.get());
            }
        }
        return nullptr;
    }

    std::variant<Namespace, int>
    Parser::parse_or_exit(std::vector<std::string> argv, std::ostream &out, std::ostream &err) const {
        std::variant<Namespace, ParseError> outcome = parse(std::move(argv));
        if (auto *namespace_value = std::get_if<Namespace>(&outcome)) {
            return std::move(*namespace_value);
        }
        ParseError error = std::move(std::get<ParseError>(outcome));
        if (auto *requested = std::get_if<HelpRequested>(&error)) {
            const Parser *target = parser_by_prog(requested->prog);
            (target != nullptr ? target : this)->print_help(out, requested->long_form);
            return 0;
        }
        if (auto *requested = std::get_if<VersionRequested>(&error)) {
            out << requested->text << "\n";
            return 0;
        }
        if (auto *usage = std::get_if<UsageError>(&error)) {
            err << usage->render() << "\n";
            return 2;
        }
        const auto *definition = std::get_if<DefinitionError>(&error);
        throw std::runtime_error(definition != nullptr ? definition->message : "解析失败");
    }

    std::string Parser::usage_spec() const {
        if (usage_) {
            return *usage_;
        }
        std::vector<std::string> parts{prog_};
        for (const auto &argument : arguments_) {
            if (argument->required && !argument->hide) {
                parts.push_back(argument->error_name());
            }
        }
        const bool has_optional = std::any_of(arguments_.begin(), arguments_.end(), [](const auto &item) {
            return !item->hide && !item->required && !item->is_positional() && item->action != Action::HELP &&
                   item->action != Action::VERSION;
        });
        if (has_optional) {
            parts.emplace_back("[OPTIONS]");
        }
        for (const Argument *argument : positionals_) {
            if (!argument->hide) {
                parts.push_back(argument->usage_spec());
            }
        }
        if (subparsers_) {
            parts.emplace_back(subparsers_->required_ ? "<COMMAND>" : "[COMMAND]");
        }
        std::string joined;
        for (std::size_t index = 0; index < parts.size(); ++index) {
            if (index != 0) {
                joined += ' ';
            }
            joined += parts[index];
        }
        return joined;
    }

    std::variant<Namespace, ParseError> Parser::parse(std::vector<std::string> argv) const {
        auto outcome = parse_run(std::move(argv), false);
        if (auto *error = std::get_if<ParseError>(&outcome)) {
            return *error;
        }
        KnownOutcome known = std::get<KnownOutcome>(std::move(outcome));
        if (!known.extras.empty()) {
            return ParseError(
                make_error("unexpected argument '" + known.extras.front() + "' found", ErrorKind::UNKNOWN_ARGUMENT));
        }
        return std::move(known.namespace_value);
    }

    std::variant<Parser::KnownOutcome, ParseError> Parser::parse_known(std::vector<std::string> argv) const {
        return parse_run(std::move(argv), true);
    }

    std::variant<Parser::KnownOutcome, ParseError> Parser::parse_run(std::vector<std::string> argv,
                                                                     bool                     tolerant) const {
        Run run(tolerant);
        try {
            std::vector<std::string> tokens = expand_files(argv);
            parse_tokens(std::move(tokens), run, {}, 0);
            finish(run);
        } catch (const Failure &failure) {
            return failure.error;
        }
        std::stable_sort(run.extras.begin(), run.extras.end(), [](const auto &left, const auto &right) {
            return left.first < right.first;
        });
        KnownOutcome outcome;
        outcome.extras.reserve(run.extras.size());
        for (const auto &[slot, token] : run.extras) {
            outcome.extras.push_back(token);
        }
        outcome.namespace_value = std::move(run.namespace_value);
        return outcome;
    }

    std::vector<std::string> Parser::expand_files(const std::vector<std::string> &tokens) const {
        if (!fromfile_prefix_chars_ || fromfile_prefix_chars_->empty()) {
            return tokens;
        }
        // 反序压栈, 弹出时按源 token 顺序展开; 嵌套层数随条目一起走
        std::vector<std::pair<std::string, std::size_t>> pending;
        pending.reserve(tokens.size());
        for (const auto &token : std::views::reverse(tokens)) {
            pending.emplace_back(token, 0);
        }
        std::vector<std::string> expanded;
        expanded.reserve(tokens.size());
        while (!pending.empty()) {
            auto [token, depth] = std::move(pending.back());
            pending.pop_back();
            if (token.empty() || fromfile_prefix_chars_->find(token.front()) == std::string::npos) {
                expanded.push_back(std::move(token));
                continue;
            }
            const std::string path = token.substr(1);
            if (depth >= kMaxIncludeDepth) {
                throw Failure{ParseError(make_error("@file nesting too deep: '" + path + "'", ErrorKind::OTHER))};
            }
            std::ifstream stream(path, std::ios::binary);
            if (!stream) {
                throw Failure{ParseError(make_error("unable to open file '" + path + "'", ErrorKind::OTHER))};
            }
            const std::string body((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            const std::vector<std::string> lines = split_lines(body);
            for (const auto &line : std::views::reverse(lines)) {
                pending.emplace_back(line, depth + 1);
            }
        }
        return expanded;
    }

    void Parser::parse_tokens(std::vector<std::string>    tokens,
                              Run                        &run,
                              std::vector<const Parser *> chain,
                              std::size_t                 base) const {
        // 每层攒下的位置参数; 命中子命令后带着剩余 token 下沉, 收尾时由深到浅分配
        struct Layer {
            const Parser                                    *parser = nullptr;
            std::vector<std::pair<std::size_t, std::string>> pending;
        };
        std::vector<Layer> layers;
        const Parser      *current = this;
        while (true) {
            run.parsers.push_back(current);
            layers.push_back(Layer{current, {}});
            auto         &pending = layers.back().pending;
            std::size_t   index   = 0;
            const Parser *child   = nullptr;
            while (index < tokens.size()) {
                const std::string token = tokens[index];
                if (token == "--") {
                    for (std::size_t offset = index + 1; offset < tokens.size(); ++offset) {
                        pending.emplace_back(base + offset, tokens[offset]);
                    }
                    break;
                }
                if (is_option_token(token)) {
                    if (auto expanded = current->expand_cluster(token, chain)) {
                        tokens.erase(tokens.begin() + static_cast<std::ptrdiff_t>(index));
                        tokens.insert(
                            tokens.begin() + static_cast<std::ptrdiff_t>(index), expanded->begin(), expanded->end());
                        continue;
                    }
                    index = current->consume_option(tokens, index, run, chain, base);
                    continue;
                }
                if (current->remainder_ready(pending.size())) {
                    for (std::size_t offset = index; offset < tokens.size(); ++offset) {
                        pending.emplace_back(base + offset, tokens[offset]);
                    }
                    break;
                }
                if (current->help_subcommand_of(token, pending.size())) {
                    std::vector<std::string> rest(tokens.begin() + static_cast<std::ptrdiff_t>(index) + 1,
                                                  tokens.end());
                    const Parser            *target = current->help_target(rest);
                    throw Failure{ParseError(HelpRequested{true, target->prog_})};
                }
                if (const Parser *found = current->subcommand_of(token, pending.size())) {
                    run.subcommand = token;
                    if (current->subparsers_ && !current->subparsers_->dest_.empty()) {
                        run.namespace_value.set(current->subparsers_->dest_,
                                                Value(current->subparsers_->canonical(token)));
                    }
                    child = found;
                    break;
                }
                pending.emplace_back(base + index, token);
                ++index;
            }
            if (child == nullptr) {
                break;
            }
            chain.push_back(current);
            std::vector<std::string> rest(tokens.begin() + static_cast<std::ptrdiff_t>(index) + 1, tokens.end());
            base    = base + index + 1;
            tokens  = std::move(rest);
            current = child;
        }
        for (auto &layer : std::views::reverse(layers)) {
            layer.parser->assign_positionals(layer.pending, run);
        }
    }

    std::optional<std::vector<std::string>> Parser::expand_cluster(std::string_view                   token,
                                                                   const std::vector<const Parser *> &chain) const {
        if (has_prefix(token, "--") || token.size() <= 2 || token.find('=') != std::string_view::npos) {
            return std::nullopt;
        }
        const Argument *first = lookup(std::string("-") + std::string(1, token[1]), chain);
        if (first == nullptr || first->takes_values()) {
            return std::nullopt;
        }
        std::vector<std::string> expanded;
        std::size_t              index = 1;
        while (index < token.size()) {
            const std::string piece    = std::string("-") + std::string(1, token[index]);
            const Argument   *argument = lookup(piece, chain);
            if (argument == nullptr) {
                // 未认出的尾巴连短横线一起留下, 由后续识别失败报出具体的短选项
                expanded.push_back(std::string("-") + std::string(token.substr(index)));
                break;
            }
            if (argument->takes_values()) {
                std::string rest(token.substr(index + 1));
                if (!rest.empty() && rest.front() == '=') {
                    rest.erase(rest.begin());
                }
                if (rest.empty()) {
                    expanded.push_back(piece);
                } else {
                    std::string joined = piece;
                    joined += '=';
                    joined += rest;
                    expanded.push_back(std::move(joined));
                }
                break;
            }
            expanded.push_back(piece);
            ++index;
        }
        return expanded;
    }

    std::size_t Parser::positional_minimum() const {
        std::int64_t needed = 0;
        for (const Argument *argument : positionals_) {
            needed += argument->min_count();
        }
        return static_cast<std::size_t>(needed);
    }

    const Parser *Parser::subcommand_of(std::string_view token, std::size_t pending) const {
        if (!subparsers_) {
            return nullptr;
        }
        const Parser *child = subparsers_->lookup(token);
        if (child == nullptr) {
            return nullptr;
        }
        return pending >= positional_minimum() ? child : nullptr;
    }

    bool Parser::help_subcommand_of(std::string_view token, std::size_t pending) const {
        if (token != "help" || !subparsers_ || subparsers_->has_command("help")) {
            return false;
        }
        return pending >= positional_minimum();
    }

    const Parser *Parser::help_target(const std::vector<std::string> &rest) const {
        const Parser *target = this;
        for (const std::string &token : rest) {
            const Parser *child = target->subparsers_ ? target->subparsers_->lookup(token) : nullptr;
            if (child == nullptr) {
                throw Failure{ParseError(
                    target->make_error("unrecognized subcommand '" + token + "'", ErrorKind::UNKNOWN_ARGUMENT))};
            }
            target = child;
        }
        return target;
    }

    bool Parser::remainder_ready(std::size_t pending) const {
        for (std::size_t index = 0; index < positionals_.size(); ++index) {
            const Argument *argument = positionals_[index];
            if (!argument->nargs.is_remainder()) {
                continue;
            }
            std::int64_t needed = 0;
            for (std::size_t before = 0; before < index; ++before) {
                needed += positionals_[before]->min_count();
            }
            return pending >= static_cast<std::size_t>(needed);
        }
        return false;
    }

    const Argument *Parser::lookup(std::string_view name, const std::vector<const Parser *> &chain) const {
        const auto direct = by_name_.find(std::string(name));
        if (direct != by_name_.end()) {
            return arguments_[direct->second].get();
        }
        if (allow_abbrev_ && has_prefix(name, "--") && name.size() > 2) {
            if (const Argument *argument = abbreviate(name)) {
                return argument;
            }
        }
        for (auto parent : std::views::reverse(chain)) {
            const auto found = parent->by_name_.find(std::string(name));
            if (found != parent->by_name_.end()) {
                const Argument *argument = parent->arguments_[found->second].get();
                if (argument->global) {
                    return argument;
                }
            }
        }
        return nullptr;
    }

    const Argument *Parser::abbreviate(std::string_view name) const {
        std::vector<const Argument *> matched;
        std::set<std::string>         dests;
        for (const std::string &option : name_order_) {
            if (!has_prefix(option, "--") || !has_prefix(option, name)) {
                continue;
            }
            const Argument *argument = arguments_[by_name_.at(option)].get();
            if (dests.insert(argument->dest).second) {
                matched.push_back(argument);
            }
        }
        if (matched.empty()) {
            return nullptr;
        }
        if (matched.size() > 1) {
            std::set<std::string> options;
            for (const std::string &option : name_order_) {
                if (has_prefix(option, name)) {
                    options.insert(option);
                }
            }
            UsageError                     error = make_error("ambiguous argument '" + std::string(name) + "'",
                                                              ErrorKind::AMBIGUOUS_ARGUMENT);
            const std::vector<std::string> listed(options.begin(), options.end());
            error.add_line("could be one of: " + format_names(listed));
            throw Failure{ParseError(std::move(error))};
        }
        return matched.front();
    }

    void Parser::throw_unknown_argument(std::string_view name, const std::vector<const Parser *> &chain) const {
        std::vector<std::string> candidates = name_order_;
        for (const Parser *parent : chain) {
            for (const std::string &option : parent->name_order_) {
                if (parent->arguments_[parent->by_name_.at(option)]->global) {
                    candidates.push_back(option);
                }
            }
        }
        UsageError error = make_error("unexpected argument '" + std::string(name) + "' found",
                                      ErrorKind::UNKNOWN_ARGUMENT);
        if (const auto suggestion = best_match(name, candidates)) {
            error.add_tip("a similar argument exists: '" + *suggestion + "'");
        } else if (has_prefix(name, "--")) {
            error.add_tip("to pass '" + std::string(name) + "' as a value, use '-- " + std::string(name) + "'");
        }
        throw Failure{ParseError(std::move(error))};
    }

    std::size_t Parser::consume_option(std::vector<std::string>          &tokens,
                                       std::size_t                        index,
                                       Run                               &run,
                                       const std::vector<const Parser *> &chain,
                                       std::size_t                        base) const {
        const std::string &token  = tokens[index];
        auto [name, inline_value] = split_option(token);
        const Argument *argument  = lookup(name, chain);
        if (argument == nullptr) {
            const auto attached = attached_short(name, chain);
            if (!attached) {
                if (run.tolerant) {
                    run.extras.emplace_back(base + index, token);
                    return index + 1;
                }
                throw_unknown_argument(name, chain);
            }
            argument     = attached->first;
            inline_value = attached->second;
        }
        if (!argument->takes_values()) {
            if (inline_value) {
                throw Failure{ParseError(make_error("unexpected value '" + *inline_value + "' for '" +
                                                        argument->error_name() + "' found; no more were expected",
                                                    ErrorKind::INVALID_VALUE))};
            }
            // 长短帮助靠命中的名字区分 (`-h` 走短帮助, `--help` 走长帮助)
            if (argument->action == Action::HELP) {
                throw Failure{ParseError(HelpRequested{has_prefix(name, "--"), prog_})};
            }
            if (argument->action == Action::VERSION) {
                throw Failure{ParseError(VersionRequested{version_text()})};
            }
            apply(*argument, {}, run);
            return index + 1;
        }
        if (inline_value) {
            apply(*argument, convert_texts(*argument, {*inline_value}), run);
            return index + 1;
        }
        auto [stop, raw] = take_values(*argument, tokens, index + 1);
        apply(*argument, convert_texts(*argument, raw), run);
        return stop;
    }

    std::optional<std::pair<const Argument *, std::string>>
    Parser::attached_short(std::string_view name, const std::vector<const Parser *> &chain) const {
        if (has_prefix(name, "--") || name.size() <= 2) {
            return std::nullopt;
        }
        const Argument *argument = lookup(name.substr(0, 2), chain);
        if (argument == nullptr || !argument->takes_values()) {
            return std::nullopt;
        }
        std::string value(name.substr(2));
        if (!value.empty() && value.front() == '=') {
            value.erase(value.begin());
        }
        return std::make_pair(argument, value);
    }

    std::pair<std::size_t, std::vector<std::string>>
    Parser::take_values(const Argument &argument, const std::vector<std::string> &tokens, std::size_t start) const {
        if (argument.nargs.is_remainder()) {
            return {tokens.size(),
                    std::vector<std::string>(tokens.begin() + static_cast<std::ptrdiff_t>(start), tokens.end())};
        }
        const std::int64_t       minimum = argument.min_count();
        const std::int64_t       maximum = argument.max_count();
        std::vector<std::string> taken;
        std::size_t              index = start;
        while (index < tokens.size()) {
            if (maximum >= 0 && static_cast<std::int64_t>(taken.size()) >= maximum) {
                break;
            }
            if (is_option_token(tokens[index]) && !argument.allow_hyphen_values) {
                break;
            }
            taken.push_back(tokens[index]);
            ++index;
        }
        if (static_cast<std::int64_t>(taken.size()) < minimum) {
            throw_missing_value(argument, minimum, static_cast<std::int64_t>(taken.size()));
        }
        return {index, taken};
    }

    void Parser::throw_missing_value(const Argument &argument, std::int64_t needed, std::int64_t given) const {
        const std::string name = argument.error_name();
        if (needed <= 1) {
            throw Failure{ParseError(
                make_error("a value is required for '" + name + "' but none was supplied", ErrorKind::MISSING_VALUE))};
        }
        const std::string verb = given == 1 ? "was" : "were";
        throw Failure{ParseError(make_error(std::to_string(needed) + " values required for '" + name + "' but " +
                                                std::to_string(given) + " " + verb + " provided",
                                            ErrorKind::MISSING_VALUE))};
    }

    std::vector<Value> Parser::convert_texts(const Argument &argument, const std::vector<std::string> &raw) const {
        std::vector<std::string> texts;
        for (const std::string &text : raw) {
            if (!argument.value_delimiter) {
                texts.push_back(text);
                continue;
            }
            const std::string &delimiter = *argument.value_delimiter;
            std::size_t        cursor    = 0;
            while (true) {
                const auto position = text.find(delimiter, cursor);
                if (position == std::string::npos) {
                    texts.push_back(text.substr(cursor));
                    break;
                }
                texts.push_back(text.substr(cursor, position - cursor));
                cursor = position + delimiter.size();
            }
        }
        std::vector<Value> values;
        values.reserve(texts.size());
        for (const std::string &text : texts) {
            values.push_back(convert(argument, text));
        }
        return values;
    }

    Value Parser::convert(const Argument &argument, std::string_view text) const {
        Value value{std::string(text)};
        if (argument.type) {
            try {
                value = (*argument.type)(text);
            } catch (const std::invalid_argument &invalid) {
                UsageError error = make_error("invalid value '" + std::string(text) + "' for '" +
                                                  argument.error_name() + "'",
                                              ErrorKind::INVALID_VALUE);
                error.set_reason(invalid.what());
                throw Failure{ParseError(std::move(error))};
            }
        }
        if (!argument.choices) {
            return value;
        }
        const std::vector<Value> choices = argument.choice_values();
        const bool               matched = std::any_of(
            choices.begin(), choices.end(), [&value](const Value &item) { return item.text() == value.text(); });
        if (matched) {
            return value;
        }
        UsageError error = make_error("invalid value '" + std::string(text) + "' for '" + argument.error_name() + "'",
                                      ErrorKind::INVALID_VALUE);
        error.add_line("[possible values: " + format_items(choices) + "]");
        std::vector<std::string> labels;
        labels.reserve(choices.size());
        for (const Value &item : choices) {
            labels.push_back(item.text());
        }
        if (const auto suggestion = best_match(text, labels)) {
            error.add_tip("a similar value exists: '" + *suggestion + "'");
        }
        throw Failure{ParseError(std::move(error))};
    }

    void Parser::apply(const Argument &argument, std::vector<Value> values, Run &run) const {
        if (argument.action == Action::HELP) {
            throw Failure{ParseError(HelpRequested{argument.primary_name() == "--help", prog_})};
        }
        if (argument.action == Action::VERSION) {
            throw Failure{ParseError(VersionRequested{version_text()})};
        }
        run.present.insert(argument.dest);
        run.seen.push_back(argument.dest);
        switch (argument.action) {
            case Action::COUNT: {
                std::int64_t count = 0;
                if (const Value *current = run.namespace_value.find(argument.dest)) {
                    if (const auto *number = current->as_int()) {
                        count = *number;
                    }
                }
                run.namespace_value.set(argument.dest, Value(count + 1));
                return;
            }
            case Action::STORE_TRUE:
            case Action::STORE_FALSE:
            case Action::STORE_CONST: {
                run.namespace_value.set(argument.dest, *argument.const_value.value());
                return;
            }
            case Action::APPEND_CONST: {
                append_value(run, argument.dest, *argument.const_value.value());
                return;
            }
            case Action::EXTEND: {
                std::vector<Value> items;
                if (const Value *current = run.namespace_value.find(argument.dest)) {
                    if (const auto *existing = current->as_list()) {
                        items = *existing;
                    }
                }
                if (items.empty()) {
                    items = default_list(argument.default_value);
                }
                items.insert(items.end(), values.begin(), values.end());
                run.namespace_value.set(argument.dest, Value::list(std::move(items)));
                return;
            }
            case Action::APPEND: {
                if (argument.collects()) {
                    append_value(run, argument.dest, Value::list(std::move(values)));
                    return;
                }
                append_value(run, argument.dest, values.empty() ? Value() : values.front());
                return;
            }
            default:
                break;
        }
        if (values.empty()) {
            run.namespace_value.set(argument.dest,
                                    argument.const_value.value() ? *argument.const_value.value() : Value());
            return;
        }
        if (argument.collects()) {
            run.namespace_value.set(argument.dest, Value::list(std::move(values)));
            return;
        }
        run.namespace_value.set(argument.dest, values.front());
    }

    void Parser::append_value(Run &run, const std::string &dest, Value item) {
        std::vector<Value> items;
        if (const Value *current = run.namespace_value.find(dest)) {
            if (const auto *existing = current->as_list()) {
                items = *existing;
            }
        }
        items.push_back(std::move(item));
        run.namespace_value.set(dest, Value::list(std::move(items)));
    }

    void Parser::assign_positionals(const std::vector<std::pair<std::size_t, std::string>> &pending, Run &run) const {
        std::vector<std::string> tokens;
        tokens.reserve(pending.size());
        for (const auto &[slot, token] : pending) {
            tokens.push_back(token);
        }
        const std::size_t total  = tokens.size();
        std::size_t       cursor = 0;
        for (std::size_t index = 0; index < positionals_.size(); ++index) {
            const Argument *argument = positionals_[index];
            std::int64_t    tail     = 0;
            for (std::size_t after = index + 1; after < positionals_.size(); ++after) {
                tail += positionals_[after]->min_count();
            }
            const std::int64_t available = std::max<std::int64_t>(0, static_cast<std::int64_t>(total - cursor) - tail);
            std::int64_t       take      = 0;
            if (argument->nargs.is_remainder()) {
                take = static_cast<std::int64_t>(total - cursor);
            } else if (argument->max_count() < 0) {
                take = available;
            } else {
                take = std::min(argument->max_count(), available);
            }
            if (take < argument->min_count()) {
                run.missing.push_back(argument);
                continue;
            }
            std::vector<std::string> chunk(tokens.begin() + static_cast<std::ptrdiff_t>(cursor),
                                           tokens.begin() + static_cast<std::ptrdiff_t>(cursor) +
                                               static_cast<std::ptrdiff_t>(take));
            cursor += static_cast<std::size_t>(take);
            store_positional(*argument, chunk, run);
        }
        for (std::size_t index = cursor; index < total; ++index) {
            run.extras.push_back(pending[index]);
        }
    }

    void Parser::store_positional(const Argument &argument, const std::vector<std::string> &chunk, Run &run) const {
        if (chunk.empty()) {
            if (argument.const_value.value() != nullptr) {
                run.namespace_value.set(argument.dest, *argument.const_value.value());
            } else if (argument.default_value.value() != nullptr) {
                run.namespace_value.set(argument.dest, *argument.default_value.value());
            } else if (argument.nargs.kind() == Nargs::Kind::ZERO_OR_MORE || argument.nargs.is_remainder()) {
                run.namespace_value.set(argument.dest, Value::list({}));
            } else {
                run.namespace_value.set(argument.dest, Value());
            }
            return;
        }
        run.present.insert(argument.dest);
        run.seen.push_back(argument.dest);
        std::vector<Value> values = convert_texts(argument, chunk);
        if (argument.collects()) {
            run.namespace_value.set(argument.dest, Value::list(std::move(values)));
            return;
        }
        run.namespace_value.set(argument.dest, values.front());
    }

    void Parser::finish(Run &run) {
        apply_overrides(run);
        apply_env(run);
        validate(run);
        apply_defaults(run);
    }

    void Parser::apply_overrides(Run &run) {
        std::map<std::string, const Argument *> registry;
        std::map<std::string, const Argument *> by_name;
        for (const Parser *parser : run.parsers) {
            for (const auto &argument : parser->arguments_) {
                // 帮助 / 版本只在解析途中返回控制流, 从不落进命名空间, 各层同名不冲突
                if (argument->action == Action::HELP || argument->action == Action::VERSION) {
                    continue;
                }
                registry[argument->dest] = argument.get();
            }
            for (const std::string &name : parser->name_order_) {
                by_name[name] = parser->arguments_[parser->by_name_.at(name)].get();
            }
        }
        const auto resolve = [&registry, &by_name](const std::string &ref) -> std::string {
            if (registry.count(ref) != 0) {
                return ref;
            }
            const auto found = by_name.find(ref);
            if (found == by_name.end()) {
                throw Failure{ParseError(
                    DefinitionError{"overrides_with / conflicts_with / requires 指向了未登记的参数: " + ref})};
            }
            return found->second->dest;
        };

        std::map<std::string, std::vector<std::string>> pairs;
        for (const auto &[dest, argument] : registry) {
            for (const std::string &ref : argument->overrides_with) {
                const std::string other = resolve(ref);
                pairs[dest].push_back(other);
                pairs[other].push_back(dest);
            }
        }
        if (pairs.empty()) {
            return;
        }

        std::set<std::string> kept;
        for (const std::string &dest : run.seen) {
            const auto found = pairs.find(dest);
            if (found != pairs.end()) {
                for (const std::string &other : found->second) {
                    kept.erase(other);
                }
            }
            kept.insert(dest);
        }
        for (const std::string &dest : run.seen) {
            if (kept.count(dest) != 0) {
                continue;
            }
            run.present.erase(dest);
            run.namespace_value.erase(dest);
        }
    }

    void Parser::apply_env(Run &run) {
        for (const Parser *parser : run.parsers) {
            for (const auto &argument : parser->arguments_) {
                if (!argument->env || run.present.count(argument->dest) != 0) {
                    continue;
                }
                const std::optional<std::string> raw = get_env(*argument->env);
                if (!raw) {
                    continue;
                }
                const std::string &text = *raw;
                if (argument->takes_values()) {
                    parser->apply(*argument, parser->convert_texts(*argument, {text}), run);
                    continue;
                }
                Value enabled;
                try {
                    enabled = parse_bool()(text);
                } catch (const std::invalid_argument &invalid) {
                    UsageError error = parser->make_error(
                        "invalid value '" + text + "' for '" + argument->error_name() + "'", ErrorKind::INVALID_VALUE);
                    error.set_reason(invalid.what());
                    throw Failure{ParseError(std::move(error))};
                }
                Value fallback(false);
                if (argument->default_value.value() != nullptr) {
                    fallback = *argument->default_value.value();
                }
                Value chosen;
                if (argument->const_value.value() != nullptr) {
                    chosen = *argument->const_value.value();
                }
                const bool *flag = enabled.as_bool();
                run.namespace_value.set(argument->dest, flag != nullptr && *flag ? chosen : fallback);
                run.present.insert(argument->dest);
            }
        }
    }

    void Parser::validate(Run &run) {
        const Parser                           *leaf = run.parsers.back();
        std::map<std::string, const Argument *> registry;
        std::vector<const Argument *>           registry_order;
        std::map<std::string, const Argument *> by_name;
        for (const Parser *parser : run.parsers) {
            for (const auto &argument : parser->arguments_) {
                // 帮助 / 版本只在解析途中返回控制流, 从不落进命名空间, 各层同名不冲突
                if (argument->action == Action::HELP || argument->action == Action::VERSION) {
                    continue;
                }
                if (registry.count(argument->dest) != 0) {
                    throw Failure{ParseError(
                        DefinitionError{"命令链上的 dest 重复: " + argument->dest + " (父命令与子命令不能共用 dest)"})};
                }
                registry[argument->dest] = argument.get();
                registry_order.push_back(argument.get());
            }
            for (const std::string &name : parser->name_order_) {
                by_name[name] = parser->arguments_[parser->by_name_.at(name)].get();
            }
        }
        const auto resolve = [&registry, &by_name](const std::string &ref) -> std::string {
            if (registry.count(ref) != 0) {
                return ref;
            }
            const auto found = by_name.find(ref);
            if (found == by_name.end()) {
                throw Failure{ParseError(
                    DefinitionError{"overrides_with / conflicts_with / requires 指向了未登记的参数: " + ref})};
            }
            return found->second->dest;
        };
        const auto label = [&registry](const std::string &dest) -> std::string {
            return registry.at(dest)->error_name();
        };

        for (const Argument *argument : registry_order) {
            if (run.present.count(argument->dest) == 0) {
                continue;
            }
            for (const std::string &ref : argument->conflicts_with) {
                const std::string dest = resolve(ref);
                if (run.present.count(dest) != 0) {
                    throw Failure{ParseError(leaf->make_error("the argument '" + argument->error_name() +
                                                                  "' cannot be used with '" + label(dest) + "'",
                                                              ErrorKind::CONFLICT))};
                }
            }
        }

        for (const Parser *parser : run.parsers) {
            for (const auto &group : parser->mutex_) {
                std::vector<const Argument *> hits;
                for (const Argument *argument : group->arguments_) {
                    if (run.present.count(argument->dest) != 0) {
                        hits.push_back(argument);
                    }
                }
                if (hits.size() > 1) {
                    throw Failure{
                        ParseError(leaf->make_error("the argument '" + hits[0]->error_name() +
                                                        "' cannot be used with '" + hits[1]->error_name() + "'",
                                                    ErrorKind::CONFLICT))};
                }
                if (hits.empty() && group->required_) {
                    UsageError error = leaf->make_error("one of the following arguments is required:",
                                                        ErrorKind::MISSING_REQUIRED);
                    for (const Argument *argument : group->arguments_) {
                        error.add_line(argument->error_name());
                    }
                    throw Failure{ParseError(std::move(error))};
                }
            }
        }

        for (const Parser *parser : run.parsers) {
            for (const auto &group : parser->value_groups_) {
                if (!group->required_) {
                    continue;
                }
                bool hit = false;
                for (const Argument *argument : group->arguments_) {
                    if (run.present.count(argument->dest) != 0) {
                        hit = true;
                        break;
                    }
                }
                if (hit) {
                    continue;
                }
                UsageError error = leaf->make_error("the following required arguments were not provided:",
                                                    ErrorKind::MISSING_REQUIRED);
                error.add_line("<" + group->name_ + ">");
                throw Failure{ParseError(std::move(error))};
            }
        }

        for (const Argument *argument : registry_order) {
            if (run.present.count(argument->dest) == 0) {
                continue;
            }
            for (const std::string &ref : argument->requires_args) {
                const std::string dest = resolve(ref);
                if (run.present.count(dest) == 0) {
                    throw Failure{ParseError(
                        leaf->make_error("the argument '" + argument->error_name() + "' requires '" + label(dest) + "'",
                                         ErrorKind::MISSING_REQUIRED))};
                }
            }
        }

        std::vector<const Argument *> missing = run.missing;
        for (const Parser *parser : run.parsers) {
            if (parser->subcommand_negates_reqs_ && !run.subcommand.empty()) {
                continue;
            }
            for (const auto &argument : parser->arguments_) {
                if (run.present.count(argument->dest) != 0) {
                    continue;
                }
                bool needed = argument->required;
                if (!needed && !argument->required_unless_present.empty()) {
                    bool satisfied = false;
                    for (const std::string &ref : argument->required_unless_present) {
                        if (run.present.count(resolve(ref)) != 0) {
                            satisfied = true;
                            break;
                        }
                    }
                    needed = !satisfied;
                }
                if (needed) {
                    missing.push_back(argument.get());
                }
            }
            if (parser == leaf && parser->subparsers_ && parser->subparsers_->required_) {
                UsageError error = leaf->make_error("the following required arguments were not provided:",
                                                    ErrorKind::MISSING_REQUIRED);
                error.add_line("<COMMAND>");
                throw Failure{ParseError(std::move(error))};
            }
        }
        if (!missing.empty()) {
            UsageError error = leaf->make_error("the following required arguments were not provided:",
                                                ErrorKind::MISSING_REQUIRED);
            for (const std::string &label_text : unique_labels(missing)) {
                error.add_line(label_text);
            }
            throw Failure{ParseError(std::move(error))};
        }
    }

    void Parser::apply_defaults(Run &run) {
        for (const Parser *parser : run.parsers) {
            for (const auto &argument : parser->arguments_) {
                if (run.namespace_value.contains(argument->dest)) {
                    continue;
                }
                DefaultValue chosen = argument->default_value;
                if (chosen.is_missing()) {
                    chosen = parser->argument_default_;
                }
                if (chosen.is_suppress()) {
                    continue;
                }
                Value value = implicit_default(*argument);
                if (chosen.value() != nullptr) {
                    value = *chosen.value();
                }
                run.namespace_value.set(argument->dest, std::move(value));
            }
        }
    }

    UsageError Parser::make_error(std::string message, ErrorKind kind) const {
        UsageError error(std::move(message));
        error.set_kind(kind);
        error.set_usage(usage_spec());
        if (dests_.count("help") != 0) {
            error.set_hint("For more information, try '--help'.");
        }
        return error;
    }

}  // namespace argparser
