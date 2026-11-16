// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <quant1x/base/argparser/help.hpp>
#include <quant1x/base/argparser/parser.hpp>

namespace {

    using argparser::Argument;
    using argparser::Choice;
    using argparser::ColorChoice;
    using argparser::DefaultValue;
    using argparser::DefinitionError;
    using argparser::ErrorKind;
    using argparser::HelpRequested;
    using argparser::Namespace;
    using argparser::Nargs;
    using argparser::ParseError;
    using argparser::Parser;
    using argparser::UsageError;
    using argparser::Value;
    using argparser::ValueGroup;
    using argparser::VersionRequested;

    int failures = 0;

    /// 断言; 失败时打印标签与上下文。
    void check(const std::string &label, bool ok, const std::string &detail = {}) {
        if (ok) {
            return;
        }
        ++failures;
        std::printf("FAIL %s\n", label.c_str());
        if (!detail.empty()) {
            std::printf("     %s\n", detail.c_str());
        }
    }

    /// 拼 argv。
    std::vector<std::string> argv(std::initializer_list<const char *> items) {
        std::vector<std::string> tokens;
        tokens.reserve(items.size());
        for (const char *item : items) {
            tokens.emplace_back(item);
        }
        return tokens;
    }

    /// 一次解析的结果: 成功与失败二选一。
    struct Parsed {
        std::optional<Namespace>  namespace_value;
        std::optional<ParseError> error;
    };

    /// 跑一次解析。
    Parsed run(const Parser &parser, std::vector<std::string> tokens) {
        Parsed parsed;
        auto   outcome = parser.parse(std::move(tokens));
        if (auto *error = std::get_if<ParseError>(&outcome)) {
            parsed.error = *error;
        } else {
            parsed.namespace_value = std::get<Namespace>(std::move(outcome));
        }
        return parsed;
    }

    /// 取命名空间里某个名字的文本形态。
    std::string text(const Namespace &namespace_value, const std::string &key) {
        const Value *value = namespace_value.find(key);
        return value == nullptr ? std::string() : value->text();
    }

    /// 取用法错误; 不是用法错误时为 `nullptr`。
    const UsageError *usage_of(const Parsed &parsed) {
        if (!parsed.error) {
            return nullptr;
        }
        return std::get_if<UsageError>(&*parsed.error);
    }

    /// 错误的渲染文本; 成功解析时为空。
    std::string rendered(const Parsed &parsed) {
        if (!parsed.error) {
            return {};
        }
        return argparser::render(*parsed.error);
    }

    /// 判断文本是否含子串。
    bool contains(const std::string &text_value, const std::string &want) {
        return text_value.find(want) != std::string::npos;
    }

    /// 本节复用的命令: 计数 / 必填取值 / 可选值 / 位置参数。
    void build_base(Parser &parser) {
        parser.with_color(ColorChoice::NEVER);
        Argument verbose;
        verbose.action = argparser::Action::COUNT;
        parser.add_argument({"-v", "--verbose"}, verbose);

        Argument output;
        output.required = true;
        parser.add_argument({"-o", "--output"}, output);

        Argument level;
        level.choices       = std::vector<Choice>{{Value("debug"), ""}, {Value("info"), ""}, {Value("warn"), ""}};
        level.default_value = DefaultValue::of(Value("info"));
        parser.add_argument({"--level"}, level);

        Argument files;
        files.nargs = Nargs::one_or_more();
        parser.add_argument({"files"}, files);
    }

    /// 两级子命令: tool → build / run, build → release / debug。
    void build_tool(Parser &parser) {
        parser.with_version("9.9.9").with_color(ColorChoice::NEVER);
        Argument quiet;
        quiet.action = argparser::Action::STORE_TRUE;
        quiet.global = true;
        parser.add_argument({"-q", "--quiet"}, quiet);

        auto    &subparsers = parser.add_subparsers("command", "Commands", true);
        Parser  &build      = subparsers.add_parser("build", "构建", {}, {"b"});
        Argument jobs;
        jobs.type          = argparser::parse_int();
        jobs.default_value = DefaultValue::of(Value(std::int64_t{4}));
        build.add_argument({"-j", "--jobs"}, jobs);

        auto &targets = build.add_subparsers("target", "Targets", false);
        targets.add_parser("release", "发布版");
        targets.add_parser("debug", "调试版");

        Parser &run_parser = subparsers.add_parser("run", "运行", {"r"});
        run_parser.add_argument({"script"}, Argument{});
    }

    /// 设置环境变量。
    void set_env(const char *name, const char *value) {
#if defined(_WIN32)
        _putenv_s(name, value);
#else
        setenv(name, value, 1);
#endif
    }

    /// 清掉环境变量。
    void clear_env(const char *name) {
#if defined(_WIN32)
        _putenv_s(name, "");
#else
        unsetenv(name);
#endif
    }

    void test_options_take_values_in_every_form() {
        Parser parser("demo");
        build_base(parser);
        Parsed parsed = run(parser, argv({"-o", "out.txt", "-vv", "--level=warn", "a.txt", "b.txt"}));
        check("选项与 = 取值",
              parsed.namespace_value && text(*parsed.namespace_value, "output") == "out.txt",
              rendered(parsed));
        check("短选项聚合计数",
              parsed.namespace_value && text(*parsed.namespace_value, "verbose") == "2",
              rendered(parsed));
        check("= 取值", parsed.namespace_value && text(*parsed.namespace_value, "level") == "warn");
        check("位置参数收成列表", parsed.namespace_value && text(*parsed.namespace_value, "files") == "a.txt, b.txt");

        // 短选项粘值 `-oout.txt` 与 `-o=out.txt` 等价
        for (const auto &tokens : {argv({"-oout.txt", "a.txt"}), argv({"-o=out.txt", "a.txt"})}) {
            Parsed glued = run(parser, tokens);
            check("短选项粘值",
                  glued.namespace_value && text(*glued.namespace_value, "output") == "out.txt",
                  rendered(glued));
        }

        // 长选项前缀缩写照常解析
        Parsed abbreviated = run(parser, argv({"-o", "x", "--verbos", "a.txt"}));
        check("前缀缩写",
              abbreviated.namespace_value && text(*abbreviated.namespace_value, "verbose") == "1",
              rendered(abbreviated));
    }

    void test_signed_numbers_are_values() {
        Parser parser("neg");
        parser.with_color(ColorChoice::NEVER);
        Argument files;
        files.nargs = Nargs::one_or_more();
        parser.add_argument({"files"}, files);
        Parsed parsed = run(parser, argv({"-1", "-.5", "+1", "-1.5", "+.5", "-"}));
        check("签名数字与 - 作取值",
              parsed.namespace_value && text(*parsed.namespace_value, "files") == "-1, -.5, +1, -1.5, +.5, -",
              rendered(parsed));
    }

