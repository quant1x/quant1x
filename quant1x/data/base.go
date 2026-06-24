package data

const baseKind Kind = 0

const (
	BaseXdxr                Kind = PluginMaskBaseData | (baseKind + 1)  // 基础数据-除权除息
	BaseRawDailyKLine       Kind = PluginMaskBaseData | (baseKind + 2)  // 基础数据-未复权K线
	BaseKLine               Kind = PluginMaskBaseData | (baseKind + 3)  // 基础数据-前复权K线
	BaseTransaction         Kind = PluginMaskBaseData | (baseKind + 4)  // 基础数据-历史成交
	BaseMinutes             Kind = PluginMaskBaseData | (baseKind + 5)  // 基础数据-分时数据
	BaseQuarterlyReports    Kind = PluginMaskBaseData | (baseKind + 6)  // 基础数据-季报
	BaseSafetyScore         Kind = PluginMaskBaseData | (baseKind + 7)  // 基础数据-安全分
	BaseWideKLine           Kind = PluginMaskBaseData | (baseKind + 8)  // 基础数据-宽表
	BasePerformanceForecast Kind = PluginMaskBaseData | (baseKind + 9)  // 基础数据-业绩预告
	BaseChipDistribution    Kind = PluginMaskBaseData | (baseKind + 10) // 基础数据-筹码分布
	BaseMinuteKLine         Kind = PluginMaskBaseData | (baseKind + 11) // 基础数据-分钟级别K线
)
