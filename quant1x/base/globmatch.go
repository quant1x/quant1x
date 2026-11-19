// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package base

// spec: quant1x/base/globmatch.py

import "strings"

// Match 判断单条模式是否匹配整条路径 (锚定)。
//
// 整条路径从根起逐段比对: `**` 匹配零到多段, 其余段按 `*` / `?` / `[...]` 通配
// (`*` 不跨段、大小写敏感、点开头的名字照常匹配)。模式以 `/**` 结尾时追加一个
// 隐式 `*`(要求目录下至少一个条目), 等价 `/**/*`。
//
// 路径与模式都可写成项目根绝对形式 (前导 `/`), 与相对形式等价: 匹配一律以项目根为锚,
// `/third_party/a.h` 与 `third_party/a.h` 是同一个位置, 两侧可混写。
//
// path 或 pattern 为空时返回 false。
func Match(path, pattern string) bool {
	if path == "" || pattern == "" {
		return false
	}
	parts := splitSegments(path)
	pats := splitSegments(pattern)
	if len(pats) == 0 {
		return false
	}
	if pats[len(pats)-1] == "**" {
		// 尾段 `/**` 表示「目录下至少一个条目」, 等价 `/**/*`
		pats = append(pats, "*")
	}
	n, m := len(parts), len(pats)
	// dp[i][j] = parts[i:] 与 pats[j:] 是否匹配
	dp := make([][]bool, n+1)
	for i := range dp {
		dp[i] = make([]bool, m+1)
	}
	dp[n][m] = true
	for j := m - 1; j >= 0; j-- {
		if pats[j] == "**" {
			dp[n][j] = dp[n][j+1]
		}
	}
	for i := n - 1; i >= 0; i-- {
		for j := m - 1; j >= 0; j-- {
			if pats[j] == "**" {
				dp[i][j] = dp[i][j+1] || dp[i+1][j]
			} else {
				dp[i][j] = dp[i+1][j+1] && segmentMatches(pats[j], parts[i])
			}
		}
	}
	return dp[0][0]
}

// MatchAny 判断任一条模式是否匹配整条路径。
func MatchAny(path string, patterns []string) bool {
	for _, pattern := range patterns {
		if Match(path, pattern) {
			return true
		}
	}
	return false
}

// splitSegments 按 `/` 切段并丢弃空段
func splitSegments(s string) []string {
	raw := strings.Split(s, "/")
	parts := make([]string, 0, len(raw))
	for _, p := range raw {
		if p != "" {
			parts = append(parts, p)
		}
	}
	return parts
}

// segmentMatches 单段通配: `*` 任意序列, `?` 单个字符, `[...]` 单字符集合
func segmentMatches(pattern, text string) bool {
	p := []rune(pattern)
	s := []rune(text)
	return matchAt(p, 0, s, 0)
}

// matchAt 递归匹配 p[i:] 与 s[j:]; `*` 以回溯枚举剩余位置
func matchAt(p []rune, i int, s []rune, j int) bool {
	for i < len(p) {
		switch p[i] {
		case '*':
			for i < len(p) && p[i] == '*' {
				i++
			}
			if i == len(p) {
				return true
			}
			for k := j; k <= len(s); k++ {
				if matchAt(p, i, s, k) {
					return true
				}
			}
			return false
		case '?':
			if j >= len(s) {
				return false
			}
			i++
			j++
		case '[':
			class, next, ok := parseClass(p, i)
			if !ok {
				if j >= len(s) || s[j] != '[' {
					return false
				}
				i++
				j++
				continue
			}
			if j >= len(s) || !class.matches(s[j]) {
				return false
			}
			i = next
			j++
		default:
			if j >= len(s) || s[j] != p[i] {
				return false
			}
			i++
			j++
		}
	}
	return j == len(s)
}

// charClass 一个 `[...]` 集合: 取反标志与闭区间表
type charClass struct {
	negated bool
	ranges  [][2]rune
}

// matches 单个字符是否命中集合
func (c charClass) matches(r rune) bool {
	hit := false
	for _, rng := range c.ranges {
		if rng[0] <= r && r <= rng[1] {
			hit = true
			break
		}
	}
	return hit != c.negated
}

// parseClass 解析 `[...]`; 未闭合时 ok 为 false (调用方按字面量 `[` 处理)
func parseClass(p []rune, start int) (charClass, int, bool) {
	i := start + 1
	if i >= len(p) {
		return charClass{}, start, false
	}
	class := charClass{}
	if p[i] == '!' {
		class.negated = true
		i++
	}
	if i < len(p) && p[i] == ']' {
		// 紧邻的 `]` 作为首个字面量
		class.ranges = append(class.ranges, [2]rune{']', ']'})
		i++
	}
	for i < len(p) && p[i] != ']' {
		low := p[i]
		if i+2 < len(p) && p[i+1] == '-' && p[i+2] != ']' {
			class.ranges = append(class.ranges, [2]rune{low, p[i+2]})
			i += 3
		} else {
			class.ranges = append(class.ranges, [2]rune{low, low})
			i++
		}
	}
	if i >= len(p) {
		return charClass{}, start, false
	}
	return class, i + 1, true
}
