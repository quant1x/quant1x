// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

// 解析失败与控制流错误, 以及"你是不是想输入"的相似项建议。
//
// 三类错误的分工:
//
//   - UsageError 命令行输入不合法 (未知参数、缺值、取值非法、校验未通过), 进程退出码 2;
//   - HelpRequested / VersionRequested 请求打印帮助 / 版本, 不算错误, 退出码 0;
//   - DefinitionError 定义期错误 (命令链 dest 重复、引用未登记的参数), 退出码 1。
//
// 错误只承载结构化字段 (Message / Reason / Lines / Tips / Usage), 具体版式由 Error() 拼出,
// 因此 Error() 的文本就是非着色版式, 也是用例的断言对象。

package argparser

import (
	"strings"
)

// ErrorKind 用法错误的类别, 对应 clap 的 `ErrorKind`。
//
// 调用方按类别分支, 不必匹配文案; 文案改了不影响判据。
type ErrorKind int

const (
	// Other 未分类。
	Other ErrorKind = iota
	// UnknownArgument 未知参数。
	UnknownArgument
	// AmbiguousArgument 长选项前缀命中多个。
	AmbiguousArgument
	// MissingValue 缺取值。
	MissingValue
	// InvalidValue 取值非法。
	InvalidValue
	// MissingRequired 缺必填项。
	MissingRequired
	// Conflict 互斥 / 冲突。
	Conflict
)

// String 类别名, 与 Python 的 `ErrorKind.value` 同形。
func (k ErrorKind) String() string {
	switch k {
	case UnknownArgument:
		return "unknown_argument"
	case AmbiguousArgument:
		return "ambiguous_argument"
	case MissingValue:
		return "missing_value"
	case InvalidValue:
		return "invalid_value"
	case MissingRequired:
		return "missing_required"
	case Conflict:
		return "conflict"
	}
	return "other"
}

// UsageError 命令行输入不合法。
//
// Kind 是类别判据; Reason 接在首行冒号之后 (如类型转换失败的原因); Lines 逐条缩进列在首行
// 之下 (如可选值清单、缺失的必填项); Tips 是 `tip:` 开头的建议行。
type UsageError struct {
	Message string
	Kind    ErrorKind
	Reason  string
	Lines   []string
	Tips    []string
	Usage   string
	Hint    string
}

// Error 非着色版式: 首行 `error:` + 缩进的细节 / 建议 + usage + 帮助提示。
func (e *UsageError) Error() string {
	head := "error: " + e.Message
	if e.Reason != "" {
		head += ": " + e.Reason
	}
	lines := []string{head}
	for _, line := range e.Lines {
		lines = append(lines, "  "+line)
	}
	if len(e.Tips) > 0 {
		lines = append(lines, "")
		for _, tip := range e.Tips {
			lines = append(lines, "  tip: "+tip)
		}
	}
	if e.Usage != "" {
		lines = append(lines, "")
		lines = append(lines, "Usage: "+e.Usage)
	}
	if e.Hint != "" {
		lines = append(lines, "")
		lines = append(lines, e.Hint)
	}
	return strings.Join(lines, "\n")
}

// ExitCode 用法错误的退出码。
func (e *UsageError) ExitCode() int { return 2 }

// HelpRequested `-h` / `--help`: 请求帮助; Long 为真表示走长帮助 (`--help`)。
type HelpRequested struct {
	Long   bool
	Parser *Parser
}

// Error 请求的形态名。
func (e *HelpRequested) Error() string {
	if e.Long {
		return "--help"
	}
	return "-h"
}

// ExitCode 帮助请求的退出码。
func (e *HelpRequested) ExitCode() int { return 0 }

// VersionRequested `-V` / `--version`: 请求打印版本。
type VersionRequested struct {
	Parser *Parser
}

// Error 请求的形态名。
func (e *VersionRequested) Error() string { return "--version" }

// ExitCode 版本请求的退出码。
func (e *VersionRequested) ExitCode() int { return 0 }

