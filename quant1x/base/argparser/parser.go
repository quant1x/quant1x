// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

// 命令定义与解析: 选项匹配、位置参数分配、子命令分发、缺省值与校验。

package argparser

import (
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"reflect"
	"sort"
	"strconv"
	"strings"
)

// ColorChoice 着色的时机, 对应 Python 的 `color` 参数。
type ColorChoice int

const (
	// ColorAuto 依目标流是否终端与 `NO_COLOR` 判定。
	ColorAuto ColorChoice = iota
	// ColorAlways 总是着色。
	ColorAlways
	// ColorNever 从不着色。
	ColorNever
)

// MutexGroup 互斥组: 组内参数最多出现一个; Required 为真时至少出现一个。
type MutexGroup struct {
	parser    *Parser
	Required  bool
	Arguments []*Argument
}

// ValueGroup 多值组: 组内参数可以同时出现; Required 为真时至少要出现一个。
type ValueGroup struct {
	parser    *Parser
	Name      string
	Required  bool
	Arguments []*Argument
}

// AddArgument 登记一个参数并纳入本组。
func (g *ValueGroup) AddArgument(names []string, argument Argument) (*Argument, error) {
	arg, err := g.parser.AddArgument(names, argument)
	if err != nil {
		return nil, err
	}
	g.Arguments = append(g.Arguments, arg)
	return arg, nil
}

// AddArgument 登记一个参数并纳入本互斥组。
func (g *MutexGroup) AddArgument(names []string, argument Argument) (*Argument, error) {
	arg, err := g.parser.AddArgument(names, argument)
	if err != nil {
		return nil, err
	}
	g.Arguments = append(g.Arguments, arg)
	return arg, nil
}

// Command 一条子命令的展示信息。
type Command struct {
	Name    string
	Aliases []string
	Parser  *Parser
}

// ArgumentGroup 帮助里的一个参数分组, 只影响渲染不影响解析。
type ArgumentGroup struct {
	Parser      *Parser
	Title       string
	Description string
}

// AddArgument 登记一个参数并把它归到本组。
func (g *ArgumentGroup) AddArgument(names []string, argument Argument) (*Argument, error) {
	arg, err := g.Parser.AddArgument(names, argument)
	if err != nil {
		return nil, err
	}
	arg.Group = g.Title
	return arg, nil
}

// SubParsers 子命令集合: 名字 (含别名) 到命令的映射, 并记录帮助里的展示顺序。
type SubParsers struct {
	parent   *Parser
	Dest     string
	Title    string
	Required bool
	Help     string
	commands map[string]*Parser
	order    []*Parser
	visible  map[string][]string
}

// AddParser 登记一条子命令; aliases 不出现在帮助里, visibleAliases 出现。
func (s *SubParsers) AddParser(name, about string, aliases, visibleAliases []string) (*Parser, error) {
	child := NewParser(s.parent.prog + " " + name)
	child.name = name
	child.about = about
	child.version = s.parent.version
	child.registerVersion()
	child.allowAbbrev = s.parent.allowAbbrev
	child.color = s.parent.color
	child.maxWidth = s.parent.maxWidth
	child.autoWidth = s.parent.autoWidth
	child.parent = s.parent
	for _, key := range append(append([]string{name}, aliases...), visibleAliases...) {
		if _, ok := s.commands[key]; ok {
			return nil, fmt.Errorf("子命令名重复登记: %q", key)
		}
		s.commands[key] = child
	}
	s.order = append(s.order, child)
	s.visible[name] = append([]string{}, visibleAliases...)
	return child, nil
}

// Lookup 按名字或别名找子命令。
func (s *SubParsers) Lookup(name string) *Parser { return s.commands[name] }

// HasCommand 该名字 (含别名) 是否已被登记成子命令。
func (s *SubParsers) HasCommand(name string) bool {
	_, ok := s.commands[name]
	return ok
}

// Canonical 名字归一成注册时的正式名 (别名解析回正名)。
func (s *SubParsers) Canonical(name string) string {
	if command, ok := s.commands[name]; ok {
		if command.name != "" {
			return command.name
		}
	}
	return name
}

// Commands 按注册顺序给出子命令。
func (s *SubParsers) Commands() []Command {
	commands := make([]Command, 0, len(s.order))
	for _, command := range s.order {
		commands = append(commands, Command{
			Name:    command.name,
			Aliases: s.visible[command.name],
			Parser:  command,
		})
	}
	return commands
}

// GroupInfo 帮助里的一个分组的标题与说明。
type GroupInfo struct {
	Title       string
	Description string
}

// Parser 一条命令: 参数定义、解析入口与 usage 片段。子命令本身也是 Parser。
type Parser struct {
	prog                  string
	about                 string
	longAbout             string
	version               string
	epilog                string
	usage                 string
	allowAbbrev           bool
	argumentDefault       DefaultValue
	color                 ColorChoice
	maxWidth              int
	autoWidth             bool
	subcommandNegatesReqs bool
	fromfilePrefixChars   string
	arguments             []*Argument
	byName                map[string]*Argument
	nameOrder             []string
	positionals           []*Argument
	dests                 map[string]*Argument
	groups                []GroupInfo
	mutex                 []*MutexGroup
	valueGroups           []*ValueGroup
	subparsers            *SubParsers
	parent                *Parser
	name                  string
}

// NewParser 起一条命令; 缺省带 `-h` / `--help`。
func NewParser(prog string) *Parser {
	parser := &Parser{
		prog:            prog,
		allowAbbrev:     true,
		argumentDefault: Missing,
		color:           ColorAuto,
		maxWidth:        100,
		byName:          map[string]*Argument{},
		dests:           map[string]*Argument{},
	}
	zero, _ := NargsFixed(0)
	parser.push(&Argument{
		Names:   []string{"-h", "--help"},
		Dest:    "help",
		Action:  Help,
		Nargs:   zero,
		Default: Suppress,
		Help:    "Print help",
	})
	return parser
}

// ParserFromEnv 按可执行文件名起 prog, 对应 Python 侧取 `sys.argv[0]` 的文件名。
func ParserFromEnv() *Parser {
	prog := "prog"
	if len(os.Args) > 0 {
		if name := filepath.Base(os.Args[0]); name != "" && name != "." {
			prog = name
		}
	}
	return NewParser(prog)
}

// WithAbout 设置一段简介。
func (p *Parser) WithAbout(about string) *Parser { p.about = about; return p }

// WithLongAbout 设置长简介 (`--help` 用)。
func (p *Parser) WithLongAbout(about string) *Parser { p.longAbout = about; return p }

// WithVersion 设置版本号并登记 `-V` / `--version`; 子命令没自带时沿用父命令的。
func (p *Parser) WithVersion(version string) *Parser {
	p.version = version
	p.registerVersion()
	return p
}

// WithEpilog 设置尾注。
func (p *Parser) WithEpilog(epilog string) *Parser { p.epilog = epilog; return p }

// WithUsage 用给定文本替换 usage 行里的命令片段。
func (p *Parser) WithUsage(usage string) *Parser { p.usage = usage; return p }

// WithoutAbbrev 关掉长选项前缀匹配。
func (p *Parser) WithoutAbbrev() *Parser { p.allowAbbrev = false; return p }

