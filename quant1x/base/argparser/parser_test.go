// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package argparser

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// parseOK 解析并断言成功。
func parseOK(t *testing.T, parser *Parser, argv []string) *Namespace {
	t.Helper()
	namespace, err := parser.Parse(argv)
	if err != nil {
		t.Fatalf("%v 解析失败: %v", argv, err)
	}
	return namespace
}

// parseErr 解析并断言失败。
func parseErr(t *testing.T, parser *Parser, argv []string) error {
	t.Helper()
	_, err := parser.Parse(argv)
	if err == nil {
		t.Fatalf("%v 应当报错, 实际解析成功", argv)
	}
	return err
}

// failure 断言用法错误并返回它。
func failure(t *testing.T, parser *Parser, argv []string) *UsageError {
	t.Helper()
	usage, ok := parseErr(t, parser, argv).(*UsageError)
	if !ok {
		t.Fatalf("%v 应当是用法错误", argv)
	}
	return usage
}

// base 本节复用的命令: 计数 / 必填取值 / 可选值 / 位置参数。
func base(t *testing.T) *Parser {
	t.Helper()
	parser := NewParser("demo").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"-v", "--verbose"}, Argument{Action: Count}); err != nil {
		t.Fatalf("登记 --verbose 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"-o", "--output"}, Argument{Required: true}); err != nil {
		t.Fatalf("登记 --output 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--level"}, Argument{
		Choices: []Choice{{Value: "debug"}, {Value: "info"}, {Value: "warn"}},
		Default: ValueOf("info"),
	}); err != nil {
		t.Fatalf("登记 --level 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"files"}, Argument{Nargs: NargsOneOrMore()}); err != nil {
		t.Fatalf("登记 files 失败: %v", err)
	}
	return parser
}

// tool 两级子命令: tool → build / run, build → release / debug。
func tool(t *testing.T) *Parser {
	t.Helper()
	parser := NewParser("tool").WithVersion("9.9.9").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"-q", "--quiet"}, Argument{Action: StoreTrue, Global: true}); err != nil {
		t.Fatalf("登记 --quiet 失败: %v", err)
	}
	subparsers, err := parser.AddSubparsers("command", "Commands", true)
	if err != nil {
		t.Fatalf("登记子命令失败: %v", err)
	}
	build, err := subparsers.AddParser("build", "构建", nil, []string{"b"})
	if err != nil {
		t.Fatalf("登记 build 失败: %v", err)
	}
	if _, err := build.AddArgument([]string{"-j", "--jobs"}, Argument{
		Type:    typeParser(ParseInt(nil, nil)),
		Default: ValueOf(int64(4)),
	}); err != nil {
		t.Fatalf("登记 --jobs 失败: %v", err)
	}
	targets, err := build.AddSubparsers("target", "Targets", false)
	if err != nil {
		t.Fatalf("登记 target 失败: %v", err)
	}
	if _, err := targets.AddParser("release", "发布版", nil, nil); err != nil {
		t.Fatalf("登记 release 失败: %v", err)
	}
	if _, err := targets.AddParser("debug", "调试版", nil, nil); err != nil {
		t.Fatalf("登记 debug 失败: %v", err)
	}
	run, err := subparsers.AddParser("run", "运行", []string{"r"}, nil)
	if err != nil {
		t.Fatalf("登记 run 失败: %v", err)
	}
	if _, err := run.AddArgument([]string{"script"}, Argument{}); err != nil {
		t.Fatalf("登记 script 失败: %v", err)
	}
	return parser
}

// typeParser 取转换器的地址。
func typeParser(parser ValueParser) *ValueParser { return &parser }

// text 取命名空间里某个名字的文本形态。
func text(t *testing.T, namespace *Namespace, key string) string {
	t.Helper()
	value, ok := namespace.Get(key)
	if !ok {
		return ""
	}
	return FormatValue(value)
}

// TestOptionsTakeValuesInEveryForm 覆盖长短选项、`=`、短选项聚合与位置参数收集。
func TestOptionsTakeValuesInEveryForm(t *testing.T) {
	parser := base(t)
	namespace := parseOK(t, parser, []string{"-o", "out.txt", "-vv", "--level=warn", "a.txt", "b.txt"})
	if got := text(t, namespace, "output"); got != "out.txt" {
		t.Fatalf("output = %q", got)
	}
	if got := text(t, namespace, "verbose"); got != "2" {
		t.Fatalf("verbose = %q", got)
	}
	if got := text(t, namespace, "level"); got != "warn" {
		t.Fatalf("level = %q", got)
	}
	if got := text(t, namespace, "files"); got != "a.txt, b.txt" {
		t.Fatalf("files = %q", got)
	}

	// 短选项粘值 `-oout.txt` 与 `-o=out.txt` 等价
	for _, argv := range [][]string{{"-oout.txt", "a.txt"}, {"-o=out.txt", "a.txt"}} {
		namespace := parseOK(t, parser, argv)
		if got := text(t, namespace, "output"); got != "out.txt" {
			t.Fatalf("%v 的 output = %q", argv, got)
		}
	}

	// 长选项前缀缩写照常解析
	namespace = parseOK(t, parser, []string{"-o", "x", "--verbos", "a.txt"})
	if got := text(t, namespace, "verbose"); got != "1" {
		t.Fatalf("前缀缩写的 verbose = %q", got)
	}
}

