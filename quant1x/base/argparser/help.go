// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package argparser

// 版式: usage 行、分节帮助、错误块与 ANSI 着色。
//
// 版式固定, 不随内容猜测: 章节顺序是 `Usage` → `Commands` → `Arguments` → `Options` → 自定义分组
// → `epilog`; 左列超过半幅时说明文字换到下一行并对齐缩进。颜色只加在标题与选项字面量上,
// `[default: ...]` / `[env: ...]` / `[possible values: ...]` 一律保留原样, 便于用例按文本断言。
//
// 宽度按码点计 (与 Python 的 `len()` 同口径), 因此中文说明与英文说明的折行位置一致。

import (
	"strings"
	"unicode/utf8"
)

const (
	styleReset   = "\x1b[0m"
	styleHeading = "\x1b[1m\x1b[4m"
	styleBold    = "\x1b[1m"
	styleLiteral = "\x1b[32m"
	styleError   = "\x1b[31m"
	styleWarning = "\x1b[33m"
)

// Style ANSI 着色开关; 关闭时所有方法原样返回文本, 因此版式与宽度计算只看可见字符。
type Style struct {
	enabled bool
}

// NewStyle 按开关构造。
func NewStyle(enabled bool) Style { return Style{enabled: enabled} }

// Enabled 是否着色。
func (s Style) Enabled() bool { return s.enabled }

// wrap 套上 ANSI 颜色码; 关闭着色或空串时原样返回。
func (s Style) wrap(code, text string) string {
	if !s.enabled || text == "" {
		return text
	}
	return code + text + styleReset
}

// Heading 章节标题与 `Usage:` / `error:` 前缀。
func (s Style) Heading(text string) string { return s.wrap(styleHeading, text) }

// Bold 强调。
func (s Style) Bold(text string) string { return s.wrap(styleBold, text) }

// Literal 选项与命令字面量。
func (s Style) Literal(text string) string { return s.wrap(styleLiteral, text) }

// Error 前缀 `error:`。
func (s Style) Error(text string) string { return s.wrap(styleError, text) }

// Warning 前缀 `tip:`。
func (s Style) Warning(text string) string { return s.wrap(styleWarning, text) }

// helpRow 表格行: 左列 (选项 / 命令 / 位置参数) 与右列说明。
type helpRow struct {
	left string
	text string
}

// widthOf 可见宽度: 码点个数, 与 Python 的 `len()` 同口径。
func widthOf(text string) int { return utf8.RuneCountInString(text) }

// spaces 生成 count 个空格; 非正数给空串。
func spaces(count int) string {
	if count <= 0 {
		return ""
	}
	return strings.Repeat(" ", count)
}

// wrapOne 单段文字按宽度折行; width 是**含缩进在内**的整行宽度。
func wrapOne(paragraph string, width int, initialIndent, subsequentIndent string) []string {
	if width < 20 {
		width = 20
	}
	lines := []string{}
	line := initialIndent
	filled := false
	for _, word := range strings.Fields(paragraph) {
		if filled && widthOf(line)+1+widthOf(word) > width {
			lines = append(lines, line)
			line = subsequentIndent
			filled = false
		}
		if filled {
			line += " "
		}
		line += word
		filled = true
	}
	if filled {
		lines = append(lines, line)
	} else {
		lines = append(lines, "")
	}
	return lines
}

// wrapParagraphs 按段落换行; 段间保留一个空行位。
func wrapParagraphs(text string, width int, indent string, firstIndent *string) []string {
	lines := []string{}
	for index, paragraph := range strings.Split(text, "\n") {
		head := indent
		if index == 0 && firstIndent != nil {
			head = *firstIndent
		}
		lines = append(lines, wrapOne(paragraph, width, head, indent)...)
	}
	return lines
}