// WithoutHelp 不再自动登记 `-h` / `--help`。
func (p *Parser) WithoutHelp() *Parser {
	kept := make([]*Argument, 0, len(p.arguments))
	for _, argument := range p.arguments {
		if argument.Action == Help {
			for _, name := range argument.Names {
				delete(p.byName, name)
			}
			delete(p.dests, argument.Dest)
			continue
		}
		kept = append(kept, argument)
	}
	p.arguments = kept
	order := make([]string, 0, len(p.nameOrder))
	for _, name := range p.nameOrder {
		if _, exists := p.byName[name]; exists {
			order = append(order, name)
		}
	}
	p.nameOrder = order
	return p
}

// WithArgumentDefault 设置所有参数的兜底默认值。
func (p *Parser) WithArgumentDefault(value DefaultValue) *Parser {
	p.argumentDefault = value
	return p
}

// WithColor 设置着色的时机。
func (p *Parser) WithColor(color ColorChoice) *Parser { p.color = color; return p }

// WithMaxWidth 设置帮助的折行宽度。
func (p *Parser) WithMaxWidth(width int) *Parser { p.maxWidth = width; return p }

// WithFromfilePrefixChars 把 `@file` 形态的 token 展开成文件里的行; 传空串等于不展开。
// 名字沿用 Python 侧的 fromfile_prefix_chars, 与 WithVersion / WithMaxWidth 同一层。
func (p *Parser) WithFromfilePrefixChars(chars string) *Parser {
	p.fromfilePrefixChars = chars
	return p
}

// WithSubcommandNegatesReqs 子命令出现时不再要求本命令的必填参数。
func (p *Parser) WithSubcommandNegatesReqs(flag bool) *Parser {
	p.subcommandNegatesReqs = flag
	return p
}

// registerVersion 有版本号就登记 `-V` / `--version`; 已占住 `version` 这个 dest 的不再重复登记。
func (p *Parser) registerVersion() {
	if p.version == "" {
		return
	}
	if _, ok := p.dests["version"]; ok {
		return
	}
	zero, _ := NargsFixed(0)
	p.push(&Argument{
		Names:   []string{"-V", "--version"},
		Dest:    "version",
		Action:  Version,
		Nargs:   zero,
		Default: Suppress,
		Help:    "Print version",
	})
}

// AddArgument 登记一个参数: 首个名字以 `-` 开头是选项, 否则是位置参数。
func (p *Parser) AddArgument(names []string, argument Argument) (*Argument, error) {
	if len(names) == 0 {
		return nil, errors.New("至少需要一个参数名")
	}
	positional := !strings.HasPrefix(names[0], "-")
	if positional {
		if len(names) > 1 {
			return nil, fmt.Errorf("位置参数只能有一个名字: %v", names)
		}
		for _, existing := range p.positionals {
			if existing.Nargs.IsRemainder() {
				return nil, errors.New("REMAINDER 位置参数必须是最后一个, 其后不能再加位置参数")
			}
		}
		if argument.Required {
			return nil, errors.New("位置参数的必填性由 nargs 决定, 不接受 required")
		}
	}
	var optionNames []string
	if !positional {
		optionNames = names
	}
	allNames := append(append(append([]string{}, optionNames...), argument.Alias...), argument.VisibleAlias...)
	for _, name := range allNames {
		if !strings.HasPrefix(name, "-") {
			return nil, fmt.Errorf("选项名必须以 '-' 开头: %q", name)
		}
		if isSignedNumber(name) {
			return nil, fmt.Errorf("签名数字不能作选项名, 会被整体当成取值: %q", name)
		}
	}

	dest := argument.Dest
	if dest == "" {
		if positional {
			dest = strings.ReplaceAll(names[0], "-", "_")
		} else {
			dest = destOfOption(optionNames)
		}
	}
	action := argument.Action
	nargs := argument.Nargs
	constValue := argument.Const
	defaultValue := argument.Default
	if !action.TakesValues() {
		if !(nargs.kind == nargsOne || (nargs.kind == nargsFixed && nargs.count == 0)) {
			return nil, fmt.Errorf("action=%s 不吃取值, 不能配 nargs=%q", action, nargs.Kind())
		}
		zero, _ := NargsFixed(0)
		nargs = zero
		switch action {
		case StoreTrue, StoreFalse:
			if constValue.IsMissing() {
				constValue = ValueOf(action == StoreTrue)
			}
			if defaultValue.IsMissing() {
				if flag, ok := constValue.Get(); ok {
					enabled, _ := flag.(bool)
					defaultValue = ValueOf(!enabled)
				}
			}
		case Count:
			if defaultValue.IsMissing() {
				defaultValue = ValueOf(int64(0))
			}
		case StoreConst, AppendConst:
			if constValue.IsMissing() {
				return nil, fmt.Errorf("action=%s 必须给 const", action)
			}
		}
	} else if argument.ValueDelimiter != "" && nargs.kind == nargsOne {
		fixed, _ := NargsFixed(1)
		nargs = fixed
	}

	typeParser := argument.Type
	if typeParser == nil && action.TakesValues() && !defaultValue.IsMissing() {
		typeParser = inferParser(defaultValue)
	}

	for _, name := range allNames {
		if _, ok := p.byName[name]; ok {
			return nil, fmt.Errorf("选项名重复登记: %q", name)
		}
	}
	if _, ok := p.dests[dest]; ok {
		return nil, fmt.Errorf("dest 重复登记: %q", dest)
	}

	registered := &Argument{
		Names:                 optionNames,
		Dest:                  dest,
		Action:                action,
		Nargs:                 nargs,
		Type:                  typeParser,
		Choices:               argument.Choices,
		Default:               defaultValue,
		Const:                 constValue,
		Required:              argument.Required,
		Metavar:               argument.Metavar,
		Help:                  argument.Help,
		LongHelp:              argument.LongHelp,
		Env:                   argument.Env,
		Alias:                 argument.Alias,
		VisibleAlias:          argument.VisibleAlias,
		ConflictsWith:         argument.ConflictsWith,
		Requires:              argument.Requires,
		OverridesWith:         argument.OverridesWith,
		RequiredUnlessPresent: argument.RequiredUnlessPresent,
		ValueDelimiter:        argument.ValueDelimiter,
		Hide:                  argument.Hide,
		HideDefault:           argument.HideDefault,
		AllowHyphenValues:     argument.AllowHyphenValues,
		Global:                argument.Global,
		Group:                 argument.Group,
	}
	p.push(registered)
	return registered, nil
}

// AddGroup 登记一个帮助分组; 该组的参数在帮助里单独成节。
func (p *Parser) AddGroup(title, description string) *ArgumentGroup {
	p.groups = append(p.groups, GroupInfo{Title: title, Description: description})
	return &ArgumentGroup{Parser: p, Title: title, Description: description}
}

// AddMutuallyExclusiveGroup 登记一个互斥组。
func (p *Parser) AddMutuallyExclusiveGroup(required bool) *MutexGroup {
	group := &MutexGroup{parser: p, Required: required}
	p.mutex = append(p.mutex, group)
	return group
}

