package provider

import (
	"fmt"

	"gitee.com/quant1x/quant1x/quant1x/config"
	"gitee.com/quant1x/quant1x/quant1x/data"
	"gitee.com/quant1x/quant1x/quant1x/encoding"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/level1"
	"gitee.com/quant1x/quant1x/quant1x/logger"
)

// tdxFetchRawSecurityBars 执行底层 level1 SecurityBars 请求并返回原始响应列表（未转换）。
func tdxFetchRawSecurityBars(securityCode exchange.SecurityCode, category level1.KLineType, start, count uint16) ([]level1.SecurityBar, error) {
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
func tdxUpdateKLine(securityCode exchange.SecurityCode, _date exchange.Timestamp) {
	_ = _date
	if securityCode.Type == exchange.SecurityUnknown {
		logger.Debugf("[DataKLine] unknown security type for code %s", securityCode.String())
		return
	}

	// 1. 确定缓存文件并读取本地缓存
	cacheFilename := config.GetKlineFilename(securityCode.String(), true)
	var cacheKLines []data.KLine
	err := encoding.CsvToSlices(cacheFilename, &cacheKLines)
	if err != nil {
		logger.Debugf("[DataKLine] load cache failed for %s: %v", securityCode.String(), err)
		// 继续更新
	}
	klinesLength := len(cacheKLines)
	klinesOffsetDays := data.MaxCachedDaysToDropOnIncrementalUpdate
	adjustTimes := 0

	// 默认起始日期（使用 datasets.MarketFirstDate，与 C++ 的 market_first_date 等价）
	currentStartDate := exchange.GetFirstMarketDate(securityCode.Exchange)
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
		// 查找第一个未复权的 K 线记录，以决定是否需要复权
		firstNotAdjustedIdx := -1
		for i := klinesLength - 1; i >= 0; i-- {
			if cacheKLines[i].AdjustmentCount == 0 {
				firstNotAdjustedIdx = i
			} else {
				break
			}
		}
		if firstNotAdjustedIdx < 0 {
			firstNotAdjustedIdx = klinesLength - klinesOffsetDays
		}
		firstNotAdjustedBar := cacheKLines[firstNotAdjustedIdx]
		adjustTimes = firstNotAdjustedBar.AdjustmentCount
		currentStartDate, _ = exchange.ParseTimestamp(firstNotAdjustedBar.Date)
		logger.Debugf("[DataKLine] [%s]: cached klines=%d, adjustTimes=%d, start from %s", securityCode.String(), klinesLength, adjustTimes, currentStartDate.OnlyDate())
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
		logger.Debugf("[DataKLine] empty date range for %s", securityCode.String())
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
		reply, err := tdxFetchRawSecurityBars(securityCode, level1.KLineDaily, start, count)
		if err != nil {
			logger.Debugf("[DataKLine] fetch error for %s start=%d count=%d: %v", securityCode.String(), start, count, err)
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
	incremental := make([]data.KLine, 0, elementCount)
	for _, vec := range hs {
		for _, row := range vec {
			dts := exchange.PreMarketTimestamp(row.Year, row.Month, row.Day)
			if dts.Less(startT) || dts.Greater(endT) {
				continue
			}
			kx := data.KLine{
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
	dividends, _ := tdxGetXdxrList(securityCode)
	if isFreshFetchRequireAdjustment {
		data.ApplyForwardAdjustmentForEvent(incremental, currentStartDate.OnlyDate(), dividends)
	}

	// 7. 合并本地缓存与增量数据
	var klines []data.KLine
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
		data.ApplyForwardAdjustmentForEvent(klines, currentStartDate.OnlyDate(), dividends)
	}

	// 9. 保存缓存（确保父目录存在）
	encoding.SlicesToCsv(cacheFilename, klines, true)
}

// DataKLine implements the cache adapter style updater similar to C++ DataKLine
type DataKLine struct{}

// 实现 DataAdapter 的 Schema 方法
func (d *DataKLine) Kind() data.Kind { return data.BaseKLine }
func (d *DataKLine) Owner() string   { return data.DefaultDataProvider }
func (d *DataKLine) Key() string     { return "day" }
func (d *DataKLine) Name() string    { return "日K线" }
func (d *DataKLine) Usage() string   { return "日K线" }

func (d *DataKLine) Print(code exchange.SecurityCode, dates ...exchange.Timestamp) {
	// no-op, matches C++ stub
	_ = code
	_ = dates
}

// Update mirrors the C++ DataKLine::Update behavior: read local cache, determine
// date window, page-fetch from level1, reverse/merge results, apply forward
// adjustments when appropriate, and save back the cache file.
func (d *DataKLine) Update(code exchange.SecurityCode, _date exchange.Timestamp) {
	tdxUpdateKLine(code, _date)
}

func init() {
	// register DataKLine plugin
	if err := data.Register(&DataKLine{}); err != nil {
		logger.Errorf("[DataKLine] failed to register plugin: %v", err)
	}
}
