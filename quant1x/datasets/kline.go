// 单一实现（基于 C++ datasets/kline.cpp）
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
	"gitee.com/quant1x/quant1x/quant1x/data"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/level1"
)

// KLine 对应 C++ 中的 datasets::KLine，用于表示日线数据。
type KLine struct {
	Date            string  `csv:"date"`             // 日期 YYYY-MM-DD
	Open            float64 `csv:"open"`             // 开盘价
	Close           float64 `csv:"close"`            // 收盘价
	High            float64 `csv:"high"`             // 最高价
	Low             float64 `csv:"low"`              // 最低价
	Volume          float64 `csv:"volume"`           // 成交量(股)
	Amount          float64 `csv:"amount"`           // 成交额(元)
	Up              int     `csv:"up"`               // 涨家数
	Down            int     `csv:"down"`             // 跌家数
	Datetime        string  `csv:"datetime"`         // 日期时间 YYYY-MM-DD HH:MM:SS
	AdjustmentCount int     `csv:"adjustment_count"` // 复权次数
}

const maxKlineLookbackDays = 1

// FetchKLines 从 level1 拉取 K 线数据并转换为 datasets.KLine。
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
			// level1 返回的成交量单位为"手"，与 C++ 保持一致：卷转换为股时使用 vol * 100
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

// CumulativeAdjustment 表示累计复权因子。
// 字段与 C++ 中的 `factors::CumulativeAdjustment` 保持一致。
type CumulativeAdjustment struct {
	M                    float64 // multiplicative factor
	A                    float64 // additive factor
	ShareAdjustmentRatio float64 // share adjustment ratio for volumes
	No                   int     // number of adjustments applied
}

// Adjust 根据复权因子对 KLine 执行前复权调整。
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

// SaveKline 将 K 线写入 CSV 文件（包含表头）。
func SaveKline(filename string, values []KLine) error {
	f, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer f.Close()
	w := csv.NewWriter(f)
	defer w.Flush()

	header := []string{"date", "open", "close", "high", "low", "volume", "amount", "up", "down", "datetime", "adjustment_count"}
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

// ReadKlineFromCSV 从 CSV 文件中读取 KLine 条目；解析错误会返回错误。
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
	// 期望第一行为表头
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

// DataKLine 实现了与 C++ DataKLine 类似的缓存适配器更新逻辑
type DataKLine struct{}

// 实现 data.DataAdapter 的 Schema 方法
func (d *DataKLine) Kind() data.Kind { return BaseKLine }
func (d *DataKLine) Owner() string   { return data.DefaultDataProvider }
func (d *DataKLine) Key() string     { return "kline" }
func (d *DataKLine) Name() string    { return "日K线" }
func (d *DataKLine) Usage() string   { return "" }

// Print 实现 data.DataAdapter.Print（可变参数日期）
func (d *DataKLine) Print(code string, dates ...exchange.Timestamp) {
	_ = code
	_ = dates
}

// fetchRawSecurityBars 执行底层 level1 SecurityBars 请求并返回原始响应列表（未转换）。
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

// Update 对应 C++ DataKLine::Update 的行为：读取本地缓存、确定时间窗口、分页拉取 level1 数据、
// 反转与合并结果、在适当时机应用前复权，并写回缓存文件。
func (d *DataKLine) Update(code string, _date exchange.Timestamp) {
	// 1. 确定缓存文件并读取本地缓存
	cacheFilename := config.GetKlineFilename(code, true)
	cacheKLines, _ := ReadKlineFromCSV(cacheFilename) // ignore parse errors, follow C++ behavior
	klinesLength := len(cacheKLines)
	klinesOffsetDays := maxKlineLookbackDays
	adjustTimes := 0

	// 默认起始日期（使用 datasets.MarketFirstDate，与 C++ 的 market_first_date 等价）
	currentStartDate := MarketFirstDate
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

	// 2. 确定结束日期
	// 使用当前时间的盘前时间作为结束日期，并生成每日序列作为拉取区间（C++ 使用交易日历的 date_range）
	currentEndDate := exchange.NowTimestamp().PreMarketTime()
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

	// 3. 分页从 level1 拉取数据
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

	// 4. 反转分段数据（服务器按最新->最旧返回）
	for i, j := 0, len(hs)-1; i < j; i, j = i+1, j-1 {
		hs[i], hs[j] = hs[j], hs[i]
	}

	// 5. 转换为增量 K 线并调整单位（成交量单位从手转为股）
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

	// 6. 判断是否需要复权处理
	isFreshFetchRequireAdjustment := adjustTimes == 1
	dividends, _ := LoadXdxr(code)
	if isFreshFetchRequireAdjustment {
		ApplyForwardAdjustmentForEvent(incremental, currentStartDate.OnlyDate(), dividends)
	}

	// 7. 合并本地缓存与增量数据
	var klines []KLine
	if klinesLength > klinesOffsetDays {
		klines = append(klines, cacheKLines[:klinesLength-klinesOffsetDays]...)
	}
	if len(klines) == 0 {
		klines = incremental
	} else {
		klines = append(klines, incremental...)
	}

	// 8. 若不是仅更新最新记录，则对全量数据做前复权处理
	if !isFreshFetchRequireAdjustment {
		ApplyForwardAdjustmentForEvent(klines, currentStartDate.OnlyDate(), dividends)
	}

	// 9. 保存缓存（确保父目录存在）
	if err := os.MkdirAll(filepath.Dir(cacheFilename), 0o755); err != nil {
		log.Printf("[DataKLine] failed to create parent dir for %s: %v", cacheFilename, err)
		return
	}
	if err := SaveKline(cacheFilename, klines); err != nil {
		log.Printf("[DataKLine] save_kline failed: %v", err)
	}
}

func init() {
	// register DataKLine plugin (ignore error if already registered)
	_ = data.Register(&DataKLine{})
}
