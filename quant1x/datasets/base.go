package datasets

import (
	"fmt"

	"gitee.com/quant1x/quant1x/quant1x/cache"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
)

// baseKind is the local offset for base data kinds (mirrors C++ baseKind)
const baseKind cache.Kind = 0

const (
	BaseXdxr                cache.Kind = cache.PluginMaskBaseData | (baseKind + 1)  // 基础数据-除权除息
	BaseRawDailyKLine       cache.Kind = cache.PluginMaskBaseData | (baseKind + 2)  // 基础数据-未复权K线
	BaseKLine               cache.Kind = cache.PluginMaskBaseData | (baseKind + 3)  // 基础数据-前复权K线
	BaseTransaction         cache.Kind = cache.PluginMaskBaseData | (baseKind + 4)  // 基础数据-历史成交
	BaseMinutes             cache.Kind = cache.PluginMaskBaseData | (baseKind + 5)  // 基础数据-分时数据
	BaseQuarterlyReports    cache.Kind = cache.PluginMaskBaseData | (baseKind + 6)  // 基础数据-季报
	BaseSafetyScore         cache.Kind = cache.PluginMaskBaseData | (baseKind + 7)  // 基础数据-安全分
	BaseWideKLine           cache.Kind = cache.PluginMaskBaseData | (baseKind + 8)  // 基础数据-宽表
	BasePerformanceForecast cache.Kind = cache.PluginMaskBaseData | (baseKind + 9)  // 基础数据-业绩预告
	BaseChipDistribution    cache.Kind = cache.PluginMaskBaseData | (baseKind + 10) // 基础数据-筹码分布
	BaseMinuteKLine         cache.Kind = cache.PluginMaskBaseData | (baseKind + 11) // 基础数据-分钟级别K线
)

// MarketFirstDate is the market first-listing date as a pre-market Timestamp
var MarketFirstDate exchange.Timestamp

func init() {
	// 与 C++ 严格保持一致：解析常量并取盘前时间。
	ts, err := exchange.NewTimestampFromString(exchange.MarketCnFirstListTime)
	if err != nil {
		panic(fmt.Sprintf("datasets: failed to parse MarketCnFirstListTime: %v", err))
	}
	MarketFirstDate = ts.PreMarketTime()
}
