package level1

import "fmt"

const (
	TickTransactionPerRequestMax = 1800
)

type TransactionDirection int64

const (
	TickTransactionDirectionBuy  = 0 // 买盘
	TickTransactionDirectionSell = 1 // 卖盘
	TickTransactionDirectionNone = 2 // 中性盘

	// 明确表示竞价时段

	TickTransactionAuctionClose = 8 // 收盘集合竞价
)

// TickTransaction mirrors the C++ TickTransaction structure.
type TickTransaction struct {
	Time      string  // 成交时间 HH:MM
	Price     float64 // 成交价格
	Vol       int64   // 成交量(股)
	Num       int64   // 成交笔数
	Amount    float64 // 成交金额
	Direction int64   // 买卖方向
}

func (t TickTransaction) String() string {
	return fmt.Sprintf("time: %s price: %v vol: %d num: %d amount: %v direction: %d", t.Time, t.Price, t.Vol, t.Num, t.Amount, t.Direction)
}

type TransactionReply struct {
	Count uint16
	List  []TickTransaction
}