    void test_nargs_shapes() {
        Parser parser("shapes");
        parser.with_color(ColorChoice::NEVER);
        Argument pair;
        pair.nargs = Nargs::fixed(2);
        parser.add_argument({"--pair"}, pair);
        Argument maybe;
        maybe.nargs = Nargs::optional();
        parser.add_argument({"--maybe"}, maybe);
        Argument many;
        many.nargs = Nargs::zero_or_more();
        parser.add_argument({"--many"}, many);
        Argument list;
        list.value_delimiter = ",";
        parser.add_argument({"--list"}, list);
        Argument rest;
        rest.nargs = Nargs::remainder();
        parser.add_argument({"--rest"}, rest);

        Parsed parsed = run(
            parser,
            argv({"--pair", "a", "b", "--maybe", "--many", "x", "y", "--list", "1,2,3", "--rest", "z", "--flag"}));
        check("nargs=2", parsed.namespace_value && text(*parsed.namespace_value, "pair") == "a, b", rendered(parsed));
        check("nargs=?", parsed.namespace_value && text(*parsed.namespace_value, "maybe") == "None");
        check("nargs=*", parsed.namespace_value && text(*parsed.namespace_value, "many") == "x, y");
        check("分隔符拆值", parsed.namespace_value && text(*parsed.namespace_value, "list") == "1, 2, 3");
        check("REMAINDER 收下选项样 token",
              parsed.namespace_value && text(*parsed.namespace_value, "rest") == "z, --flag");
    }

    void test_positional_variadic_leaves_room() {
        Parser parser("var");
        parser.with_color(ColorChoice::NEVER);
        Argument src;
        src.nargs = Nargs::zero_or_more();
        parser.add_argument({"src"}, src);
        parser.add_argument({"dst"}, Argument{});
        Parsed parsed = run(parser, argv({"a", "b", "c"}));
        check("变长位置参数留位",
              parsed.namespace_value && text(*parsed.namespace_value, "src") == "a, b",
              rendered(parsed));
        check("后一个位置参数拿到末位", parsed.namespace_value && text(*parsed.namespace_value, "dst") == "c");
    }

    void test_actions() {
        Parser parser("actions");
        parser.with_color(ColorChoice::NEVER);
        Argument count;
        count.action = argparser::Action::COUNT;
        parser.add_argument({"-v"}, count);
        Argument on;
        on.action = argparser::Action::STORE_TRUE;
        parser.add_argument({"--on"}, on);
        Argument off;
        off.action = argparser::Action::STORE_FALSE;
        parser.add_argument({"--off"}, off);
        Argument mode;
        mode.action        = argparser::Action::STORE_CONST;
        mode.const_value   = DefaultValue::of(Value("fast"));
        mode.default_value = DefaultValue::of(Value("slow"));
        parser.add_argument({"--mode"}, mode);
        Argument append;
        append.action = argparser::Action::APPEND;
        parser.add_argument({"-D"}, append);
        Argument append_const;
        append_const.action      = argparser::Action::APPEND_CONST;
        append_const.const_value = DefaultValue::of(Value("T"));
        parser.add_argument({"--tag"}, append_const);
        Argument extend;
        extend.action = argparser::Action::EXTEND;
        extend.nargs  = Nargs::one_or_more();
        parser.add_argument({"-I"}, extend);

        Parsed parsed = run(parser,
                            argv({"-vvv", "--on", "--off", "--mode", "-D", "A", "-D", "B", "--tag", "-I", "x", "y"}));
        check("count", parsed.namespace_value && text(*parsed.namespace_value, "v") == "3", rendered(parsed));
        check("store_true", parsed.namespace_value && text(*parsed.namespace_value, "on") == "true");
        check("store_false", parsed.namespace_value && text(*parsed.namespace_value, "off") == "false");
        check("store_const", parsed.namespace_value && text(*parsed.namespace_value, "mode") == "fast");
        check("append", parsed.namespace_value && text(*parsed.namespace_value, "D") == "A, B");
        check("append_const", parsed.namespace_value && text(*parsed.namespace_value, "tag") == "T");
        check("extend", parsed.namespace_value && text(*parsed.namespace_value, "I") == "x, y");
    }

    void test_converters_and_choices() {
        Parser parser("demo");
        build_base(parser);
        Parsed            parsed = run(parser, argv({"-o", "out.txt", "--level", "warnn", "a.txt"}));
        const UsageError *usage  = usage_of(parsed);
        check("可选值非法",
              usage != nullptr && contains(usage->render(), "invalid value 'warnn' for '--level <LEVEL>'"),
              rendered(parsed));
        check("可选值清单", usage != nullptr && contains(usage->render(), "[possible values: debug, info, warn]"));
        check("可选值建议", usage != nullptr && contains(usage->render(), "a similar value exists: 'warn'"));

        Parser ranged("num");
        ranged.with_color(ColorChoice::NEVER);
        Argument number;
        number.type = argparser::parse_int(1, 10);
        ranged.add_argument({"--n"}, number);
        Parsed overflow = run(ranged, argv({"--n", "11"}));
        check("整数越界", contains(rendered(overflow), "11 is not in 1..=10"), rendered(overflow));
        Parsed invalid = run(ranged, argv({"--n", "abc"}));
        check("非法整数", contains(rendered(invalid), "invalid digit found in string"), rendered(invalid));
    }

    void test_unknown_option_suggests() {
        Parser parser("demo");
        build_base(parser);
        Parsed            parsed = run(parser, argv({"-o", "x", "--verbsoe", "a.txt"}));
        const UsageError *usage  = usage_of(parsed);
        check("未知选项文案",
              usage != nullptr && contains(usage->render(), "unexpected argument '--verbsoe' found"),
              rendered(parsed));
        check("未知选项建议", usage != nullptr && contains(usage->render(), "a similar argument exists: '--verbose'"));
        check("未知选项类别", usage != nullptr && usage->kind() == ErrorKind::UNKNOWN_ARGUMENT);

        Parsed fallback = run(parser, argv({"-o", "x", "--zzz", "a.txt"}));
        check("`--` 提示",
              contains(rendered(fallback), "to pass '--zzz' as a value, use '-- --zzz'"),
              rendered(fallback));
    }

    void test_ambiguous_prefix_lists_candidates() {
        Parser parser("amb");
        parser.with_color(ColorChoice::NEVER);
        parser.add_argument({"--output"}, Argument{});
        parser.add_argument({"--outdir"}, Argument{});
        Parsed            parsed = run(parser, argv({"--out", "x"}));
        const UsageError *usage  = usage_of(parsed);
        check("歧义类别", usage != nullptr && usage->kind() == ErrorKind::AMBIGUOUS_ARGUMENT, rendered(parsed));
        check("歧义候选", usage != nullptr && contains(usage->render(), "could be one of: --outdir, --output"));
    }

