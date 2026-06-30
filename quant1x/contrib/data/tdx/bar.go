package tdx

import (
	"fmt"

	"github.com/quant1x/quant1x/quant1x/config"
	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/level1/std"
	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/encoding"
	logger "github.com/quant1x/quant1x/quant1x/log"
)

// tdxFetchRawSecurityBars 执行底层 level1 SecurityBarsContext 请求并返回原始响应列表(未转换).
func tdxFetchRawSecurityBars(securityCode data.InstrumentInfo, category std.BarFreq, start, count uint16) ([]std.SecurityBar, error) {
	conn, release, err := GetStdConnection()
	if err != nil {
		return nil, fmt.Errorf("level1 client acquire failed: %w", err)
	}
	if release != nil {
		defer release()
	}
	if conn == nil || conn.Conn() == nil {
		return nil, fmt.Errorf("nil connection from level1 client")
	}

	msg := std.NewSecurityBarsContext(securityCode, category, start, count)
	if err := tdxproto.TransactMessageSync(conn, msg); err != nil {
		return nil, fmt.Errorf("security bars request failed: %w", err)
	}
	return msg.List, nil
}

// Update 对应 C++ DataKLine::Update 的行为: 读取本地缓存, 确定时间窗口, 分页拉取 level1 数据,
// 反转与合并结果, 在适当时机应用前复权, 并写回缓存文件.
func tdxUpdateBar(symbol data.InstrumentInfo, _date data.Timestamp) {
	_ = _date
	if symbol.Type == data.SecurityTypeUnknown {
		logger.Debugf("[DataKLine] unknown security type for code %s", symbol.Symbol())
		return
	}

	// 1. 确定缓存文件并读取本地缓存
	cacheFilename := config.GetBarFilename(symbol.Symbol(), true)
	var cacheBars []data.KLine
	err := encoding.CsvToSlices(cacheFilename, &cacheBars)
	if err != nil {
		logger.Debugf("[DataKLine] load cache failed for %s: %v", symbol.Symbol(), err)
		// 继续更新
	}
	barsLength := len(cacheBars)
	barsOffsetDays := data.MaxCachedDaysToDropOnIncrementalUpdate
	adjustTimes := 0

	// 默认起始日期(使用 datasets.MarketFirstDate, 与 C++ 的 market_first_date 等价)
	currentStartDate := data.GetFirstMarketDate(symbol.Exchange)
	if barsLength > 0 {
		if barsOffsetDays > barsLength {
			barsOffsetDays = barsLength
		}
		// use the cached bar at offset as the start date
		idx := barsLength - barsOffsetDays
		if idx >= 0 && idx < barsLength {
			if ts, err := data.ParseTimestamp(cacheBars[idx].Date); err == nil {
				currentStartDate = ts.PreMarketTime()
			}
		}
		// 查找第一个未复权的 K 线记录, 以决定是否需要复权
		firstNotAdjustedIdx := -1
		for i := barsLength - 1; i >= 0; i-- {
			if cacheBars[i].AdjustmentCount == 0 {
				firstNotAdjustedIdx = i
			} else {
				break
			}
		}
		if firstNotAdjustedIdx < 0 {
			firstNotAdjustedIdx = barsLength - barsOffsetDays
		}
		firstNotAdjustedBar := cacheBars[firstNotAdjustedIdx]
		adjustTimes = firstNotAdjustedBar.AdjustmentCount
		currentStartDate, _ = data.ParseTimestamp(firstNotAdjustedBar.Date)
		logger.Debugf("[DataKLine] [%s]: cached bars=%d, adjustTimes=%d, start from %s", symbol.Symbol(), barsLength, adjustTimes, currentStartDate.OnlyDate())
	}

	// 2. 确定结束日期
	// 使用当前时间的盘前时间作为结束日期, 并生成每日序列作为拉取区间(C++ 使用交易日历的 date_range)
	currentEndDate := data.NowTimestamp().PreMarketTime()
	startT := currentStartDate
	endT := currentEndDate
	var ts []data.Timestamp
	for t := startT; !t.Greater(endT); t = t.Offset(24, 0, 0, 0) {
		ts = append(ts, t)
	}
	total := len(ts)
	if total == 0 {
		logger.Debugf("[DataKLine] empty date range for %s", symbol.Symbol())
		return
	}

	// 3. 分页从 level1 拉取数据
	step := uint16(std.SecurityBarsMax)
	var hs [][]std.SecurityBar
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
		reply, err := tdxFetchRawSecurityBars(symbol, std.FreqDaily, start, count)
		if err != nil {
			logger.Debugf("[DataKLine] fetch error for %s start=%d count=%d: %v", symbol.Symbol(), start, count, err)
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

	// 4. 反转分段数据(服务器按最新->最旧返回)
	for i, j := 0, len(hs)-1; i < j; i, j = i+1, j-1 {
		hs[i], hs[j] = hs[j], hs[i]
	}

	// 5. 转换为增量 K 线并调整单位(成交量单位从手转为股)
	incrementalBars := make([]data.KLine, 0, elementCount)
	for _, vec := range hs {
		for _, row := range vec {
			dts := data.PreMarketTimestamp(row.Year, row.Month, row.Day)
			if dts.Less(startT) || dts.Greater(endT) {
				continue
			}
			bx := data.KLine{
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
			incrementalBars = append(incrementalBars, bx)
		}
	}

	// 6. 判断是否需要复权处理
	isFreshFetchRequireAdjustment := adjustTimes == 1
	dividends, _ := tdxGetXdxrList(symbol)
	if isFreshFetchRequireAdjustment {
		data.ApplyForwardAdjustmentForEvent(incrementalBars, currentStartDate.OnlyDate(), dividends)
	}

	// 7. 合并本地缓存与增量数据
	var bars []data.KLine
	if barsLength > barsOffsetDays {
		bars = append(bars, cacheBars[:barsLength-barsOffsetDays]...)
	}
	if len(bars) == 0 {
		bars = incrementalBars
	} else {
		bars = append(bars, incrementalBars...)
	}

	// 8. 若不是仅更新最新记录, 则对全量数据做前复权处理
	if !isFreshFetchRequireAdjustment {
		data.ApplyForwardAdjustmentForEvent(bars, currentStartDate.OnlyDate(), dividends)
	}

	// 9. 保存缓存(确保父目录存在)
	encoding.SlicesToCsv(cacheFilename, bars, true)
}

// DataKLine implements the cache adapter style updater similar to C++ DataKLine
type DataKLine struct{}

// 实现 DataAdapter 的 Schema 方法
func (d *DataKLine) Kind() data.Kind { return data.BaseKLine }
func (d *DataKLine) Owner() string   { return data.DefaultDataProvider }
func (d *DataKLine) Key() string     { return "day" }
func (d *DataKLine) Name() string    { return "日K线" }
func (d *DataKLine) Usage() string   { return "日K线" }

func (d *DataKLine) Print(code data.InstrumentInfo, dates ...data.Timestamp) {
	// no-op, matches C++ stub
	_ = code
	_ = dates
}

// Update mirrors the C++ DataKLine::Update behavior: read local cache, determine
// date window, page-fetch from level1, reverse/merge results, apply forward
// adjustments when appropriate, and save back the cache file.
func (d *DataKLine) Update(code data.InstrumentInfo, _date data.Timestamp) {
	tdxUpdateBar(code, _date)
}

func init() {
	// register DataKLine plugin
	if err := data.Register(&DataKLine{}); err != nil {
		logger.Errorf("[DataKLine] failed to register plugin: %v", err)
	}
}