// TestSignedNumbersAreValues 签名数字与 `-` 一律当位置参数取值。
func TestSignedNumbersAreValues(t *testing.T) {
	parser := NewParser("neg").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"files"}, Argument{Nargs: NargsOneOrMore()}); err != nil {
		t.Fatalf("登记 files 失败: %v", err)
	}
	namespace := parseOK(t, parser, []string{"-1", "-.5", "+1", "-1.5", "+.5", "-"})
	if got := text(t, namespace, "files"); got != "-1, -.5, +1, -1.5, +.5, -" {
		t.Fatalf("files = %q", got)
	}
}

// TestNargsShapes 覆盖整数 / `?` / `*` / `+` / REMAINDER 与分隔符。
func TestNargsShapes(t *testing.T) {
	parser := NewParser("shapes").WithColor(ColorNever)
	fixed, _ := NargsFixed(2)
	if _, err := parser.AddArgument([]string{"--pair"}, Argument{Nargs: fixed}); err != nil {
		t.Fatalf("登记 --pair 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--maybe"}, Argument{Nargs: NargsOptional()}); err != nil {
		t.Fatalf("登记 --maybe 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--many"}, Argument{Nargs: NargsZeroOrMore()}); err != nil {
		t.Fatalf("登记 --many 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--list"}, Argument{ValueDelimiter: ","}); err != nil {
		t.Fatalf("登记 --list 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--rest"}, Argument{Nargs: NargsRemainder()}); err != nil {
		t.Fatalf("登记 --rest 失败: %v", err)
	}
	namespace := parseOK(t, parser, []string{
		"--pair", "a", "b", "--maybe", "--many", "x", "y", "--list", "1,2,3", "--rest", "z", "--flag",
	})
	if got := text(t, namespace, "pair"); got != "a, b" {
		t.Fatalf("pair = %q", got)
	}
	if got := text(t, namespace, "maybe"); got != "None" {
		t.Fatalf("maybe = %q", got)
	}
	if got := text(t, namespace, "many"); got != "x, y" {
		t.Fatalf("many = %q", got)
	}
	if got := text(t, namespace, "list"); got != "1, 2, 3" {
		t.Fatalf("list = %q", got)
	}
	if got := text(t, namespace, "rest"); got != "z, --flag" {
		t.Fatalf("rest = %q", got)
	}
}

// TestPositionalVariadicLeavesRoom 变长位置参数先给后面的位置参数留够最小值。
func TestPositionalVariadicLeavesRoom(t *testing.T) {
	parser := NewParser("var").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"src"}, Argument{Nargs: NargsZeroOrMore()}); err != nil {
		t.Fatalf("登记 src 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"dst"}, Argument{}); err != nil {
		t.Fatalf("登记 dst 失败: %v", err)
	}
	namespace := parseOK(t, parser, []string{"a", "b", "c"})
	if got := text(t, namespace, "src"); got != "a, b" {
		t.Fatalf("src = %q", got)
	}
	if got := text(t, namespace, "dst"); got != "c" {
		t.Fatalf("dst = %q", got)
	}
}

