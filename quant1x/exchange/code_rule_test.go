package exchange

import (
	"fmt"
	"strings"
	"testing"
)

func TestCodeRuleBasic(t *testing.T) {
	testCases := []string{
		// 上交所
		"600000", "sh600000", "688001", "510300", "588000",
		"501005", "900901", "113050", "730001", "000001",
		// 深交所
		"000001.SZ", "300750", "159915", "200725", "123456", "150012",
		// 北交所（含最新 920xxx）
		"830799", "871234", "889088", "920003", // 万达轴承
		// 指数/板块
		"399001", "880888",
		// 异常
		"12345", "abc123", "92000", // 非6位
	}

	fmt.Printf("%-14s | %-6s | %-10s | %s\n", "输入", "市场", "类型", "描述")
	fmt.Println(strings.Repeat("-", 70))
	for _, c := range testCases {
		mkt, typ, desc := DetectSecurity(c)
		fmt.Printf("%-14s | %-6s | %-10s | %s\n", c, string(mkt), string(typ), desc)
	}
}
