package tdx

import (
	"encoding/csv"
	"fmt"
	"os"
	"strconv"

	"gitee.com/quant1x/quant1x/quant1x/data/exchange"
	"github.com/quant1x/quant1x/quant1x/config"
	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/level1/std"
	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/data/schema"
	"github.com/quant1x/quant1x/quant1x/encoding"
	logger "github.com/quant1x/quant1x/quant1x/log"
)

// DataBarRaw 实现了与 C++ DataBarRaw 类似的缓存适配器更新逻辑
type DataBarRaw struct{}

// 实现 DataAdapter 的 Schema 方法
func (d *DataBarRaw) Kind() data.Kind { return data.BaseRawDailyKLine }
func (d *DataBarRaw) Owner() string   { return data.DefaultDataProvider }
func (d *DataBarRaw) Key() string     { return "day_raw" }
func (d *DataBarRaw) Name() string    { return "日K线RAW" }
func (d *DataBarRaw) Usage() string   { return "日K线RAW" }

// Print 实现 DataAdapter.Print(可变参数日期)
func (d *DataBarRaw) Print(code data.InstrumentInfo, dates ...exchange.Timestamp) {
	_ = code
	_ = dates
}

// Update 对应 C++ DataBarRaw::Update 的行为: 读取本地缓存, 确定时间窗口, 分页拉取 level1 数据,
// 反转与合并结果, 并写回缓存文件.
func (d *DataBarRaw) Update(code data.InstrumentInfo, _date data.Timestamp) {
	// 1. 确定缓存文件并读取本地缓存
	cacheFilename := config.GetBarFilename(code.Symbol(), false)
	var cacheBars []BarRaw
	encoding.CsvToSlices(cacheFilename, &cacheBars)

	barsLength := len(cacheBars)
	barsOffsetDays := schema.MaxCachedDaysToDropOnIncrementalUpdate

	// 默认起始日期(使用 datasets.MarketFirstDate, 与 C++ 的 market_first_date 等价)
	currentStartDate := data.GetFirstMarketDate(code.Exchange)
	if barsLength > 0 {
		if barsOffsetDays > barsLength {
			barsOffsetDays = barsLength
		}
		bar := cacheBars[barsLength-barsOffsetDays]
		if ts, err := data.ParseTimestamp(bar.Date); err == nil {
			currentStartDate = ts
		}
	}

	// 2. 确定结束日期
	currentEndDate := data.NowTimestamp().PreMarketTime()
	logger.Debugf("[dataset::BarRaw] [%s]: from %s to %s", code, currentStartDate.OnlyDate(), currentEndDate.OnlyDate())

	step := uint16(std.SecurityBarsMax)
	start := uint16(0)
	hs := make([][]std.SecurityBar, 0)
	elementCount := 0

	for {
		count := step
		reply, err := tdxFetchRawSecurityBars(code, std.FreqDaily, start, count)
		if err != nil || len(reply) == 0 {
			break
		}

		elementCount += len(reply)
		hs = append(hs, reply)

		lastBar := reply[len(reply)-1]
		lastBarDate := data.PreMarketTimestamp(lastBar.Year, lastBar.Month, lastBar.Day)

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

	incrementalBars := make([]BarRaw, 0)

	for _, vec := range hs {
		for _, row := range vec {
			dateTime := data.PreMarketTimestamp(row.Year, row.Month, row.Day)

			if dateTime.Less(currentStartDate) || dateTime.Greater(currentEndDate) {
				continue
			}

			bx := BarRaw{
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
			incrementalBars = append(incrementalBars, bx)
		}
	}

	// 7. 合并
	bars := make([]BarRaw, 0)
	if barsLength > barsOffsetDays {
		bars = append(bars, cacheBars[:barsLength-barsOffsetDays]...)
	}
	bars = append(bars, incrementalBars...)

	// 9. 保存
	if err := saveBarRaw(cacheFilename, bars); err != nil {
		logger.Errorf("[dataset::BarRaw] save error: %v", err)
	}
}

func init() {
	// 注册DataBarRaw插件
	if err := data.Register(&DataBarRaw{}); err != nil {
		logger.Errorf("[dataset::BarRaw] failed to register plugin: %v", err)
	}
}

// BarRaw 对应 C++ 中的 data::BarRaw, 用于表示原始日线数据.
type BarRaw struct {
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

//const maxBarRawLookbackDays = 1

// saveBarRaw 将原始 K 线写入 CSV 文件(包含表头).
func saveBarRaw(filename string, values []BarRaw) error {
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

// ReadBarRawFromCSV 从 CSV 文件中读取 BarRaw 条目；解析错误会返回错误.
func ReadBarRawFromCSV(filename string) ([]BarRaw, error) {
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
	out := make([]BarRaw, 0, len(rows)-1)
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
		k := BarRaw{
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

// LoadBarRaw 从缓存文件加载原始K线数据.
func LoadBarRaw(code string) ([]BarRaw, error) {
	filename := config.GetBarFilename(code, false)
	logger.Debugf("[data::BarRaw] bar file: %s", filename)
	return ReadBarRawFromCSV(filename)
}