// AddValueGroup 登记一个多值组: 组内参数可同时出现, required 表示至少出现一个。
func (p *Parser) AddValueGroup(name string, required bool) *ValueGroup {
	group := &ValueGroup{parser: p, Name: name, Required: required}
	p.valueGroups = append(p.valueGroups, group)
	return group
}

// ValueGroups 多值组, 按登记顺序。
func (p *Parser) ValueGroups() []*ValueGroup { return p.valueGroups }

// AddSubparsers 登记子命令集合; dest 给出时把命中 (含别名) 的原样写进命名空间。
func (p *Parser) AddSubparsers(dest, title string, required bool) (*SubParsers, error) {
	if p.subparsers != nil {
		return nil, errors.New("子命令集合只能登记一次")
	}
	if title == "" {
		title = "Commands"
	}
	p.subparsers = &SubParsers{
		parent:   p,
		Dest:     dest,
		Title:    title,
		Required: required,
		commands: map[string]*Parser{},
		visible:  map[string][]string{},
	}
	return p.subparsers, nil
}

// push 登记一条已构造好的参数定义; 内置的 `-h` / `-V` 走这里, 不再走定义期校验。
func (p *Parser) push(argument *Argument) {
	p.arguments = append(p.arguments, argument)
	p.dests[argument.Dest] = argument
	for _, name := range argument.Names {
		p.byName[name] = argument
		p.nameOrder = append(p.nameOrder, name)
	}
	for _, name := range argument.Alias {
		p.byName[name] = argument
		p.nameOrder = append(p.nameOrder, name)
	}
	for _, name := range argument.VisibleAlias {
		p.byName[name] = argument
		p.nameOrder = append(p.nameOrder, name)
	}
	if argument.IsPositional() {
		p.positionals = append(p.positionals, argument)
	}
}

// Prog 命令全名。
func (p *Parser) Prog() string { return p.prog }

// About 一段简介。
func (p *Parser) About() string { return p.about }

// LongAbout 长简介。
func (p *Parser) LongAbout() string { return p.longAbout }

// Version 版本号; 子命令没自带时沿用父命令的。
func (p *Parser) Version() string {
	if p.version != "" {
		return p.version
	}
	if p.parent != nil {
		return p.parent.Version()
	}
	return ""
}

// Name 在父命令里的子命令名; 顶层命令为空。
func (p *Parser) Name() string { return p.name }

// Parent 父命令; 顶层命令为 nil。
func (p *Parser) Parent() *Parser { return p.parent }

// VersionText 版本行的正文: `<prog> <version>`。
func (p *Parser) VersionText() string {
	return strings.TrimSpace(p.prog + " " + p.Version())
}

// Epilog 尾注。
func (p *Parser) Epilog() string { return p.epilog }

// Color 着色的时机。
func (p *Parser) Color() ColorChoice { return p.color }

// MaxWidth 帮助的折行宽度。
func (p *Parser) MaxWidth() int { return p.maxWidth }

// WithAutoWidth 帮助宽度按 COLUMNS 取; 取不到或非法时仍用 MaxWidth。
func (p *Parser) WithAutoWidth() *Parser { p.autoWidth = true; return p }

// Width 帮助的生效宽度。
func (p *Parser) Width() int {
	if p.autoWidth {
		if raw := strings.TrimSpace(os.Getenv("COLUMNS")); raw != "" {
			if parsed, err := strconv.Atoi(raw); err == nil && parsed > 0 {
				return parsed
			}
		}
	}
	return p.maxWidth
}

// Arguments 本命令登记的参数 (含位置参数), 按注册顺序。
func (p *Parser) Arguments() []*Argument { return append([]*Argument{}, p.arguments...) }

// Positionals 本命令的位置参数。
func (p *Parser) Positionals() []*Argument { return append([]*Argument{}, p.positionals...) }

// Groups 帮助分组, 按登记顺序。
func (p *Parser) Groups() []GroupInfo { return append([]GroupInfo{}, p.groups...) }

// MutexGroups 互斥组, 按登记顺序。
func (p *Parser) MutexGroups() []*MutexGroup { return append([]*MutexGroup{}, p.mutex...) }

// Subparsers 子命令集合; 没登记则为 nil。
func (p *Parser) Subparsers() *SubParsers { return p.subparsers }

// ColorEnabled 是否着色: Always / Never 直接定, Auto 看 stdout 与 `NO_COLOR`。
func (p *Parser) ColorEnabled() bool {
	switch p.color {
	case ColorAlways:
		return true
	case ColorNever:
		return false
	}
	if _, ok := os.LookupEnv("NO_COLOR"); ok {
		return false
	}
	info, err := os.Stdout.Stat()
	if err != nil {
		return false
	}
	return info.Mode()&os.ModeCharDevice != 0
}

// IsOptionToken `-` / `--` 与签名数字以外的 `-` 开头 token 是选项。
func IsOptionToken(token string) bool {
	if token == "--" {
		return true
	}
	if !strings.HasPrefix(token, "-") || token == "-" {
		return false
	}
	return !isSignedNumber(token)
}

// FormatHelp 整段帮助文本 (不含末尾换行); longHelp 为真时用长简介与长说明。
func (p *Parser) FormatHelp(longHelp bool) string { return RenderHelp(p, longHelp) }

// FormatUsage 单行用法文本 (`Usage:` 前缀在内)。
func (p *Parser) FormatUsage() string { return RenderUsage(p) }

// PrintHelp 把帮助写到流, 末尾补一个换行。
func (p *Parser) PrintHelp(writer io.Writer, longHelp bool) {
	fmt.Fprintf(writer, "%s\n", p.FormatHelp(longHelp))
}

// ParseOrExit 解析并落到进程出口: 帮助 / 版本写 out 退出码 0, 用法错误写 err 退出码 2。
//
// 成功返回 (命名空间, 0); 返回 nil 命名空间表示文本已经打印完毕, 调用方按退出码退出进程。
// 定义期错误照 Python 侧一样 panic, 不退化成一次正常退出。
func (p *Parser) ParseOrExit(argv []string, out io.Writer, err io.Writer) (*Namespace, int) {
	namespace, failure := p.Parse(argv)
	if failure == nil {
		return namespace, 0
	}
	switch errorValue := failure.(type) {
	case *HelpRequested:
		target := errorValue.Parser
		if target == nil {
			target = p
		}
		target.PrintHelp(out, errorValue.Long)
		return nil, 0
	case *VersionRequested:
		target := errorValue.Parser
		if target == nil {
			target = p
		}
		fmt.Fprintf(out, "%s\n", target.VersionText())
		return nil, 0
	case *UsageError:
		fmt.Fprintf(err, "%s\n", errorValue.Error())
		return nil, errorValue.ExitCode()
	default:
		panic(failure)
	}
}

