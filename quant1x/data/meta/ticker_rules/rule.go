// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// CodeRule — 证券代码规则, 与 Python data/meta/ticker_rules/rule.py 对齐

package ticker_rules

import (
	"strings"

	"github.com/quant1x/quant1x/quant1x/data/meta"
)

// PrefixType 规则前缀类型
type PrefixType uint8

const (
	PrefixStr   PrefixType = iota // 字符串前缀
	PrefixRange                   // 数字范围
)

// RulePrefix 规则前缀: 可以是字符串前缀或数字范围
type RulePrefix struct {
	Type       PrefixType
	Str        string // 当 Type == PrefixStr 时使用
	RangeStart string // 当 Type == PrefixRange 时使用
	RangeEnd   string // 当 Type == PrefixRange 时使用
}

// NewStrPrefix 创建字符串前缀
func NewStrPrefix(s string) RulePrefix {
	return RulePrefix{Type: PrefixStr, Str: s}
}

// NewRangePrefix 创建数字范围前缀
func NewRangePrefix(start, end string) RulePrefix {
	return RulePrefix{Type: PrefixRange, RangeStart: start, RangeEnd: end}
}

// Matches 检查代码是否匹配此前缀
func (p *RulePrefix) Matches(code string) bool {
	if p.Type == PrefixStr {
		if p.Str == "" {
			return true // 空前缀匹配一切(如美股默认规则)
		}
		return len(code) >= len(p.Str) && code[:len(p.Str)] == p.Str
	}
	// 对于数字范围, 按字符串比较(代码可能是前导零的数字字符串)
	return code >= p.RangeStart && code <= p.RangeEnd
}

// MatchLength 返回前缀长度(用于最佳匹配排序)
func (p *RulePrefix) MatchLength() int {
	if p.Type == PrefixStr {
		return len(p.Str)
	}
	// 对于范围, 返回起始值的长度作为匹配长度
	return len(p.RangeStart)
}

// MaxValueLength 返回范围的最大可能长度
func (p *RulePrefix) MaxValueLength() int {
	if p.Type == PrefixStr {
		return len(p.Str)
	}
	if len(p.RangeStart) > len(p.RangeEnd) {
		return len(p.RangeStart)
	}
	return len(p.RangeEnd)
}

// CodeRule 证券代码规则
type CodeRule struct {
	Exchange       meta.Exchange       // 交易所
	Prefix         RulePrefix          // 代码前缀
	InstrumentType meta.InstrumentType // 证券类型
	Name           string              // 证券类型名称
	Desc           string              // 规则描述
}

// MatchRule 根据代码前缀匹配最优规则
func MatchRule(code string, rules []CodeRule) CodeRule {
	code = strings.ToUpper(strings.TrimSpace(code))

	var bestMatch *CodeRule
	bestLen := 0

	for i := range rules {
		prefix := &rules[i].Prefix
		if prefix.Matches(code) {
			length := prefix.MatchLength()
			if length > bestLen {
				bestLen = length
				bestMatch = &rules[i]
			} else if bestLen == 0 && length == 0 {
				// 空前缀在无其他匹配时使用
				bestMatch = &rules[i]
				break
			}
		}
	}

	if bestMatch != nil {
		return *bestMatch
	}
	return CodeRule{
		Exchange:       meta.UNKNOWN,
		Prefix:         NewStrPrefix(""),
		InstrumentType: meta.InstrumentTypeUnknown,
		Name:           "",
		Desc:           "未匹配到规则",
	}
}

// GlobalRules 全局规则(跨市场优先)
func GlobalRules() []CodeRule {
	return []CodeRule{
		{
			Exchange:       meta.SSE,
			Prefix:         NewStrPrefix("880"),
			InstrumentType: meta.InstrumentTypeSector,
			Name:           "板块指数",
			Desc:           "通达信",
		},
		{
			Exchange:       meta.SSE,
			Prefix:         NewStrPrefix("881"),
			InstrumentType: meta.InstrumentTypeSector,
			Name:           "板块指数",
			Desc:           "通达信",
		},
	}
}
