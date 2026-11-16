// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package argparser

// 帮助与错误版式的用例: usage 组成、章节顺序、标注、折行与着色。
//
// 与 `help_test.py` 的同名用例逐条对应; 期望值只由 Python 侧生成。

import (
	"strings"
	"testing"
)

// section 取某个章节标题之后、下一个空行之前的行。
func section(t *testing.T, text, title string) []string {
	t.Helper()
	lines := []string{}
	found := false
	for _, line := range strings.Split(text, "\n") {
		if !found {
			if line == title {
				found = true
			}
			continue
		}
		if strings.TrimSpace(line) == "" {
			break
		}
		lines = append(lines, line)
	}
	return lines
}

// displayWidth 码点个数, 与 Python 的 `len()` 同口径。
func displayWidth(text string) int { return widthOf(text) }

// countOf 子串出现次数。
func countOf(text, want string) int { return strings.Count(text, want) }

// plain 去掉 ANSI 转义, 便于比较可见版式。
func plain(text string) string {
	var body strings.Builder
	for index := 0; index < len(text); index++ {
		if text[index] == 0x1b && index+1 < len(text) && text[index+1] == '[' {
			index += 2
			for index < len(text) && text[index] != 'm' {
				index++
			}
			continue
		}
		body.WriteByte(text[index])
	}
	return body.String()
}

func TestHelpUsageComposition(t *testing.T) {
	bare := NewParser("bare").WithColor(ColorNever)
	if got := bare.UsageSpec(); got != "bare" {
		t.Fatalf("裸命令 usage: %q", got)
	}

	opt := NewParser("opt").WithColor(ColorNever)
	mustAdd(t, opt, []string{"-a", "--all"}, Argument{Action: StoreTrue, Help: "全部"})
	mustAdd(t, opt, []string{"--out"}, Argument{Required: true, Help: "输出"})
	mustAdd(t, opt, []string{"files"}, Argument{Nargs: NargsOneOrMore(), Help: "文件"})
	if got := opt.UsageSpec(); got != "opt --out <OUT> [OPTIONS] <FILES>..." {
		t.Fatalf("选项与位置参数: %q", got)
	}

	cmd := NewParser("cmd").WithColor(ColorNever)
	if _, err := cmd.AddSubparsers("", "Commands", true); err != nil {
		t.Fatalf("登记子命令失败: %v", err)
	}
	if got := cmd.UsageSpec(); got != "cmd <COMMAND>" {
		t.Fatalf("必填子命令: %q", got)
	}

	optional := NewParser("cmd2").WithColor(ColorNever)
	if _, err := optional.AddSubparsers("", "Commands", false); err != nil {
		t.Fatalf("登记子命令失败: %v", err)
	}
	if got := optional.UsageSpec(); got != "cmd2 [COMMAND]" {
		t.Fatalf("可选子命令: %q", got)
	}

	own := NewParser("own").WithColor(ColorNever).WithUsage("own <自定义>")
	mustAdd(t, own, []string{"--x"}, Argument{Help: "忽略"})
	if got := own.UsageSpec(); got != "own <自定义>" {
		t.Fatalf("自定义 usage: %q", got)
	}
	if got := own.FormatUsage(); got != "Usage: own <自定义>" {
		t.Fatalf("format_usage: %q", got)
	}
}

// helpTool 帮助章节复用的命令: 简介、长简介、分组、子命令齐全。
func helpTool(t *testing.T) *Parser {
	t.Helper()
	parser := NewParser("tool").
		WithAbout("工具集").
		WithLongAbout("工具集的完整说明, 只在 --help 里出现。").
		WithEpilog("示例: tool build -j 8").
		WithVersion("1.0.0").
		WithColor(ColorNever)
	mustAdd(t, parser, []string{"-v", "--verbose"}, Argument{Action: Count, Help: "输出更详细"})
	mustAdd(t, parser, []string{"--level"}, Argument{
		Choices: []Choice{{Value: "debug"}, {Value: "info"}, {Value: "warn"}},
		Default: ValueOf("info"),
		Help:    "日志级别",
	})
	mustAdd(t, parser, []string{"--jobs"}, Argument{
		Type:    typeParser(ParseInt(nil, nil)),
		Default: ValueOf(int64(4)),
		Env:     "TOOL_JOBS",
		Help:    "并发数",
	})
	mustAdd(t, parser, []string{"--secret"}, Argument{Hide: true, Help: "不展示"})
	mustAdd(t, parser, []string{"--quiet"}, Argument{Action: StoreTrue, HideDefault: true, Help: "安静"})
	mustAdd(t, parser, []string{"src"}, Argument{Nargs: NargsZeroOrMore(), Help: "源文件"})
	subparsers, err := parser.AddSubparsers("command", "Commands", false)
	if err != nil {
		t.Fatalf("登记子命令失败: %v", err)
	}
	if _, err := subparsers.AddParser("build", "构建", nil, []string{"b"}); err != nil {
		t.Fatalf("登记 build 失败: %v", err)
	}
	if _, err := subparsers.AddParser("run", "运行", nil, nil); err != nil {
		t.Fatalf("登记 run 失败: %v", err)
	}
	return parser
}

