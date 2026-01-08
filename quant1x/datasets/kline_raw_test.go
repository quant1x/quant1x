package datasets

import (
	"testing"

	"gitee.com/quant1x/quant1x/quant1x/data"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
)

func TestKLineRaw(t *testing.T) {
	code := "sh600000"
	adapter := data.GetDataAdapter(BaseRawDailyKLine)
	if adapter == nil {
		t.Fatalf("GetDataAdapter returned nil")
	}
	klineRawAdapter, ok := adapter.(*DataKLineRaw)
	if !ok {
		t.Fatalf("GetDataAdapter returned wrong type: %T", adapter)
	}

	// 打印当前数据
	t.Logf("=== Before Update ===")
	klineRawAdapter.Print(code)

	// 更新数据
	klineRawAdapter.Update(code, exchange.Timestamp{})

	// 打印更新后数据
	t.Logf("=== After Update ===")
	klineRawAdapter.Print(code)
}
