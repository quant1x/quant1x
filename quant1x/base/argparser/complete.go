// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package argparser

// 由命令定义生成 bash / zsh / fish 的补全脚本。
//
// 候选词只看「已敲定的子命令链」: 选项名、子命令名、位置参数的 `choices`;
// 不区分「正在写取值」的语境, 也不补文件路径。

import (
	"fmt"
	"slices"
	"strings"
	"unicode"
)

// Shells 支持的 shell, 按登记顺序。
var Shells = []string{"bash", "zsh", "fish"}

// Complete 生成 `shell` 的补全脚本; `prog` 为空时取 `parser.Prog()`。
//
// `shell` 不是 bash / zsh / fish 之一时返回错误。
func Complete(parser *Parser, shell, prog string) (string, error) {
	if !slices.Contains(Shells, shell) {
		return "", fmt.Errorf("不支持的 shell: %s", shell)
	}
	name := prog
	if name == "" {
		name = parser.Prog()
	}
	nodes := walkCommands(parser)
	switch shell {
	case "bash":
		return bashScript(name, nodes), nil
	case "zsh":
		return zshScript(name, nodes), nil
	}
	return fishScript(name, nodes), nil
}

// commandNode 一个命令在状态机里的位置: 子命令链与命令。
type commandNode struct {
	chain  []string
	parser *Parser
}

// walkCommands 按前序收集 (子命令链, 命令); 链为空的是顶层命令。
func walkCommands(parser *Parser) []commandNode {
	nodes := []commandNode{}
	var walk func(chain []string, current *Parser)
	walk = func(chain []string, current *Parser) {
		nodes = append(nodes, commandNode{chain: append([]string{}, chain...), parser: current})
		subparsers := current.Subparsers()
		if subparsers == nil {
			return
		}
		for _, command := range subparsers.Commands() {
			walk(append(chain, command.Name), command.Parser)
		}
	}
	walk(nil, parser)
	return nodes
}

// completeWords 一个命令的候选词: 选项名、子命令名、位置参数的 `choices`, 按登记顺序去重。
func completeWords(parser *Parser) []string {
	collected := []string{}
	for _, argument := range parser.Arguments() {
		if !argument.Hide {
			collected = append(collected, argument.Names...)
		}
	}
	for _, argument := range globalArguments(parser) {
		if !argument.Hide {
			collected = append(collected, argument.Names...)
		}
	}
	if subparsers := parser.Subparsers(); subparsers != nil {
		for _, command := range subparsers.Commands() {
			collected = append(collected, command.Name)
		}
	}
	for _, argument := range parser.Positionals() {
		for _, value := range argument.ChoiceValues() {
			collected = append(collected, fmt.Sprint(value))
		}
	}
	unique := []string{}
	for _, word := range collected {
		if word == "" || hasSpace(word) || slices.Contains(unique, word) {
			continue
		}
		unique = append(unique, word)
	}
	return unique
}

// completionFunction 脚本里的函数名: 非标识符字符一律换成下划线。
func completionFunction(name string) string {
	var builder strings.Builder
	builder.WriteString("_argparser_")
	for _, char := range name {
		if char == '_' || unicode.IsLetter(char) || unicode.IsDigit(char) {
			builder.WriteRune(char)
		} else {
			builder.WriteRune('_')
		}
	}
	return builder.String()
}

// completeTransitions 状态机里的每条边: 键形如 `|alpha`、`alpha|beta`, 值是命中后的命令链文本。
func completeTransitions(nodes []commandNode) [][2]string {
	edges := [][2]string{}
	for _, node := range nodes {
		if len(node.chain) == 0 {
			continue
		}
		key := strings.Join(node.chain[:len(node.chain)-1], " ") + "|" + node.chain[len(node.chain)-1]
		edges = append(edges, [2]string{key, strings.Join(node.chain, " ")})
	}
	return edges
}

// bashScript 生成 bash 补全脚本: 先按已输入词推进到命令链, 再按链给出候选词。
func bashScript(name string, nodes []commandNode) string {
	function := completionFunction(name)
	var out strings.Builder
	fmt.Fprintf(&out, "%s() {\n    local cur cmd word\n", function)
	out.WriteString("    cur=\"${COMP_WORDS[COMP_CWORD]}\"\n    cmd=\"\"\n")
	if edges := completeTransitions(nodes); len(edges) > 0 {
		out.WriteString("    for word in \"${COMP_WORDS[@]:1:COMP_CWORD-1}\"; do\n")
		out.WriteString("        case \"$cmd|$word\" in\n")
		for _, edge := range edges {
			fmt.Fprintf(&out, "            \"%s\") cmd=\"%s\" ;;\n", edge[0], edge[1])
		}
		out.WriteString("        esac\n    done\n")
	}
	out.WriteString("    case \"$cmd\" in\n")
	for _, node := range nodes {
		fmt.Fprintf(&out, "        \"%s\") COMPREPLY=( $(compgen -W \"%s\" -- \"$cur\") ) ;;\n",
			strings.Join(node.chain, " "), strings.Join(completeWords(node.parser), " "))
	}
	out.WriteString("    esac\n}\n")
	fmt.Fprintf(&out, "complete -F %s %s\n", function, name)
	return out.String()
}