    void test_missing_value_and_required() {
        Parser parser("demo");
        build_base(parser);
        Parsed missing = run(parser, argv({"-o"}));
        check("缺取值类别", usage_of(missing) != nullptr && usage_of(missing)->kind() == ErrorKind::MISSING_VALUE);
        check("缺取值文案",
              contains(rendered(missing), "a value is required for '--output <OUTPUT>' but none was supplied"),
              rendered(missing));

        Parsed required = run(parser, argv({"a.txt"}));
        check("缺必填类别", usage_of(required) != nullptr && usage_of(required)->kind() == ErrorKind::MISSING_REQUIRED);
        check("缺必填文案",
              contains(rendered(required), "the following required arguments were not provided:") &&
                  contains(rendered(required), "--output <OUTPUT>"));

        Parser multi("multi");
        multi.with_color(ColorChoice::NEVER);
        Argument triple;
        triple.nargs = Nargs::fixed(3);
        multi.add_argument({"--triple"}, triple);
        check("多值缺量",
              contains(rendered(run(multi, argv({"--triple", "a"}))),
                       "3 values required for '--triple <TRIPLE>' but 1 was provided"));
    }

    void test_validation_order() {
        Parser parser("check");
        parser.with_color(ColorChoice::NEVER);
        Argument first;
        first.required = true;
        parser.add_argument({"--a"}, first);
        Argument second;
        second.conflicts_with = {"--a"};
        parser.add_argument({"--b"}, second);
        Argument third;
        third.requires_args = {"--a"};
        parser.add_argument({"--c"}, third);

        Parsed conflict = run(parser, argv({"--a", "1", "--b", "2"}));
        check("冲突然别", usage_of(conflict) != nullptr && usage_of(conflict)->kind() == ErrorKind::CONFLICT);
        check("冲突文案",
              contains(rendered(conflict), "the argument '--b <B>' cannot be used with '--a <A>'"),
              rendered(conflict));

        Parsed dependent = run(parser, argv({"--c", "3"}));
        check("依赖类别", usage_of(dependent) != nullptr && usage_of(dependent)->kind() == ErrorKind::MISSING_REQUIRED);
        check("依赖文案", contains(rendered(dependent), "the argument '--c <C>' requires '--a <A>'"));

        Parser mutex("mutex");
        mutex.with_color(ColorChoice::NEVER);
        auto &group = mutex.add_mutually_exclusive_group(true);
        group.add_argument({"--x"}, Argument{});
        group.add_argument({"--y"}, Argument{});
        check("必选互斥组",
              contains(rendered(run(mutex, argv({}))), "one of the following arguments is required:") &&
                  contains(rendered(run(mutex, argv({}))), "  --x <X>\n  --y <Y>"));
        check(
            "互斥冲突",
            contains(rendered(run(mutex, argv({"--x", "1", "--y", "2"}))), "'--x <X>' cannot be used with '--y <Y>'"));
    }

    void test_env_fallback() {
        Parser parser("env");
        parser.with_color(ColorChoice::NEVER);
        Argument level;
        level.choices = std::vector<Choice>{{Value("debug"), ""}, {Value("info"), ""}, {Value("warn"), ""}};
        level.env     = "ARGPARSER_LEVEL";
        parser.add_argument({"--level"}, level);
        Argument dry;
        dry.action = argparser::Action::STORE_TRUE;
        dry.env    = "ARGPARSER_DRY";
        parser.add_argument({"--dry-run"}, dry);

        set_env("ARGPARSER_LEVEL", "warn");
        set_env("ARGPARSER_DRY", "yes");
        Parsed parsed = run(parser, {});
        check("env 兜底取值",
              parsed.namespace_value && text(*parsed.namespace_value, "level") == "warn",
              rendered(parsed));
        check("env 布尔开关", parsed.namespace_value && text(*parsed.namespace_value, "dry_run") == "true");

        Parsed overridden = run(parser, argv({"--level", "info"}));
        check("命令行优先于 env", overridden.namespace_value && text(*overridden.namespace_value, "level") == "info");

        set_env("ARGPARSER_LEVEL", "verbose");
        set_env("ARGPARSER_DRY", "yes");
        check("env 取值非法也报错",
              contains(rendered(run(parser, {})), "invalid value 'verbose' for '--level <LEVEL>'"));

        set_env("ARGPARSER_LEVEL", "info");
        set_env("ARGPARSER_DRY", "maybe");
        check("env 布尔非法也报错", contains(rendered(run(parser, {})), "invalid value 'maybe' for '--dry-run'"));
        clear_env("ARGPARSER_LEVEL");
        clear_env("ARGPARSER_DRY");
    }

    void test_defaults_and_suppress() {
        Parser parser("defaults");
        parser.with_color(ColorChoice::NEVER).with_argument_default(DefaultValue::of(Value("fallback")));
        parser.add_argument({"--name"}, Argument{});
        Argument quiet;
        quiet.default_value = DefaultValue::suppress();
        parser.add_argument({"--quiet"}, quiet);
        Argument count;
        count.action = argparser::Action::COUNT;
        parser.add_argument({"--count"}, count);

        Parsed parsed = run(parser, {});
        check("argument_default 兜底",
              parsed.namespace_value && text(*parsed.namespace_value, "name") == "fallback",
              rendered(parsed));
        check("SUPPRESS 不落命名空间", parsed.namespace_value && !parsed.namespace_value->contains("quiet"));
        check("count 隐式零值", parsed.namespace_value && text(*parsed.namespace_value, "count") == "0");

        Parser inferred("infer");
        inferred.with_color(ColorChoice::NEVER);
        Argument port;
        port.default_value = DefaultValue::of(Value(std::int64_t{80}));
        inferred.add_argument({"--port"}, port);
        Parsed       typed = run(inferred, argv({"--port", "8080"}));
        const Value *value = typed.namespace_value ? typed.namespace_value->find("port") : nullptr;
        check("按默认值推断转换器",
              value != nullptr && value->as_int() != nullptr && *value->as_int() == 8080,
              rendered(typed));
    }