// UsageSpec usage 行 (`Usage:` 之后) 的字符串形态。
func (p *Parser) UsageSpec() string {
	if p.usage != "" {
		return p.usage
	}
	parts := []string{p.prog}
	for _, arg := range p.arguments {
		if arg.Required && !arg.Hide {
			parts = append(parts, arg.ErrorName())
		}
	}
	hasOptional := false
	for _, arg := range p.arguments {
		if !arg.Hide && !arg.Required && !arg.IsPositional() && arg.Action != Help && arg.Action != Version {
			hasOptional = true
			break
		}
	}
	if hasOptional {
		parts = append(parts, "[OPTIONS]")
	}
	for _, arg := range p.positionals {
		if !arg.Hide {
			parts = append(parts, arg.UsageSpec())
		}
	}
	if p.subparsers != nil {
		if p.subparsers.Required {
			parts = append(parts, "<COMMAND>")
		} else {
			parts = append(parts, "[COMMAND]")
		}
	}
	return strings.Join(parts, " ")
}

// ---- 解析 ----------------------------------------------------------

// extraToken 未匹配的 token 及其扫描序号; 序号只用于还原命令行顺序。
type extraToken struct {
	slot  int
	token string
}

// run 一次解析的共享状态: 命名空间、出现记录、参与的命令链与待报缺口。
type run struct {
	tolerant  bool
	namespace *Namespace
	present   map[string]bool
	// seen 命令行上出现的 dest, 按出现顺序 (覆盖判定用)。
	seen       []string
	parsers    []*Parser
	missing    []*Argument
	extras     []extraToken
	subcommand string
}

// newRun 按容忍模式新建一次解析的共享状态。
func newRun(tolerant bool) *run {
	return &run{
		tolerant:  tolerant,
		namespace: NewNamespace(),
		present:   map[string]bool{},
	}
}

// sortedExtras 按扫描序号还原成命令行顺序。
func (r *run) sortedExtras() []string {
	sort.SliceStable(r.extras, func(left, right int) bool {
		return r.extras[left].slot < r.extras[right].slot
	})
	tokens := make([]string, len(r.extras))
	for index, item := range r.extras {
		tokens[index] = item.token
	}
	return tokens
}

// slots 给一段 token 配扫描序号。
func slots(tokens []string, start int) []extraToken {
	assigned := make([]extraToken, len(tokens))
	for index, token := range tokens {
		assigned[index] = extraToken{slot: start + index, token: token}
	}
	return assigned
}

// Parse 解析命令行; 输入非法返回错误, 请求帮助 / 版本返回对应错误类型。
func (p *Parser) Parse(argv []string) (*Namespace, error) {
	namespace, extras, err := p.parseRun(argv, false)
	if err != nil {
		return nil, err
	}
	if len(extras) > 0 {
		return nil, p.fail("unexpected argument '"+extras[0]+"' found", UnknownArgument)
	}
	return namespace, nil
}

// ParseKnown 同 Parse, 但认不出的 token 收进返回值而不报错。
//
// 未知的选项只把该 token 本身收进 extras, 不连带吞掉它后面的 token。
func (p *Parser) ParseKnown(argv []string) (*Namespace, []string, error) {
	return p.parseRun(argv, true)
}

// parseRun 两个解析入口的共用骨架; tolerant 决定未知选项是报错还是收进 extras。
func (p *Parser) parseRun(argv []string, tolerant bool) (*Namespace, []string, error) {
	tokens := argv
	if tokens == nil {
		tokens = os.Args[1:]
	}
	tokens, err := p.expandFiles(tokens, 0)
	if err != nil {
		return nil, nil, err
	}
	r := newRun(tolerant)
	if err := p.parseTokens(tokens, r, nil, 0); err != nil {
		return nil, nil, err
	}
	if err := p.finish(r); err != nil {
		return nil, nil, err
	}
	return r.namespace, r.sortedExtras(), nil
}

// maxIncludeDepth `@file` 展开的嵌套层数上限; 只用来挡住自引用, 正常用法远到不了。
const maxIncludeDepth = 16

// expandFiles 把 `@file` 形态的 token 展开成文件里的行: 每行原样作一个 token
// (不拆词、不去空白、不认注释), 展开结果里再出现的前缀继续展开。
func (p *Parser) expandFiles(tokens []string, depth int) ([]string, error) {
	if p.fromfilePrefixChars == "" {
		return tokens, nil
	}
	expanded := make([]string, 0, len(tokens))
	for _, token := range tokens {
		if token == "" || strings.IndexByte(p.fromfilePrefixChars, token[0]) < 0 {
			expanded = append(expanded, token)
			continue
		}
		path := token[1:]
		if depth >= maxIncludeDepth {
			return nil, p.fail("@file nesting too deep: '"+path+"'", Other)
		}
		body, err := os.ReadFile(path)
		if err != nil {
			return nil, p.fail("unable to open file '"+path+"'", Other)
		}
		nested, err := p.expandFiles(splitLines(string(body)), depth+1)
		if err != nil {
			return nil, err
		}
		expanded = append(expanded, nested...)
	}
	return expanded, nil
}

// splitLines 按 LF 拆行, 行尾的 CR 去掉, 末尾那个空元素不算一行。
func splitLines(body string) []string {
	lines := strings.Split(body, "\n")
	if len(lines) > 0 && lines[len(lines)-1] == "" {
		lines = lines[:len(lines)-1]
	}
	for index := range lines {
		lines[index] = strings.TrimSuffix(lines[index], "\r")
	}
	return lines
}

// parseTokens 扫描一段 token; 命中子命令时把剩余 token 整段交给子命令。
//
// base 是本段首个 token 在整条命令行里的位置, 子命令续着它往下排。
func (p *Parser) parseTokens(tokens []string, r *run, chain []*Parser, base int) error {
	r.parsers = append(r.parsers, p)
	var pending []extraToken
	index := 0
	for index < len(tokens) {
		token := tokens[index]
		if token == "--" {
			pending = append(pending, slots(tokens[index+1:], base+index+1)...)
			break
		}
		if IsOptionToken(token) {
			expanded, split, err := p.expandCluster(token, chain)
			if err != nil {
				return err
			}
			if split {
				merged := make([]string, 0, len(tokens)+len(expanded))
				merged = append(merged, tokens[:index]...)
				merged = append(merged, expanded...)
				merged = append(merged, tokens[index+1:]...)
				tokens = merged
				continue
			}
			next, err := p.consumeOption(tokens, index, r, chain, base)
			if err != nil {
				return err
			}
			index = next
			continue
		}
		if p.remainderReady(pending) {
			pending = append(pending, slots(tokens[index:], base+index)...)
			break
		}
		if p.helpSubcommandOf(token, pending) {
			target, err := p.helpTarget(tokens[index+1:])
			if err != nil {
				return err
			}
			return &HelpRequested{Long: true, Parser: target}
		}
		if child := p.subcommandOf(token, pending); child != nil {
			r.subcommand = token
			if p.subparsers != nil && p.subparsers.Dest != "" {
				r.namespace.Set(p.subparsers.Dest, p.subparsers.Canonical(token))
			}
			nextChain := append(append([]*Parser{}, chain...), p)
			if err := child.parseTokens(tokens[index+1:], r, nextChain, base+index+1); err != nil {
				return err
			}
			index = len(tokens)
			break
		}
		pending = append(pending, extraToken{slot: base + index, token: token})
		index++
	}
	return p.assignPositionals(pending, r)
}

