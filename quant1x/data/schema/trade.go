package schema

import "fmt"

// 已知的买卖方向常量（来源于交易所或行情商文档）
const (
	DirectionBuy     int64 = 0 // 主动买入（吃卖盘）
	DirectionSell    int64 = 1 // 主动卖出（吃买盘）
	DirectionNeutral int64 = 2 // 中性盘（如集合竞价）
	DirectionBlock   int64 = 8 // 或 DirectionSpecial
)

// Transaction 表示交易所原始成交记录，支持两种模式：
//   - 逐笔模式：每条记录为单笔成交，Num = 1
//   - 聚合模式（如3秒快照）：每条记录为时间窗口内多笔成交的聚合，Num ≥ 1
//
// 注意：Time 为当日时间，格式 HH:MM（部分数据源可能为 HH:MM:SS，但本系统统一视为 HH:MM）
type Transaction struct {
	Time      string  `name:"时间" csv:"time"`        // 成交时间 HH:MM
	Price     float64 `name:"价格" csv:"price"`       // 成交价格
	Volume    int64   `name:"成交量" csv:"volume"`     // 成交量(股)
	Num       int64   `name:"成交笔数" csv:"num"`       // 成交笔数
	Amount    float64 `name:"成交金额" csv:"amount"`    // 成交金额
	Direction int64   `name:"买卖方向" csv:"direction"` // 买卖方向
}

// DirectionString 返回交易方向的字符串表示。
//
// BuyOrSell 是交易所返回的原始买卖方向编码，常见值包括：
//
//	0 = 主动买入（吃卖盘）
//	1 = 主动卖出（吃买盘）
//	2 = 中性盘（如集合竞价）
//	8 = 特殊交易（如大宗、盘后，具体含义依交易所而定）
//
// 其他值可能出现，应视为扩展类型，避免硬编码判断。
func (t Transaction) DirectionString() string {
	switch t.Direction {
	case DirectionBuy:
		return "Buy"
	case DirectionSell:
		return "Sell"
	case DirectionNeutral:
		return "Neutral"
	case DirectionBlock:
		return "BlockTrade" // 或 "AfterHours"
	default:
		return fmt.Sprintf("Unknown(%d)", t.Direction)
	}
}

// for Stringer interface
func (t Transaction) String() string {
	return fmt.Sprintf("time: %s price: %v volume: %d num: %d amount: %v direction: %d", t.Time, t.Price, t.Volume, t.Num, t.Amount, t.Direction)
}
