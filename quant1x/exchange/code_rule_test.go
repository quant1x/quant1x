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
		fmt.Printf("%-14s | %-6s | %-10s | %s\n", c, string(mkt), typ.String(), desc)
	}
}

func TestDetect_Scenarios(t *testing.T) {
	tests := []struct {
		name string
		in   string
		want SecurityCode
	}{
		{"sh prefix", "sh600000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "600000", Type: TypeStock}},
		{"plain 6-digit SSE", "600000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "600000", Type: TypeStock}},
		{"sz prefix", "sz000001", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "000001", Type: TypeStock}},
		{"hk suffix", "00700.hk", SecurityCode{Market: ExchangeIdHongKong, Symbol: "00700", Type: TypeStock}},
		{"us suffix", "appl.us", SecurityCode{Market: ExchangeIdUSA, Symbol: "appl", Type: TypeStock}},
		{"us upper suffix", "APPL.US", SecurityCode{Market: ExchangeIdUSA, Symbol: "appl", Type: TypeStock}},

		// invalid / error formats
		{"too short numeric", "123", SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: TypeUnknown}},
		{"four digits numeric", "6006", SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: TypeUnknown}},
		{"four digits numeric", "6006", SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: TypeUnknown}},
		{"000001 (sz)", "000001", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "000001", Type: TypeStock}},
		{"880005 (block->sh)", "880005", SecurityCode{Market: ExchangeIdShangHai, Symbol: "880005", Type: TypeBlock}},
		{"five digits -> hk", "60060", SecurityCode{Market: ExchangeIdHongKong, Symbol: "60060", Type: TypeBond}},

				// 从规则表抽取的静态样例（数值前缀）——只向 tests 切片添加用例
				{"global 880", "880000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "880000", Type: TypeBlock}},
				{"global 881", "881000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "881000", Type: TypeBlock}},

				// SSE 相关
				{"sse ETF 51", "510000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "510000", Type: TypeETF}},
				{"sse ETF 588", "588000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "588000", Type: TypeETF}},
				{"sse fund 50", "500000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "500000", Type: TypeFund}},
				{"sse fund 52", "520000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "520000", Type: TypeFund}},
				{"sse stock 688", "688000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "688000", Type: TypeStock}},
				{"sse stock 689", "689000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "689000", Type: TypeStock}},
				{"sse bstock 900", "900000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "900000", Type: TypeBStock}},
				{"sse ipo 730", "730000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "730000", Type: TypeIPO}},

				// SZSE 相关
				{"sz index 399", "399000", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "399000", Type: TypeIndex}},
				{"sz etf 159", "159000", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "159000", Type: TypeETF}},
				{"sz fund 150", "150000", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "150000", Type: TypeFund}},
				{"sz gem 300", "300000", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "300000", Type: TypeStock}},
				{"sz bstock 200", "200000", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "200000", Type: TypeBStock}},

				// BJSE 相关
				{"bj new 920", "920000", SecurityCode{Market: ExchangeIdBeiJing, Symbol: "920000", Type: TypeStock}},
				{"bj 83", "830000", SecurityCode{Market: ExchangeIdBeiJing, Symbol: "830000", Type: TypeStock}},
				{"bj 87", "870000", SecurityCode{Market: ExchangeIdBeiJing, Symbol: "870000", Type: TypeStock}},
				{"bj bond 82", "820000", SecurityCode{Market: ExchangeIdBeiJing, Symbol: "820000", Type: TypeBond}},

				// HK 相关（5位）
				{"hk etf 028", "02800", SecurityCode{Market: ExchangeIdHongKong, Symbol: "02800", Type: TypeETF}},
				{"hk stock 0", "00000", SecurityCode{Market: ExchangeIdHongKong, Symbol: "00000", Type: TypeStock}},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := Detect(tt.in)
			if got.Market != tt.want.Market {
				t.Fatalf("Detect(%q).Market = %d, want %d", tt.in, got.Market, tt.want.Market)
			}
			if got.Symbol != tt.want.Symbol {
				t.Fatalf("Detect(%q).Symbol = %q, want %q", tt.in, got.Symbol, tt.want.Symbol)
			}
			if got.Type != tt.want.Type {
				t.Fatalf("Detect(%q).Type = %v, want %v", tt.in, got.Type, tt.want.Type)
			}
		})
	}
}

func TestAllCodeRules(t *testing.T) {
	type tableEntry struct {
		rules  []CodeRule
		expMkt ExchangeId
		lenReq int // expected code length: 6 for A-shares, 5 for HK
	}

	tables := []tableEntry{
		{globalRules, ExchangeIdShangHai, 6},
		{sseRules, ExchangeIdShangHai, 6},
		{szseRules, ExchangeIdShenZhen, 6},
		{bjseRules, ExchangeIdBeiJing, 6},
		{hkseRules, ExchangeIdHongKong, 5},
	}

	for _, tb := range tables {
		for _, r := range tb.rules {
			// skip non-numeric prefixes (e.g. HSI)
			isNumeric := true
			for i := 0; i < len(r.Prefix); i++ {
				if r.Prefix[i] < '0' || r.Prefix[i] > '9' {
					isNumeric = false
					break
				}
			}
			if !isNumeric {
				continue
			}

			// build a sample code by padding zeros to required length
			pre := r.Prefix
			if len(pre) >= tb.lenReq {
				// if prefix already long enough, just take prefix truncated
				pre = pre[:tb.lenReq]
			}
			sample := pre + strings.Repeat("0", tb.lenReq-len(pre))

			t.Run(fmt.Sprintf("rule-%s-%s", pre, sample), func(t *testing.T) {
				got := Detect(sample)
				if got.Symbol != sample {
					t.Fatalf("Detect(%q).Symbol = %q, want %q", sample, got.Symbol, sample)
				}
				if got.Market == ExchangeIdUnknown {
					t.Fatalf("Detect(%q).Market = Unknown, want recognized market (rulePrefix=%s)", sample, r.Prefix)
				}

				// derive expected type using the same precedence as Detect
				var expectedType SecurityType = TypeUnknown
				if tb.lenReq == 6 {
					if typ, _ := matchRule(sample, globalRules); typ != TypeUnknown {
						expectedType = typ
					} else if typ, _ := matchRule(sample, szseRules); typ != TypeUnknown {
						expectedType = typ
					} else if typ, _ := matchRule(sample, bjseRules); typ != TypeUnknown {
						expectedType = typ
					} else if typ, _ := matchRule(sample, sseRules); typ != TypeUnknown {
						expectedType = typ
					}
				} else if tb.lenReq == 5 {
					if typ, _ := matchRule(sample, hkseRules); typ != TypeUnknown {
						expectedType = typ
					}
				}

				if got.Type != expectedType {
					t.Fatalf("Detect(%q).Type = %v, want %v (rulePrefix=%s)", sample, got.Type, expectedType, r.Prefix)
				}
			})
		}
	}
}