// expandCluster 短选项聚合 `-abc` 拆成 `-a -b -c`; 拆不动时 split 为 false。
func (p *Parser) expandCluster(token string, chain []*Parser) ([]string, bool, error) {
	if strings.HasPrefix(token, "--") || len(token) <= 2 || strings.Contains(token, "=") {
		return nil, false, nil
	}
	first, err := p.lookup("-"+token[1:2], chain)
	if err != nil {
		return nil, false, err
	}
	if first == nil || first.TakesValues() {
		return nil, false, nil
	}
	var expanded []string
	index := 1
	for index < len(token) {
		piece := "-" + token[index:index+1]
		arg, err := p.lookup(piece, chain)
		if err != nil {
			return nil, false, err
		}
		if arg == nil {
			// 未认出的尾巴连短横线一起留下, 由后续识别失败报出具体的短选项
			expanded = append(expanded, "-"+token[index:])
			break
		}
		if arg.TakesValues() {
			rest := token[index+1:]
			rest = strings.TrimPrefix(rest, "=")
			if rest == "" {
				expanded = append(expanded, piece)
			} else {
				expanded = append(expanded, piece+"="+rest)
			}
			break
		}
		expanded = append(expanded, piece)
		index++
	}
	return expanded, true, nil
}

// positionalMinimum 本命令位置参数至少要几个 token 才够填。
func (p *Parser) positionalMinimum() int {
	needed := 0
	for _, arg := range p.positionals {
		needed += arg.MinCount()
	}
	return needed
}

// subcommandOf token 是子命令名且本命令的位置参数已够填, 才当成子命令。
func (p *Parser) subcommandOf(token string, pending []extraToken) *Parser {
	if p.subparsers == nil {
		return nil
	}
	child := p.subparsers.Lookup(token)
	if child == nil {
		return nil
	}
	if len(pending) < p.positionalMinimum() {
		return nil
	}
	return child
}

// helpSubcommandOf token 是否命中自动生成的 `help` 子命令。
//
// 本命令位置参数尚未够填时 `help` 仍是普通位置 token; 用户自己登记了 `help` 子命令时以用户的为准。
func (p *Parser) helpSubcommandOf(token string, pending []extraToken) bool {
	if token != "help" || p.subparsers == nil || p.subparsers.HasCommand("help") {
		return false
	}
	return len(pending) >= p.positionalMinimum()
}

// helpTarget `help a b` 的下沉目标: 按子命令名逐级走, 走到认不出的名字即报错。
func (p *Parser) helpTarget(rest []string) (*Parser, error) {
	target := p
	for _, token := range rest {
		var child *Parser
		if target.subparsers != nil {
			child = target.subparsers.Lookup(token)
		}
		if child == nil {
			return nil, target.fail("unrecognized subcommand '"+token+"'", UnknownArgument)
		}
		target = child
	}
	return target, nil
}

// remainderReady REMAINDER 位置参数的前序位置参数已经填够, 后续 token 全归它。
func (p *Parser) remainderReady(pending []extraToken) bool {
	for index, arg := range p.positionals {
		if !arg.Nargs.IsRemainder() {
			continue
		}
		needed := 0
		for _, item := range p.positionals[:index] {
			needed += item.MinCount()
		}
		return len(pending) >= needed
	}
	return false
}

// lookup 先本命令 (含长选项前缀匹配), 再祖先命令上的 Global 参数。
func (p *Parser) lookup(name string, chain []*Parser) (*Argument, error) {
	if arg, ok := p.byName[name]; ok {
		return arg, nil
	}
	if p.allowAbbrev && strings.HasPrefix(name, "--") && len(name) > 2 {
		arg, err := p.abbreviate(name)
		if err != nil {
			return nil, err
		}
		if arg != nil {
			return arg, nil
		}
	}
	for index := len(chain) - 1; index >= 0; index-- {
		if found, ok := chain[index].byName[name]; ok && found.Global {
			return found, nil
		}
	}
	return nil, nil
}

// abbreviate 长选项唯一前缀匹配; 命中多个即报歧义。
func (p *Parser) abbreviate(name string) (*Argument, error) {
	matched := map[string]*Argument{}
	var order []string
	for _, option := range p.nameOrder {
		if !strings.HasPrefix(option, "--") || !strings.HasPrefix(option, name) {
			continue
		}
		arg := p.byName[option]
		if _, ok := matched[arg.Dest]; !ok {
			matched[arg.Dest] = arg
			order = append(order, arg.Dest)
		}
	}
	if len(order) == 0 {
		return nil, nil
	}
	if len(order) > 1 {
		seen := map[string]bool{}
		var options []string
		for _, option := range p.nameOrder {
			if strings.HasPrefix(option, name) && !seen[option] {
				seen[option] = true
				options = append(options, option)
			}
		}
		sort.Strings(options)
		return nil, p.fail("ambiguous argument '"+name+"'", AmbiguousArgument,
			withLines("could be one of: "+FormatNames(options)))
	}
	return matched[order[0]], nil
}

// unknownArgument 未知选项: 给出相似项建议, 长选项再补一句怎么当取值传。
func (p *Parser) unknownArgument(name string, chain []*Parser) *UsageError {
	candidates := append([]string{}, p.nameOrder...)
	for _, parent := range chain {
		for _, option := range parent.nameOrder {
			if parent.byName[option].Global {
				candidates = append(candidates, option)
			}
		}
	}
	var tips []string
	if suggestion, ok := BestMatch(name, candidates); ok {
		tips = append(tips, "a similar argument exists: '"+suggestion+"'")
	} else if strings.HasPrefix(name, "--") {
		tips = append(tips, "to pass '"+name+"' as a value, use '-- "+name+"'")
	}
	return p.fail("unexpected argument '"+name+"' found", UnknownArgument, withTips(tips...))
}

// consumeOption 处理一个选项 token, 返回下一个待处理下标。
func (p *Parser) consumeOption(tokens []string, index int, r *run, chain []*Parser, base int) (int, error) {
	token := tokens[index]
	name, inline, hasInline := splitOption(token)
	arg, err := p.lookup(name, chain)
	if err != nil {
		return 0, err
	}
	if arg == nil {
		attached, value, ok, err := p.attachedShort(name, chain)
		if err != nil {
			return 0, err
		}
		if !ok {
			if r.tolerant {
				r.extras = append(r.extras, extraToken{slot: base + index, token: token})
				return index + 1, nil
			}
			return 0, p.unknownArgument(name, chain)
		}
		arg, inline, hasInline = attached, value, true
	}
	if !arg.TakesValues() {
		if hasInline {
			return 0, p.fail(
				"unexpected value '"+inline+"' for '"+arg.ErrorName()+"' found; no more were expected",
				InvalidValue,
			)
		}
		// 长短帮助靠命中的名字区分 (`-h` 走短帮助, `--help` 走长帮助)
		switch arg.Action {
		case Help:
			return 0, &HelpRequested{Long: strings.HasPrefix(name, "--"), Parser: p}
		case Version:
			return 0, &VersionRequested{Parser: p}
		}
		if err := p.apply(arg, nil, r); err != nil {
			return 0, err
		}
		return index + 1, nil
	}
	if hasInline {
		values, err := p.convertTexts(arg, []string{inline})
		if err != nil {
			return 0, err
		}
		if err := p.apply(arg, values, r); err != nil {
			return 0, err
		}
		return index + 1, nil
	}
	stop, raw, err := p.takeValues(arg, tokens, index+1)
	if err != nil {
		return 0, err
	}
	values, err := p.convertTexts(arg, raw)
	if err != nil {
		return 0, err
	}
	if err := p.apply(arg, values, r); err != nil {
		return 0, err
	}
	return stop, nil
}