// argumentText 一行说明文字: 帮助正文 + 默认值 / 环境变量 / 可选值。
func argumentText(argument *Argument, longHelp bool) string {
	text := ""
	if longHelp && argument.LongHelp != "" {
		text = argument.LongHelp
	} else {
		text = argument.Help
	}
	extras := []string{}
	showsDefault := argument.Action == Store || argument.Action == Append || argument.Action == Extend
	if showsDefault && !argument.HideDefault {
		if value, ok := argument.Default.Get(); ok && value != nil {
			extras = append(extras, "[default: "+FormatValue(value)+"]")
		}
	}
	if argument.Env != "" {
		extras = append(extras, "[env: "+argument.Env+"]")
	}
	choices := argument.ChoiceValues()
	if len(choices) > 0 {
		extras = append(extras, "[possible values: "+FormatItems(choices)+"]")
	}
	if len(extras) > 0 {
		text = strings.TrimSpace(text + " " + strings.Join(extras, " "))
	}
	if longHelp {
		notes := []string{}
		for _, value := range choices {
			if note := argument.ChoiceHelp(value); note != "" {
				notes = append(notes, "- "+FormatValue(value)+": "+note)
			}
		}
		if len(notes) > 0 {
			body := strings.Join(notes, "\n")
			if text == "" {
				text = body
			} else {
				text += "\n" + body
			}
		}
	}
	return text
}

// globalArguments 祖先命令上标记为 `global` 的参数: 子命令里同样可用, 也出现在子命令帮助里。
func globalArguments(parser *Parser) []*Argument {
	collected := []*Argument{}
	for parent := parser.Parent(); parent != nil; parent = parent.Parent() {
		for _, argument := range parent.Arguments() {
			if argument.Global {
				collected = append(collected, argument)
			}
		}
	}
	for left, right := 0, len(collected)-1; left < right; left, right = left+1, right-1 {
		collected[left], collected[right] = collected[right], collected[left]
	}
	return collected
}

// visibleOptions 本命令的选项 + 祖先命令下沉的 `global` 参数; 帮助 / 版本排最后 (与 clap 一致)。
func visibleOptions(parser *Parser) []*Argument {
	options := []*Argument{}
	for _, argument := range parser.Arguments() {
		if !argument.IsPositional() && !argument.Hide {
			options = append(options, argument)
		}
	}
	for _, argument := range globalArguments(parser) {
		if !argument.Hide {
			options = append(options, argument)
		}
	}
	ordered := []*Argument{}
	for _, argument := range options {
		if argument.Action != Help && argument.Action != Version {
			ordered = append(ordered, argument)
		}
	}
	for _, argument := range options {
		if argument.Action == Help || argument.Action == Version {
			ordered = append(ordered, argument)
		}
	}
	return ordered
}

// renderTable `标题 + 两列表格`: 左列对齐, 右列折行并与右列首字符对齐。
func renderTable(title string, rows []helpRow, width int, style Style, description string) string {
	if len(rows) == 0 {
		return ""
	}
	widest := 0
	for _, row := range rows {
		if size := widthOf(row.left); size > widest {
			widest = size
		}
	}
	leftWidth := widest
	if half := width / 3; half < 16 {
		if leftWidth > 16 {
			leftWidth = 16
		}
	} else if leftWidth > half {
		leftWidth = half
	}
	const gutter = 2
	base := 2 + leftWidth + gutter
	lines := []string{title}
	if description != "" {
		indent := "  "
		lines = append(lines, wrapParagraphs(description, width, indent, &indent)...)
		lines = append(lines, "")
	}
	for _, row := range rows {
		head := style.Literal("  " + row.left)
		if row.text == "" {
			lines = append(lines, head)
			continue
		}
		if widthOf(row.left) > leftWidth {
			lines = append(lines, head)
			lines = append(lines, wrapParagraphs(row.text, width, "    ", nil)...)
			continue
		}
		indent := spaces(base)
		chunks := wrapParagraphs(row.text, width, indent, &indent)
		pad := spaces(base - 2 - widthOf(row.left))
		lines = append(lines, head+pad+strings.TrimLeft(chunks[0], " "))
		lines = append(lines, chunks[1:]...)
	}
	return strings.Join(lines, "\n")
}

