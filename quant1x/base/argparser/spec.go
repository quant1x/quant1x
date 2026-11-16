// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

// Package argparser 提供 argparse 语义的命令行解析, 与 Python spec `quant1x/base/argparser/*.py` 同口径。
//
// 解析是单趟左到右的扫描: 遇到选项就地取走它的取值, 其余 token 攒起来交给位置参数;
// 遇到子命令名则把剩余 token 整段交给子命令, 此前攒下的 token 仍归本命令的位置参数。
//
// 失败一律返回 error, 不打印也不退出; 输出与退出码由调用方按错误类型决定。
package argparser

import (
	"errors"
	"fmt"
	"math"
	"strconv"
	"strings"
)

// Action 参数出现时怎么取值、怎么落进命名空间, 对应 Python 的 Action。
type Action int

const (
	// Store 落单个取值或取值列表。
	Store Action = iota
	// StoreTrue 开关, 出现即为真。
	StoreTrue
	// StoreFalse 反开关, 出现即为假。
	StoreFalse
	// StoreConst 落常量。
	StoreConst
	// Append 逐次追加一项。
	Append
	// AppendConst 逐次追加同一个常量。
	AppendConst
	// Extend 把每次的取值展开进同一个列表。
	Extend
	// Count 出现次数。
	Count
	// Help 请求帮助。
	Help
	// Version 请求版本。
	Version
)

// String 返回动作名, 与 Python 的 `Action.value` 同形。
func (a Action) String() string {
	switch a {
	case Store:
		return "store"
	case StoreTrue:
		return "store_true"
	case StoreFalse:
		return "store_false"
	case StoreConst:
		return "store_const"
	case Append:
		return "append"
	case AppendConst:
		return "append_const"
	case Extend:
		return "extend"
	case Count:
		return "count"
	case Help:
		return "help"
	case Version:
		return "version"
	}
	return "unknown"
}

// TakesValues 该动作是否从命令行吃掉取值。
func (a Action) TakesValues() bool {
	return a == Store || a == Append || a == Extend
}

// CoerceAction 把动作名归一成 Action; 无法识别即返回 error。
//
// 键做过小写与去下划线 / 中划线归一, 兼容 clap 的 `SetTrue` 一类拼法。
func CoerceAction(text string) (Action, error) {
	key := strings.ToLower(strings.TrimSpace(text))
	key = strings.NewReplacer("_", "", "-", "").Replace(key)
	switch key {
	case "store", "set":
		return Store, nil
	case "storetrue", "settrue":
		return StoreTrue, nil
	case "storefalse", "setfalse":
		return StoreFalse, nil
	case "storeconst":
		return StoreConst, nil
	case "append":
		return Append, nil
	case "appendconst":
		return AppendConst, nil
	case "extend":
		return Extend, nil
	case "count":
		return Count, nil
	case "help":
		return Help, nil
	case "version":
		return Version, nil
	}
	return Store, fmt.Errorf("无法识别的 action: %q", text)
}

// nargsKind 取值形态的种类。
type nargsKind int

const (
	nargsOne nargsKind = iota
	nargsFixed
	nargsOptional
	nargsZeroOrMore
	nargsOneOrMore
	nargsRemainder
)

// Nargs 取值形态; 零值是一个取值。
type Nargs struct {
	kind  nargsKind
	count int
}

// NargsOne 一个取值。
func NargsOne() Nargs { return Nargs{} }

// NargsFixed 恰好 N 个取值; 负数返回 error。
func NargsFixed(count int) (Nargs, error) {
	if count < 0 {
		return Nargs{}, fmt.Errorf("nargs 不能为负: %d", count)
	}
	return Nargs{kind: nargsFixed, count: count}, nil
}

// NargsOptional `?`: 最多一个取值。
func NargsOptional() Nargs { return Nargs{kind: nargsOptional} }

// NargsZeroOrMore `*`: 任意个取值。
func NargsZeroOrMore() Nargs { return Nargs{kind: nargsZeroOrMore} }

// NargsOneOrMore `+`: 至少一个取值。
func NargsOneOrMore() Nargs { return Nargs{kind: nargsOneOrMore} }

// NargsRemainder 其余 token 全部收下, 对应 `argparse.REMAINDER`。
func NargsRemainder() Nargs { return Nargs{kind: nargsRemainder} }

// Kind 取值形态的种类名, 与 Python 的 `nargs` 写法同形。
func (n Nargs) Kind() string {
	switch n.kind {
	case nargsFixed:
		return strconv.Itoa(n.count)
	case nargsOptional:
		return "?"
	case nargsZeroOrMore:
		return "*"
	case nargsOneOrMore:
		return "+"
	case nargsRemainder:
		return "..."
	}
	return ""
}

