// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <quant1x/base/argparser/complete.hpp>
#include <quant1x/base/argparser/parser.hpp>
#include <quant1x/base/argparser/spec.hpp>

namespace {

    using argparser::Argument;
    using argparser::Choice;
    using argparser::Parser;
    using argparser::Value;

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

    /// 顶层三个选项 + 两条子命令, 子命令各带一个选项; 与 Python 用例的 `tool()` 同形。
    void build_tool(Parser &parser) {
        parser.with_about("演示");
        Argument verbose;
        verbose.action = argparser::Action::STORE_TRUE;
        verbose.help   = "verbose output";
        verbose.global = true;
        parser.add_argument({"-v", "--verbose"}, verbose);
        Argument mode;
        mode.help = "mode";
        parser.add_argument({"--mode"}, mode);
        Argument secret;
        secret.hide = true;
        parser.add_argument({"--secret"}, secret);
        Argument kind;
        kind.choices = std::vector<Choice>{{Value("x"), ""}, {Value("y"), ""}};
        parser.add_argument({"kind"}, kind);
        argparser::SubParsers &subs = parser.add_subparsers();
        Argument               file;
        file.help = "input file";
        subs.add_parser("alpha", "alpha command").add_argument({"--file"}, file);
        Argument count;
        count.type = argparser::parse_int();
        subs.add_parser("beta", "beta command").add_argument({"--count"}, count);
    }

    const char *const BASH = R"BASH(_argparser_demo() {
    local cur cmd word
    cur="${COMP_WORDS[COMP_CWORD]}"
    cmd=""
    for word in "${COMP_WORDS[@]:1:COMP_CWORD-1}"; do
        case "$cmd|$word" in
            "|alpha") cmd="alpha" ;;
            "|beta") cmd="beta" ;;
        esac
    done
    case "$cmd" in
        "") COMPREPLY=( $(compgen -W "-h --help -v --verbose --mode alpha beta x y" -- "$cur") ) ;;
        "alpha") COMPREPLY=( $(compgen -W "-h --help --file -v --verbose" -- "$cur") ) ;;
        "beta") COMPREPLY=( $(compgen -W "-h --help --count -v --verbose" -- "$cur") ) ;;
    esac
}
complete -F _argparser_demo demo
)BASH";

    const char *const ZSH = R"ZSH(#compdef demo

_argparser_demo() {
    local -a candidates
    local cmd="" word
    for word in ${words[2,CURRENT-1]}; do
        case "$cmd|$word" in
            "|alpha") cmd="alpha" ;;
            "|beta") cmd="beta" ;;
        esac
    done
    case "$cmd" in
        "") candidates=(-h --help -v --verbose --mode alpha beta x y) ;;
        "alpha") candidates=(-h --help --file -v --verbose) ;;
        "beta") candidates=(-h --help --count -v --verbose) ;;
    esac
    compadd -- "${candidates[@]}"
}

_argparser_demo "$@"
)ZSH";

    const char *const FISH = R"FISH(complete -c demo -s h -l help -d 'Print help'
complete -c demo -s v -l verbose -d 'verbose output'
complete -c demo -l mode -d 'mode'
complete -c demo -f -a alpha -d 'alpha command'
complete -c demo -f -a beta -d 'beta command'
complete -c demo -f -a 'x y'
complete -c demo -n '__fish_seen_subcommand_from alpha' -s h -l help -d 'Print help'
complete -c demo -n '__fish_seen_subcommand_from alpha' -l file -d 'input file'
complete -c demo -n '__fish_seen_subcommand_from beta' -s h -l help -d 'Print help'
complete -c demo -n '__fish_seen_subcommand_from beta' -l count
)FISH";

    void test_scripts_match_the_python_sample() {
        Parser parser("demo");
        build_tool(parser);
        check("bash 脚本与 Python 样本一致",
              argparser::complete(parser, "bash") == BASH,
              argparser::complete(parser, "bash"));
        check("zsh 脚本与 Python 样本一致",
              argparser::complete(parser, "zsh") == ZSH,
              argparser::complete(parser, "zsh"));
        check("fish 脚本与 Python 样本一致",
              argparser::complete(parser, "fish") == FISH,
              argparser::complete(parser, "fish"));
    }

    void test_prog_override_and_function_name() {
        Parser parser("demo");
        build_tool(parser);
        const std::string script = argparser::complete(parser, "bash", std::string("my-tool"));
        check("prog 可被覆盖", script.find("complete -F _argparser_my_tool my-tool") != std::string::npos, script);
        check("函数名与 prog 归一", script.rfind("_argparser_my_tool() {\n", 0) == 0, script);
    }

    void test_unknown_shell_is_rejected() {
        Parser parser("demo");
        build_tool(parser);
        bool rejected = false;
        try {
            argparser::complete(parser, "powershell");
        } catch (const std::invalid_argument &error) {
            rejected = std::string(error.what()).find("powershell") != std::string::npos;
        }
        check("未知 shell 抛 invalid_argument", rejected);
    }

    void test_command_without_subparsers_has_no_state_machine() {
        Parser            parser("solo");
        const std::string script = argparser::complete(parser, "bash");
        check("无子命令时不生成状态机", script.find("$cmd|$word") == std::string::npos, script);
        check("无子命令时仍有顶层分支", script.find("        \"\") COMPREPLY=") != std::string::npos, script);
    }

    void test_empty_help_writes_no_detail() {
        Parser   parser("blank");
        Argument flag;
        flag.help = std::string();
        parser.add_argument({"--flag"}, flag);
        argparser::SubParsers &subs = parser.add_subparsers();
        subs.add_parser("child");
        const std::string script = argparser::complete(parser, "fish");
        check("空说明文字不写 -d", script.find("-d ''") == std::string::npos, script);
    }

}  // namespace

int main() {
    test_scripts_match_the_python_sample();
    test_prog_override_and_function_name();
    test_unknown_shell_is_rejected();
    test_command_without_subparsers_has_no_state_machine();
    test_empty_help_writes_no_detail();
    if (failures != 0) {
        std::printf("%d 处断言失败\n", failures);
        return 1;
    }
    std::printf("argparser complete: 全部断言通过\n");
    return 0;
}