    void test_subcommands() {
        Parser parser("tool");
        build_tool(parser);
        Parsed parsed = run(parser, argv({"b", "-j", "8", "release"}));
        check("别名归一成正名",
              parsed.namespace_value && text(*parsed.namespace_value, "command") == "build",
              rendered(parsed));
        check("二级子命令", parsed.namespace_value && text(*parsed.namespace_value, "target") == "release");
        check("子命令取值", parsed.namespace_value && text(*parsed.namespace_value, "jobs") == "8");

        Parsed forwarded = run(parser, argv({"run", "-q", "x.sh"}));
        check("全局参数下沉",
              forwarded.namespace_value && text(*forwarded.namespace_value, "quiet") == "true",
              rendered(forwarded));

        check("缺必填子命令", contains(rendered(run(parser, {})), "<COMMAND>"));
        check("缺子命令位置参数", contains(rendered(run(parser, argv({"run"}))), "<SCRIPT>"));
    }

    void test_help_subcommand() {
        Parser parser("tool");
        build_tool(parser);
        Parsed      parsed = run(parser, argv({"help", "build"}));
        const auto *help   = parsed.error ? std::get_if<HelpRequested>(&*parsed.error) : nullptr;
        check(
            "help 子命令走长帮助", help != nullptr && help->long_form && help->prog == "tool build", rendered(parsed));
        check("help 认不出的名字",
              contains(rendered(run(parser, argv({"help", "nosuch"}))), "unrecognized subcommand 'nosuch'"));
    }

    void test_help_and_version_flags() {
        Parser parser("demo");
        parser.with_version("1.2.3").with_color(ColorChoice::NEVER);
        Parsed      short_help = run(parser, argv({"-h"}));
        const auto *help       = short_help.error ? std::get_if<HelpRequested>(&*short_help.error) : nullptr;
        check("短帮助", help != nullptr && !help->long_form && help->prog == "demo", rendered(short_help));

        for (const char *flag : {"-V", "--version"}) {
            Parsed      parsed  = run(parser, argv({flag}));
            const auto *version = parsed.error ? std::get_if<VersionRequested>(&*parsed.error) : nullptr;
            check(
                std::string("版本标志 ") + flag, version != nullptr && version->text == "demo 1.2.3", rendered(parsed));
        }

        Parser tool_parser("tool");
        build_tool(tool_parser);
        Parsed      parsed  = run(tool_parser, argv({"build", "--version"}));
        const auto *version = parsed.error ? std::get_if<VersionRequested>(&*parsed.error) : nullptr;
        check("子命令沿用父命令版本", version != nullptr && version->text == "tool build 9.9.9", rendered(parsed));
    }

    void test_parse_known_extras() {
        Parser parser("known");
        parser.with_color(ColorChoice::NEVER);
        parser.add_argument({"--flag"}, Argument{});
        auto  outcome = parser.parse_known(argv({"--flag", "v", "--zzz", "left", "--yyy", "--flag", "w", "tail"}));
        auto *known   = std::get_if<Parser::KnownOutcome>(&outcome);
        check("parse_known 成功", known != nullptr);
        if (known != nullptr) {
            check("未知选项只收自身", text(known->namespace_value, "flag") == "w");
            check("extras 保命令行顺序", known->extras == argv({"--zzz", "left", "--yyy", "tail"}));
        }
    }

    void test_remainder_boundary_rejects_option_token() {
        Parser parser("rem");
        parser.with_color(ColorChoice::NEVER);
        parser.add_argument({"head"}, Argument{});
        Argument rest;
        rest.nargs = Nargs::remainder();
        parser.add_argument({"rest"}, rest);
        Parsed parsed = run(parser, argv({"a", "--flag"}));
        check("REMAINDER 边界拒绝选项",
              usage_of(parsed) != nullptr && usage_of(parsed)->kind() == ErrorKind::UNKNOWN_ARGUMENT,
              rendered(parsed));
    }

    /// 必选组与 `required_unless_present`: 条件满足才放行, 覆盖先于必填判定。
    void test_value_group_and_required_unless_present() {
        Parser grouped("grouped");
        grouped.with_color(ColorChoice::NEVER);
        ValueGroup &inputs = grouped.add_value_group("input", true);
        inputs.add_argument({"--file"}, Argument{});
        inputs.add_argument({"--dir"}, Argument{});

        Parsed one = run(grouped, {"--file", "x"});
        check("多值组: 出现一个即可", one.namespace_value && text(*one.namespace_value, "file") == "x", rendered(one));

        Parsed both = run(grouped, {"--file", "x", "--dir", "y"});
        check("多值组: 成员可以同时出现",
              both.namespace_value && text(*both.namespace_value, "dir") == "y",
              rendered(both));

        Parsed            empty         = run(grouped, {});
        const UsageError *group_failure = usage_of(empty);
        check("多值组: 一个都不出现报缺",
              group_failure != nullptr && group_failure->kind() == ErrorKind::MISSING_REQUIRED &&
                  rendered(empty).find("<input>") != std::string::npos,
              rendered(empty));

        Parser gated("gated");
        gated.with_color(ColorChoice::NEVER);
        Argument token;
        token.required_unless_present = {"user"};
        gated.add_argument({"--token"}, token);
        gated.add_argument({"--user"}, Argument{});
        Argument key;
        key.overrides_with = {"user"};
        gated.add_argument({"--key"}, key);

        check("required_unless_present: 条件满足则不报缺",
              usage_of(run(gated, {"--user", "u"})) == nullptr,
              rendered(run(gated, {"--user", "u"})));
        const UsageError *without = usage_of(run(gated, {}));
        check("required_unless_present: 条件不满足报缺",
              without != nullptr && without->kind() == ErrorKind::MISSING_REQUIRED,
              rendered(run(gated, {})));
        const UsageError *overridden = usage_of(run(gated, {"--user", "u", "--key", "k"}));
        check("覆盖先于必填判定: 被覆盖掉的不再算出现过",
              overridden != nullptr && overridden->kind() == ErrorKind::MISSING_REQUIRED,
              rendered(run(gated, {"--user", "u", "--key", "k"})));
    }

    /// 帮助宽度: 默认不读 `COLUMNS`, 开了 auto_width 才读。
    void test_auto_width_reads_columns() {
        Parser fixed("demo");
        fixed.with_max_width(44);
        check("默认不读 COLUMNS", fixed.width() == 44, std::to_string(fixed.width()));

        set_env("COLUMNS", "60");
        check("未开 auto_width 时 COLUMNS 不生效", fixed.width() == 44, std::to_string(fixed.width()));

        Parser auto_width_parser("demo");
        auto_width_parser.with_max_width(44).with_auto_width();
        check(
            "开了 auto_width 后读 COLUMNS", auto_width_parser.width() == 60, std::to_string(auto_width_parser.width()));

        set_env("COLUMNS", "abc");
        check(
            "COLUMNS 非法时退回 max_width", auto_width_parser.width() == 44, std::to_string(auto_width_parser.width()));

        set_env("COLUMNS", "0");
        check(
            "COLUMNS 非正时退回 max_width", auto_width_parser.width() == 44, std::to_string(auto_width_parser.width()));

        clear_env("COLUMNS");
        check(
            "COLUMNS 缺省时退回 max_width", auto_width_parser.width() == 44, std::to_string(auto_width_parser.width()));
    }

