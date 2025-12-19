package datasets

import (
	"testing"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
)

func TestHistoryTransaction(t *testing.T) {
	code := "sh000001"
	ts, err := exchange.NewTimestampFromString("2025-12-19")
	if err != nil {
		t.Errorf("NewTimestampFromString() error = %v", err)
		return
	}
	updateTransactionData(code, ts, HistoricalTransactionDataFirstTime)
}
