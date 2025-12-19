// CLEANED: single implementation
package datasets

import (
	"encoding/csv"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"gitee.com/quant1x/quant1x/quant1x/config"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/level1"
)

// KLine mirrors the C++ datasets::KLine used elsewhere in the project.
type KLine struct {
	Date            string
	Open            float64
	Close           float64
	High            float64
	Low             float64
	Volume          float64
	Amount          float64
	Up              int
	Down            int
	Datetime        string
	AdjustmentCount int
}

const maxKlineLookbackDays = 1

// FetchKLines pulls K-line data from level1 and converts it to datasets.KLine.
func FetchKLines(securityCode string, category level1.KLineType, start, count uint16) ([]KLine, error) {
	conn, release, err := level1.GetStdConnection()
	if err != nil {
		return nil, fmt.Errorf("level1 client acquire failed: %w", err)
	}
	if release != nil {
		defer release()
	}
	if conn == nil || conn.Conn() == nil {
		return nil, fmt.Errorf("nil connection from level1 client")
	}

	req := level1.NewSecurityBarsRequest(securityCode, category, start, count)
	resp := level1.NewSecurityBarsResponse(req.IsIndex, uint16(req.Param.Category))

	if err := level1.Process(conn, req, resp); err != nil {
		return nil, fmt.Errorf("security bars request failed: %w", err)
	}

	out := make([]KLine, 0, len(resp.List))
	for _, b := range resp.List {
		date := b.DateTime
		if idx := strings.Index(b.DateTime, " "); idx >= 0 {
			date = b.DateTime[:idx]
		}
		kl := KLine{
			Date:  date,
			Open:  b.Open,
			Close: b.Close,
			High:  b.High,
			Low:   b.Low,
			// level1 returns volume in "lots" (手) — match C++ where volumeshare = vol * 100 (股)
			Volume:          b.Vol * 100,
			Amount:          b.Amount,
			Up:              int(b.UpCount),
			Down:            int(b.DownCount),
			Datetime:        b.DateTime,
			AdjustmentCount: 0,
		}
		out = append(out, kl)
	}
	return out, nil
}

// CumulativeAdjustment represents the cumulative forward-adjustment factors.
// This mirrors the fields used in the C++ `factors::CumulativeAdjustment`.
type CumulativeAdjustment struct {
	M                    float64 // multiplicative factor
	A                    float64 // additive factor
	ShareAdjustmentRatio float64 // share adjustment ratio for volumes
	No                   int     // number of adjustments applied
}

// Adjust applies forward-adjustment to the KLine according to adj.
func (k *KLine) Adjust(adj CumulativeAdjustment) {
	// compute adjusted prices
	k.Open = k.Open*adj.M + adj.A
	k.Close = k.Close*adj.M + adj.A
	k.High = k.High*adj.M + adj.A
	k.Low = k.Low*adj.M + adj.A

	// compute average price before changing volume (matches C++ logic)
	var ap float64
	if k.Volume != 0 {
		ap = k.Amount / k.Volume
	}
	apAdjusted := ap*adj.M + adj.A

	// adjust volume
	k.Volume = k.Volume * (1.0 + adj.ShareAdjustmentRatio)

	// recalc amount using adjusted avg price and new volume
	k.Amount = k.Volume * apAdjusted

	k.AdjustmentCount = adj.No
}

// SaveKline writes klines into a CSV file with header.
func SaveKline(filename string, values []KLine) error {
	f, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer f.Close()
	w := csv.NewWriter(f)
	defer w.Flush()

	header := []string{"Date", "Open", "Close", "High", "Low", "Volume", "Amount", "Up", "Down", "Datetime", "AdjustmentCount"}
	if err := w.Write(header); err != nil {
		return err
	}
	for _, r := range values {
		row := []string{
			r.Date,
			fmt.Sprintf("%g", r.Open),
			fmt.Sprintf("%g", r.Close),
			fmt.Sprintf("%g", r.High),
			fmt.Sprintf("%g", r.Low),
			fmt.Sprintf("%g", r.Volume),
			fmt.Sprintf("%g", r.Amount),
			fmt.Sprintf("%d", r.Up),
			fmt.Sprintf("%d", r.Down),
			r.Datetime,
			fmt.Sprintf("%d", r.AdjustmentCount),
		}
		if err := w.Write(row); err != nil {
			return err
		}
	}
	return nil
}