// DefinitionError 定义期错误, 对应 Python 侧直接抛出的 ValueError, 不属于用法错误。
type DefinitionError struct {
	Message string
}

// Error 出错原因。
func (e *DefinitionError) Error() string { return e.Message }

// ExitCode 定义期错误的退出码。
func (e *DefinitionError) ExitCode() int { return 1 }

// ExitCode 取错误对应的进程退出码; 未分类错误按用法错误算。
func ExitCode(err error) int {
	switch item := err.(type) {
	case *UsageError:
		return item.ExitCode()
	case *HelpRequested:
		return item.ExitCode()
	case *VersionRequested:
		return item.ExitCode()
	case *DefinitionError:
		return item.ExitCode()
	}
	return 2
}

// Jaro Jaro 相似度, 匹配窗口为 `max(长度) / 2 - 1`。
func Jaro(left, right string) float64 {
	leftRunes := []rune(left)
	rightRunes := []rune(right)
	if left == right {
		return 1.0
	}
	leftLen, rightLen := len(leftRunes), len(rightRunes)
	if leftLen == 0 || rightLen == 0 {
		return 0.0
	}
	window := max(leftLen, rightLen)/2 - 1
	if window < 0 {
		window = 0
	}
	leftHit := make([]bool, leftLen)
	rightHit := make([]bool, rightLen)
	matches := 0
	for index, char := range leftRunes {
		start := max(0, index-window)
		stop := min(index+window+1, rightLen)
		for position := start; position < stop; position++ {
			if !rightHit[position] && rightRunes[position] == char {
				leftHit[index] = true
				rightHit[position] = true
				matches++
				break
			}
		}
	}
	if matches == 0 {
		return 0.0
	}
	transpositions := 0
	cursor := 0
	for index := 0; index < leftLen; index++ {
		if !leftHit[index] {
			continue
		}
		for !rightHit[cursor] {
			cursor++
		}
		if leftRunes[index] != rightRunes[cursor] {
			transpositions++
		}
		cursor++
	}
	transpositions /= 2
	return (float64(matches)/float64(leftLen) +
		float64(matches)/float64(rightLen) +
		float64(matches-transpositions)/float64(matches)) / 3
}

// JaroWinkler Jaro-Winkler 相似度: 对相同前缀 (最多 4 字符) 加权。
func JaroWinkler(left, right string) float64 {
	score := Jaro(left, right)
	prefix := 0
	leftRunes, rightRunes := []rune(left), []rune(right)
	for index := 0; index < len(leftRunes) && index < len(rightRunes); index++ {
		if leftRunes[index] != rightRunes[index] || prefix == 4 {
			break
		}
		prefix++
	}
	return score + float64(prefix)*0.1*(1.0-score)
}

// 相似度阈值, 与 clap 的 `did_you_mean` 一致 (Jaro-Winkler > 0.8)。
const matchThreshold = 0.8

// BestMatch 在候选里挑最像 `word` 的一个; 相似度不到阈值时 ok 为 false (宁缺勿滥)。
func BestMatch(word string, candidates []string) (string, bool) {
	return BestMatchAt(word, candidates, matchThreshold)
}

// BestMatchAt 同 BestMatch, 阈值由调用方给。
func BestMatchAt(word string, candidates []string, threshold float64) (string, bool) {
	best := ""
	bestScore := threshold
	for _, candidate := range candidates {
		if candidate == "" || candidate == word {
			continue
		}
		score := JaroWinkler(word, candidate)
		if score > bestScore {
			best, bestScore = candidate, score
		}
	}
	return best, best != ""
}

// FormatItems 把取值拼成 `a, b, c` 形态。
func FormatItems(values []any) string {
	parts := make([]string, len(values))
	for index, value := range values {
		parts[index] = FormatValue(value)
	}
	return strings.Join(parts, ", ")
}

// FormatNames 把名字拼成 `a, b, c` 形态。
func FormatNames(names []string) string {
	return strings.Join(names, ", ")
}
