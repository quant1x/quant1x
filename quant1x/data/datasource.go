package data

import (
	_ "unsafe" // for go:linkname

	"github.com/quant1x/quant1x/quant1x/data/schema"
	"github.com/quant1x/quant1x/quant1x/std"
)

type AdjustmentType string

const (
	AdjustNone     AdjustmentType = "none"     // 不复权
	AdjustForward  AdjustmentType = "forward"  // 前复权
	AdjustBackward AdjustmentType = "backward" // 后复权
)

// 数据源基础错误码
const (
	ErrDataSourceBase           = 10000                 // 数据源基础错误码
	ErrDataSourceNotImplemented = ErrDataSourceBase + 1 // 未实现
	ErrDataSourceNoData         = ErrDataSourceBase + 2 // 无数据, 比如没有交易日数据
	ErrDataSourceEOF            = ErrDataSourceBase + 3 // 数据读取到末尾
)

var (
	ErrNotImplemented = std.NewException(ErrDataSourceNotImplemented, "not implemented") // 未实现错误
	ErrNoData         = std.NewException(ErrDataSourceNoData, "no data")                 // 无数据错误
	ErrDataEOF        = std.NewException(ErrDataSourceEOF, "data eof")                   // 数据读取到末尾错误
)

// DataSource 数据源接口
//
// instrument 格式为:
//   - 1. <exchange_code>.<ticker>, 如 "NASDAQ.AAPL", 美股市场全部采用此格式
//   - 2. <exchange_code><ticker>, 如 "SH600000", A股市场全部采用此格式
//   - 3. <ticker>.<exchange_code>, 如 "600000.SH", A股, 港股和美股市场均支持此格式
type DataSource interface {
	// GetF10 获取指定证券的 F10 信息
	GetF10(instrument string) (F10, error)
	// GetKLines 获取指定范围K线数据
	//
	//  instrument 证券代码, 支持多种格式, 详见 DataSource 接口说明
	//  startDate 和 endDate 格式为 "YYYY-MM-DD"
	//  frequency 格式为 "<n>min", "1d", "1w", "1m" 等
	//  adjust 参数可选, 默认为前复权
	GetKLines(instrument string, startDate, endDate string, frequency string, adjust ...AdjustmentType) ([]schema.Bar, error)

	// GetTransactions 获取指定交易日的成交数据.
	GetTransactions(instrument string, date string) ([]schema.Transaction, error)

	// GetTradeTicks 返回交易所推送的逐笔成交记录(每笔独立, Num=1).
	// 数据来源于 Level-2 逐笔成交流.
	GetTradeTicks(instrument string, date string) ([]schema.Transaction, error)

	// GetTradeDetails 返回交易所推送的分笔成交记录(原生3秒快照, Num≥1).
	// 数据来源于 Level-2 成交明细(分笔)流, 非人为聚合.
	GetTradeDetails(instrument string, date string) ([]schema.Transaction, error)
}

// DataHandler returns a DataSource instance for handling data operations.
//
//go:linkname DataHandler
func DataHandler() DataSource