// TestActions 覆盖十种动作的落值。
func TestActions(t *testing.T) {
	parser := NewParser("actions").WithColor(ColorNever)
	add := func(names []string, argument Argument) {
		t.Helper()
		if _, err := parser.AddArgument(names, argument); err != nil {
			t.Fatalf("登记 %v 失败: %v", names, err)
		}
	}
	add([]string{"-v"}, Argument{Action: Count})
	add([]string{"--on"}, Argument{Action: StoreTrue})
	add([]string{"--off"}, Argument{Action: StoreFalse})
	add([]string{"--mode"}, Argument{Action: StoreConst, Const: ValueOf("fast"), Default: ValueOf("slow")})
	add([]string{"-D"}, Argument{Action: Append})
	add([]string{"--tag"}, Argument{Action: AppendConst, Const: ValueOf("T")})
	add([]string{"-I"}, Argument{Action: Extend, Nargs: NargsOneOrMore()})
	namespace := parseOK(t, parser, []string{"-vvv", "--on", "--off", "--mode", "-D", "A", "-D", "B", "--tag", "-I", "x", "y"})
	if got := text(t, namespace, "v"); got != "3" {
		t.Fatalf("count = %q", got)
	}
	if got := text(t, namespace, "on"); got != "true" {
		t.Fatalf("store_true = %q", got)
	}
	if got := text(t, namespace, "off"); got != "false" {
		t.Fatalf("store_false = %q", got)
	}
	if got := text(t, namespace, "mode"); got != "fast" {
		t.Fatalf("store_const = %q", got)
	}
	if got := text(t, namespace, "D"); got != "A, B" {
		t.Fatalf("append = %q", got)
	}
	if got := text(t, namespace, "tag"); got != "T" {
		t.Fatalf("append_const = %q", got)
	}
	if got := text(t, namespace, "I"); got != "x, y" {
		t.Fatalf("extend = %q", got)
	}
}

// TestConvertersAndChoices 覆盖类型转换、区间与可选值建议。
func TestConvertersAndChoices(t *testing.T) {
	parser := base(t)
	usage := failure(t, parser, []string{"-o", "out.txt", "--level", "warnn", "a.txt"})
	if !strings.Contains(usage.Error(), "invalid value 'warnn' for '--level <LEVEL>'") {
		t.Fatalf("可选值非法的文案不对: %v", usage)
	}
	if !strings.Contains(usage.Error(), "[possible values: debug, info, warn]") {
		t.Fatalf("缺少可选值清单: %v", usage)
	}
	if !strings.Contains(usage.Error(), "a similar value exists: 'warn'") {
		t.Fatalf("缺少相似值建议: %v", usage)
	}

	ranged := NewParser("num").WithColor(ColorNever)
	low, high := int64(1), int64(10)
	if _, err := ranged.AddArgument([]string{"--n"}, Argument{Type: typeParser(ParseInt(&low, &high))}); err != nil {
		t.Fatalf("登记 --n 失败: %v", err)
	}
	if err := parseErr(t, ranged, []string{"--n", "11"}); !strings.Contains(err.Error(), "11 is not in 1..=10") {
		t.Fatalf("越界文案不对: %v", err)
	}
	if err := parseErr(t, ranged, []string{"--n", "abc"}); !strings.Contains(err.Error(), "invalid digit found in string") {
		t.Fatalf("非法整数文案不对: %v", err)
	}
}

// TestUnknownOptionSuggests 未知选项给建议; 无从建议的长选项给出 `--` 提示。
func TestUnknownOptionSuggests(t *testing.T) {
	parser := base(t)
	usage := failure(t, parser, []string{"-o", "x", "--verbsoe", "a.txt"})
	if !strings.Contains(usage.Error(), "unexpected argument '--verbsoe' found") {
		t.Fatalf("未知选项文案不对: %v", usage)
	}
	if !strings.Contains(usage.Error(), "a similar argument exists: '--verbose'") {
		t.Fatalf("缺少相似项建议: %v", usage)
	}
	if usage.Kind != UnknownArgument {
		t.Fatalf("类别 = %v", usage.Kind)
	}

	usage = failure(t, parser, []string{"-o", "x", "--zzz", "a.txt"})
	if !strings.Contains(usage.Error(), "to pass '--zzz' as a value, use '-- --zzz'") {
		t.Fatalf("缺少 `--` 提示: %v", usage)
	}
}

// TestAmbiguousPrefixListsCandidates 前缀命中多个时报歧义并列出候选。
func TestAmbiguousPrefixListsCandidates(t *testing.T) {
	parser := NewParser("amb").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"--output"}, Argument{}); err != nil {
		t.Fatalf("登记 --output 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--outdir"}, Argument{}); err != nil {
		t.Fatalf("登记 --outdir 失败: %v", err)
	}
	usage := failure(t, parser, []string{"--out", "x"})
	if usage.Kind != AmbiguousArgument {
		t.Fatalf("类别 = %v", usage.Kind)
	}
	if !strings.Contains(usage.Error(), "could be one of: --outdir, --output") {
		t.Fatalf("候选清单不对: %v", usage)
	}
}

