package datasets

import (
	"path/filepath"
	"testing"
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
	// check numeric fields
	if a.Open != b.Open || a.Close != b.Close || a.High != b.High || a.Low != b.Low {
		t.Fatalf("price fields mismatch: got %+v want %+v", a, b)
	}
	if a.Volume != b.Volume || a.Amount != b.Amount {
		t.Fatalf("volume/amount mismatch: got %+v want %+v", a, b)
	}
}

// TestFetchKLinesIntegration is a true integration-style test that uses the
// real `level1.Client()` (no mocks, no local server) and an XDXR cache file
// that must be checked into the repository at `testdata/xdxr/<code>.csv`.
//
// Requirements:
//   - Do NOT create or use simulated XDXR files at runtime (no temp dirs).
//   - The test will look for `testdata/xdxr/600000.SZ.csv` inside the package
//     directory and will fail if that file is missing.
//   - This test uses the real network via `level1.Client()`; it will fail if
//     your environment cannot reach the Level1 servers.
// (Integration test removed — tests must not rely on resolver hooks.)

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

	// volume should be scaled by 1 + ShareAdjustmentRatio
	if k.Volume != 100*(1+0.5) {
		t.Fatalf("Volume adjustment wrong: %v", k.Volume)
	}

	// amount should be recalculated: original ap = 1000/100 = 10
	// apAdjusted = ap*M + A = 10*2 +1 =21
	// new amount = newVolume * apAdjusted = 150 * 21 = 3150
	if k.Amount != 3150 {
		t.Fatalf("Amount recalculation wrong: %v", k.Amount)
	}

	if k.AdjustmentCount != 3 {
		t.Fatalf("AdjustmentCount wrong: %v", k.AdjustmentCount)
	}
}
