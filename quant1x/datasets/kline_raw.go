// 单一实现（基于 C++ datasets/kline_raw.cpp）
package datasets

import (
	"encoding/csv"
	"fmt"
	"log"
	"os"
	"strconv"

	"gitee.com/quant1x/quant1x/quant1x/config"
	"gitee.com/quant1x/quant1x/quant1x/data"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/level1"
)

// KLineRaw 对应 C++ 中的 datasets::KLineRaw，用于表示原始日线数据。
type KLineRaw struct {
	Date     string  `name:"日期" csv:"date"`        // 日期 YYYY-MM-DD
	Open     float64 `name:"开盘价" csv:"open"`       // 开盘价
	Close    float64 `name:"收盘价" csv:"close"`      // 收盘价
	High     float64 `name:"最高价" csv:"high"`       // 最高价
	Low      float64 `name:"最低价" csv:"low"`        // 最低价
	Volume   float64 `name:"成交量(股)" csv:"volume"`  // 成交量(股)
	Amount   float64 `name:"成交额(元)" csv:"amount"`  // 成交额(元)
	Up       int     `name:"上涨家数 / 外盘" csv:"up"`   // 上涨家数 / 外盘
	Down     int     `name:"下跌家数 / 内盘" csv:"down"` // 下跌家数 / 内盘
	Datetime string  `name:"时间" csv:"datetime"`    // 时间
}

const maxKlineRawLookbackDays = 1

// FetchKLineRaw 从 level1 拉取原始 K 线数据并转换为 datasets.KLineRaw。
func FetchKLineRaw(securityCode string, start, count uint16, klineType level1.KLineType) ([]level1.SecurityBar, error) {
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

	req := level1.NewSecurityBarsRequest(securityCode, klineType, start, count)
	resp := level1.NewSecurityBarsResponse(req.IsIndex, uint16(req.Param.Category))
	if err := level1.Process(conn, req, resp); err != nil {
		return nil, fmt.Errorf("security bars request failed: %w", err)
	}

	return resp.List, nil
}

// SaveKlineRaw 将原始 K 线写入 CSV 文件（包含表头）。
func SaveKlineRaw(filename string, values []KLineRaw) error {
	f, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer f.Close()
	w := csv.NewWriter(f)
	defer w.Flush()

	header := []string{"date", "open", "close", "high", "low", "volume", "amount", "up", "down", "datetime"}
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
		}
		if err := w.Write(row); err != nil {
			return err
		}
	}
	return nil
}

