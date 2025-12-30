package data

import (
	"math"
	"sort"
	"time"
)

// CumulativeAdjustment 累计复权因子, 对应仿射变换: P' = M * P + A
type CumulativeAdjustment struct {
	M                    float64 // 乘性因子（Multiplier），处理比例调整（如送股）
	A                    float64 // 加性因子（Additive），处理平移调整（如分红）
	ShareAdjustmentRatio float64 // 股本调整比率，用于成交量复权（V' = V * (1 + ratio)）
	No                   int     // 本次复权调整的序号（从1开始），用于追踪应用顺序
}

// XdxrInfo 表示一条除权除息事件
type XdxrInfo struct {
	Date          string  `name:"日期" csv:"date"`                 // 除权除息日期 YYYY-MM-DD
	Category      int     `name:"类别" csv:"category"`             // 事件类别
	Name          string  `name:"名称" csv:"name"`                 // 事件名称
	FenHong       float64 `name:"分红金额" csv:"fen_hong"`           // 分红金额
	PeiGuJia      float64 `name:"配股价格" csv:"pei_gu_jia"`         // 配股价格
	SongZhuanGu   float64 `name:"送转股数" csv:"song_zhuan_gu"`      // 送转股数
	PeiGu         float64 `name:"配股数" csv:"pei_gu"`              // 配股数
	SuoGu         float64 `name:"缩股数" csv:"suo_gu"`              // 缩股数
	QianLiuTong   float64 `name:"除权前流通股本" csv:"qian_liu_tong"`   // 除权前流通股本
	HouLiuTong    float64 `name:"除权后流通股本" csv:"hou_liu_tong"`    // 除权后流通股本
	QianZongGuBen float64 `name:"除权前总股本" csv:"qian_zong_gu_ben"` // 除权前总股本
	HouZongGuBen  float64 `name:"除权后总股本" csv:"hou_zong_gu_ben"`  // 除权后总股本
	FenShu        float64 `name:"份数" csv:"fen_shu"`              // 份数
	XingQuanJia   float64 `name:"行权价格" csv:"xing_quan_jia"`      // 行权价格
}

// ComputeShareAdjustmentRatio 对应 C++ 中的 XdxrInfo::computeShareAdjustmentRatio
func (x *XdxrInfo) ComputeShareAdjustmentRatio() float64 {
	return (x.SongZhuanGu + x.PeiGu - x.SuoGu + x.FenShu) / 10.0
}

// ComputeMonetaryAdjustment 对应 C++ 中的 XdxrInfo::computeMonetaryAdjustment
func (x *XdxrInfo) ComputeMonetaryAdjustment() float64 {
	return (x.PeiGu*x.PeiGuJia - x.FenHong + x.FenShu*x.XingQuanJia) / 10.0
}

// AdjustFactor 计算并返回完整的复权调整参数，对应 C++ 中的 XdxrInfo::adjustFactor。
// 返回的 CumulativeAdjustment.No 字段为 0，应在应用时由调用方设置为实际调整序号。
func (x *XdxrInfo) AdjustFactor() CumulativeAdjustment {
	A := x.ComputeMonetaryAdjustment()
	B := x.ComputeShareAdjustmentRatio()

	var m, a float64
	if math.Abs(1.0+B) > 1e-10 {
		m = 1.0 / (1.0 + B)
		a = A * m
	} else {
		m = 1.0
		a = A
	}

	return CumulativeAdjustment{
		M:                    m,
		A:                    a,
		ShareAdjustmentRatio: B, // 即 B = (SongZhuanGu + PeiGu - SuoGu + FenShu) / 10.0
		No:                   0, // 由 ApplyForwardAdjustmentForEvent 填充
	}
}

// ApplyForwardAdjustmentForEvent 使用提供的除权除息事件对 K 线执行前复权处理。
//
//	eventStartDate 是用于过滤 IPO 早期事件的起始日期（格式 YYYY-MM-DD）。
func ApplyForwardAdjustmentForEvent(klines []KLine, eventStartDate string, dividends []XdxrInfo) {
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
	eligibleXdxrEvents := make([]XdxrInfo, 0, len(dividends))
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
