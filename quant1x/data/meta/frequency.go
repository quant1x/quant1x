// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

package meta

import (
	"errors"
	"fmt"
	"regexp"
	"strconv"
	"strings"
)

// TimeUnit 标准化的时间单位枚举，覆盖 pandas 常见别名。
// 所有单位均为固定长度（不包括月、年等日历单位）。
type TimeUnit string

const (
	NANOSECOND  TimeUnit = "ns"
	MICROSECOND TimeUnit = "us"
	MILLISECOND TimeUnit = "ms"
	SECOND      TimeUnit = "s"
	MINUTE      TimeUnit = "min"
	HOUR        TimeUnit = "h"
	DAY         TimeUnit = "d"
	WEEK        TimeUnit = "w"
	MONTH       TimeUnit = "m"
	YEAR        TimeUnit = "y"
)

// SecondsPerUnit 每单位对应的秒数（float64 支持纳秒）
var SecondsPerUnit = map[TimeUnit]float64{
	NANOSECOND:  1e-9,
	MICROSECOND: 1e-6,
	MILLISECOND: 1e-3,
	SECOND:      1.0,
	MINUTE:      60.0,
	HOUR:        3600.0,
	DAY:         86400.0,
	WEEK:        604800.0,
	MONTH:       2592000.0,
	YEAR:        31536000.0,
}

// Frequency 表示一个标准化的频率值: num x unit。
// 例如: 5 分钟 → Frequency{Num: 5, Unit: MINUTE}
type Frequency struct {
	Num  int
	Unit TimeUnit
}

// ToTotalSeconds 返回总秒数（可用于比较、排序、计算）
func (f Frequency) ToTotalSeconds() float64 {
	return float64(f.Num) * SecondsPerUnit[f.Unit]
}

// String 返回频率字符串表示
func (f Frequency) String() string {
	return fmt.Sprintf("%d%s", f.Num, f.Unit)
}

// CacheKey 返回缓存键
func (f Frequency) CacheKey() string {
	if f.Unit == DAY {
		return "day"
	}
	return fmt.Sprintf("%d%s", f.Num, f.Unit)
}

// 预定义的频率常量
var (
	FREQ_DAILY  = Frequency{Num: 1, Unit: DAY}   // 日线
	FREQ_WEEKLY = Frequency{Num: 1, Unit: WEEK}  // 周线
	FREQ_MONTHLY = Frequency{Num: 1, Unit: MONTH} // 月线
	FREQ_YEARLY  = Frequency{Num: 1, Unit: YEAR}  // 年线
)

// pandas 单位别名映射表
var pandasUnitAliases = map[string]TimeUnit{
	// nanosecond
	"N":  NANOSECOND,
	"ns": NANOSECOND,
	// microsecond
	"U":  MICROSECOND,
	"us": MICROSECOND,
	"µs": MICROSECOND,
	// millisecond
	"L":  MILLISECOND,
	"ms": MILLISECOND,
	// second
	"S": SECOND,
	"s": SECOND,
	// minute
	"T": MINUTE,
	"min": MINUTE,
	// hour
	"H": HOUR,
	"h": HOUR,
	// day
	"D": DAY,
	"d": DAY,
	// week
	"W": WEEK,
	"w": WEEK,
	// month
	"M": MONTH,
	"m": MONTH,
	// year
	"Y": YEAR,
	"y": YEAR,
}

// ParseFrequencyString 解析 pandas 风格的频率字符串（如 '5T', '1H', '30s'）为标准化 Frequency。
//
// 参数:
//   freq: 频率字符串，如 "5T", "1h", "90s"
//
// 返回:
//   Frequency{Num: 5, Unit: MINUTE}
//
// 错误:
//   无效格式或不支持的单位
func ParseFrequencyString(freq string) (Frequency, error) {
	s := strings.TrimSpace(freq)
	if s == "" {
		return Frequency{}, errors.New("frequency string is empty")
	}

	// 正则提取数字前缀和单位后缀
	re := regexp.MustCompile(`^(\d*)(.*)$`)
	match := re.FindStringSubmatch(s)
	if match == nil {
		return Frequency{}, fmt.Errorf("invalid frequency format: %s", s)
	}

	numStr := match[1]
	unitStr := match[2]

	num := 1
	if numStr != "" {
		var err error
		num, err = strconv.Atoi(numStr)
		if err != nil {
			return Frequency{}, fmt.Errorf("invalid number format: %s", numStr)
		}
	}

	if unitStr == "" {
		return Frequency{}, errors.New("missing unit in frequency string")
	}

	unit, ok := pandasUnitAliases[unitStr]
	if !ok {
		return Frequency{}, fmt.Errorf("unsupported or unknown frequency unit: %q", unitStr)
	}

	return Frequency{Num: num, Unit: unit}, nil
}

// ToTotalSeconds 便捷函数：将频率转为总秒数
func ToTotalSeconds(freq interface{}) (float64, error) {
	var f Frequency
	var err error

	switch v := freq.(type) {
	case string:
		f, err = ParseFrequencyString(v)
		if err != nil {
			return 0, err
		}
	case Frequency:
		f = v
	default:
		return 0, errors.New("invalid frequency type: must be string or Frequency")
	}

	return f.ToTotalSeconds(), nil
}

// IsFixedDuration 判断是否为固定时长（所有当前支持的单位都是固定的）。
// 未来若加入 'M'（月）、'Y'（年），此处需调整。
func IsFixedDuration(freq interface{}) bool {
	// 当前所有单位均为固定长度
	return true
}

// ToFrequency 便捷函数：将字符串转换为 Frequency
func ToFrequency(freq string) Frequency {
	f, err := ParseFrequencyString(freq)
	if err != nil {
		panic(err)
	}
	return f
}
