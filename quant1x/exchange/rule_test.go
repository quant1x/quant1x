package exchange

import (
	"testing"
)

func TestStringWithLocale(t *testing.T) {
	if got := SecurityStock.StringWithLocale("zh"); got != "股票" {
		t.Fatalf("Stock.StringWithLocale(zh) = %q; want %q", got, "股票")
	}
	if got := SecurityStock.StringWithLocale("en"); got != "Stock" {
		t.Fatalf("Stock.StringWithLocale(en) = %q; want %q", got, "Stock")
	}
	if got := SecurityUnknown.StringWithLocale("zh"); got != "未知" {
		t.Fatalf("Unknown.StringWithLocale(zh) = %q; want %q", got, "未知")
	}
}

func TestSetLocaleAffectsString(t *testing.T) {
	// ensure deterministic behavior by explicit SetLocale
	SetLocale("zh")
	if got := SecurityStock.String(); got != "股票" {
		t.Fatalf("after SetLocale(zh), Stock.String() = %q; want %q", got, "股票")
	}

	SetLocale("en")
	if got := SecurityStock.String(); got != "Stock" {
		t.Fatalf("after SetLocale(en), Stock.String() = %q; want %q", got, "Stock")
	}
}

func TestDetect_Scenarios(t *testing.T) {
	tests := []struct {
		name string
		in   string
		want SecurityCode
	}{
		{"sh prefix", "sh600000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "600000", Type: SecurityStock}},
		{"plain 6-digit SSE", "600000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "600000", Type: SecurityStock}},
		{"sz prefix", "sz000001", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "000001", Type: SecurityStock}},
		{"hk suffix", "00700.hk", SecurityCode{Market: ExchangeIdHongKong, Symbol: "00700", Type: SecurityStock}},
		{"us suffix", "appl.us", SecurityCode{Market: ExchangeIdUSA, Symbol: "appl", Type: SecurityStock}},
		{"us upper suffix", "APPL.US", SecurityCode{Market: ExchangeIdUSA, Symbol: "appl", Type: SecurityStock}},

		// invalid / error formats
		{"too short numeric", "123", SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}},
		{"four digits numeric", "6006", SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}},
		{"four digits numeric", "6006", SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}},
		{"000001 (sz)", "000001", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "000001", Type: SecurityStock}},
		{"880005 (block->sh)", "880005", SecurityCode{Market: ExchangeIdShangHai, Symbol: "880005", Type: SecurityBlock}},
		{"five digits -> hk", "60060", SecurityCode{Market: ExchangeIdHongKong, Symbol: "60060", Type: SecurityBond}},

		// 从规则表抽取的静态样例（数值前缀）——只向 tests 切片添加用例
		{"global 880", "880000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "880000", Type: SecurityBlock}},
		{"global 881", "881000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "881000", Type: SecurityBlock}},
		// SSE 相关
		{"sse ETF 51", "sh510000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "510000", Type: SecurityETF}},
		{"sse ETF 588", "sh588000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "588000", Type: SecurityETF}},
		{"sse fund 50", "sh500000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "500000", Type: SecurityFund}},
		{"sse fund 52", "sh520000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "520000", Type: SecurityETF}},
		{"sse stock 688", "688000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "688000", Type: SecurityStock}},
		{"sse stock 689", "689000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "689000", Type: SecurityStock}},
		{"sse bstock 900", "900000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "900000", Type: SecurityBStock}},
		{"sse ipo 730", "sh730000", SecurityCode{Market: ExchangeIdShangHai, Symbol: "730000", Type: SecurityIPO}},
		// SZSE 相关
		{"sz index 399", "399000", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "399000", Type: SecurityIndex}},
		{"sz etf 159", "159000", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "159000", Type: SecurityETF}},
		{"sz fund 150", "150000", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "150000", Type: SecurityFund}},
		{"sz gem 300", "300000", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "300000", Type: SecurityStock}},
		{"sz bstock 200", "200000", SecurityCode{Market: ExchangeIdShenZhen, Symbol: "200000", Type: SecurityBStock}},
		// BJSE 相关
		{"bj new 920", "920000", SecurityCode{Market: ExchangeIdBeiJing, Symbol: "920000", Type: SecurityStock}},
		{"bj 83", "830000", SecurityCode{Market: ExchangeIdBeiJing, Symbol: "830000", Type: SecurityStock}},
		{"bj 87", "870000", SecurityCode{Market: ExchangeIdBeiJing, Symbol: "870000", Type: SecurityStock}},
		{"bj bond 82", "bj820000", SecurityCode{Market: ExchangeIdBeiJing, Symbol: "820000", Type: SecurityBond}},

		// HK 相关（5位）
		{"hk etf 028", "02800", SecurityCode{Market: ExchangeIdHongKong, Symbol: "02800", Type: SecurityETF}},
		{"hk stock 0", "00000", SecurityCode{Market: ExchangeIdHongKong, Symbol: "00000", Type: SecurityStock}},
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
