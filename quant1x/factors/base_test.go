package factors

import (
	"fmt"
	"math"
	"testing"

	"github.com/quant1x/quant1x/quant1x/config"
	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/data"
)

func TestGetCrossSectionForwardAdjustedKlines(t *testing.T) {
	code := "sh600000"
	asOfDate := "2024-12-26"

	klines := GetCrossSectionForwardAdjustedKlines(code, asOfDate)

	if len(klines) == 0 {
		t.Errorf("No klines returned for %s as of %s", code, asOfDate)
		return
	}

	fmt.Printf("Loaded %d adjusted kline records for %s\n", len(klines), code)

	// Check some basic properties
	for i, kline := range klines {
		if kline.Date == "" {
			t.Errorf("Kline %d has empty date", i)
		}
	}

	// Display sample data (similar to Python __main__)
	fmt.Println("\n=== 复权前后对比 (2024年样本数据) ===")
	sampleCount := 0
	for _, kline := range klines {
		if len(kline.Date) >= 4 && kline.Date[:4] == "2024" {
			fmt.Printf("日期: %s\n", kline.Date)
			fmt.Printf("  复权: 开=%.2f, 高=%.2f, 低=%.2f, 收=%.2f\n", kline.Open, kline.High, kline.Low, kline.Close)
			sampleCount++
			if sampleCount >= 5 {
				break
			}
		}
	}
}

func TestCombineAdjustmentsInPeriod(t *testing.T) {
	// Load some test XDXR data
	xdxrList, err := data.LoadXdxr("sh600000")
	if err != nil {
		t.Skipf("Skipping test due to missing XDXR data: %v", err)
		return
	}

	if len(xdxrList) == 0 {
		t.Skip("No XDXR data available")
		return
	}

	// Create test timestamps
	startDate := data.PreMarketTimestamp(2024, 1, 1)
	endDate := data.PreMarketTimestamp(2024, 12, 31)

	adjustments := CombineAdjustmentsInPeriod(xdxrList, startDate, endDate)

	fmt.Printf("Combined %d adjustments for period\n", len(adjustments))

	// Basic validation
	for i, adj := range adjustments {
		if adj.M <= 0 {
			t.Errorf("Adjustment %d has invalid M: %.6f", i, adj.M)
		}
		if adj.No < 1 {
			t.Errorf("Adjustment %d has invalid No: %d", i, adj.No)
		}
	}
}

func TestCheckKlineOffset(t *testing.T) {
	// Create mock kline data
	klines := []data.KLineRaw{
		{Date: "2024-01-01"},
		{Date: "2024-01-02"},
		{Date: "2024-01-03"},
		{Date: "2024-01-04"},
		{Date: "2024-01-05"},
	}

	offset := CheckKlineOffset(klines, "2024-01-03")
	expected := 2
	if offset != expected {
		t.Errorf("CheckKlineOffset returned %d, expected %d", offset, expected)
	}

	// Test edge cases
	offset = CheckKlineOffset(klines, "2024-01-01")
	if offset != 4 {
		t.Errorf("CheckKlineOffset for first date returned %d, expected 4", offset)
	}

	offset = CheckKlineOffset(klines, "2025-01-01")
	if offset != -1 {
		t.Errorf("CheckKlineOffset for future date returned %d, expected -1", offset)
	}
}

func TestIpoDateFromXdxrs(t *testing.T) {
	// Load XDXR data
	xdxrList, err := data.LoadXdxr("sh600000")
	if err != nil {
		t.Skipf("Skipping test due to missing XDXR data: %v", err)
		return
	}

	ipoDate := IpoDateFromXdxrs(xdxrList)
	if ipoDate != nil {
		fmt.Printf("IPO date found: %s\n", *ipoDate)
	} else {
		fmt.Println("No IPO date found")
	}
}

func TestCumulativeAdjustment(t *testing.T) {
	ts := data.PreMarketTimestamp(2024, 1, 1)
	adj := CumulativeAdjustment{
		Timestamp:            ts,
		M:                    0.9,
		A:                    0.1,
		MonetaryAdjustment:   1.0,
		ShareAdjustmentRatio: 0.1,
		No:                   1,
	}

	// Test ToString
	str := adj.ToString()
	if str == "" {
		t.Error("ToString returned empty string")
	}
	fmt.Printf("Adjustment string: %s\n", str)

	// Test Apply
	price := 10.0
	adjusted := adj.Apply(price)
	expected := price*adj.M + adj.A
	if adjusted != expected {
		t.Errorf("Apply returned %.6f, expected %.6f", adjusted, expected)
	}

	// Test Inverse
	inverse := adj.Inverse(adjusted)
	if inverse != price {
		t.Errorf("Inverse returned %.6f, expected %.6f", inverse, price)
	}
}

