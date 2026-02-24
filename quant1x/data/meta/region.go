// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

package meta

import (
	"fmt"
)

// Region 市场区域, 用于收敛货币和时区
type Region string

const (
	RegionCN       Region = "CN"       // 中国
	RegionHK       Region = "HK"       // 香港
	RegionUS       Region = "US"       // 美国
	RegionUK       Region = "UK"       // 英国
	RegionSG       Region = "SG"       // 新加坡
	RegionJP       Region = "JP"       // 日本
	RegionOFFSHORE Region = "OS"       // 离岸市场
	RegionONSHORE  Region = "ON"       // 内地市场
	RegionGLB      Region = "GLB"      // 全球市场
	RegionUNKNOWN  Region = "UNKNOWN"  // 未知区域
)

// Currency 返回主要货币
func (r Region) Currency() string {
	currencies := map[Region]string{
		RegionCN:       "CNY",
		RegionHK:       "HKD",
		RegionUS:       "USD",
		RegionUK:       "GBP",
		RegionSG:       "SGD",
		RegionJP:       "JPY",
		RegionOFFSHORE: "USD",
		RegionONSHORE:  "CNY",
	}
	if currency, ok := currencies[r]; ok {
		return currency
	}
	return "USD"
}

// Timezone 返回主要时区
func (r Region) Timezone() string {
	timezones := map[Region]string{
		RegionCN:       "Asia/Shanghai",
		RegionHK:       "Asia/Hong_Kong",
		RegionUS:       "America/New_York",
		RegionUK:       "Europe/London",
		RegionSG:       "Asia/Singapore",
		RegionJP:       "Asia/Tokyo",
		RegionOFFSHORE: "America/New_York",
		RegionONSHORE:  "Asia/Shanghai",
	}
	if timezone, ok := timezones[r]; ok {
		return timezone
	}
	return "UTC"
}

// String 实现 Stringer 接口
func (r Region) String() string {
	return string(r)
}

// ParseRegion 解析字符串为 Region
func ParseRegion(s string) (Region, error) {
	region := Region(s)
	switch region {
	case RegionCN, RegionHK, RegionUS, RegionUK, RegionSG, RegionJP,
		RegionOFFSHORE, RegionONSHORE, RegionGLB, RegionUNKNOWN:
		return region, nil
	default:
		return "", fmt.Errorf("unknown region: %s", s)
	}
}