    /// `overrides_with` 双向生效: 一对里后出现的留下, 先出现的退回默认值。
    void test_overrides_with_keeps_the_last_one() {
        Parser parser("over");
        parser.with_color(ColorChoice::NEVER);
        Argument first;
        first.default_value = DefaultValue::of(Value("A"));
        parser.add_argument({"--a"}, first);
        Argument second;
        second.default_value  = DefaultValue::of(Value("B"));
        second.overrides_with = {"a"};
        parser.add_argument({"--b"}, second);

        Parsed last_b = run(parser, {"--a", "1", "--b", "2"});
        check("后出现的留下",
              last_b.namespace_value && text(*last_b.namespace_value, "b") == "2" &&
                  text(*last_b.namespace_value, "a") == "A",
              rendered(last_b));

        Parsed last_a = run(parser, {"--b", "2", "--a", "1"});
        check("反向时同样是后出现的留下",
              last_a.namespace_value && text(*last_a.namespace_value, "a") == "1" &&
                  text(*last_a.namespace_value, "b") == "B",
              rendered(last_a));
    }

    void test_from_file_prefix_expands_lines() {
        const std::filesystem::path dir = std::filesystem::temp_directory_path() / "argparser-include";
        std::filesystem::create_directories(dir);
        const std::filesystem::path inner   = dir / "inner.txt";
        const std::filesystem::path outer   = dir / "outer.txt";
        const std::filesystem::path missing = dir / "nope.txt";
        {
            std::ofstream stream(inner, std::ios::binary);
            stream << "--output\ninner.txt\nb.txt\n";
        }
        {
            std::ofstream stream(outer, std::ios::binary);
            stream << "@" << inner.string() << "\n";
        }

        Parser parser("files");
        parser.with_color(ColorChoice::NEVER).with_from_file_prefix_chars("@");
        parser.add_argument({"-o", "--output"}, Argument{});
        Argument files;
        files.nargs = Nargs::zero_or_more();
        parser.add_argument({"files"}, files);

        Parsed expanded = run(parser, {std::string("@") + outer.string()});
        check("@file 嵌套继续展开",
              expanded.namespace_value && text(*expanded.namespace_value, "output") == "inner.txt" &&
                  text(*expanded.namespace_value, "files") == "b.txt",
              rendered(expanded));

        Parsed            broken  = run(parser, {std::string("@") + missing.string()});
        const UsageError *failure = usage_of(broken);
        check("@file 打不开按用法错误报",
              failure != nullptr && failure->kind() == ErrorKind::OTHER &&
                  failure->message().find("unable to open file") != std::string::npos,
              rendered(broken));

        std::filesystem::remove_all(dir);
    }

    void test_definition_errors_are_rejected() {
        Parser parser("dup");
        parser.with_color(ColorChoice::NEVER);
        parser.add_argument({"--a"}, Argument{});
        bool rejected = false;
        try {
            parser.add_argument({"--a"}, Argument{});
        } catch (const std::invalid_argument &) {
            rejected = true;
        }
        check("选项名重复", rejected);

        rejected = false;
        try {
            parser.add_argument({"--b"}, Argument{.dest = "a"});
        } catch (const std::invalid_argument &) {
            rejected = true;
        }
        check("dest 重复", rejected);

        Parser bad("bad");
        rejected = false;
        try {
            bad.add_argument({"-1"}, Argument{});
        } catch (const std::invalid_argument &) {
            rejected = true;
        }
        check("签名数字的选项名", rejected);

        Parser remainder("rem");
        remainder.with_color(ColorChoice::NEVER);
        Argument rest;
        rest.nargs = Nargs::remainder();
        remainder.add_argument({"rest"}, rest);
        rejected = false;
        try {
            remainder.add_argument({"tail"}, Argument{});
        } catch (const std::invalid_argument &) {
            rejected = true;
        }
        check("REMAINDER 之后加位置参数", rejected);

        Parser required("req");
        rejected = false;
        try {
            required.add_argument({"f"}, Argument{.required = true});
        } catch (const std::invalid_argument &) {
            rejected = true;
        }
        check("位置参数不接受 required", rejected);
    }

    void test_duplicated_dest_across_commands() {
        Parser parser("chain");
        parser.with_color(ColorChoice::NEVER);
        Argument verbose;
        verbose.action = argparser::Action::STORE_TRUE;
        verbose.global = true;
        parser.add_argument({"-v"}, verbose);
        auto    &subparsers = parser.add_subparsers("", "Commands", false);
        Parser  &child      = subparsers.add_parser("c");
        Argument child_verbose;
        child_verbose.action = argparser::Action::STORE_TRUE;
        child.add_argument({"-v"}, child_verbose);
        Parsed parsed = run(parser, argv({"c"}));
        check("命令链 dest 重复是定义期错误",
              parsed.error && std::holds_alternative<DefinitionError>(*parsed.error),
              rendered(parsed));
    }

    void test_usage_spec_shapes() {
        Parser parser("usage");
        parser.with_color(ColorChoice::NEVER);
        Argument output;
        output.required = true;
        parser.add_argument({"--out"}, output);
        parser.add_argument({"--opt"}, Argument{});
        Argument one;
        one.nargs = Nargs::optional();
        parser.add_argument({"one"}, one);
        Argument many;
        many.nargs = Nargs::one_or_more();
        parser.add_argument({"many"}, many);
        check("usage 组成", parser.usage_spec() == "usage --out <OUT> [OPTIONS] [ONE] <MANY>...", parser.usage_spec());
    }

    void test_jaro_winkler_and_best_match() {
        const double score = argparser::jaro_winkler("--verbose", "--version");
        check("相似度数值", score > 0.8825 && score < 0.8826, std::to_string(score));
        const auto match = argparser::best_match("--flah", {"--help", "--flag", "--output"});
        check("建议取最近候选", match && *match == "--flag");
        check("无从建议", !argparser::best_match("--zzz", {"--help", "--flag", "--output"}));
    }

    void test_value_text_follows_python_rules() {
        check("None 文本", Value().text() == "None");
        check("真值文本", Value(true).text() == "true");
        check("假值文本", Value(false).text() == "false");
        check("整数文本", Value(std::int64_t{3}).text() == "3");
        check("整数值浮点补零", Value(2.0).text() == "2.0");
        check("小数文本", Value(2.5).text() == "2.5");
        check("字符串文本", Value("text").text() == "text");
        check("列表文本", Value::list({Value("a"), Value("b")}).text() == "a, b");
    }