// ReadKlineFromCSV reads KLine entries from a CSV file. On error returns an empty slice and the error.
func ReadKlineFromCSV(filename string) ([]KLine, error) {
	f, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	r := csv.NewReader(f)
	rows, err := r.ReadAll()
	if err != nil {
		return nil, err
	}
	if len(rows) == 0 {
		return nil, nil
	}
	// Expect header in first row
	out := make([]KLine, 0, len(rows)-1)
	for i := 1; i < len(rows); i++ {
		rec := rows[i]
		if len(rec) < 11 {
			continue
		}
		open, _ := strconv.ParseFloat(rec[1], 64)
		closev, _ := strconv.ParseFloat(rec[2], 64)
		high, _ := strconv.ParseFloat(rec[3], 64)
		low, _ := strconv.ParseFloat(rec[4], 64)
		vol, _ := strconv.ParseFloat(rec[5], 64)
		amt, _ := strconv.ParseFloat(rec[6], 64)
		up, _ := strconv.Atoi(rec[7])
		down, _ := strconv.Atoi(rec[8])
		datetime := rec[9]
		adjCount, _ := strconv.Atoi(rec[10])
		k := KLine{
			Date:            rec[0],
			Open:            open,
			Close:           closev,
			High:            high,
			Low:             low,
			Volume:          vol,
			Amount:          amt,
			Up:              up,
			Down:            down,
			Datetime:        datetime,
			AdjustmentCount: adjCount,
		}
		out = append(out, k)
	}
	return out, nil
}

// DataKLine implements the cache adapter style updater similar to C++ DataKLine
type DataKLine struct{}

func (d *DataKLine) Print(code string, dates []exchange.Timestamp) {
	// no-op, matches C++ stub
	_ = code
	_ = dates
}

// fetchRawSecurityBars performs a low-level level1 SecurityBars request and
// returns the raw list of level1.SecurityBar (unconverted).
func fetchRawSecurityBars(securityCode string, category level1.KLineType, start, count uint16) ([]level1.SecurityBar, error) {
	conn, release, err := level1.GetStdConnection()
	if err != nil {
		return nil, fmt.Errorf("level1 client acquire failed: %w", err)
	}
	if release != nil {
		defer release()
	}
	if conn == nil || conn.Conn() == nil {
		return nil, fmt.Errorf("nil connection from level1 client")
	}

	req := level1.NewSecurityBarsRequest(securityCode, category, start, count)
	resp := level1.NewSecurityBarsResponse(req.IsIndex, uint16(req.Param.Category))
	if err := level1.Process(conn, req, resp); err != nil {
		return nil, fmt.Errorf("security bars request failed: %w", err)
	}
	return resp.List, nil
}