// TestMissingValueAndRequired 缺取值与缺必填项的文案。
func TestMissingValueAndRequired(t *testing.T) {
	parser := base(t)
	usage := failure(t, parser, []string{"-o"})
	if usage.Kind != MissingValue {
		t.Fatalf("类别 = %v", usage.Kind)
	}
	if !strings.Contains(usage.Error(), "a value is required for '--output <OUTPUT>' but none was supplied") {
		t.Fatalf("缺取值文案不对: %v", usage)
	}

	usage = failure(t, parser, []string{"a.txt"})
	if usage.Kind != MissingRequired {
		t.Fatalf("类别 = %v", usage.Kind)
	}
	if !strings.Contains(usage.Error(), "the following required arguments were not provided:") ||
		!strings.Contains(usage.Error(), "--output <OUTPUT>") {
		t.Fatalf("缺必填文案不对: %v", usage)
	}

	multi := NewParser("multi").WithColor(ColorNever)
	fixed, _ := NargsFixed(3)
	if _, err := multi.AddArgument([]string{"--triple"}, Argument{Nargs: fixed}); err != nil {
		t.Fatalf("登记 --triple 失败: %v", err)
	}
	usage = failure(t, multi, []string{"--triple", "a"})
	if !strings.Contains(usage.Error(), "3 values required for '--triple <TRIPLE>' but 1 was provided") {
		t.Fatalf("多值缺量文案不对: %v", usage)
	}
}

// TestValidationOrder 互斥 → 冲突 → 依赖 → 必填。
func TestValidationOrder(t *testing.T) {
	parser := NewParser("check").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"--a"}, Argument{Required: true}); err != nil {
		t.Fatalf("登记 --a 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--b"}, Argument{ConflictsWith: []string{"--a"}}); err != nil {
		t.Fatalf("登记 --b 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--c"}, Argument{Requires: []string{"--a"}}); err != nil {
		t.Fatalf("登记 --c 失败: %v", err)
	}
	usage := failure(t, parser, []string{"--a", "1", "--b", "2"})
	if usage.Kind != Conflict {
		t.Fatalf("冲突类别 = %v", usage.Kind)
	}
	if !strings.Contains(usage.Error(), "the argument '--b <B>' cannot be used with '--a <A>'") {
		t.Fatalf("冲突文案不对: %v", usage)
	}

	usage = failure(t, parser, []string{"--c", "3"})
	if usage.Kind != MissingRequired {
		t.Fatalf("依赖类别 = %v", usage.Kind)
	}
	if !strings.Contains(usage.Error(), "the argument '--c <C>' requires '--a <A>'") {
		t.Fatalf("依赖文案不对: %v", usage)
	}

	mutex := NewParser("mutex").WithColor(ColorNever)
	group := mutex.AddMutuallyExclusiveGroup(true)
	for _, name := range []string{"--x", "--y"} {
		if _, err := group.AddArgument([]string{name}, Argument{}); err != nil {
			t.Fatalf("登记 %s 失败: %v", name, err)
		}
	}
	usage = failure(t, mutex, []string{})
	if !strings.Contains(usage.Error(), "one of the following arguments is required:") ||
		!strings.Contains(usage.Error(), "  --x <X>\n  --y <Y>") {
		t.Fatalf("互斥组文案不对: %v", usage)
	}
	usage = failure(t, mutex, []string{"--x", "1", "--y", "2"})
	if !strings.Contains(usage.Error(), "'--x <X>' cannot be used with '--y <Y>'") {
		t.Fatalf("互斥冲突文案不对: %v", usage)
	}
}

// TestEnvFallback 命令行没出现时读环境变量; 非法环境变量报取值非法。
func TestEnvFallback(t *testing.T) {
	parser := NewParser("env").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"--level"}, Argument{
		Choices: []Choice{{Value: "debug"}, {Value: "info"}, {Value: "warn"}},
		Env:     "DEMO_LEVEL",
	}); err != nil {
		t.Fatalf("登记 --level 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--dry-run"}, Argument{Action: StoreTrue, Env: "DEMO_DRY"}); err != nil {
		t.Fatalf("登记 --dry-run 失败: %v", err)
	}
	t.Setenv("DEMO_LEVEL", "warn")
	t.Setenv("DEMO_DRY", "yes")
	namespace := parseOK(t, parser, []string{})
	if got := text(t, namespace, "level"); got != "warn" {
		t.Fatalf("level = %q", got)
	}
	if got := text(t, namespace, "dry_run"); got != "true" {
		t.Fatalf("dry_run = %q", got)
	}

	// 命令行优先于环境变量
	namespace = parseOK(t, parser, []string{"--level", "info"})
	if got := text(t, namespace, "level"); got != "info" {
		t.Fatalf("level = %q", got)
	}

	t.Setenv("DEMO_LEVEL", "verbose")
	usage := failure(t, parser, []string{})
	if !strings.Contains(usage.Error(), "invalid value 'verbose' for '--level <LEVEL>'") {
		t.Fatalf("非法环境变量文案不对: %v", usage)
	}

	t.Setenv("DEMO_LEVEL", "info")
	t.Setenv("DEMO_DRY", "maybe")
	usage = failure(t, parser, []string{})
	if !strings.Contains(usage.Error(), "invalid value 'maybe' for '--dry-run'") {
		t.Fatalf("非法布尔环境变量文案不对: %v", usage)
	}
}

