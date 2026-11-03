package config

import (
	"errors"
	"slices"
	"strings"

	"gopkg.in/yaml.v3"
)

// ErrTimeFormat 时间格式错误
var ErrTimeFormat = errors.New("时间格式错误")

// TimeRange 时间范围
type TimeRange struct {
	Begin string `yaml:"begin" json:"begin"`
	End   string `yaml:"end" json:"end"`
}

func (this TimeRange) String() string {
	return this.Begin + "~" + this.End
}

// Parse 解析时间范围, 支持 "HH:MM:SS~HH:MM:SS" 形式
func (this *TimeRange) Parse(text string) error {
	text = strings.TrimSpace(text)
	arr := strings.SplitN(text, "~", 2)
	if len(arr) != 2 {
		return ErrTimeFormat
	}
	begin := strings.TrimSpace(arr[0])
	end := strings.TrimSpace(arr[1])
	if len(begin) == 0 || len(end) == 0 {
		return ErrTimeFormat
	}
	if begin > end {
		begin, end = end, begin
	}
	this.Begin = begin
	this.End = end
	return nil
}

// TradingSession 交易时段
type TradingSession struct {
	Sessions []TimeRange `yaml:"sessions" json:"sessions"`
}

// Parse 解析交易时段, 支持 "HH:MM:SS~HH:MM:SS,HH:MM:SS~HH:MM:SS" 形式
func (this *TradingSession) Parse(text string) error {
	text = strings.TrimSpace(text)
	if len(text) == 0 {
		return ErrTimeFormat
	}
	var sessions []TimeRange
	for _, part := range strings.Split(text, ",") {
		part = strings.TrimSpace(part)
		if len(part) == 0 {
			continue
		}
		var tr TimeRange
		if err := tr.Parse(part); err != nil {
			return err
		}
		sessions = append(sessions, tr)
	}
	if len(sessions) == 0 {
		return ErrTimeFormat
	}
	sortTimeRange(sessions)
	this.Sessions = sessions
	return nil
}

// UnmarshalYAML 支持标量与序列两种配置形式
func (this *TradingSession) UnmarshalYAML(node *yaml.Node) error {
	if node == nil {
		return ErrTimeFormat
	}
	switch node.Kind {
	case yaml.ScalarNode:
		return this.Parse(node.Value)
	case yaml.SequenceNode:
		var sessions []TimeRange
		if err := node.Decode(&sessions); err != nil {
			return err
		}
		if len(sessions) == 0 {
			return ErrTimeFormat
		}
		sortTimeRange(sessions)
		this.Sessions = sessions
		return nil
	default:
		return ErrTimeFormat
	}
}

// UnmarshalText 按 default 标签填充默认值
func (this *TradingSession) UnmarshalText(text []byte) error {
	return this.Parse(string(text))
}

func sortTimeRange(sessions []TimeRange) {
	slices.SortFunc(sessions, func(a, b TimeRange) int {
		if c := strings.Compare(a.Begin, b.Begin); c != 0 {
			return c
		}
		return strings.Compare(a.End, b.End)
	})
}
