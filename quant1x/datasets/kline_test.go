package datasets

import (
	"path/filepath"
	"testing"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/level1"
	"gitee.com/quant1x/quant1x/quant1x/runtime"
)

func TestSaveAndReadKlineCSV(t *testing.T) {
	td := t.TempDir()
	fname := filepath.Join(td, "kline_test.csv")

	want := []KLine{{
		Date:            "2025-01-01",
		Open:            1.1,
		Close:           2.2,
		High:            3.3,
		Low:             0.9,
		Volume:          1000,
		Amount:          1100,
		Up:              5,
		Down:            3,
		Datetime:        "2025-01-01 09:30",
		AdjustmentCount: 0,
	}}

	if err := SaveKline(fname, want); err != nil {
		t.Fatalf("SaveKline failed: %v", err)
	}

	got, err := ReadKlineFromCSV(fname)
	if err != nil {
		t.Fatalf("ReadKlineFromCSV failed: %v", err)
	}

	if len(got) != len(want) {
		t.Fatalf("expected %d rows, got %d", len(want), len(got))
	}

	a := got[0]
	b := want[0]
	if a.Date != b.Date || a.Datetime != b.Datetime || a.Up != b.Up || a.Down != b.Down {
		t.Fatalf("row mismatch: got %+v want %+v", a, b)
	}
	// 检查数值字段
	if a.Open != b.Open || a.Close != b.Close || a.High != b.High || a.Low != b.Low {
		t.Fatalf("price fields mismatch: got %+v want %+v", a, b)
	}
	if a.Volume != b.Volume || a.Amount != b.Amount {
		t.Fatalf("volume/amount mismatch: got %+v want %+v", a, b)
	}
}

// TestFetchKLinesIntegration 是一个集成测试，使用真实的 `level1.Client()`（无模拟、无本地服务器），
// 并依赖已存入仓库的 XDXR 缓存文件（路径格式：`testdata/xdxr/<code>.csv`）。
//
// 要求：
//   - 运行时不要创建或使用模拟的 XDXR 文件（不要使用临时目录）。
//   - 测试会在包目录下查找 `testdata/xdxr/600000.SZ.csv`，若缺失则失败。
//   - 本测试通过真实网络访问 `level1.Client()`；若环境无法连通 Level1 服务器，测试会失败。
// （已移除依赖解析器钩子的集成测试）

func TestAdjust(t *testing.T) {
	k := KLine{
		Open:   10,
		Close:  11,
		High:   12,
		Low:    9,
		Volume: 100,
		Amount: 1000,
	}
	adj := CumulativeAdjustment{M: 2, A: 1, ShareAdjustmentRatio: 0.5, No: 3}
	k.Adjust(adj)

	if k.Open != 10*2+1 {
		t.Fatalf("Open adjustment wrong: %v", k.Open)
	}
	if k.Close != 11*2+1 {
		t.Fatalf("Close adjustment wrong: %v", k.Close)
	}
	if k.High != 12*2+1 {
		t.Fatalf("High adjustment wrong: %v", k.High)
	}
	if k.Low != 9*2+1 {
		t.Fatalf("Low adjustment wrong: %v", k.Low)
	}

	// 成交量应按 1 + ShareAdjustmentRatio 缩放
	if k.Volume != 100*(1+0.5) {
		t.Fatalf("Volume adjustment wrong: %v", k.Volume)
	}

	// amount 应重新计算：原始平均价 ap = 1000/100 = 10
	// apAdjusted = ap*M + A = 10*2 + 1 = 21
	// 新的 amount = newVolume * apAdjusted = 150 * 21 = 3150
	if k.Amount != 3150 {
		t.Fatalf("Amount recalculation wrong: %v", k.Amount)
	}

	if k.AdjustmentCount != 3 {
		t.Fatalf("AdjustmentCount wrong: %v", k.AdjustmentCount)
	}
}

func TestKLineDaily_update(t *testing.T) {
	defer runtime.WaitForShutdown(1)
	code := "600000.sh"
	securityCode := exchange.CorrectSecurityCode(code)
	list, err := FetchKLines(securityCode, level1.KLineDaily, 0, 10)
	if err != nil {
		t.Fatalf("FetchKLines failed: %v", err)
	}
	if len(list) == 0 {
		t.Fatalf("FetchKLines returned empty list")
	}
}