    /// 取某个章节标题之后、下一个空行之前的行。
    std::vector<std::string> section(const std::string &body, const std::string &title) {
        std::vector<std::string> lines;
        std::size_t              start = 0;
        bool                     found = false;
        while (start <= body.size()) {
            const std::size_t end  = body.find('\n', start);
            const std::string line = body.substr(start, end == std::string::npos ? std::string::npos : end - start);
            if (!found) {
                if (line == title) {
                    found = true;
                }
            } else if (line.find_first_not_of(" \t") == std::string::npos) {
                break;
            } else {
                lines.push_back(line);
            }
            if (end == std::string::npos) {
                break;
            }
            start = end + 1;
        }
        return lines;
    }

    /// 码点个数, 与 Python 的 `len()` 同口径。
    std::size_t display_width(const std::string &body) {
        std::size_t total = 0;
        for (unsigned char byte : body) {
            if ((byte & 0xC0) != 0x80) {
                ++total;
            }
        }
        return total;
    }

    /// 子串出现次数。
    std::size_t count_of(const std::string &body, const std::string &want) {
        std::size_t total = 0;
        std::size_t start = 0;
        while (true) {
            const std::size_t found = body.find(want, start);
            if (found == std::string::npos) {
                break;
            }
            ++total;
            start = found + want.size();
        }
        return total;
    }

    /// 去掉 ANSI 转义, 便于比较可见版式。
    std::string plain(const std::string &body) {
        std::string text;
        std::size_t index = 0;
        while (index < body.size()) {
            if (body[index] == '\x1b' && index + 1 < body.size() && body[index + 1] == '[') {
                index += 2;
                while (index < body.size() && body[index] != 'm') {
                    ++index;
                }
                ++index;
                continue;
            }
            text.push_back(body[index]);
            ++index;
        }
        return text;
    }

    void test_help_usage_composition() {
        Parser bare("bare");
        bare.with_color(ColorChoice::NEVER);
        check("裸命令 usage", bare.usage_spec() == "bare", bare.usage_spec());

        Parser opt("opt");
        opt.with_color(ColorChoice::NEVER);
        Argument all;
        all.action = argparser::Action::STORE_TRUE;
        all.help   = "全部";
        opt.add_argument({"-a", "--all"}, all);
        Argument out;
        out.required = true;
        out.help     = "输出";
        opt.add_argument({"--out"}, out);
        Argument files;
        files.nargs = Nargs::one_or_more();
        files.help  = "文件";
        opt.add_argument({"files"}, files);
        check("选项与位置参数", opt.usage_spec() == "opt --out <OUT> [OPTIONS] <FILES>...", opt.usage_spec());

        Parser cmd("cmd");
        cmd.with_color(ColorChoice::NEVER);
        cmd.add_subparsers("", "Commands", true);
        check("必填子命令", cmd.usage_spec() == "cmd <COMMAND>", cmd.usage_spec());

        Parser optional("cmd2");
        optional.with_color(ColorChoice::NEVER);
        optional.add_subparsers("", "Commands", false);
        check("可选子命令", optional.usage_spec() == "cmd2 [COMMAND]", optional.usage_spec());

        Parser own("own");
        own.with_color(ColorChoice::NEVER).with_usage("own <自定义>");
        Argument x;
        x.help = "忽略";
        own.add_argument({"--x"}, x);
        check("自定义 usage", own.usage_spec() == "own <自定义>", own.usage_spec());
        check("format_usage 前缀", own.format_usage() == "Usage: own <自定义>", own.format_usage());
    }

    /// 帮助章节复用: 简介、长简介、分组、子命令齐全。
    void build_help_tool(Parser &parser) {
        parser.with_about("工具集")
            .with_long_about("工具集的完整说明, 只在 --help 里出现。")
            .with_epilog("示例: tool build -j 8")
            .with_version("1.0.0")
            .with_color(ColorChoice::NEVER);
        Argument verbose;
        verbose.action = argparser::Action::COUNT;
        verbose.help   = "输出更详细";
        parser.add_argument({"-v", "--verbose"}, verbose);
        Argument level;
        level.choices       = std::vector<Choice>{{Value("debug"), ""}, {Value("info"), ""}, {Value("warn"), ""}};
        level.default_value = DefaultValue::of(Value("info"));
        level.help          = "日志级别";
        parser.add_argument({"--level"}, level);
        Argument jobs;
        jobs.type          = argparser::parse_int();
        jobs.default_value = DefaultValue::of(Value(std::int64_t{4}));
        jobs.env           = "TOOL_JOBS";
        jobs.help          = "并发数";
        parser.add_argument({"--jobs"}, jobs);
        Argument secret;
        secret.hide = true;
        secret.help = "不展示";
        parser.add_argument({"--secret"}, secret);
        Argument quiet;
        quiet.action       = argparser::Action::STORE_TRUE;
        quiet.hide_default = true;
        quiet.help         = "安静";
        parser.add_argument({"--quiet"}, quiet);
        Argument src;
        src.nargs = Nargs::zero_or_more();
        src.help  = "源文件";
        parser.add_argument({"src"}, src);
        auto &subparsers = parser.add_subparsers("command", "Commands", false);
        subparsers.add_parser("build", "构建", {}, {"b"});
        subparsers.add_parser("run", "运行");
    }

    void test_help_sections_and_content() {
        Parser parser("tool");
        build_help_tool(parser);
        const std::string text = parser.format_help(false);
        check("简介在最前", text.rfind("工具集\n", 0) == 0, text.substr(0, 40));
        const std::size_t usage_at     = text.find("Usage:");
        const std::size_t commands_at  = text.find("Commands:");
        const std::size_t arguments_at = text.find("Arguments:");
        const std::size_t options_at   = text.find("Options:");
        check("章节顺序", usage_at < commands_at && commands_at < arguments_at && arguments_at < options_at);
        const std::string epilog    = "示例: tool build -j 8";
        const std::size_t epilog_at = text.rfind(epilog);
        check("epilog 收尾",
              epilog_at != std::string::npos && epilog_at + epilog.size() == text.size(),
              text.substr(text.size() > 40 ? text.size() - 40 : 0));
        check("隐藏参数不出现", !contains(text, "--secret"));
        check("计数字面量", contains(text, "-v, --verbose..."));
        check("默认值标注", contains(text, "[default: info]"));
        check("环境变量标注", contains(text, "[env: TOOL_JOBS]"));
        check("可选值标注", contains(text, "[possible values: debug, info, warn]"));
        check("开关说明", contains(text, "安静"));
        check("开关不标默认值", !contains(text, "[default: false]"));
        check("版本行", contains(text, "-V, --version"));
        check("帮助行", contains(text, "-h, --help"));

        const std::string long_text = parser.format_help(true);
        check("长简介顶替简介", long_text.rfind("工具集的完整说明", 0) == 0);
        check("长帮助不重复简介", count_of(long_text, "工具集") == 1, long_text.substr(0, 60));

        const auto options = section(text, "Options:");
        check("选项行数", options.size() == 6, std::to_string(options.size()));
        check("帮助在倒数第二行", options.size() >= 2 && contains(options[options.size() - 2], "Print help"));
        check("版本在最后一行", !options.empty() && contains(options.back(), "Print version"));

        const auto commands = section(text, "Commands:");
        check("子命令带别名",
              !commands.empty() && commands.front().find("build, b") != std::string::npos,
              commands.empty() ? std::string() : commands.front());
        check("help 子命令在最后", !commands.empty() && commands.back().find("help") != std::string::npos);
        check("help 子命令说明", !commands.empty() && contains(commands.back(), "Print this message"));

        const auto positionals = section(text, "Arguments:");
        check("位置参数左列",
              !positionals.empty() && positionals.front().find("[SRC]...") != std::string::npos,
              positionals.empty() ? std::string() : positionals.front());
    }

