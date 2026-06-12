package level1

import (
	"fmt"
	"testing"

	"github.com/quant1x/quant1x/quant1x/exchange"
)

func TestFinanceInfo(t *testing.T) {
	conn, release, err := GetStdConnection()
	if err != nil {
		t.Fatalf("GetStdConnection() returned error: %v", err)
	}
	defer release()

	req := &FinanceRequest{
		Codes: []exchange.InstrumentInfo{
			{Exchange: exchange.ExchangeSSE, Ticker: "600600"},
			{Exchange: exchange.ExchangeSZSE, Ticker: "000001"},
		},
	}
	resp := &FinanceResponse{}
	if err := Process(conn, req, resp); err != nil {
		t.Fatalf("Process() returned error: %v", err)
	}
	fmt.Printf("FinanceResponse: %+v\n", resp)
	if resp.Count != 2 {
		t.Fatalf("expected Count=2 got %d", resp.Count)
	}
}
