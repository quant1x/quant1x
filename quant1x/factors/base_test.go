package factors

import (
	"fmt"
	"math"
	"testing"

	"github.com/quant1x/quant1x/quant1x/config"
	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/data"
)

func TestGetCrossSectionForwardAdjustedBars(t *testing.T) {
	code := "sh600000"
	asOfDate := "2024-12-26"

	bars := GetCrossSectionForwardAdjustedBars(code, asOfDate)

	if len(bars) == 0 {
		t.Errorf("No bars returned for %s as of %s", code, asOfDate)
		return
	}

	fmt.Printf("Loaded %d adjusted bar records for %s\n", len(bars), code)

	// Check some basic properties
	for i, bar := range bars {
		if bar.Date == "" {
			t.Errorf("Bar %d has empty date", i)
		}
	}

	// Display sample data (similar to Python __main__)
	fmt.Println("\n=== 复权前后对比 (2024年样本数据) ===")
	sampleCount := 0
	for _, bar := range bars {
		if len(bar.Date) >= 4 && bar.Date[:4] == "2024" {
			fmt.Printf("日期: %s\n", bar.Date)
			fmt.Printf("  复权: 开=%.2f, 高=%.2f, 低=%.2f, 收=%.2f\n", bar.Open, bar.High, bar.Low, bar.Close)
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

func TestCheckBarOffset(t *testing.T) {
	// Create mock bar data
	bars := []data.KLineRaw{
		{Date: "2024-01-01"},
		{Date: "2024-01-02"},
		{Date: "2024-01-03"},
		{Date: "2024-01-04"},
		{Date: "2024-01-05"},
	}

	offset := CheckBarOffset(bars, "2024-01-03")
	expected := 2
	if offset != expected {
		t.Errorf("CheckBarOffset returned %d, expected %d", offset, expected)
	}

	// Test edge cases
	offset = CheckBarOffset(bars, "2024-01-01")
	if offset != 4 {
		t.Errorf("CheckBarOffset for first date returned %d, expected 4", offset)
	}

	offset = CheckBarOffset(bars, "2025-01-01")
	if offset != -1 {
		t.Errorf("CheckBarOffset for future date returned %d, expected -1", offset)
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

func TestCompareWithCachedBars(t *testing.T) {
	code := "sh600000"

	// Load cached bar data
	cacheFilename := config.GetBarFilename(code, true)
	cachedBars, err := data.ReadBarFromCSV(cacheFilename)
	if err != nil || len(cachedBars) == 0 {
		t.Skipf("Skipping test due to missing cached bar data: %v", err)
		return
	}

	firstCachedDate := cachedBars[0].Date
	lastCachedDate := cachedBars[len(cachedBars)-1].Date
	fmt.Printf("data.bar cache date range: %s to %s\n", firstCachedDate, lastCachedDate)

	// Use GetCrossSectionForwardAdjustedBars to get adjusted data for the same date range
	adjustedBars := GetCrossSectionForwardAdjustedBars(code, lastCachedDate)

	if len(adjustedBars) == 0 {
		t.Errorf("GetCrossSectionForwardAdjustedBars returned empty data")
		return
	}

	// Find the first data with the same date
	var firstAdjusted *data.KLine
	firstCached := cachedBars[0]

	for i := range adjustedBars {
		if adjustedBars[i].Date == firstCached.Date {
			firstAdjusted = adjustedBars[i]
			break
		}
	}

	if firstAdjusted == nil {
		t.Errorf("GetCrossSectionForwardAdjustedBars does not contain date %s", firstCached.Date)
		fmt.Printf("adjusted_bars length: %d\n", len(adjustedBars))
		if len(adjustedBars) > 0 {
			fmt.Printf("first: %s, last: %s\n", adjustedBars[0].Date, adjustedBars[len(adjustedBars)-1].Date)
		}
		return
	}

	fmt.Printf("\nData comparison on %s:\n", firstCached.Date)
	fmt.Printf("GetCrossSectionForwardAdjustedBars:\n")
	fmt.Printf("  Open: %.4f, High: %.4f, Low: %.4f, Close: %.4f\n", firstAdjusted.Open, firstAdjusted.High, firstAdjusted.Low, firstAdjusted.Close)
	fmt.Printf("  Volume: %.0f, Amount: %.0f\n", firstAdjusted.Volume, firstAdjusted.Amount)

	fmt.Printf("data.bar cache:\n")
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
		fmt.Printf("  GetCrossSectionForwardAdjustedBars: %d\n", firstAdjusted.AdjustmentCount)
		fmt.Printf("  data.bar: %d\n", firstCached.AdjustmentCount)
	}
}
