package level1

import (
	"testing"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
)

func TestFinanceInfo(t *testing.T) {
	conn, release, err := GetStdConnection()
	if err != nil {
		t.Fatalf("GetStdConnection() returned error: %v", err)
	}
	defer release()

	req := &FinanceRequest{
		Count:  1,
		Market: uint8(exchange.ExchangeIdShangHai),
	}
	copy(req.Code[:], "600000")
	resp := &FinanceResponse{}
	if err := Process(conn, req, resp); err != nil {
		t.Fatalf("Process() returned error: %v", err)
	}
	if resp.Count != 1 {
		t.Fatalf("expected Count=1 got %d", resp.Count)
	}
	info := resp.Info
	if info.Code != "sh600000" {
		t.Fatalf("expected Code=600000 got %s", info.Code)
	}
	if info.LiuTongGuBen <= 0 {
		t.Fatalf("expected LiuTongGuBen>0 got %f", info.LiuTongGuBen)
	}
}