    void test_help_long_choices() {
        Parser parser("level");
        parser.with_color(ColorChoice::NEVER);
        Argument level;
        level.choices       = std::vector<Choice>{{Value("debug"), "逐条打印"}, {Value("info"), "常规"}};
        level.default_value = DefaultValue::of(Value("info"));
        level.help          = "级别";
        level.long_help     = "日志级别, 影响输出量";
        parser.add_argument({"--level"}, level);
        const std::string short_text = parser.format_help(false);
        check("短帮助用短说明", !contains(short_text, "影响输出量"));
        check("短帮助用短文案", contains(short_text, "级别"));
        const std::string long_text = parser.format_help(true);
        check("长帮助用长说明", contains(long_text, "影响输出量"));
        check("长帮助列可选值说明", contains(long_text, "- debug: 逐条打印"));
    }

    void test_help_groups() {
        Parser parser("grouped");
        parser.with_color(ColorChoice::NEVER);
        auto    &group = parser.add_group("Database", "数据库相关");
        Argument host;
        host.default_value = DefaultValue::of(Value("localhost"));
        host.help          = "主机";
        group.add_argument({"--host"}, host);
        Argument port;
        port.type          = argparser::parse_int();
        port.default_value = DefaultValue::of(Value(std::int64_t{5432}));
        port.help          = "端口";
        group.add_argument({"--port"}, port);
        Argument debug;
        debug.action = argparser::Action::STORE_TRUE;
        debug.help   = "调试";
        parser.add_argument({"--debug"}, debug);
        const std::string text = parser.format_help(false);
        check("分组标题", contains(text, "Database:"));
        check("分组说明", contains(text, "数据库相关"));
        check("分组参数在分组下", text.find("--host") > text.find("Database:"));
        check("普通参数在分组前", text.find("--debug") < text.find("Database:"));
    }

    void test_help_alignment_and_wrapping() {
        Parser aligned("align");
        aligned.with_color(ColorChoice::NEVER);
        Argument all;
        all.help = "全部";
        aligned.add_argument({"-a", "--all"}, all);
        Argument beta;
        beta.metavar = "BETA";
        beta.help    = "乙";
        aligned.add_argument({"--beta"}, beta);
        const std::string text = aligned.format_help(false);
        const auto        rows = section(text, "Options:");
        check("说明列对齐",
              rows.size() >= 2 && rows[0].find("全部") != std::string::npos &&
                  rows[0].find("全部") == rows[1].find("乙"),
              rows.size() >= 2 ? rows[0] + " | " + rows[1] : std::string());
        check(
            "左列缩进", !rows.empty() && rows[0].rfind("  -a, --all", 0) == 0, rows.empty() ? std::string() : rows[0]);

        Parser wrap("wrap");
        wrap.with_color(ColorChoice::NEVER).with_max_width(44);
        Argument longer;
        longer.metavar = "VALUE";
        longer.help    = "这是一段足够长的说明文字, 必须在窄幅下折行并保持对齐缩进";
        wrap.add_argument({"--long"}, longer);
        const std::string narrow      = wrap.format_help(false);
        const auto        option_rows = section(narrow, "Options:");
        check("窄幅折行", option_rows.size() >= 2, std::to_string(option_rows.size()));
        check("续行缩进 18 列",
              option_rows.size() >= 2 && option_rows[1].rfind(std::string(18, ' '), 0) == 0,
              option_rows.size() >= 2 ? option_rows[1] : std::string());
        bool        fits  = true;
        std::size_t start = 0;
        while (start <= narrow.size()) {
            const std::size_t end  = narrow.find('\n', start);
            const std::string line = narrow.substr(start, end == std::string::npos ? std::string::npos : end - start);
            fits                   = fits && display_width(line) <= 44;
            if (end == std::string::npos) {
                break;
            }
            start = end + 1;
        }
        check("整幅不超宽", fits);
    }

    void test_help_subcommand_globals() {
        Parser parser("tool");
        parser.with_color(ColorChoice::NEVER);
        Argument quiet;
        quiet.action = argparser::Action::STORE_TRUE;
        quiet.global = true;
        quiet.help   = "安静模式";
        parser.add_argument({"-q", "--quiet"}, quiet);
        auto    &subparsers = parser.add_subparsers("command", "Commands", false);
        Parser  &build      = subparsers.add_parser("build", "构建");
        Argument jobs;
        jobs.type          = argparser::parse_int();
        jobs.default_value = DefaultValue::of(Value(std::int64_t{4}));
        jobs.help          = "并发数";
        build.add_argument({"-j", "--jobs"}, jobs);
        const Parser     *child      = parser.subparsers()->lookup("build");
        const std::string child_text = child == nullptr ? std::string() : child->format_help(false);
        check("子命令 usage", contains(child_text, "Usage: tool build [OPTIONS]"), child_text.substr(0, 40));
        check("继承全局参数", contains(child_text, "-q, --quiet"));
        check("自身参数", contains(child_text, "-j, --jobs"));

        Parser nested("nested");
        nested.with_color(ColorChoice::NEVER);
        auto   &nested_commands = nested.add_subparsers("command", "Commands", false);
        Parser &nested_build    = nested_commands.add_parser("build", "构建");
        auto   &targets         = nested_build.add_subparsers("target", "Targets", false);
        targets.add_parser("release", "发布版");
        check("嵌套子命令列 help", contains(nested_build.format_help(false), "Print this message"));

        Parsed      long_help = run(parser, argv({"build", "--help"}));
        const auto *requested = long_help.error ? std::get_if<HelpRequested>(&*long_help.error) : nullptr;
        check("长帮助请求", requested != nullptr && requested->long_form && requested->prog == "tool build");
        Parsed      short_help = run(parser, argv({"-h"}));
        const auto *brief      = short_help.error ? std::get_if<HelpRequested>(&*short_help.error) : nullptr;
        check("短帮助请求", brief != nullptr && !brief->long_form && brief->prog == "tool");
    }