// Count 整数形态的取值个数; 其余形态为 0。
func (n Nargs) Count() int {
	if n.kind == nargsFixed {
		return n.count
	}
	return 0
}

// IsRemainder 是否 REMAINDER 形态。
func (n Nargs) IsRemainder() bool { return n.kind == nargsRemainder }

// Min 最少吃几个取值。
func (n Nargs) Min() int {
	switch n.kind {
	case nargsFixed:
		return n.count
	case nargsOptional, nargsZeroOrMore, nargsRemainder:
		return 0
	}
	return 1
}

// Max 最多吃几个取值; 无上限返回 -1。
func (n Nargs) Max() int {
	switch n.kind {
	case nargsFixed:
		return n.count
	case nargsOptional:
		return 1
	case nargsZeroOrMore, nargsOneOrMore, nargsRemainder:
		return -1
	}
	return 1
}

// Collects 取值是否恒以列表形态落进命名空间。
func (n Nargs) Collects() bool {
	switch n.kind {
	case nargsFixed, nargsZeroOrMore, nargsOneOrMore, nargsRemainder:
		return true
	}
	return false
}

// defaultValueKind 缺省值来源的种类。
type defaultValueKind int

const (
	defaultValueMissing defaultValueKind = iota
	defaultValueSuppress
	defaultValueGiven
)

// DefaultValue 缺省值来源: 没给 (`Missing`) / 不写进命名空间 (`Suppress`) / 具体取值。
//
// 零值是 `Missing`, 与"默认值为 nil"区分; 显式给出 nil 用 `ValueOf(nil)`。
type DefaultValue struct {
	kind  defaultValueKind
	value any
}

var (
	// Missing 没有显式给出默认值。
	Missing = DefaultValue{}
	// Suppress 默认值取本标记表示不写进命名空间, 对应 `argparse.SUPPRESS`。
	Suppress = DefaultValue{kind: defaultValueSuppress}
)

// ValueOf 显式给出一个默认值。
func ValueOf(value any) DefaultValue {
	return DefaultValue{kind: defaultValueGiven, value: value}
}

// IsMissing 是否没给默认值。
func (d DefaultValue) IsMissing() bool { return d.kind == defaultValueMissing }

// IsSuppress 是否 `Suppress`。
func (d DefaultValue) IsSuppress() bool { return d.kind == defaultValueSuppress }

// Get 取默认值本体; 没给默认值时 ok 为 false。
func (d DefaultValue) Get() (any, bool) {
	if d.kind != defaultValueGiven {
		return nil, false
	}
	return d.value, true
}

// String 默认值的文本形态。
func (d DefaultValue) String() string {
	switch d.kind {
	case defaultValueMissing:
		return "MISSING"
	case defaultValueSuppress:
		return "SUPPRESS"
	}
	return FormatValue(d.value)
}

// ValueParser 取值转换器: 把命令行上的字符串翻成取值, 非法取值返回 error。
//
// TypeName 只用于文案 (帮助里的取值说明), 不参与转换。
type ValueParser struct {
	Convert  func(text string) (any, error)
	TypeName string
}

// Parse 转换一个取值。
func (p ValueParser) Parse(text string) (any, error) { return p.Convert(text) }

// rangeText 把上下界写成 clap 风格的开区间记法。
func rangeText(minimum, maximum *float64) string {
	switch {
	case minimum == nil && maximum == nil:
		return ""
	case minimum == nil:
		return "..=" + floatText(*maximum)
	case maximum == nil:
		return floatText(*minimum) + ".."
	}
	return floatText(*minimum) + "..=" + floatText(*maximum)
}

// checkRange 越界即返回 error, 文案与 clap 的 ranged parser 同形。
func checkRange(value float64, minimum, maximum *float64) error {
	text := rangeText(minimum, maximum)
	if minimum != nil && value < *minimum {
		return fmt.Errorf("%s is not in %s", floatText(value), text)
	}
	if maximum != nil && value > *maximum {
		return fmt.Errorf("%s is not in %s", floatText(value), text)
	}
	return nil
}

// ParseInt 十进制整数转换器, 可带闭区间上下界。
func ParseInt(minimum, maximum *int64) ValueParser {
	return ValueParser{
		TypeName: "INTEGER",
		Convert: func(text string) (any, error) {
			value, err := strconv.ParseInt(text, 10, 64)
			if err != nil {
				return nil, errors.New("invalid digit found in string")
			}
			if err := checkIntRange(value, minimum, maximum); err != nil {
				return nil, err
			}
			return value, nil
		},
	}
}

// ParseFloat 浮点转换器, 可带闭区间上下界。
func ParseFloat(minimum, maximum *float64) ValueParser {
	return ValueParser{
		TypeName: "FLOAT",
		Convert: func(text string) (any, error) {
			value, err := strconv.ParseFloat(text, 64)
			if err != nil {
				return nil, errors.New("invalid float literal")
			}
			if err := checkRange(value, minimum, maximum); err != nil {
				return nil, err
			}
			return value, nil
		},
	}
}

