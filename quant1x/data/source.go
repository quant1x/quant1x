package data

import (
	"gitee.com/quant1x/quant1x/quant1x/std"
)

type AdjustmentType string

const (
	AdjustNone     AdjustmentType = "none"     // 不复权
	AdjustForward  AdjustmentType = "forward"  // 前复权
	AdjustBackward AdjustmentType = "backward" // 后复权
)

// 数据源基础错误码
const (
	ErrDataSourceBase     = 10000                 // 数据源基础错误码
	ErrCodeNotImplemented = ErrDataSourceBase + 1 // 未实现
	ErrCodeNoData         = ErrDataSourceBase + 2 // 无数据
)

var (
	ErrNotImplemented = std.NewException(ErrCodeNotImplemented, "not implemented") // 未实现错误
	ErrNoData         = std.NewException(ErrCodeNoData, "no data")                 // 无数据错误
)

// DataSource 定义了获取 K 线数据的接口。
type DataSource interface {
	// GetF10 获取指定证券的 F10 信息
	GetF10(code string) (F10, error)
	// GetKLines 获取指定范围K线（最通用）
	GetKLines(code string, startDate, endDate, period string, adjust AdjustmentType) ([]KLine, error)

	// GetLatestKLines 获取最近N根K线（策略常用）
	GetLatestKLines(code string, date string, count int, period string, adjust AdjustmentType) ([]KLine, error)

	// GetTransactions 获取指定交易日的成交数据。
	GetTransactions(code string, date string) ([]Transaction, error)

	// GetTradeTicks 返回交易所推送的逐笔成交记录（每笔独立，Num=1）。
	// 数据来源于 Level-2 逐笔成交流。
	GetTradeTicks(code string, date string) ([]Transaction, error)

	// GetTradeDetails 返回交易所推送的分笔成交记录（原生3秒快照，Num≥1）。
	// 数据来源于 Level-2 成交明细（分笔）流，非人为聚合。
	GetTradeDetails(code string, date string) ([]Transaction, error)
}