// TestDefaultsAndSuppress 缺省值优先级与 SUPPRESS。
func TestDefaultsAndSuppress(t *testing.T) {
	parser := NewParser("defaults").WithColor(ColorNever).WithArgumentDefault(ValueOf("fallback"))
	if _, err := parser.AddArgument([]string{"--name"}, Argument{}); err != nil {
		t.Fatalf("登记 --name 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--quiet"}, Argument{Default: Suppress}); err != nil {
		t.Fatalf("登记 --quiet 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--count"}, Argument{Action: Count}); err != nil {
		t.Fatalf("登记 --count 失败: %v", err)
	}
	namespace := parseOK(t, parser, []string{})
	if got := text(t, namespace, "name"); got != "fallback" {
		t.Fatalf("name = %q", got)
	}
	if namespace.Contains("quiet") {
		t.Fatal("SUPPRESS 不该写进命名空间")
	}
	if got := text(t, namespace, "count"); got != "0" {
		t.Fatalf("count = %q", got)
	}

	inferred := NewParser("infer").WithColor(ColorNever)
	if _, err := inferred.AddArgument([]string{"--port"}, Argument{Default: ValueOf(int64(80))}); err != nil {
		t.Fatalf("登记 --port 失败: %v", err)
	}
	namespace = parseOK(t, inferred, []string{"--port", "8080"})
	if value, _ := namespace.Get("port"); value != int64(8080) {
		t.Fatalf("按默认值推断的转换器没生效: %#v", value)
	}
}

// TestSubcommands 子命令、别名、dest、全局参数与必填。
func TestSubcommands(t *testing.T) {
	parser := tool(t)
	namespace := parseOK(t, parser, []string{"b", "-j", "8", "release"})
	if got := text(t, namespace, "command"); got != "build" {
		t.Fatalf("别名没归一成正名: %q", got)
	}
	if got := text(t, namespace, "target"); got != "release" {
		t.Fatalf("二级子命令 = %q", got)
	}
	if got := text(t, namespace, "jobs"); got != "8" {
		t.Fatalf("jobs = %q", got)
	}

	// 全局参数下沉到子命令
	namespace = parseOK(t, parser, []string{"run", "-q", "x.sh"})
	if got := text(t, namespace, "quiet"); got != "true" {
		t.Fatalf("全局参数没生效: %q", got)
	}

	usage := failure(t, parser, []string{})
	if !strings.Contains(usage.Error(), "<COMMAND>") {
		t.Fatalf("缺必填子命令的文案不对: %v", usage)
	}

	usage = failure(t, parser, []string{"run"})
	if !strings.Contains(usage.Error(), "<SCRIPT>") {
		t.Fatalf("缺子命令位置参数的文案不对: %v", usage)
	}
}

// TestHelpSubcommand 自动生成的 help 子命令可下沉, 认不出的名字报错。
func TestHelpSubcommand(t *testing.T) {
	parser := tool(t)
	requested, ok := parseErr(t, parser, []string{"help", "build"}).(*HelpRequested)
	if !ok {
		t.Fatal("应当是帮助请求")
	}
	if !requested.Long || requested.Parser.Prog() != "tool build" {
		t.Fatalf("帮助目标不对: long=%v prog=%q", requested.Long, requested.Parser.Prog())
	}

	usage := failure(t, parser, []string{"help", "nosuch"})
	if !strings.Contains(usage.Error(), "unrecognized subcommand 'nosuch'") {
		t.Fatalf("未知子命令文案不对: %v", usage)
	}
}