// parseBoolWords 布尔取值的可识别写法, 大小写不敏感。
var parseBoolWords = map[string]bool{
	"true":  true,
	"yes":   true,
	"on":    true,
	"1":     true,
	"false": false,
	"no":    false,
	"off":   false,
	"0":     false,
}

// ParseBool 布尔转换器: `true/false`、`yes/no`、`on/off`、`1/0`, 大小写不敏感。
func ParseBool() ValueParser {
	return ValueParser{
		TypeName: "BOOL",
		Convert: func(text string) (any, error) {
			value, ok := parseBoolWords[strings.ToLower(strings.TrimSpace(text))]
			if !ok {
				return nil, errors.New("provided string was not one of true/false/yes/no/on/off/1/0")
			}
			return value, nil
		},
	}
}

// intRangeText 把整数上下界写成 clap 风格的开区间记法。
func intRangeText(minimum, maximum *int64) string {
	switch {
	case minimum == nil && maximum == nil:
		return ""
	case minimum == nil:
		return "..=" + strconv.FormatInt(*maximum, 10)
	case maximum == nil:
		return strconv.FormatInt(*minimum, 10) + ".."
	}
	return strconv.FormatInt(*minimum, 10) + "..=" + strconv.FormatInt(*maximum, 10)
}

// checkIntRange 整数越界即返回 error, 文案与 clap 的 ranged parser 同形。
func checkIntRange(value int64, minimum, maximum *int64) error {
	text := intRangeText(minimum, maximum)
	if minimum != nil && value < *minimum {
		return fmt.Errorf("%d is not in %s", value, text)
	}
	if maximum != nil && value > *maximum {
		return fmt.Errorf("%d is not in %s", value, text)
	}
	return nil
}

// Choice 一个可选值及其说明。
type Choice struct {
	Value any
	Help  string
}

// Argument 单个命令行参数的定义, 由 `Parser.AddArgument` 登记。
//
// 位置参数没有 Names; 选项至少有一个以 `-` 开头的名字。
type Argument struct {
	Names         []string
	Dest          string
	Action        Action
	Nargs         Nargs
	Type          *ValueParser
	Choices       []Choice
	Default       DefaultValue
	Const         DefaultValue
	Required      bool
	Metavar       string
	Help          string
	LongHelp      string
	Env           string
	Alias         []string
	VisibleAlias  []string
	ConflictsWith []string
	Requires      []string
	// OverridesWith 与之互相覆盖的参数(名字或 dest); 一对里命令行上后出现的留下。
	OverridesWith []string
	// RequiredUnlessPresent 列出的参数里只要有一个出现过, 本参数就不算必填。
	RequiredUnlessPresent []string
	ValueDelimiter        string
	Hide                  bool
	HideDefault           bool
	AllowHyphenValues     bool
	Global                bool
	Group                 string
}

// IsPositional 位置参数: 没有选项名, 按命令行顺序吃取值。
func (a *Argument) IsPositional() bool { return len(a.Names) == 0 }

// TakesValues 是否吃取值; `store_true` / `count` 这类开关为假。
func (a *Argument) TakesValues() bool { return a.Action.TakesValues() }

// Collects 取值是否恒以列表形态落进命名空间。
func (a *Argument) Collects() bool {
	return a.ValueDelimiter != "" || a.Nargs.Collects()
}

// MinCount 最少吃几个取值。
func (a *Argument) MinCount() int { return a.Nargs.Min() }

// MaxCount 最多吃几个取值; 无上限返回 -1。
func (a *Argument) MaxCount() int { return a.Nargs.Max() }

// ValueName 取值的显示名: Metavar 优先, 其次大写化的 Dest。
func (a *Argument) ValueName() string {
	if a.Metavar != "" {
		return a.Metavar
	}
	return strings.ToUpper(a.Dest)
}

// ValueSpec 取值占位, 如 `<FILE>`。
func (a *Argument) ValueSpec() string { return "<" + a.ValueName() + ">" }

// PrimaryName 指代该参数时优先用长选项; 只有短选项时退回它。
func (a *Argument) PrimaryName() string {
	for _, name := range a.Names {
		if strings.HasPrefix(name, "--") {
			return name
		}
	}
	if len(a.Names) > 0 {
		return a.Names[0]
	}
	return a.Dest
}

// ErrorName 错误文案里的指代, 如 `--output <FILE>` / `<FILE>` / `-v`。
func (a *Argument) ErrorName() string {
	if a.IsPositional() {
		return a.ValueSpec()
	}
	name := a.PrimaryName()
	if a.TakesValues() {
		return name + " " + a.ValueSpec()
	}
	return name
}