// ReadKlineRawFromCSV 从 CSV 文件中读取 KLineRaw 条目；解析错误会返回错误。
func ReadKlineRawFromCSV(filename string) ([]KLineRaw, error) {
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
	out := make([]KLineRaw, 0, len(rows)-1)
	for i := 1; i < len(rows); i++ {
		rec := rows[i]
		if len(rec) < 10 {
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
		k := KLineRaw{
			Date:     rec[0],
			Open:     open,
			Close:    closev,
			High:     high,
			Low:      low,
			Volume:   vol,
			Amount:   amt,
			Up:       up,
			Down:     down,
			Datetime: datetime,
		}
		out = append(out, k)
	}
	return out, nil
}

// LoadKlineRaw 从缓存文件加载原始K线数据。
func LoadKlineRaw(code string) ([]KLineRaw, error) {
	filename := config.GetKlineFilename(code, false)
	log.Printf("[dataset::KLineRaw] kline file: %s", filename)
	return ReadKlineRawFromCSV(filename)
}

// DataKLineRaw 实现了与 C++ DataKLineRaw 类似的缓存适配器更新逻辑
type DataKLineRaw struct{}

// 实现 data.DataAdapter 的 Schema 方法
func (d *DataKLineRaw) Kind() data.Kind { return BaseRawDailyKLine }
func (d *DataKLineRaw) Owner() string   { return data.DefaultDataProvider }
func (d *DataKLineRaw) Key() string     { return "day_raw" }
func (d *DataKLineRaw) Name() string    { return "日K线RAW" }
func (d *DataKLineRaw) Usage() string   { return "日K线RAW" }

// Print 实现 data.DataAdapter.Print（可变参数日期）
func (d *DataKLineRaw) Print(code string, dates ...exchange.Timestamp) {
	_ = code
	_ = dates
}

// Update 对应 C++ DataKLineRaw::Update 的行为：读取本地缓存、确定时间窗口、分页拉取 level1 数据、
// 反转与合并结果，并写回缓存文件。
func (d *DataKLineRaw) Update(code string, _date exchange.Timestamp) {
	// 1. 确定缓存文件并读取本地缓存
	cacheFilename := config.GetKlineFilename(code, false)
	cacheKLines, _ := ReadKlineRawFromCSV(cacheFilename) // ignore parse errors, follow C++ behavior
	klinesLength := len(cacheKLines)
	klinesOffsetDays := maxKlineRawLookbackDays

	// 默认起始日期（使用 datasets.MarketFirstDate，与 C++ 的 market_first_date 等价）
	currentStartDate := MarketFirstDate
	if klinesLength > 0 {
		if klinesOffsetDays > klinesLength {
			klinesOffsetDays = klinesLength
		}
		kline := cacheKLines[klinesLength-klinesOffsetDays]
		if ts, err := exchange.ParseTimestamp(kline.Date); err == nil {
			currentStartDate = ts
		}
	}

	// 2. 确定结束日期
	currentEndDate := exchange.NowTimestamp().PreMarketTime()
	log.Printf("[dataset::KLineRaw] [%s]: from %s to %s", code, currentStartDate.OnlyDate(), currentEndDate.OnlyDate())

	step := uint16(level1.SecurityBarsMax)
	start := uint16(0)
	hs := make([][]level1.SecurityBar, 0)
	elementCount := 0

	for {
		count := step
		reply, err := FetchKLineRaw(code, start, uint16(count), level1.KLineDaily)
		if err != nil || len(reply) == 0 {
			break
		}

		elementCount += len(reply)
		hs = append(hs, reply)

		lastBar := reply[len(reply)-1]
		lastBarDate := exchange.PreMarketTimestamp(lastBar.Year, lastBar.Month, lastBar.Day)

		if lastBarDate.Less(currentStartDate) {
			break
		}

		if len(reply) < int(count) {
			break
		}

		start += uint16(count)
	}

	// 反转切片以获得正确的顺序
	for i, j := 0, len(hs)-1; i < j; i, j = i+1, j-1 {
		hs[i], hs[j] = hs[j], hs[i]
	}

	incrementalKLines := make([]KLineRaw, 0)

	for _, vec := range hs {
		for _, row := range vec {
			dateTime := exchange.PreMarketTimestamp(row.Year, row.Month, row.Day)

			if dateTime.Less(currentStartDate) || dateTime.Greater(currentEndDate) {
				continue
			}

			kx := KLineRaw{
				Date:     dateTime.OnlyDate(),
				Open:     row.Open,
				Close:    row.Close,
				High:     row.High,
				Low:      row.Low,
				Volume:   row.Vol * 100, // Convert to shares
				Amount:   row.Amount,
				Up:       int(row.UpCount),
				Down:     int(row.DownCount),
				Datetime: row.DateTime,
			}
			incrementalKLines = append(incrementalKLines, kx)
		}
	}

	// 7. 合并
	klines := make([]KLineRaw, 0)
	if klinesLength > klinesOffsetDays {
		klines = append(klines, cacheKLines[:klinesLength-klinesOffsetDays]...)
	}
	klines = append(klines, incrementalKLines...)

	// 9. 保存
	if err := SaveKlineRaw(cacheFilename, klines); err != nil {
		log.Printf("[dataset::KLineRaw] save error: %v", err)
	}
}

func init() {
	// 注册DataKLineRaw插件
	if err := data.Register(&DataKLineRaw{}); err != nil {
		log.Printf("[dataset::KLineRaw] failed to register plugin: %v", err)
	}
}
