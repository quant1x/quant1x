package data

import (
	"sort"
	"time"

	"github.com/quant1x/quant1x/quant1x/data/schema"
)

// CumulativeAdjustment 累计复权因子, 对应仿射变换: P' = M * P + A
type CumulativeAdjustment struct {
	M                    float64 // 乘性因子（Multiplier），处理比例调整（如送股）
	A                    float64 // 加性因子（Additive），处理平移调整（如分红）
	ShareAdjustmentRatio float64 // 股本调整比率，用于成交量复权（V' = V * (1 + ratio)）
	No                   int     // 本次复权调整的序号（从1开始），用于追踪应用顺序
}

// ApplyForwardAdjustmentForEvent 使用提供的除权除息事件对 K 线执行前复权处理。
//
//	eventStartDate 是用于过滤 IPO 早期事件的起始日期（格式 YYYY-MM-DD）。
func ApplyForwardAdjustmentForEvent(klines []schema.Bar, eventStartDate string, dividends []schema.XdxrInfo) {
	if len(klines) == 0 || len(dividends) == 0 {
		return
	}
	latestKLineDate := klines[len(klines)-1].Date
	// compute next day (approximate next trading day)
	d, err := time.Parse(LayoutTradeDate, latestKLineDate)
	if err != nil {
		return
	}
	// 使用近似下一日作为事件筛选截止日期（确保包含当日事件）
	cutoffDate := d.Add(24 * time.Hour).Format(LayoutTradeDate)

	// 筛选除权除息事件（Category == 1）且日期 <= cutoffDate
	eligibleXdxrEvents := make([]schema.XdxrInfo, 0, len(dividends))
	for _, v := range dividends {
		if v.Category == 1 && v.Date <= cutoffDate {
			eligibleXdxrEvents = append(eligibleXdxrEvents, v)
		}
	}
	// 按日期升序排序（从最早事件开始复权）
	sort.Slice(eligibleXdxrEvents, func(i, j int) bool { return eligibleXdxrEvents[i].Date < eligibleXdxrEvents[j].Date })
	filterStartDate := eventStartDate
	for _, info := range eligibleXdxrEvents {
		if info.Date <= filterStartDate {
			// skip events before or on the start date
			continue
		}
		adj := info.AdjustFactor()
		for i := range klines {
			if klines[i].Date >= info.Date {
				break
			}
			// 填充调整序号（No）
			adj.No = klines[i].AdjustmentCount + 1
			klines[i].Adjust(adj)
		}
	}
}
