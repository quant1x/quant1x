// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package argparser

// 补全生成的用例: 三种 shell 的脚本骨架、候选词与转义。
//
// 与 `complete_test.py` 的同名用例逐条对应; 期望值只由 Python 侧生成。

import (
	"strings"
	"testing"
)

const completeBash = `_argparser_demo() {
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
`

const completeZsh = `#compdef demo

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
`

const completeFish = `complete -c demo -s h -l help -d 'Print help'
complete -c demo -s v -l verbose -d 'verbose output'
complete -c demo -l mode -d 'mode'
complete -c demo -f -a alpha -d 'alpha command'
complete -c demo -f -a beta -d 'beta command'
complete -c demo -f -a 'x y'
complete -c demo -n '__fish_seen_subcommand_from alpha' -s h -l help -d 'Print help'
complete -c demo -n '__fish_seen_subcommand_from alpha' -l file -d 'input file'
complete -c demo -n '__fish_seen_subcommand_from beta' -s h -l help -d 'Print help'
complete -c demo -n '__fish_seen_subcommand_from beta' -l count
`

// completeTool 顶层三个选项 + 两条子命令, 子命令各带一个选项; 与 Python 用例的 `tool()` 同形。
func completeTool(t *testing.T) *Parser {
	t.Helper()
	parser := NewParser("demo").WithAbout("演示")
	mustAdd(t, parser, []string{"-v", "--verbose"}, Argument{Action: StoreTrue, Help: "verbose output", Global: true})
	mustAdd(t, parser, []string{"--mode"}, Argument{Help: "mode"})
	mustAdd(t, parser, []string{"--secret"}, Argument{Hide: true})
	mustAdd(t, parser, []string{"kind"}, Argument{Choices: []Choice{{Value: "x"}, {Value: "y"}}})
	subparsers, err := parser.AddSubparsers("", "Commands", false)
	if err != nil {
		t.Fatalf("登记子命令失败: %v", err)
	}
	alpha, err := subparsers.AddParser("alpha", "alpha command", nil, nil)
	if err != nil {
		t.Fatalf("登记 alpha 失败: %v", err)
	}
	mustAdd(t, alpha, []string{"--file"}, Argument{Help: "input file"})
	beta, err := subparsers.AddParser("beta", "beta command", nil, nil)
	if err != nil {
		t.Fatalf("登记 beta 失败: %v", err)
	}
	mustAdd(t, beta, []string{"--count"}, Argument{})
	return parser
}

func TestCompleteScriptsMatchThePythonSample(t *testing.T) {
	parser := completeTool(t)
	for _, sample := range []struct {
		shell string
		want  string
	}{
		{"bash", completeBash},
		{"zsh", completeZsh},
		{"fish", completeFish},
	} {
		script, err := Complete(parser, sample.shell, "")
		if err != nil {
			t.Fatalf("%s 生成失败: %v", sample.shell, err)
		}
		if script != sample.want {
			t.Fatalf("%s 脚本与 Python 样本不一致\n--- 实际 ---\n%s", sample.shell, script)
		}
	}
}

func TestCompleteProgOverrideAndFunctionName(t *testing.T) {
	parser := completeTool(t)
	script, err := Complete(parser, "bash", "my-tool")
	if err != nil {
		t.Fatalf("生成失败: %v", err)
	}
	if !strings.Contains(script, "complete -F _argparser_my_tool my-tool") {
		t.Fatalf("prog 未被覆盖: %s", script)
	}
	if !strings.HasPrefix(script, "_argparser_my_tool() {\n") {
		t.Fatalf("函数名未归一: %s", script)
	}
}

func TestCompleteRejectsUnknownShell(t *testing.T) {
	parser := completeTool(t)
	if _, err := Complete(parser, "powershell", ""); err == nil {
		t.Fatal("未知 shell 应返回错误")
	} else if !strings.Contains(err.Error(), "powershell") {
		t.Fatalf("错误消息里应带上 shell 名: %v", err)
	}
}

func TestCompleteWithoutSubparsersHasNoStateMachine(t *testing.T) {
	parser := NewParser("solo")
	script, err := Complete(parser, "bash", "")
	if err != nil {
		t.Fatalf("生成失败: %v", err)
	}
	if strings.Contains(script, "$cmd|$word") {
		t.Fatalf("无子命令时不该有状态机: %s", script)
	}
	if !strings.Contains(script, "        \"\") COMPREPLY=") {
		t.Fatalf("无子命令时仍应有顶层分支: %s", script)
	}
}

func TestCompleteSkipsEmptyHelp(t *testing.T) {
	parser := NewParser("blank")
	mustAdd(t, parser, []string{"--flag"}, Argument{Help: ""})
	if _, err := parser.AddSubparsers("", "Commands", false); err != nil {
		t.Fatalf("登记子命令失败: %v", err)
	}
	script, err := Complete(parser, "fish", "")
	if err != nil {
		t.Fatalf("生成失败: %v", err)
	}
	if strings.Contains(script, "-d ''") {
		t.Fatalf("空说明文字不该写 -d: %s", script)
	}
}