    void test_help_error_block() {
        Parser parser("demo");
        parser.with_about("演示").with_color(ColorChoice::NEVER);
        Argument level;
        level.choices = std::vector<Choice>{{Value("debug"), ""}, {Value("info"), ""}, {Value("warn"), ""}};
        level.help    = "级别";
        parser.add_argument({"--level"}, level);
        const std::string text = rendered(run(parser, argv({"--level", "warnn"})));
        check("错误首行", text.rfind("error: invalid value 'warnn' for '--level <LEVEL>'", 0) == 0, text.substr(0, 60));
        check("错误列可选值", contains(text, "\n  [possible values: debug, info, warn]"));
        check("错误给建议", contains(text, "\n  tip: a similar value exists: 'warn'"));
        check("错误带 usage", contains(text, "\nUsage: demo [OPTIONS]"));
        const std::string hint = "For more information, try '--help'.";
        check("错误带帮助提示",
              text.size() >= hint.size() && text.compare(text.size() - hint.size(), hint.size(), hint) == 0,
              text.substr(text.size() > 40 ? text.size() - 40 : 0));

        Parser no_help("nohelp");
        no_help.with_color(ColorChoice::NEVER).without_help();
        Argument x;
        x.help = "甲";
        no_help.add_argument({"--x"}, x);
        check("无帮助选项时不提帮助", !contains(rendered(run(no_help, argv({"--y"}))), "For more information"));

        UsageError custom("自定义");
        custom.set_reason("原因");
        custom.add_line("明细");
        custom.add_tip("建议");
        custom.set_usage("demo");
        check("非着色错误同版式", custom.render().rfind("error: 自定义: 原因", 0) == 0, custom.render());
        check("着色错误只在 error: 上加色",
              argparser::render_error(custom, true).rfind("\x1b[31merror:\x1b[0m", 0) == 0);
    }

    void test_help_color() {
        Parser colored("color");
        colored.with_color(ColorChoice::ALWAYS);
        Argument x;
        x.help = "甲";
        colored.add_argument({"--x"}, x);
        const std::string colored_text = colored.format_help(false);
        check("始终着色", contains(colored_text, "\x1b["));

        Parser mono("color");
        mono.with_color(ColorChoice::NEVER);
        Argument same;
        same.help = "甲";
        mono.add_argument({"--x"}, same);
        const std::string plain_text = mono.format_help(false);
        check("关着色无转义", !contains(plain_text, "\x1b["));
        check("着色不改版式", plain(colored_text) == plain_text);
        check("错误关着色无转义", !contains(rendered(run(mono, argv({"--y"}))), "\x1b["));
    }

    void test_help_print_and_exit() {
        Parser parser("demo");
        parser.with_version("1.0.0").with_color(ColorChoice::NEVER);
        Argument x;
        x.help = "甲";
        parser.add_argument({"--x"}, x);

        std::ostringstream printed;
        parser.print_help(printed, false);
        check("print_help 末尾换行", printed.str() == parser.format_help(false) + "\n");

        std::ostringstream out;
        std::ostringstream err;
        auto               outcome = parser.parse_or_exit(argv({"--help"}), out, err);
        check("帮助退出码 0", std::get_if<int>(&outcome) != nullptr && *std::get_if<int>(&outcome) == 0);
        check("帮助写 stdout", out.str() == parser.format_help(true) + "\n");
        check("帮助不写 stderr", err.str().empty());

        out.str("");
        err.str("");
        outcome = parser.parse_or_exit(argv({"-V"}), out, err);
        check("版本退出码 0", std::get_if<int>(&outcome) != nullptr && *std::get_if<int>(&outcome) == 0);
        check("版本行带换行", out.str() == "demo 1.0.0\n", out.str());

        out.str("");
        err.str("");
        outcome = parser.parse_or_exit(argv({"--y"}), out, err);
        check("用法错误退出码 2", std::get_if<int>(&outcome) != nullptr && *std::get_if<int>(&outcome) == 2);
        check("用法错误写 stderr", out.str().empty() && !err.str().empty());
        check("用法错误末尾换行", !err.str().empty() && err.str().back() == '\n');

        out.str("");
        err.str("");
        Parsed parsed = run(parser, argv({"--x", "1"}));
        outcome       = parser.parse_or_exit(argv({"--x", "1"}), out, err);
        check("成功返回命名空间", std::get_if<Namespace>(&outcome) != nullptr && parsed.namespace_value.has_value());
        check("成功不打印", out.str().empty() && err.str().empty());

        Parser tool("tool");
        tool.with_color(ColorChoice::NEVER);
        auto   &subparsers = tool.add_subparsers("command", "Commands", false);
        Parser &build      = subparsers.add_parser("build", "构建");
        out.str("");
        err.str("");
        outcome = tool.parse_or_exit(argv({"build", "--help"}), out, err);
        check("子命令帮助定位",
              std::get_if<int>(&outcome) != nullptr && out.str() == build.format_help(true) + "\n",
              out.str().substr(0, 30));
    }

}  // namespace

int main() {
    test_options_take_values_in_every_form();
    test_signed_numbers_are_values();
    test_nargs_shapes();
    test_positional_variadic_leaves_room();
    test_actions();
    test_converters_and_choices();
    test_unknown_option_suggests();
    test_ambiguous_prefix_lists_candidates();
    test_missing_value_and_required();
    test_validation_order();
    test_env_fallback();
    test_defaults_and_suppress();
    test_subcommands();
    test_help_subcommand();
    test_help_and_version_flags();
    test_parse_known_extras();
    test_remainder_boundary_rejects_option_token();
    test_from_file_prefix_expands_lines();
    test_overrides_with_keeps_the_last_one();
    test_auto_width_reads_columns();
    test_value_group_and_required_unless_present();
    test_definition_errors_are_rejected();
    test_duplicated_dest_across_commands();
    test_usage_spec_shapes();
    test_jaro_winkler_and_best_match();
    test_value_text_follows_python_rules();
    test_help_usage_composition();
    test_help_sections_and_content();
    test_help_long_choices();
    test_help_groups();
    test_help_alignment_and_wrapping();
    test_help_subcommand_globals();
    test_help_error_block();
    test_help_color();
    test_help_print_and_exit();
    if (failures != 0) {
        std::printf("%d 处断言失败\n", failures);
        return 1;
    }
    std::printf("argparser: 全部断言通过\n");
    return 0;
}