func TestHelpSectionsAndContent(t *testing.T) {
	parser := helpTool(t)
	text := parser.FormatHelp(false)
	if !strings.HasPrefix(text, "工具集\n") {
		t.Fatalf("简介在最前: %q", text)
	}
	usageAt := strings.Index(text, "Usage:")
	commandsAt := strings.Index(text, "Commands:")
	argumentsAt := strings.Index(text, "Arguments:")
	optionsAt := strings.Index(text, "Options:")
	if !(usageAt < commandsAt && commandsAt < argumentsAt && argumentsAt < optionsAt) {
		t.Fatalf("章节顺序错乱: %d %d %d %d", usageAt, commandsAt, argumentsAt, optionsAt)
	}
	if !strings.HasSuffix(text, "示例: tool build -j 8") {
		t.Fatalf("epilog 收尾: %q", text[maxInt(0, len(text)-40):])
	}
	if strings.Contains(text, "--secret") {
		t.Fatalf("隐藏参数不出现")
	}
	for _, want := range []string{
		"-v, --verbose...",
		"[default: info]",
		"[env: TOOL_JOBS]",
		"[possible values: debug, info, warn]",
		"安静",
		"-V, --version",
		"-h, --help",
	} {
		if !strings.Contains(text, want) {
			t.Fatalf("缺少 %q:\n%s", want, text)
		}
	}
	if strings.Contains(text, "[default: false]") {
		t.Fatalf("开关不标默认值")
	}

	longText := parser.FormatHelp(true)
	if !strings.HasPrefix(longText, "工具集的完整说明") {
		t.Fatalf("长简介顶替简介: %q", longText[:minInt(40, len(longText))])
	}
	if countOf(longText, "工具集") != 1 {
		t.Fatalf("长帮助不重复简介: %d", countOf(longText, "工具集"))
	}

	options := section(t, text, "Options:")
	if len(options) != 6 {
		t.Fatalf("选项行数: %d %v", len(options), options)
	}
	if !strings.Contains(options[len(options)-2], "Print help") {
		t.Fatalf("帮助在倒数第二行: %v", options)
	}
	if !strings.Contains(options[len(options)-1], "Print version") {
		t.Fatalf("版本在最后一行: %v", options)
	}

	commands := section(t, text, "Commands:")
	if len(commands) == 0 || !strings.HasPrefix(strings.TrimSpace(commands[0]), "build, b") {
		t.Fatalf("子命令带别名: %v", commands)
	}
	last := commands[len(commands)-1]
	if !strings.HasPrefix(strings.TrimSpace(last), "help") || !strings.Contains(last, "Print this message") {
		t.Fatalf("help 子命令在最后: %v", commands)
	}

	positionals := section(t, text, "Arguments:")
	if len(positionals) == 0 || !strings.HasPrefix(strings.TrimSpace(positionals[0]), "[SRC]...") {
		t.Fatalf("位置参数左列: %v", positionals)
	}
}