// zshScript 生成 zsh 补全脚本: 结构同 bash, 候选走 compadd。
func zshScript(name string, nodes []commandNode) string {
	function := completionFunction(name)
	var out strings.Builder
	fmt.Fprintf(&out, "#compdef %s\n\n%s() {\n    local -a candidates\n    local cmd=\"\" word\n", name, function)
	if edges := completeTransitions(nodes); len(edges) > 0 {
		out.WriteString("    for word in ${words[2,CURRENT-1]}; do\n")
		out.WriteString("        case \"$cmd|$word\" in\n")
		for _, edge := range edges {
			fmt.Fprintf(&out, "            \"%s\") cmd=\"%s\" ;;\n", edge[0], edge[1])
		}
		out.WriteString("        esac\n    done\n")
	}
	out.WriteString("    case \"$cmd\" in\n")
	for _, node := range nodes {
		fmt.Fprintf(&out, "        \"%s\") candidates=(%s) ;;\n",
			strings.Join(node.chain, " "), strings.Join(completeWords(node.parser), " "))
	}
	out.WriteString("    esac\n    compadd -- \"${candidates[@]}\"\n}\n\n")
	fmt.Fprintf(&out, "%s \"$@\"\n", function)
	return out.String()
}

// fishScript 生成 fish 补全脚本: 每个选项一条 complete -c, 子命令用 __fish_seen_subcommand_from 限定。
func fishScript(name string, nodes []commandNode) string {
	var out strings.Builder
	for _, node := range nodes {
		condition := ""
		if len(node.chain) > 0 {
			condition = " -n '__fish_seen_subcommand_from " + node.chain[len(node.chain)-1] + "'"
		}
		for _, argument := range node.parser.Arguments() {
			if argument.Hide || argument.IsPositional() {
				continue
			}
			spelled := []string{}
			for _, flag := range argument.Names {
				if word, ok := fishFlag(flag); ok {
					spelled = append(spelled, word)
				}
			}
			if len(spelled) == 0 {
				continue
			}
			detail := ""
			if argument.Help != "" {
				detail = " -d '" + shellQuote(argument.Help) + "'"
			}
			fmt.Fprintf(&out, "complete -c %s%s %s%s\n", name, condition, strings.Join(spelled, " "), detail)
		}
		if subparsers := node.parser.Subparsers(); subparsers != nil {
			for _, command := range subparsers.Commands() {
				detail := ""
				if about := command.Parser.About(); about != "" {
					detail = " -d '" + shellQuote(about) + "'"
				}
				fmt.Fprintf(&out, "complete -c %s%s -f -a %s%s\n", name, condition, command.Name, detail)
			}
		}
		spelled := []string{}
		for _, argument := range node.parser.Positionals() {
			for _, value := range argument.ChoiceValues() {
				word := fmt.Sprint(value)
				if !hasSpace(word) {
					spelled = append(spelled, word)
				}
			}
		}
		if len(spelled) > 0 {
			fmt.Fprintf(&out, "complete -c %s%s -f -a '%s'\n", name, condition, strings.Join(spelled, " "))
		}
	}
	return out.String()
}

// fishFlag fish 的单选项拼法: `-h` → `-s h`, `--help` → `-l help`; 其余形态不补。
func fishFlag(flag string) (string, bool) {
	if strings.HasPrefix(flag, "--") && len(flag) > 2 {
		return "-l " + flag[2:], true
	}
	if len(flag) == 2 && strings.HasPrefix(flag, "-") {
		return "-s " + flag[1:], true
	}
	return "", false
}

// shellQuote 把说明文字放进单引号: 反斜杠与单引号各转义一层。
func shellQuote(text string) string {
	return strings.ReplaceAll(strings.ReplaceAll(text, "\\", "\\\\"), "'", "\\'")
}

// hasSpace 含空白字符。
func hasSpace(text string) bool {
	for _, char := range text {
		if unicode.IsSpace(char) {
			return true
		}
	}
	return false
}