// TestHelpAndVersionFlags `-h` / `--help` 与 `-V` / `--version` 走控制流。
func TestHelpAndVersionFlags(t *testing.T) {
	parser := NewParser("demo").WithVersion("1.2.3").WithColor(ColorNever)
	short, ok := parseErr(t, parser, []string{"-h"}).(*HelpRequested)
	if !ok || short.Long || short.Parser.Prog() != "demo" {
		t.Fatalf("短帮助不对: %+v", short)
	}
	long, ok := parseErr(t, parser, []string{"--help"}).(*HelpRequested)
	if !ok || !long.Long {
		t.Fatalf("长帮助不对: %+v", long)
	}
	for _, flag := range []string{"-V", "--version"} {
		version, ok := parseErr(t, parser, []string{flag}).(*VersionRequested)
		if !ok {
			t.Fatalf("%s 应当是版本请求", flag)
		}
		if got := version.Parser.VersionText(); got != "demo 1.2.3" {
			t.Fatalf("%s 的版本行 = %q", flag, got)
		}
	}

	// 子命令沿用父命令版本, 版本行带命令全名
	namespace := parseOK(t, tool(t), []string{"build", "-j", "2", "debug"})
	if got := text(t, namespace, "jobs"); got != "2" {
		t.Fatalf("jobs = %q", got)
	}
	version, ok := parseErr(t, tool(t), []string{"build", "--version"}).(*VersionRequested)
	if !ok {
		t.Fatal("子命令的 --version 应当是版本请求")
	}
	if got := version.Parser.VersionText(); got != "tool build 9.9.9" {
		t.Fatalf("子命令版本行 = %q", got)
	}
}

// TestValueGroupAndRequiredUnlessPresent 覆盖必选组与「除非某些参数出现」的必填条件。
func TestValueGroupAndRequiredUnlessPresent(t *testing.T) {
	grouped := NewParser("grouped").WithColor(ColorNever)
	inputs := grouped.AddValueGroup("input", true)
	if _, err := inputs.AddArgument([]string{"--file"}, Argument{}); err != nil {
		t.Fatalf("登记 --file 失败: %v", err)
	}
	if _, err := inputs.AddArgument([]string{"--dir"}, Argument{}); err != nil {
		t.Fatalf("登记 --dir 失败: %v", err)
	}

	if got := text(t, parseOK(t, grouped, []string{"--file", "x"}), "file"); got != "x" {
		t.Fatalf("file = %q", got)
	}
	if got := text(t, parseOK(t, grouped, []string{"--file", "x", "--dir", "y"}), "dir"); got != "y" {
		t.Fatalf("dir = %q", got)
	}
	empty := failure(t, grouped, []string{})
	if empty.Kind != MissingRequired || !strings.Contains(empty.Error(), "<input>") {
		t.Fatalf("多值组报错 = %s", empty.Error())
	}

	gated := NewParser("gated").WithColor(ColorNever)
	if _, err := gated.AddArgument([]string{"--token"}, Argument{RequiredUnlessPresent: []string{"user"}}); err != nil {
		t.Fatalf("登记 --token 失败: %v", err)
	}
	if _, err := gated.AddArgument([]string{"--user"}, Argument{}); err != nil {
		t.Fatalf("登记 --user 失败: %v", err)
	}
	if _, err := gated.AddArgument([]string{"--key"}, Argument{OverridesWith: []string{"user"}}); err != nil {
		t.Fatalf("登记 --key 失败: %v", err)
	}

	parseOK(t, gated, []string{"--user", "u"})
	if got := failure(t, gated, []string{}); got.Kind != MissingRequired {
		t.Fatalf("条件不满足时类别 = %v", got.Kind)
	}
	// 被 --key 覆盖掉的 --user 不再算出现过, 于是 --token 又变回必填
	if got := failure(t, gated, []string{"--user", "u", "--key", "k"}); got.Kind != MissingRequired {
		t.Fatalf("被覆盖后类别 = %v", got.Kind)
	}
}

// TestAutoWidthReadsColumns 覆盖帮助宽度: 默认不读 COLUMNS, 开了 AutoWidth 才读。
func TestAutoWidthReadsColumns(t *testing.T) {
	fixed := NewParser("demo").WithMaxWidth(44)
	if got := fixed.Width(); got != 44 {
		t.Fatalf("默认宽度 = %d, 期望 44", got)
	}

	t.Setenv("COLUMNS", "60")
	if got := fixed.Width(); got != 44 {
		t.Fatalf("未开 AutoWidth 时宽度 = %d, 期望 44", got)
	}

	auto := NewParser("demo").WithMaxWidth(44).WithAutoWidth()
	if got := auto.Width(); got != 60 {
		t.Fatalf("开了 AutoWidth 后宽度 = %d, 期望 60", got)
	}

	t.Setenv("COLUMNS", "abc")
	if got := auto.Width(); got != 44 {
		t.Fatalf("COLUMNS 非法时宽度 = %d, 期望 44", got)
	}

	t.Setenv("COLUMNS", "0")
	if got := auto.Width(); got != 44 {
		t.Fatalf("COLUMNS 非正时宽度 = %d, 期望 44", got)
	}
}

