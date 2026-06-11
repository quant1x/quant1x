package exchange

import (
	"testing"
)

func TestDetect_Scenarios(t *testing.T) {
	tests := []struct {
		name string
		in   string
		want InstrumentInfo
	}{
		{"sh prefix", "sh600000", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "600000", Type: SecurityTypeStock}},
		{"plain 6-digit SSE", "600000", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "600000", Type: SecurityTypeStock}},
		{"sz prefix", "sz000001", InstrumentInfo{Exchange: ExchangeSZSE, Ticker: "000001", Type: SecurityTypeStock}},
		{"hk suffix", "00700.hk", InstrumentInfo{Exchange: ExchangeHKEX, Ticker: "00700", Type: SecurityTypeStock}},
		{"us suffix", "appl.us", InstrumentInfo{Exchange: ExchangeUS, Ticker: "appl", Type: SecurityTypeStock}},
		{"us upper suffix", "APPL.US", InstrumentInfo{Exchange: ExchangeUS, Ticker: "appl", Type: SecurityTypeStock}},

		// invalid / error formats
		{"too short numeric", "123", InstrumentInfo{Exchange: ExchangeUnknown, Ticker: "", Type: SecurityTypeUnknown}},
		{"four digits numeric", "6006", InstrumentInfo{Exchange: ExchangeUnknown, Ticker: "", Type: SecurityTypeUnknown}},
		{"four digits numeric", "6006", InstrumentInfo{Exchange: ExchangeUnknown, Ticker: "", Type: SecurityTypeUnknown}},
		{"000001 (sz)", "000001", InstrumentInfo{Exchange: ExchangeSZSE, Ticker: "000001", Type: SecurityTypeStock}},
		{"880005 (block->sh)", "880005", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "880005", Type: SecurityTypeBlock}},
		{"five digits -> hk", "60060", InstrumentInfo{Exchange: ExchangeHKEX, Ticker: "60060", Type: SecurityTypeBond}},
		// 从规则表抽取的静态样例（数值前缀）——只向 tests 切片添加用例
		{"global 880", "880000", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "880000", Type: SecurityTypeBlock}},
		{"global 881", "881000", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "881000", Type: SecurityTypeBlock}},
		// SSE 相关
		{"sse ETF 51", "sh510000", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "510000", Type: SecurityTypeETF}},
		{"sse ETF 588", "sh588000", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "588000", Type: SecurityTypeETF}},
		{"sse fund 50", "sh500000", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "500000", Type: SecurityTypeFund}},
		{"sse fund 52", "sh520000", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "520000", Type: SecurityTypeETF}},
		{"sse stock 688", "688000", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "688000", Type: SecurityTypeStock}},
		{"sse stock 689", "689000", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "689000", Type: SecurityTypeStock}},
		{"sse bstock 900", "900000", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "900000", Type: SecurityTypeStockB}},
		{"sse ipo 730", "sh730000", InstrumentInfo{Exchange: ExchangeSSE, Ticker: "730000", Type: SecurityTypeIPO}},
		// SZSE 相关
		{"sz index 399", "399000", InstrumentInfo{Exchange: ExchangeSZSE, Ticker: "399000", Type: SecurityTypeIndex}},
		{"sz etf 159", "159000", InstrumentInfo{Exchange: ExchangeSZSE, Ticker: "159000", Type: SecurityTypeETF}},
		{"sz fund 150", "150000", InstrumentInfo{Exchange: ExchangeSZSE, Ticker: "150000", Type: SecurityTypeFund}},
		{"sz gem 300", "300000", InstrumentInfo{Exchange: ExchangeSZSE, Ticker: "300000", Type: SecurityTypeStock}},
		{"sz bstock 200", "200000", InstrumentInfo{Exchange: ExchangeSZSE, Ticker: "200000", Type: SecurityTypeStockB}},
		// BJSE 相关
		{"bj new 920", "920000", InstrumentInfo{Exchange: ExchangeBSE, Ticker: "920000", Type: SecurityTypeStock}},
		{"bj 83", "830000", InstrumentInfo{Exchange: ExchangeBSE, Ticker: "830000", Type: SecurityTypeStock}},
		{"bj 87", "870000", InstrumentInfo{Exchange: ExchangeBSE, Ticker: "870000", Type: SecurityTypeStock}},
		{"bj bond 82", "bj820000", InstrumentInfo{Exchange: ExchangeBSE, Ticker: "820000", Type: SecurityTypeBond}},

		// HK 相关（5位）
		{"hk etf 028", "02800", InstrumentInfo{Exchange: ExchangeHKEX, Ticker: "02800", Type: SecurityTypeETF}},
		{"hk stock 0", "00000", InstrumentInfo{Exchange: ExchangeHKEX, Ticker: "00000", Type: SecurityTypeStock}},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := DetectSymbol(tt.in)
			if got.Exchange != tt.want.Exchange {
				t.Fatalf("Detect(%q).Exchange = %s, want %s", tt.in, got.Exchange, tt.want.Exchange)
			}
			if got.Ticker != tt.want.Ticker {
				t.Fatalf("Detect(%q).Ticker = %q, want %q", tt.in, got.Ticker, tt.want.Ticker)
			}
			if got.Type != tt.want.Type {
				t.Fatalf("Detect(%q).Type = %v, want %v", tt.in, got.Type, tt.want.Type)
			}
		})
	}
}