func TestHelpLongChoices(t *testing.T) {
	parser := NewParser("level").WithColor(ColorNever)
	mustAdd(t, parser, []string{"--level"}, Argument{
		Choices:  []Choice{{Value: "debug", Help: "逐条打印"}, {Value: "info", Help: "常规"}},
		Default:  ValueOf("info"),
		Help:     "级别",
		LongHelp: "日志级别, 影响输出量",
	})
	shortText := parser.FormatHelp(false)
	if strings.Contains(shortText, "影响输出量") {
		t.Fatalf("短帮助不该用长说明")
	}
	if !strings.Contains(shortText, "级别") {
		t.Fatalf("短帮助缺短文案")
	}
	longText := parser.FormatHelp(true)
	if !strings.Contains(longText, "影响输出量") {
		t.Fatalf("长帮助缺长说明")
	}
	if !strings.Contains(longText, "- debug: 逐条打印") {
		t.Fatalf("长帮助缺可选值说明:\n%s", longText)
	}
}

func TestHelpGroups(t *testing.T) {
	parser := NewParser("grouped").WithColor(ColorNever)
	group := parser.AddGroup("Database", "数据库相关")
	if _, err := group.AddArgument([]string{"--host"}, Argument{
		Default: ValueOf("localhost"),
		Help:    "主机",
	}); err != nil {
		t.Fatalf("登记 --host 失败: %v", err)
	}
	if _, err := group.AddArgument([]string{"--port"}, Argument{
		Type:    typeParser(ParseInt(nil, nil)),
		Default: ValueOf(int64(5432)),
		Help:    "端口",
	}); err != nil {
		t.Fatalf("登记 --port 失败: %v", err)
	}
	mustAdd(t, parser, []string{"--debug"}, Argument{Action: StoreTrue, Help: "调试"})

	text := parser.FormatHelp(false)
	if !strings.Contains(text, "Database:") || !strings.Contains(text, "数据库相关") {
		t.Fatalf("分组标题或说明缺失:\n%s", text)
	}
	if strings.Index(text, "--host") < strings.Index(text, "Database:") {
		t.Fatalf("分组参数应在分组下")
	}
	if strings.Index(text, "--debug") > strings.Index(text, "Database:") {
		t.Fatalf("普通参数应在分组前")
	}
}

func TestHelpAlignmentAndWrapping(t *testing.T) {
	aligned := NewParser("align").WithColor(ColorNever)
	mustAdd(t, aligned, []string{"-a", "--all"}, Argument{Help: "全部"})
	mustAdd(t, aligned, []string{"--beta"}, Argument{Metavar: "BETA", Help: "乙"})
	text := aligned.FormatHelp(false)
	rows := section(t, text, "Options:")
	if len(rows) < 2 {
		t.Fatalf("选项行数: %v", rows)
	}
	firstAt := strings.Index(rows[0], "全部")
	secondAt := strings.Index(rows[1], "乙")
	if firstAt < 0 || firstAt != secondAt {
		t.Fatalf("说明列不对齐: %q | %q", rows[0], rows[1])
	}
	if !strings.HasPrefix(rows[0], "  -a, --all") {
		t.Fatalf("左列缩进: %q", rows[0])
	}

	wrap := NewParser("wrap").WithColor(ColorNever).WithMaxWidth(44)
	mustAdd(t, wrap, []string{"--long"}, Argument{
		Metavar: "VALUE",
		Help:    "这是一段足够长的说明文字, 必须在窄幅下折行并保持对齐缩进",
	})
	narrow := wrap.FormatHelp(false)
	optionRows := section(t, narrow, "Options:")
	if len(optionRows) < 2 {
		t.Fatalf("窄幅应当折行: %v", optionRows)
	}
	if !strings.HasPrefix(optionRows[1], strings.Repeat(" ", 18)) {
		t.Fatalf("续行缩进 18 列: %q", optionRows[1])
	}
	for _, line := range strings.Split(narrow, "\n") {
		if displayWidth(line) > 44 {
			t.Fatalf("整幅超宽: %q", line)
		}
	}
}

