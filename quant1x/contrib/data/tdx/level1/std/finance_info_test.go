package std

import (
	"fmt"
	"testing"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
	"github.com/quant1x/quant1x/quant1x/data"
)

func TestFinanceInfo(t *testing.T) {
	conn, release, err := GetStdConnection()
	if err != nil {
		t.Fatalf("GetStdConnection() returned error: %v", err)
	}
	defer release()

	ctx := NewFinanceInfoContext([]data.InstrumentInfo{
		{Exchange: data.ExchangeSSE, Ticker: "600600"},
		{Exchange: data.ExchangeSZSE, Ticker: "000001"},
	})
	if err := tdxproto.TransactMessageSync(conn, ctx); err != nil {
		t.Fatalf("TransactMessageSync() returned error: %v", err)
	}
	fmt.Printf("FinanceInfoContext: %+v\n", ctx)
	if ctx.RespCount != 2 {
		t.Fatalf("expected RespCount=2 got %d", ctx.RespCount)
	}
}