// attachedShort `-ovalue` / `-o=value`: 短选项后面的部分就是它的取值。
func (p *Parser) attachedShort(name string, chain []*Parser) (*Argument, string, bool, error) {
	if strings.HasPrefix(name, "--") || len(name) <= 2 {
		return nil, "", false, nil
	}
	arg, err := p.lookup(name[:2], chain)
	if err != nil {
		return nil, "", false, err
	}
	if arg == nil || !arg.TakesValues() {
		return nil, "", false, nil
	}
	value := strings.TrimPrefix(name[2:], "=")
	return arg, value, true, nil
}

// takeValues 按 nargs 取走取值, 返回 (下一下标, 原始取值)。
func (p *Parser) takeValues(arg *Argument, tokens []string, start int) (int, []string, error) {
	if arg.Nargs.IsRemainder() {
		return len(tokens), append([]string{}, tokens[start:]...), nil
	}
	minimum, maximum := arg.MinCount(), arg.MaxCount()
	var taken []string
	index := start
	for index < len(tokens) {
		if maximum >= 0 && len(taken) >= maximum {
			break
		}
		if IsOptionToken(tokens[index]) && !arg.AllowHyphenValues {
			break
		}
		taken = append(taken, tokens[index])
		index++
	}
	if len(taken) < minimum {
		return 0, nil, p.failMissingValue(arg, minimum, len(taken))
	}
	return index, taken, nil
}

// failMissingValue 取值不足的报错; 单个与多个的文案分开, 与 clap 同形。
func (p *Parser) failMissingValue(arg *Argument, needed, given int) *UsageError {
	name := arg.ErrorName()
	if needed <= 1 {
		return p.fail("a value is required for '"+name+"' but none was supplied", MissingValue)
	}
	verb := "were"
	if given == 1 {
		verb = "was"
	}
	return p.fail(fmt.Sprintf("%d values required for '%s' but %d %s provided", needed, name, given, verb), MissingValue)
}

// convertTexts 按 value_delimiter 拆分后逐个转换。
func (p *Parser) convertTexts(arg *Argument, raw []string) ([]any, error) {
	var texts []string
	for _, text := range raw {
		if arg.ValueDelimiter != "" {
			texts = append(texts, strings.Split(text, arg.ValueDelimiter)...)
			continue
		}
		texts = append(texts, text)
	}
	values := make([]any, 0, len(texts))
	for _, text := range texts {
		value, err := p.convert(arg, text)
		if err != nil {
			return nil, err
		}
		values = append(values, value)
	}
	return values, nil
}

// convert 转换单个取值并校验可选值。
func (p *Parser) convert(arg *Argument, text string) (any, error) {
	var value any = text
	if arg.Type != nil {
		converted, err := arg.Type.Parse(text)
		if err != nil {
			return nil, p.fail("invalid value '"+text+"' for '"+arg.ErrorName()+"'", InvalidValue, withReason(err.Error()))
		}
		value = converted
	}
	choices := arg.ChoiceValues()
	if choices != nil {
		matched := false
		for _, candidate := range choices {
			if reflect.DeepEqual(candidate, value) {
				matched = true
				break
			}
		}
		if !matched {
			labels := make([]string, len(choices))
			for index, candidate := range choices {
				labels[index] = FormatValue(candidate)
			}
			var tips []string
			if suggestion, ok := BestMatch(text, labels); ok {
				tips = append(tips, "a similar value exists: '"+suggestion+"'")
			}
			return nil, p.fail("invalid value '"+text+"' for '"+arg.ErrorName()+"'", InvalidValue,
				withLines("[possible values: "+FormatItems(choices)+"]"),
				withTips(tips...))
		}
	}
	return value, nil
}

// apply 把一次出现落进命名空间, 按动作决定覆盖 / 追加 / 计数。
func (p *Parser) apply(arg *Argument, values []any, r *run) error {
	switch arg.Action {
	case Help:
		return &HelpRequested{Long: arg.PrimaryName() == "--help", Parser: p}
	case Version:
		return &VersionRequested{Parser: p}
	}
	r.present[arg.Dest] = true
	r.seen = append(r.seen, arg.Dest)
	switch arg.Action {
	case Count:
		base, ok := r.namespace.Get(arg.Dest)
		count, isCount := base.(int64)
		if !ok || !isCount {
			count, _ = implicitDefault(arg).(int64)
		}
		r.namespace.Set(arg.Dest, count+1)
		return nil
	case StoreTrue, StoreFalse, StoreConst:
		r.namespace.Set(arg.Dest, mustValue(arg.Const))
		return nil
	case AppendConst:
		p.appendValue(r, arg.Dest, mustValue(arg.Const))
		return nil
	case Extend:
		base, ok := r.namespace.Get(arg.Dest)
		items, isList := base.([]any)
		if !ok || !isList {
			items = defaultList(arg.Default)
		} else {
			items = append([]any{}, items...)
		}
		items = append(items, values...)
		r.namespace.Set(arg.Dest, items)
		return nil
	case Append:
		if arg.Collects() {
			p.appendValue(r, arg.Dest, values)
			return nil
		}
		var item any
		if len(values) > 0 {
			item = values[0]
		}
		p.appendValue(r, arg.Dest, item)
		return nil
	}
	if len(values) == 0 {
		r.namespace.Set(arg.Dest, mustValue(arg.Const))
		return nil
	}
	if arg.Collects() {
		r.namespace.Set(arg.Dest, values)
		return nil
	}
	r.namespace.Set(arg.Dest, values[0])
	return nil
}

// appendValue 往命名空间里的列表追加一项; 首次出现时新开列表。
func (p *Parser) appendValue(r *run, dest string, item any) {
	current, ok := r.namespace.Get(dest)
	var items []any
	if ok {
		if existing, isList := current.([]any); isList {
			items = append([]any{}, existing...)
		}
	}
	items = append(items, item)
	r.namespace.Set(dest, items)
}

// assignPositionals 把攒下的非选项 token 按定义顺序分给位置参数。
//
// 变长形态 (`*` / `+` / `?`) 先给后面的位置参数留够最小值, 剩下的归自己。
func (p *Parser) assignPositionals(pending []extraToken, r *run) error {
	tokens := make([]string, len(pending))
	for index, item := range pending {
		tokens[index] = item.token
	}
	cursor := 0
	total := len(tokens)
	for index, arg := range p.positionals {
		tail := 0
		for _, item := range p.positionals[index+1:] {
			tail += item.MinCount()
		}
		available := total - cursor - tail
		if available < 0 {
			available = 0
		}
		var take int
		switch {
		case arg.Nargs.IsRemainder():
			take = total - cursor
		case arg.MaxCount() < 0:
			take = available
		default:
			take = min(arg.MaxCount(), available)
		}
		if take < arg.MinCount() {
			r.missing = append(r.missing, arg)
			continue
		}
		chunk := tokens[cursor : cursor+take]
		cursor += take
		if err := p.storePositional(arg, chunk, r); err != nil {
			return err
		}
	}
	if cursor < total {
		r.extras = append(r.extras, pending[cursor:]...)
	}
	return nil
}