func TestHelpSubcommandGlobals(t *testing.T) {
	parser := NewParser("tool").WithColor(ColorNever)
	mustAdd(t, parser, []string{"-q", "--quiet"}, Argument{Action: StoreTrue, Global: true, Help: "安静模式"})
	subparsers, err := parser.AddSubparsers("command", "Commands", false)
	if err != nil {
		t.Fatalf("登记子命令失败: %v", err)
	}
	build, err := subparsers.AddParser("build", "构建", nil, nil)
	if err != nil {
		t.Fatalf("登记 build 失败: %v", err)
	}
	if _, err := build.AddArgument([]string{"-j", "--jobs"}, Argument{
		Type:    typeParser(ParseInt(nil, nil)),
		Default: ValueOf(int64(4)),
		Help:    "并发数",
	}); err != nil {
		t.Fatalf("登记 --jobs 失败: %v", err)
	}
	child := parser.Subparsers().Lookup("build")
	if child == nil {
		t.Fatalf("子命令 build 未登记")
	}
	childText := child.FormatHelp(false)
	if !strings.Contains(childText, "Usage: tool build [OPTIONS]") {
		t.Fatalf("子命令 usage:\n%s", childText)
	}
	if !strings.Contains(childText, "-q, --quiet") {
		t.Fatalf("子命令缺继承的全局参数:\n%s", childText)
	}
	if !strings.Contains(childText, "-j, --jobs") {
		t.Fatalf("子命令缺自身参数:\n%s", childText)
	}

	nested := NewParser("nested").WithColor(ColorNever)
	nestedCommands, err := nested.AddSubparsers("command", "Commands", false)
	if err != nil {
		t.Fatalf("登记子命令失败: %v", err)
	}
	nestedBuild, err := nestedCommands.AddParser("build", "构建", nil, nil)
	if err != nil {
		t.Fatalf("登记 build 失败: %v", err)
	}
	targets, err := nestedBuild.AddSubparsers("target", "Targets", false)
	if err != nil {
		t.Fatalf("登记目标失败: %v", err)
	}
	if _, err := targets.AddParser("release", "发布版", nil, nil); err != nil {
		t.Fatalf("登记 release 失败: %v", err)
	}
	if !strings.Contains(nestedBuild.FormatHelp(false), "Print this message") {
		t.Fatalf("嵌套子命令应列出 help")
	}

	_, err = parser.Parse([]string{"build", "--help"})
	requested, ok := err.(*HelpRequested)
	if !ok || !requested.Long || requested.Parser.Prog() != "tool build" {
		t.Fatalf("长帮助请求: %v", err)
	}
	_, err = parser.Parse([]string{"-h"})
	brief, ok := err.(*HelpRequested)
	if !ok || brief.Long || brief.Parser.Prog() != "tool" {
		t.Fatalf("短帮助请求: %v", err)
	}
}

func TestHelpErrorBlock(t *testing.T) {
	parser := NewParser("demo").WithAbout("演示").WithColor(ColorNever)
	mustAdd(t, parser, []string{"--level"}, Argument{
		Choices: []Choice{{Value: "debug"}, {Value: "info"}, {Value: "warn"}},
		Help:    "级别",
	})
	errorText := failure(t, parser, []string{"--level", "warnn"}).Error()
	if !strings.HasPrefix(errorText, "error: invalid value 'warnn' for '--level <LEVEL>'") {
		t.Fatalf("错误首行: %q", errorText)
	}
	for _, want := range []string{
		"\n  [possible values: debug, info, warn]",
		"\n  tip: a similar value exists: 'warn'",
		"\nUsage: demo [OPTIONS]",
	} {
		if !strings.Contains(errorText, want) {
			t.Fatalf("缺少 %q:\n%s", want, errorText)
		}
	}
	if !strings.HasSuffix(errorText, "For more information, try '--help'.") {
		t.Fatalf("错误帮助提示: %q", errorText[maxInt(0, len(errorText)-40):])
	}

	noHelp := NewParser("nohelp").WithColor(ColorNever).WithoutHelp()
	mustAdd(t, noHelp, []string{"--x"}, Argument{Help: "甲"})
	if strings.Contains(failure(t, noHelp, []string{"--y"}).Error(), "For more information") {
		t.Fatalf("无帮助选项时不该提帮助")
	}

	custom := &UsageError{
		Message: "自定义",
		Reason:  "原因",
		Lines:   []string{"明细"},
		Tips:    []string{"建议"},
		Usage:   "demo",
	}
	if got := custom.Error(); !strings.HasPrefix(got, "error: 自定义: 原因") {
		t.Fatalf("非着色版式: %q", got)
	}
	if got := RenderError(custom, true); !strings.HasPrefix(got, "\x1b[31merror:\x1b[0m") {
		t.Fatalf("着色版式: %q", got)
	}
}

