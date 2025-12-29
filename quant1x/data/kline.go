package data

// MaxCachedDaysToDropOnIncrementalUpdate 是增量更新缓存清理的最大天数。
//
// 在执行增量更新前，从缓存数据中最多需要移除最近的若干个交易日数据。
// 该机制确保在 A 股除权除息日等场景下，当日数据能被正确覆盖。
// 由于 A 股的复权处理以交易日为单位，且同一天内可能多次更新数据，
// 因此需先删除缓存中已有的当日记录，再插入最新增量数据。
const MaxCachedDaysToDropOnIncrementalUpdate = 1

// KLine 表示一条 K 线数据
type KLine struct {
	Date            string  `name:"日期" csv:"date"`               // 日期 YYYY-MM-DD
	Open            float64 `name:"开盘价" csv:"open"`              // 开盘价
	Close           float64 `name:"收盘价" csv:"close"`             // 收盘价
	High            float64 `name:"最高价" csv:"high"`              // 最高价
	Low             float64 `name:"最低价" csv:"low"`               // 最低价
	Volume          float64 `name:"成交量(股)" csv:"volume"`         // 成交量(股)
	Amount          float64 `name:"成交额(元)" csv:"amount"`         // 成交额(元)
	Up              int     `name:"涨家数" csv:"up"`                // 涨家数
	Down            int     `name:"跌家数" csv:"down"`              // 跌家数
	Datetime        string  `name:"日期时间" csv:"datetime"`         // 日期时间 YYYY-MM-DD HH:MM:SS.mmm
	AdjustmentCount int     `name:"复权次数" csv:"adjustment_count"` // 复权次数
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
