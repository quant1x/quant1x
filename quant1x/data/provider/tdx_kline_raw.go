package provider

import (
	"gitee.com/quant1x/quant1x/quant1x/config"
	"gitee.com/quant1x/quant1x/quant1x/data"
	"gitee.com/quant1x/quant1x/quant1x/encoding"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/level1"
	"gitee.com/quant1x/quant1x/quant1x/logger"
)

// DataKLineRaw 实现了与 C++ DataKLineRaw 类似的缓存适配器更新逻辑
type DataKLineRaw struct{}

// 实现 DataAdapter 的 Schema 方法
func (d *DataKLineRaw) Kind() data.Kind { return data.BaseRawDailyKLine }
func (d *DataKLineRaw) Owner() string   { return data.DefaultDataProvider }
func (d *DataKLineRaw) Key() string     { return "day_raw" }
func (d *DataKLineRaw) Name() string    { return "日K线RAW" }
func (d *DataKLineRaw) Usage() string   { return "日K线RAW" }

// Print 实现 DataAdapter.Print（可变参数日期）
func (d *DataKLineRaw) Print(code exchange.SecurityCode, dates ...exchange.Timestamp) {
	_ = code
	_ = dates
}

// Update 对应 C++ DataKLineRaw::Update 的行为：读取本地缓存、确定时间窗口、分页拉取 level1 数据、
// 反转与合并结果，并写回缓存文件。
func (d *DataKLineRaw) Update(code exchange.SecurityCode, _date exchange.Timestamp) {
	// 1. 确定缓存文件并读取本地缓存
	cacheFilename := config.GetKlineFilename(code.String(), false)
	var cacheKLines []data.KLineRaw
	encoding.CsvToSlices(cacheFilename, &cacheKLines)

	klinesLength := len(cacheKLines)
	klinesOffsetDays := data.MaxCachedDaysToDropOnIncrementalUpdate

	// 默认起始日期（使用 datasets.MarketFirstDate，与 C++ 的 market_first_date 等价）
	currentStartDate := exchange.GetFirstMarketDate(code.Market)
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
	logger.Debugf("[dataset::KLineRaw] [%s]: from %s to %s", code, currentStartDate.OnlyDate(), currentEndDate.OnlyDate())

	step := uint16(level1.SecurityBarsMax)
	start := uint16(0)
	hs := make([][]level1.SecurityBar, 0)
	elementCount := 0

	for {
		count := step
		reply, err := tdxFetchRawSecurityBars(code, level1.KLineDaily, start, count)
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

	incrementalKLines := make([]data.KLineRaw, 0)

	for _, vec := range hs {
		for _, row := range vec {
			dateTime := exchange.PreMarketTimestamp(row.Year, row.Month, row.Day)

			if dateTime.Less(currentStartDate) || dateTime.Greater(currentEndDate) {
				continue
			}

			kx := data.KLineRaw{
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
	klines := make([]data.KLineRaw, 0)
	if klinesLength > klinesOffsetDays {
		klines = append(klines, cacheKLines[:klinesLength-klinesOffsetDays]...)
	}
	klines = append(klines, incrementalKLines...)

	// 9. 保存
	if err := data.SaveKlineRaw(cacheFilename, klines); err != nil {
		logger.Errorf("[dataset::KLineRaw] save error: %v", err)
	}
}

func init() {
	// 注册DataKLineRaw插件
	if err := data.Register(&DataKLineRaw{}); err != nil {
		logger.Errorf("[dataset::KLineRaw] failed to register plugin: %v", err)
	}
}