func TestCompareWithCachedKlines(t *testing.T) {
	code := "sh600000"

	// Load cached kline data
	cacheFilename := config.GetKlineFilename(code, true)
	cachedKlines, err := data.ReadKlineFromCSV(cacheFilename)
	if err != nil || len(cachedKlines) == 0 {
		t.Skipf("Skipping test due to missing cached kline data: %v", err)
		return
	}

	firstCachedDate := cachedKlines[0].Date
	lastCachedDate := cachedKlines[len(cachedKlines)-1].Date
	fmt.Printf("data.kline cache date range: %s to %s\n", firstCachedDate, lastCachedDate)

	// Use GetCrossSectionForwardAdjustedKlines to get adjusted data for the same date range
	adjustedKlines := GetCrossSectionForwardAdjustedKlines(code, lastCachedDate)

	if len(adjustedKlines) == 0 {
		t.Errorf("GetCrossSectionForwardAdjustedKlines returned empty data")
		return
	}

	// Find the first data with the same date
	var firstAdjusted *data.KLine
	firstCached := cachedKlines[0]

	for i := range adjustedKlines {
		if adjustedKlines[i].Date == firstCached.Date {
			firstAdjusted = adjustedKlines[i]
			break
		}
	}

	if firstAdjusted == nil {
		t.Errorf("GetCrossSectionForwardAdjustedKlines does not contain date %s", firstCached.Date)
		fmt.Printf("adjusted_klines length: %d\n", len(adjustedKlines))
		if len(adjustedKlines) > 0 {
			fmt.Printf("first: %s, last: %s\n", adjustedKlines[0].Date, adjustedKlines[len(adjustedKlines)-1].Date)
		}
		return
	}

	fmt.Printf("\nData comparison on %s:\n", firstCached.Date)
	fmt.Printf("GetCrossSectionForwardAdjustedKlines:\n")
	fmt.Printf("  Open: %.4f, High: %.4f, Low: %.4f, Close: %.4f\n", firstAdjusted.Open, firstAdjusted.High, firstAdjusted.Low, firstAdjusted.Close)
	fmt.Printf("  Volume: %.0f, Amount: %.0f\n", firstAdjusted.Volume, firstAdjusted.Amount)

	fmt.Printf("data.kline cache:\n")
	fmt.Printf("  Open: %.4f, High: %.4f, Low: %.4f, Close: %.4f\n", firstCached.Open, firstCached.High, firstCached.Low, firstCached.Close)
	fmt.Printf("  Volume: %.0f, Amount: %.0f\n", firstCached.Volume, firstCached.Amount)

	// Compare data
	fmt.Printf("\nDifferences:\n")
	fmt.Printf("  Open price: %.6f\n", math.Abs(firstAdjusted.Open-firstCached.Open))
	fmt.Printf("  Close price: %.6f\n", math.Abs(firstAdjusted.Close-firstCached.Close))
	fmt.Printf("  High price: %.6f\n", math.Abs(firstAdjusted.High-firstCached.High))
	fmt.Printf("  Low price: %.6f\n", math.Abs(firstAdjusted.Low-firstCached.Low))
	fmt.Printf("  Volume: %.0f\n", math.Abs(firstAdjusted.Volume-firstCached.Volume))
	fmt.Printf("  Amount: %.0f\n", math.Abs(firstAdjusted.Amount-firstCached.Amount))

	if math.Abs(firstAdjusted.Open-firstCached.Open) < 0.0001 &&
		math.Abs(firstAdjusted.Close-firstCached.Close) < 0.0001 &&
		math.Abs(firstAdjusted.High-firstCached.High) < 0.0001 &&
		math.Abs(firstAdjusted.Low-firstCached.Low) < 0.0001 &&
		math.Abs(firstAdjusted.Volume-firstCached.Volume) < 1 &&
		math.Abs(firstAdjusted.Amount-firstCached.Amount) < 1 {
		fmt.Println("SUCCESS: Data matches completely!")
	} else {
		fmt.Println("ERROR: Data differs")

		// Check adjustment count
		fmt.Printf("Adjustment count comparison:\n")
		fmt.Printf("  GetCrossSectionForwardAdjustedKlines: %d\n", firstAdjusted.AdjustmentCount)
		fmt.Printf("  data.kline: %d\n", firstCached.AdjustmentCount)
	}
}