// commandRows 子命令章节的行: 名字 + 展示别名; 自动生成的 `help` 排在最后。
func commandRows(parser *Parser) []helpRow {
	subparsers := parser.Subparsers()
	if subparsers == nil {
		return []helpRow{}
	}
	rows := []helpRow{}
	for _, command := range subparsers.Commands() {
		head := command.Name
		if len(command.Aliases) > 0 {
			head += ", " + FormatNames(command.Aliases)
		}
		about := ""
		if command.Parser != nil {
			about = command.Parser.About()
		}
		rows = append(rows, helpRow{left: head, text: about})
	}
	if !subparsers.HasCommand("help") {
		rows = append(rows, helpRow{
			left: "help",
			text: "Print this message or the help of the given subcommand(s)",
		})
	}
	return rows
}

// RenderUsage 单行用法文本 (`Usage:` 前缀在内)。
func RenderUsage(parser *Parser) string {
	style := NewStyle(parser.ColorEnabled())
	return style.Heading("Usage:") + " " + parser.UsageSpec()
}

// RenderHelp 整段帮助文本, 段间空一行; longHelp 为真时用长简介与长说明。
func RenderHelp(parser *Parser, longHelp bool) string {
	style := NewStyle(parser.ColorEnabled())
	width := parser.Width()
	blocks := []string{}
	about := parser.About()
	if longHelp && parser.LongAbout() != "" {
		about = parser.LongAbout()
	}
	if about != "" {
		blocks = append(blocks, strings.Join(wrapParagraphs(about, width, "", nil), "\n"))
	}
	blocks = append(blocks, RenderUsage(parser))
	if subparsers := parser.Subparsers(); subparsers != nil {
		title := style.Heading(subparsers.Title + ":")
		if section := renderTable(title, commandRows(parser), width, style, subparsers.Help); section != "" {
			blocks = append(blocks, section)
		}
	}
	positionals := []helpRow{}
	for _, argument := range parser.Positionals() {
		if argument.Hide {
			continue
		}
		positionals = append(positionals, helpRow{
			left: argument.HelpNames(),
			text: argumentText(argument, longHelp),
		})
	}
	if len(positionals) > 0 {
		blocks = append(blocks, renderTable(style.Heading("Arguments:"), positionals, width, style, ""))
	}
	options := visibleOptions(parser)
	groups := parser.Groups()
	defaultOptions := []helpRow{}
	for _, argument := range options {
		if argument.Group != "" && hasGroup(groups, argument.Group) {
			continue
		}
		defaultOptions = append(defaultOptions, helpRow{
			left: argument.HelpNames(),
			text: argumentText(argument, longHelp),
		})
	}
	if len(defaultOptions) > 0 {
		blocks = append(blocks, renderTable(style.Heading("Options:"), defaultOptions, width, style, ""))
	}
	for _, group := range groups {
		grouped := []helpRow{}
		for _, argument := range options {
			if argument.Group != group.Title {
				continue
			}
			grouped = append(grouped, helpRow{
				left: argument.HelpNames(),
				text: argumentText(argument, longHelp),
			})
		}
		if len(grouped) == 0 {
			continue
		}
		blocks = append(blocks, renderTable(style.Heading(group.Title+":"), grouped, width, style,
			group.Description))
	}
	if epilog := parser.Epilog(); epilog != "" {
		blocks = append(blocks, strings.Join(wrapParagraphs(epilog, width, "", nil), "\n"))
	}
	return strings.Join(blocks, "\n\n")
}

// hasGroup 该标题是否登记成了本命令的分组。
func hasGroup(groups []GroupInfo, title string) bool {
	for _, group := range groups {
		if group.Title == title {
			return true
		}
	}
	return false
}

// RenderError 错误块: 首行 `error:` + 缩进的细节 / 建议 + usage + 帮助提示; color 为假时走非着色版式。
func RenderError(e *UsageError, color bool) string {
	if !color {
		return e.Error()
	}
	style := NewStyle(true)
	body := style.Error("error:") + " " + e.Message
	if e.Reason != "" {
		body += ": " + e.Reason
	}
	for _, line := range e.Lines {
		body += "\n  " + line
	}
	if len(e.Tips) > 0 {
		body += "\n"
		for _, tip := range e.Tips {
			body += "\n  " + style.Warning("tip:") + " " + tip
		}
	}
	if e.Usage != "" {
		body += "\n\n" + style.Heading("Usage:") + " " + e.Usage
	}
	if e.Hint != "" {
		body += "\n\n" + e.Hint
	}
	return body
}