// storePositional 位置参数的落值: 空片段按 const / default / 空列表依次退让。
func (p *Parser) storePositional(arg *Argument, chunk []string, r *run) error {
	if len(chunk) == 0 {
		switch {
		case !arg.Const.IsMissing():
			r.namespace.Set(arg.Dest, mustValue(arg.Const))
		case !arg.Default.IsMissing():
			r.namespace.Set(arg.Dest, mustValue(arg.Default))
		case arg.Nargs.kind == nargsZeroOrMore, arg.Nargs.IsRemainder():
			r.namespace.Set(arg.Dest, []any{})
		default:
			r.namespace.Set(arg.Dest, nil)
		}
		return nil
	}
	r.present[arg.Dest] = true
	r.seen = append(r.seen, arg.Dest)
	values, err := p.convertTexts(arg, chunk)
	if err != nil {
		return err
	}
	if arg.Collects() {
		r.namespace.Set(arg.Dest, values)
		return nil
	}
	r.namespace.Set(arg.Dest, values[0])
	return nil
}

// finish 解析后收尾: 环境变量兜底 → 校验 → 填默认值。
func (p *Parser) finish(r *run) error {
	if err := p.applyOverrides(r); err != nil {
		return err
	}
	if err := p.applyEnv(r); err != nil {
		return err
	}
	if err := p.validate(r); err != nil {
		return err
	}
	p.applyDefaults(r)
	return nil
}

// applyOverrides `overrides_with` 双向生效: 一对里命令行上后出现的留下, 先出现的退回默认值。
func (p *Parser) applyOverrides(r *run) error {
	registry := map[string]*Argument{}
	byName := map[string]*Argument{}
	for _, parser := range r.parsers {
		for _, arg := range parser.arguments {
			// 帮助 / 版本只在解析途中返回控制流, 从不落进命名空间, 各层同名不冲突
			if arg.Action == Help || arg.Action == Version {
				continue
			}
			registry[arg.Dest] = arg
		}
		for _, name := range parser.nameOrder {
			byName[name] = parser.byName[name]
		}
	}
	resolve := func(ref string) (string, error) {
		if _, ok := registry[ref]; ok {
			return ref, nil
		}
		found, ok := byName[ref]
		if !ok {
			return "", &DefinitionError{Message: fmt.Sprintf("overrides_with / conflicts_with / requires 指向了未登记的参数: %q", ref)}
		}
		return found.Dest, nil
	}

	pairs := map[string][]string{}
	for dest, arg := range registry {
		for _, ref := range arg.OverridesWith {
			other, err := resolve(ref)
			if err != nil {
				return err
			}
			pairs[dest] = append(pairs[dest], other)
			pairs[other] = append(pairs[other], dest)
		}
	}
	if len(pairs) == 0 {
		return nil
	}

	kept := map[string]bool{}
	for _, dest := range r.seen {
		for _, other := range pairs[dest] {
			delete(kept, other)
		}
		kept[dest] = true
	}
	for _, dest := range r.seen {
		if kept[dest] {
			continue
		}
		delete(r.present, dest)
		r.namespace.Remove(dest)
	}
	return nil
}

// applyEnv 命令行没出现时读环境变量; 命中即视为"出现过"。
func (p *Parser) applyEnv(r *run) error {
	for _, parser := range r.parsers {
		for _, arg := range parser.arguments {
			if arg.Env == "" || r.present[arg.Dest] {
				continue
			}
			text, ok := os.LookupEnv(arg.Env)
			if !ok {
				continue
			}
			if arg.TakesValues() {
				values, err := parser.convertTexts(arg, []string{text})
				if err != nil {
					return err
				}
				if err := parser.apply(arg, values, r); err != nil {
					return err
				}
				continue
			}
			converted, err := ParseBool().Parse(text)
			if err != nil {
				return parser.fail("invalid value '"+text+"' for '"+arg.ErrorName()+"'", InvalidValue, withReason(err.Error()))
			}
			enabled, _ := converted.(bool)
			fallback := any(false)
			if !arg.Default.IsMissing() && !arg.Default.IsSuppress() {
				fallback = mustValue(arg.Default)
			}
			if enabled {
				r.namespace.Set(arg.Dest, mustValue(arg.Const))
			} else {
				r.namespace.Set(arg.Dest, fallback)
			}
			r.present[arg.Dest] = true
		}
	}
	return nil
}

// validate conflicts_with → 互斥组 → 多值组 → requires → 必填; 报出第一处问题。
func (p *Parser) validate(r *run) error {
	parsed := r.parsers
	leaf := parsed[len(parsed)-1]
	registry := map[string]*Argument{}
	var registryOrder []*Argument
	byName := map[string]*Argument{}
	for _, parser := range parsed {
		for _, arg := range parser.arguments {
			// 帮助 / 版本只在解析途中返回控制流, 从不落进命名空间, 各层同名不冲突
			if arg.Action == Help || arg.Action == Version {
				continue
			}
			if _, ok := registry[arg.Dest]; ok {
				return &DefinitionError{Message: fmt.Sprintf("命令链上的 dest 重复: %q (父命令与子命令不能共用 dest)", arg.Dest)}
			}
			registry[arg.Dest] = arg
			registryOrder = append(registryOrder, arg)
		}
		for _, name := range parser.nameOrder {
			byName[name] = parser.byName[name]
		}
	}
	resolve := func(ref string) (string, error) {
		if _, ok := registry[ref]; ok {
			return ref, nil
		}
		found, ok := byName[ref]
		if !ok {
			return "", &DefinitionError{Message: fmt.Sprintf("overrides_with / conflicts_with / requires 指向了未登记的参数: %q", ref)}
		}
		return found.Dest, nil
	}
	label := func(dest string) string { return registry[dest].ErrorName() }

	for _, arg := range registryOrder {
		if !r.present[arg.Dest] {
			continue
		}
		for _, ref := range arg.ConflictsWith {
			dest, err := resolve(ref)
			if err != nil {
				return err
			}
			if r.present[dest] {
				return leaf.fail(
					"the argument '"+arg.ErrorName()+"' cannot be used with '"+label(dest)+"'",
					Conflict,
				)
			}
		}
	}

	for _, parser := range parsed {
		for _, group := range parser.mutex {
			var hits []*Argument
			for _, arg := range group.Arguments {
				if r.present[arg.Dest] {
					hits = append(hits, arg)
				}
			}
			if len(hits) > 1 {
				return leaf.fail(
					"the argument '"+hits[0].ErrorName()+"' cannot be used with '"+hits[1].ErrorName()+"'",
					Conflict,
				)
			}
			if len(hits) == 0 && group.Required {
				lines := make([]string, 0, len(group.Arguments))
				for _, arg := range group.Arguments {
					lines = append(lines, arg.ErrorName())
				}
				return leaf.fail("one of the following arguments is required:", MissingRequired, withLines(lines...))
			}
		}
	}

	for _, parser := range parsed {
		for _, group := range parser.valueGroups {
			if !group.Required {
				continue
			}
			hit := false
			for _, arg := range group.Arguments {
				if r.present[arg.Dest] {
					hit = true
					break
				}
			}
			if hit {
				continue
			}
			return leaf.fail(
				"the following required arguments were not provided:",
				MissingRequired,
				withLines("<"+group.Name+">"),
			)
		}
	}

	for _, arg := range registryOrder {
		if !r.present[arg.Dest] {
			continue
		}
		for _, ref := range arg.Requires {
			dest, err := resolve(ref)
			if err != nil {
				return err
			}
			if !r.present[dest] {
				return leaf.fail(
					"the argument '"+arg.ErrorName()+"' requires '"+label(dest)+"'",
					MissingRequired,
				)
			}
		}
	}

	missing := append([]*Argument{}, r.missing...)
	for _, parser := range parsed {
		if parser.subcommandNegatesReqs && r.subcommand != "" {
			continue
		}
		for _, arg := range parser.arguments {
			if r.present[arg.Dest] {
				continue
			}
			needed := arg.Required
			if !needed && len(arg.RequiredUnlessPresent) > 0 {
				satisfied := false
				for _, ref := range arg.RequiredUnlessPresent {
					dest, err := resolve(ref)
					if err != nil {
						return err
					}
					if r.present[dest] {
						satisfied = true
						break
					}
				}
				needed = !satisfied
			}
			if needed {
				missing = append(missing, arg)
			}
		}
		if parser == leaf && parser.subparsers != nil && parser.subparsers.Required {
			return leaf.fail("the following required arguments were not provided:", MissingRequired, withLines("<COMMAND>"))
		}
	}
	if len(missing) > 0 {
		return leaf.fail(
			"the following required arguments were not provided:",
			MissingRequired,
			withLines(uniqueLabels(missing)...),
		)
	}
	return nil
}