// Update mirrors the C++ DataKLine::Update behavior: read local cache, determine
// date window, page-fetch from level1, reverse/merge results, apply forward
// adjustments when appropriate, and save back the cache file.
func (d *DataKLine) Update(code string, _date exchange.Timestamp) {
	// 1. Determine cache filename and read local cache
	cacheFilename := config.GetKlineFilename(code, true)
	cacheKLines, _ := ReadKlineFromCSV(cacheFilename) // ignore parse errors, follow C++ behavior
	klinesLength := len(cacheKLines)
	klinesOffsetDays := maxKlineLookbackDays
	adjustTimes := 0

	// default start date
	marketFirst, _ := exchange.ParseTimestamp("1990-12-19")
	currentStartDate := marketFirst.PreMarketTime()
	if klinesLength > 0 {
		if klinesOffsetDays > klinesLength {
			klinesOffsetDays = klinesLength
		}
		// use the cached kline at offset as the start date
		idx := klinesLength - klinesOffsetDays
		if idx >= 0 && idx < klinesLength {
			if ts, err := exchange.ParseTimestamp(cacheKLines[idx].Date); err == nil {
				currentStartDate = ts.PreMarketTime()
			}
		}
		adjustTimes = cacheKLines[klinesLength-1].AdjustmentCount
	}

	// 2. determine end date
	currentEndDate := exchange.NowTimestamp().PreMarketTime()
	// build simple daily date range (inclusive). C++ uses exchange::date_range of trading days;
	// here we generate calendar days as a best-effort equivalent.
	startT := currentStartDate
	endT := currentEndDate
	var ts []exchange.Timestamp
	for t := startT; !t.Greater(endT); t = t.Offset(24, 0, 0, 0) {
		ts = append(ts, t)
	}
	total := len(ts)
	if total == 0 {
		log.Printf("[DataKLine] empty date range for %s", code)
		return
	}

	// 3. page-fetch data from level1
	step := uint16(level1.SecurityBarsMax)
	var hs [][]level1.SecurityBar
	var elementCount int
	var start uint16 = 0
	for {
		var count uint16 = step
		if uint16(total)-start >= step {
			count = step
		} else {
			count = uint16(uint16(total) - start)
		}
		if count == 0 {
			break
		}
		reply, err := fetchRawSecurityBars(code, level1.KLineDaily, start, count)
		if err != nil {
			log.Printf("[DataKLine] fetch error for %s start=%d count=%d: %v", code, start, count, err)
			break
		}
		if len(reply) == 0 {
			break
		}
		elementCount += len(reply)
		hs = append(hs, reply)
		if len(reply) < int(count) {
			break
		}
		start += count
		if int(start) >= total {
			break
		}
	}

	// 4. reverse hs (server returns newest->oldest segments)
	for i, j := 0, len(hs)-1; i < j; i, j = i+1, j-1 {
		hs[i], hs[j] = hs[j], hs[i]
	}

	// 5. convert to incremental klines and adjust units
	incremental := make([]KLine, 0, elementCount)
	for _, vec := range hs {
		for _, row := range vec {
			dts := exchange.PreMarketTimestamp(row.Year, row.Month, row.Day)
			if dts.Less(startT) || dts.Greater(endT) {
				continue
			}
			kx := KLine{
				Date:            dts.OnlyDate(),
				Open:            row.Open,
				Close:           row.Close,
				High:            row.High,
				Low:             row.Low,
				Volume:          row.Vol * 100,
				Amount:          row.Amount,
				Up:              int(row.UpCount),
				Down:            int(row.DownCount),
				Datetime:        row.DateTime,
				AdjustmentCount: 0,
			}
			incremental = append(incremental, kx)
		}
	}

	// 6. determine adjustment requirement
	isFreshFetchRequireAdjustment := adjustTimes == 1
	dividends, _ := LoadXdxr(code)
	if isFreshFetchRequireAdjustment {
		ApplyForwardAdjustmentForEvent(incremental, currentStartDate.OnlyDate(), dividends)
	}

	// 7. merge cache and incremental
	var klines []KLine
	if klinesLength > klinesOffsetDays {
		klines = append(klines, cacheKLines[:klinesLength-klinesOffsetDays]...)
	}
	if len(klines) == 0 {
		klines = incremental
	} else {
		klines = append(klines, incremental...)
	}

	// 8. full forward-adjust if not fresh-only
	if !isFreshFetchRequireAdjustment {
		ApplyForwardAdjustmentForEvent(klines, currentStartDate.OnlyDate(), dividends)
	}

	// 9. save cache (ensure parent dir exists)
	if err := os.MkdirAll(filepath.Dir(cacheFilename), 0o755); err != nil {
		log.Printf("[DataKLine] failed to create parent dir for %s: %v", cacheFilename, err)
		return
	}
	if err := SaveKline(cacheFilename, klines); err != nil {
		log.Printf("[DataKLine] save_kline failed: %v", err)
	}
}