// HelpNames 帮助左列: `-o, --output <FILE>`; 位置参数按是否必填写成 `<FILE>` / `[FILE]...`。
func (a *Argument) HelpNames() string {
	if a.IsPositional() {
		return a.UsageSpec()
	}
	names := append(append([]string{}, a.Names...), a.VisibleAlias...)
	head := strings.Join(names, ", ")
	if a.Action == Count {
		head += "..."
	}
	if a.TakesValues() {
		return head + " " + a.ValueSpec()
	}
	return head
}

// UsageSpec usage 行里的位置参数占位: `<FILE>` / `[FILE]` / `<FILE>...`。
func (a *Argument) UsageSpec() string {
	name := a.ValueName()
	switch a.Nargs.kind {
	case nargsOptional:
		return "[" + name + "]"
	case nargsZeroOrMore, nargsRemainder:
		return "[" + name + "]..."
	case nargsOneOrMore, nargsFixed:
		return "<" + name + ">..."
	}
	return "<" + name + ">"
}

// ChoiceValues 可选值集合。
func (a *Argument) ChoiceValues() []any {
	if a.Choices == nil {
		return nil
	}
	values := make([]any, len(a.Choices))
	for index, choice := range a.Choices {
		values[index] = choice.Value
	}
	return values
}

// ChoiceHelp 可选值的说明文字。
func (a *Argument) ChoiceHelp(value any) string {
	for _, choice := range a.Choices {
		if fmt.Sprint(choice.Value) == fmt.Sprint(value) {
			return choice.Help
		}
	}
	return ""
}

// Namespace 解析结果的容器, 对应 `argparse.Namespace`; 保持首次写入的顺序。
type Namespace struct {
	keys   []string
	values map[string]any
}

// NewNamespace 空命名空间。
func NewNamespace() *Namespace {
	return &Namespace{values: make(map[string]any)}
}

// Get 按名字读取; 未写入时 ok 为 false。
func (n *Namespace) Get(key string) (any, bool) {
	value, ok := n.values[key]
	return value, ok
}

// Contains 该名字是否已写入。
func (n *Namespace) Contains(key string) bool {
	_, ok := n.values[key]
	return ok
}

// Set 写入一个取值; 同名覆盖且位置不变。
func (n *Namespace) Set(key string, value any) {
	if _, ok := n.values[key]; !ok {
		n.keys = append(n.keys, key)
	}
	n.values[key] = value
}

// Remove 抹掉一个已写入的取值; 没写过时什么也不做。
func (n *Namespace) Remove(key string) {
	if _, ok := n.values[key]; !ok {
		return
	}
	delete(n.values, key)
	for index, name := range n.keys {
		if name == key {
			n.keys = append(n.keys[:index], n.keys[index+1:]...)
			break
		}
	}
}

// Keys 按写入顺序给出名字。
func (n *Namespace) Keys() []string { return append([]string{}, n.keys...) }

// Len 已写入的条目数。
func (n *Namespace) Len() int { return len(n.keys) }

// AsMap 浅拷贝成普通字典。
func (n *Namespace) AsMap() map[string]any {
	copied := make(map[string]any, len(n.keys))
	for key, value := range n.values {
		copied[key] = value
	}
	return copied
}

// String 形如 `Namespace(k=v, ...)`。
func (n *Namespace) String() string {
	parts := make([]string, 0, len(n.keys))
	for _, key := range n.keys {
		parts = append(parts, fmt.Sprintf("%s=%s", key, FormatValue(n.values[key])))
	}
	return "Namespace(" + strings.Join(parts, ", ") + ")"
}

// FormatValue 取值的文本形态: 浮点补 `.0`、`inf` / `nan` 小写、布尔 `true` / `false`、列表 `a, b`。
func FormatValue(value any) string {
	switch item := value.(type) {
	case nil:
		return "None"
	case DefaultValue:
		return item.String()
	case bool:
		if item {
			return "true"
		}
		return "false"
	case int:
		return strconv.Itoa(item)
	case int64:
		return strconv.FormatInt(item, 10)
	case float64:
		return floatText(item)
	case string:
		return item
	case []any:
		parts := make([]string, len(item))
		for index, entry := range item {
			parts[index] = FormatValue(entry)
		}
		return strings.Join(parts, ", ")
	case []string:
		return strings.Join(item, ", ")
	case *Namespace:
		return item.String()
	}
	return fmt.Sprint(value)
}

// floatText 把浮点写成 Python `str(float)` 的形态。
func floatText(value float64) string {
	switch {
	case math.IsNaN(value):
		return "nan"
	case math.IsInf(value, 1):
		return "inf"
	case math.IsInf(value, -1):
		return "-inf"
	}
	text := strconv.FormatFloat(value, 'g', -1, 64)
	if !strings.ContainsAny(text, ".eE") {
		text += ".0"
	}
	return text
}