// applyDefaults 没出现在命令行上的参数填默认值; `Suppress` 表示不写进命名空间。
func (p *Parser) applyDefaults(r *run) {
	for _, parser := range r.parsers {
		for _, arg := range parser.arguments {
			if r.namespace.Contains(arg.Dest) {
				continue
			}
			chosen := arg.Default
			if chosen.IsMissing() {
				chosen = parser.argumentDefault
			}
			if chosen.IsSuppress() {
				continue
			}
			value := implicitDefault(arg)
			if !chosen.IsMissing() {
				value = mustValue(chosen)
			}
			r.namespace.Set(arg.Dest, value)
		}
	}
}

// fail 构造一个带类别、本命令 usage 与帮助提示的 UsageError。
func (p *Parser) fail(message string, kind ErrorKind, options ...failOption) *UsageError {
	failure := &UsageError{
		Message: message,
		Kind:    kind,
		Usage:   p.UsageSpec(),
	}
	if _, ok := p.dests["help"]; ok {
		failure.Hint = "For more information, try '--help'."
	}
	for _, option := range options {
		option(failure)
	}
	return failure
}

// failOption 往 UsageError 上补字段。
type failOption func(*UsageError)

// withReason 补首行冒号之后的原因。
func withReason(reason string) failOption {
	return func(failure *UsageError) { failure.Reason = reason }
}

// withLines 补缩进的明细行。
func withLines(lines ...string) failOption {
	return func(failure *UsageError) { failure.Lines = append(failure.Lines, lines...) }
}

// withTips 补 `tip:` 开头的建议行。
func withTips(tips ...string) failOption {
	return func(failure *UsageError) { failure.Tips = append(failure.Tips, tips...) }
}

// withUsage 替换 usage 行。
func withUsage(usage string) failOption {
	return func(failure *UsageError) { failure.Usage = usage }
}

// withHint 替换帮助提示行。
func withHint(hint string) failOption {
	return func(failure *UsageError) { failure.Hint = hint }
}

// mustValue 取默认值本体; 调用方已确认不是 Missing / Suppress。
func mustValue(value DefaultValue) any {
	if resolved, ok := value.Get(); ok {
		return resolved
	}
	if value.IsSuppress() {
		return nil
	}
	return nil
}

// implicitDefault 没显式给默认值时的落值。
func implicitDefault(arg *Argument) any {
	switch arg.Action {
	case StoreTrue:
		return false
	case StoreFalse:
		return true
	case Count:
		return int64(0)
	}
	if arg.IsPositional() && (arg.Nargs.kind == nargsZeroOrMore || arg.Nargs.IsRemainder()) {
		return []any{}
	}
	return nil
}

// inferParser 按默认值的类型推断取值转换器。
func inferParser(value DefaultValue) *ValueParser {
	resolved, ok := value.Get()
	if !ok {
		return nil
	}
	var parser ValueParser
	switch resolved.(type) {
	case bool:
		parser = ParseBool()
	case int, int64:
		parser = ParseInt(nil, nil)
	case float64:
		parser = ParseFloat(nil, nil)
	default:
		return nil
	}
	return &parser
}

// defaultList 把默认值当成列表初值。
func defaultList(value DefaultValue) []any {
	resolved, ok := value.Get()
	if !ok {
		return nil
	}
	switch items := resolved.(type) {
	case []any:
		return append([]any{}, items...)
	case []string:
		copied := make([]any, len(items))
		for index, item := range items {
			copied[index] = item
		}
		return copied
	}
	return nil
}

// uniqueLabels 保持顺序去重后取错误文案里的指代。
func uniqueLabels(arguments []*Argument) []string {
	seen := map[string]bool{}
	var labels []string
	for _, arg := range arguments {
		label := arg.ErrorName()
		if seen[label] {
			continue
		}
		seen[label] = true
		labels = append(labels, label)
	}
	return labels
}

// splitOption 把 `--key=value` / `-k=value` 拆成 (名字, 内联取值)。
func splitOption(token string) (string, string, bool) {
	if name, value, found := strings.Cut(token, "="); found {
		return name, value, true
	}
	return token, "", false
}

// isSignedNumber 签名数字: 可选的 `+` / `-` 号加整数或小数, 整个 token 当一个取值参数 (`-1`、`-.5`、`+3`、`-1.5`)。
func isSignedNumber(token string) bool {
	body, ok := strings.CutPrefix(token, "-")
	if !ok {
		body, ok = strings.CutPrefix(token, "+")
	}
	if !ok || body == "" {
		return false
	}
	if intPart, fracPart, found := strings.Cut(body, "."); found {
		return allDigits(intPart) && fracPart != "" && allDigits(fracPart)
	}
	return body != "" && allDigits(body)
}

// allDigits 是否全部是十进制数字; 空串视为全部满足 (`\d*` 的语义)。
func allDigits(text string) bool {
	for _, char := range text {
		if char < '0' || char > '9' {
			return false
		}
	}
	return true
}

// destOfOption 由选项名推 dest: 优先最长形态的长选项, 中划线转下划线。
func destOfOption(names []string) string {
	for _, name := range names {
		if body, ok := strings.CutPrefix(name, "--"); ok {
			return strings.ReplaceAll(body, "-", "_")
		}
	}
	if len(names) > 0 {
		return strings.ReplaceAll(strings.TrimLeft(names[0], "-"), "-", "_")
	}
	return ""
}