// TestOverridesWithKeepsTheLastOne 覆盖 `overrides_with` 的双向「后出现者留下」。
func TestOverridesWithKeepsTheLastOne(t *testing.T) {
	parser := NewParser("over").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"--a"}, Argument{Default: ValueOf("A")}); err != nil {
		t.Fatalf("登记 --a 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--b"}, Argument{Default: ValueOf("B"), OverridesWith: []string{"a"}}); err != nil {
		t.Fatalf("登记 --b 失败: %v", err)
	}

	lastB := parseOK(t, parser, []string{"--a", "1", "--b", "2"})
	if got := text(t, lastB, "b"); got != "2" {
		t.Fatalf("b = %q, 期望 2", got)
	}
	if got := text(t, lastB, "a"); got != "A" {
		t.Fatalf("被覆盖的 a = %q, 期望退回默认值 A", got)
	}

	lastA := parseOK(t, parser, []string{"--b", "2", "--a", "1"})
	if got := text(t, lastA, "a"); got != "1" {
		t.Fatalf("a = %q, 期望 1", got)
	}
	if got := text(t, lastA, "b"); got != "B" {
		t.Fatalf("被覆盖的 b = %q, 期望退回默认值 B", got)
	}
}

// TestFromFilePrefixExpandsLines 覆盖 `@file` 展开: 每行一个 token、嵌套、打不开报错。
func TestFromFilePrefixExpandsLines(t *testing.T) {
	dir := t.TempDir()
	inner := filepath.Join(dir, "inner.txt")
	outer := filepath.Join(dir, "outer.txt")
	if err := os.WriteFile(inner, []byte("--output\ninner.txt\nb.txt\n"), 0o600); err != nil {
		t.Fatalf("写 inner 失败: %v", err)
	}
	if err := os.WriteFile(outer, []byte("@"+inner+"\n"), 0o600); err != nil {
		t.Fatalf("写 outer 失败: %v", err)
	}

	parser := NewParser("files").WithColor(ColorNever).WithFromfilePrefixChars("@")
	if _, err := parser.AddArgument([]string{"-o", "--output"}, Argument{}); err != nil {
		t.Fatalf("登记 --output 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"files"}, Argument{Nargs: NargsZeroOrMore()}); err != nil {
		t.Fatalf("登记 files 失败: %v", err)
	}

	namespace := parseOK(t, parser, []string{"@" + outer})
	if got := text(t, namespace, "output"); got != "inner.txt" {
		t.Fatalf("output = %q, 期望 inner.txt", got)
	}
	if got := text(t, namespace, "files"); got != "b.txt" {
		t.Fatalf("files = %q, 期望 b.txt", got)
	}

	missing := failure(t, parser, []string{"@" + filepath.Join(dir, "nope.txt")})
	if missing.Kind != Other {
		t.Fatalf("类别 = %v, 期望 Other", missing.Kind)
	}
	if !strings.Contains(missing.Message, "unable to open file") {
		t.Fatalf("文案 = %q", missing.Message)
	}
}

// TestParseKnownKeepsExtrasInOrder 未知选项只收自身, extras 按命令行顺序排列。
func TestParseKnownKeepsExtrasInOrder(t *testing.T) {
	parser := NewParser("known").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"--flag"}, Argument{}); err != nil {
		t.Fatalf("登记 --flag 失败: %v", err)
	}
	namespace, extras, err := parser.ParseKnown([]string{"--flag", "v", "--zzz", "left", "--yyy", "--flag", "w", "tail"})
	if err != nil {
		t.Fatalf("解析失败: %v", err)
	}
	if got := text(t, namespace, "flag"); got != "w" {
		t.Fatalf("flag = %q", got)
	}
	want := []string{"--zzz", "left", "--yyy", "tail"}
	if len(extras) != len(want) {
		t.Fatalf("extras = %v, 期望 %v", extras, want)
	}
	for index, item := range want {
		if extras[index] != item {
			t.Fatalf("extras = %v, 期望 %v", extras, want)
		}
	}
}

// TestRemainderBoundaryRejectsOptionToken REMAINDER 边界上的选项 token 仍然报未知参数。
func TestRemainderBoundaryRejectsOptionToken(t *testing.T) {
	parser := NewParser("rem").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"head"}, Argument{}); err != nil {
		t.Fatalf("登记 head 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"rest"}, Argument{Nargs: NargsRemainder()}); err != nil {
		t.Fatalf("登记 rest 失败: %v", err)
	}
	usage := failure(t, parser, []string{"a", "--flag"})
	if usage.Kind != UnknownArgument {
		t.Fatalf("类别 = %v (%v)", usage.Kind, usage)
	}
}