func TestHelpColor(t *testing.T) {
	colored := NewParser("color").WithColor(ColorAlways)
	mustAdd(t, colored, []string{"--x"}, Argument{Help: "甲"})
	coloredText := colored.FormatHelp(false)
	if !strings.Contains(coloredText, "\x1b[") {
		t.Fatalf("始终着色: %q", coloredText)
	}

	mono := NewParser("color").WithColor(ColorNever)
	mustAdd(t, mono, []string{"--x"}, Argument{Help: "甲"})
	plainText := mono.FormatHelp(false)
	if strings.Contains(plainText, "\x1b[") {
		t.Fatalf("关着色不该有转义")
	}
	if plain(coloredText) != plainText {
		t.Fatalf("着色不该改版式")
	}
	if strings.Contains(failure(t, mono, []string{"--y"}).Error(), "\x1b[") {
		t.Fatalf("错误关着色不该有转义")
	}
}

func TestHelpPrintAndExit(t *testing.T) {
	parser := NewParser("demo").WithVersion("1.0.0").WithColor(ColorNever)
	mustAdd(t, parser, []string{"--x"}, Argument{Help: "甲"})

	var printed strings.Builder
	parser.PrintHelp(&printed, false)
	if printed.String() != parser.FormatHelp(false)+"\n" {
		t.Fatalf("print_help 末尾换行: %q", printed.String())
	}

	var out, err strings.Builder
	namespace, code := parser.ParseOrExit([]string{"--help"}, &out, &err)
	if namespace != nil || code != 0 {
		t.Fatalf("帮助退出码 0: %v %d", namespace, code)
	}
	if out.String() != parser.FormatHelp(true)+"\n" {
		t.Fatalf("帮助写 out: %q", out.String())
	}
	if err.Len() != 0 {
		t.Fatalf("帮助不该写 err: %q", err.String())
	}

	out.Reset()
	err.Reset()
	if _, code = parser.ParseOrExit([]string{"-V"}, &out, &err); code != 0 {
		t.Fatalf("版本退出码 0: %d", code)
	}
	if out.String() != "demo 1.0.0\n" {
		t.Fatalf("版本行带换行: %q", out.String())
	}

	out.Reset()
	err.Reset()
	if _, code = parser.ParseOrExit([]string{"--y"}, &out, &err); code != 2 {
		t.Fatalf("用法错误退出码 2: %d", code)
	}
	if out.Len() != 0 || err.Len() == 0 {
		t.Fatalf("用法错误写 err: %q", err.String())
	}
	if !strings.HasSuffix(err.String(), "\n") {
		t.Fatalf("用法错误末尾换行: %q", err.String())
	}

	out.Reset()
	err.Reset()
	namespace, code = parser.ParseOrExit([]string{"--x", "1"}, &out, &err)
	if namespace == nil || code != 0 {
		t.Fatalf("成功返回命名空间: %v %d", namespace, code)
	}
	if out.Len() != 0 || err.Len() != 0 {
		t.Fatalf("成功不打印")
	}

	tool := NewParser("tool").WithColor(ColorNever)
	subparsers, addErr := tool.AddSubparsers("command", "Commands", false)
	if addErr != nil {
		t.Fatalf("登记子命令失败: %v", addErr)
	}
	build, addErr := subparsers.AddParser("build", "构建", nil, nil)
	if addErr != nil {
		t.Fatalf("登记 build 失败: %v", addErr)
	}
	out.Reset()
	err.Reset()
	if _, code = tool.ParseOrExit([]string{"build", "--help"}, &out, &err); code != 0 {
		t.Fatalf("子命令帮助退出码 0: %d", code)
	}
	if out.String() != build.FormatHelp(true)+"\n" {
		t.Fatalf("子命令帮助写 out: %q", out.String())
	}
}

// mustAdd 登记参数, 失败即结束用例。
func mustAdd(t *testing.T, parser *Parser, names []string, argument Argument) *Argument {
	t.Helper()
	added, err := parser.AddArgument(names, argument)
	if err != nil {
		t.Fatalf("登记 %v 失败: %v", names, err)
	}
	return added
}

func minInt(left, right int) int {
	if left < right {
		return left
	}
	return right
}

func maxInt(left, right int) int {
	if left > right {
		return left
	}
	return right
}