// TestDefinitionErrorsAreRejected 定义期错误在登记时抛出。
func TestDefinitionErrorsAreRejected(t *testing.T) {
	parser := NewParser("dup").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"--a"}, Argument{}); err != nil {
		t.Fatalf("登记 --a 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--a"}, Argument{}); err == nil {
		t.Fatal("选项名重复应当报错")
	}
	if _, err := parser.AddArgument([]string{"--b"}, Argument{Dest: "a"}); err == nil {
		t.Fatal("dest 重复应当报错")
	}
	if _, err := NewParser("bad").AddArgument([]string{"-1"}, Argument{}); err == nil {
		t.Fatal("签名数字的选项名应当报错")
	}
	remainder := NewParser("rem").WithColor(ColorNever)
	if _, err := remainder.AddArgument([]string{"rest"}, Argument{Nargs: NargsRemainder()}); err != nil {
		t.Fatalf("登记 rest 失败: %v", err)
	}
	if _, err := remainder.AddArgument([]string{"tail"}, Argument{}); err == nil {
		t.Fatal("REMAINDER 之后再加位置参数应当报错")
	}
	if _, err := NewParser("req").AddArgument([]string{"f"}, Argument{Required: true}); err == nil {
		t.Fatal("位置参数不接受 required")
	}
}

// TestDuplicatedDestAcrossCommandsIsADefinitionError 命令链上 dest 重复在校验期暴露。
func TestDuplicatedDestAcrossCommandsIsADefinitionError(t *testing.T) {
	parser := NewParser("chain").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"-v"}, Argument{Action: StoreTrue, Global: true}); err != nil {
		t.Fatalf("登记 -v 失败: %v", err)
	}
	subparsers, err := parser.AddSubparsers("", "Commands", false)
	if err != nil {
		t.Fatalf("登记子命令失败: %v", err)
	}
	child, err := subparsers.AddParser("c", "", nil, nil)
	if err != nil {
		t.Fatalf("登记 c 失败: %v", err)
	}
	if _, err := child.AddArgument([]string{"-v"}, Argument{Action: StoreTrue}); err != nil {
		t.Fatalf("登记子命令的 -v 失败: %v", err)
	}
	if _, ok := parseErr(t, parser, []string{"c"}).(*DefinitionError); !ok {
		t.Fatal("应当是定义期错误")
	}
}

// TestUsageSpecShapes usage 行按 nargs 形态拼装。
func TestUsageSpecShapes(t *testing.T) {
	parser := NewParser("usage").WithColor(ColorNever)
	if _, err := parser.AddArgument([]string{"--out"}, Argument{Required: true}); err != nil {
		t.Fatalf("登记 --out 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"--opt"}, Argument{}); err != nil {
		t.Fatalf("登记 --opt 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"one"}, Argument{Nargs: NargsOptional()}); err != nil {
		t.Fatalf("登记 one 失败: %v", err)
	}
	if _, err := parser.AddArgument([]string{"many"}, Argument{Nargs: NargsOneOrMore()}); err != nil {
		t.Fatalf("登记 many 失败: %v", err)
	}
	want := "usage --out <OUT> [OPTIONS] [ONE] <MANY>..."
	if got := parser.UsageSpec(); got != want {
		t.Fatalf("usage = %q, 期望 %q", got, want)
	}
}

// TestJaroWinklerAndBestMatch 相似度与建议阈值。
func TestJaroWinklerAndBestMatch(t *testing.T) {
	if score := JaroWinkler("--verbose", "--version"); abs(score-0.8825396825396825) > 1e-12 {
		t.Fatalf("相似度 = %v", score)
	}
	candidates := []string{"--help", "--flag", "--output"}
	if got, ok := BestMatch("--flah", candidates); !ok || got != "--flag" {
		t.Fatalf("建议 = %q (%v)", got, ok)
	}
	if _, ok := BestMatch("--zzz", candidates); ok {
		t.Fatal("无从建议时不该给建议")
	}
}

// TestValueTextFollowsPythonRules 取值的文本形态。
func TestValueTextFollowsPythonRules(t *testing.T) {
	cases := []struct {
		value any
		want  string
	}{
		{nil, "None"},
		{true, "true"},
		{false, "false"},
		{int64(3), "3"},
		{2.0, "2.0"},
		{2.5, "2.5"},
		{"text", "text"},
		{[]any{"a", "b"}, "a, b"},
	}
	for _, item := range cases {
		if got := FormatValue(item.value); got != item.want {
			t.Fatalf("FormatValue(%#v) = %q, 期望 %q", item.value, got, item.want)
		}
	}
}

// abs 浮点绝对值。
func abs(value float64) float64 {
	if value < 0 {
		return -value
	}
	return value
}
