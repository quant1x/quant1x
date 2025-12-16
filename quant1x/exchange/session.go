package exchange

import (
	"math"
	"sync"
	"time"
)

// ======================================================================
// 状态掩码标志（bitmask flags）
// ======================================================================

const (
	MaskClosed      uint8 = 0x00 // 无任何状态, 收盘, 休市
	MaskActive      uint8 = 0x01 // 是否活跃（可用于处理订单）
	MaskTrading     uint8 = 0x02 // 正常连续竞价阶段
	MaskCallAuction uint8 = 0x04 // 集合竞价阶段
	MaskOrder       uint8 = 0x08 // 是否可委托
	MaskCancelable  uint8 = 0x10 // 是否允许撤单
	MaskOpening     uint8 = 0x20 // 开盘, 集合竞价, 09:15~09:25
	MaskClosing     uint8 = 0x40 // 收盘, 集合竞价, 14:57~15:00
	MaskHalt        uint8 = 0x80 // 暂停交易（市场活跃但不能撮合, 熔断或临时停牌）
)

// TimeStatus 时间状态类型
type TimeStatus uint8

// ======================================================================
// 时间状态枚举（使用掩码组合）
// ======================================================================

const (
	ExchangeClosing               TimeStatus = TimeStatus(MaskClosed)                                              // 当日收盘（默认状态，不可交易）
	ExchangePreMarket             TimeStatus = TimeStatus(MaskActive)                                              // 盘前（活跃但未开始交易）
	ExchangeSuspend               TimeStatus = TimeStatus(MaskHalt)                                                // 休市中（非活跃，不可交易）
	ExchangeContinuousTrading     TimeStatus = TimeStatus(MaskActive | MaskOrder | MaskTrading)                    // 连续竞价（上午/下午，可撤单）
	ExchangeTrading               TimeStatus = ExchangeContinuousTrading                                           // 连续竞价, 盘中交易别名
	ExchangeCallAuction           TimeStatus = TimeStatus(MaskActive | MaskOrder | MaskCallAuction)                // 集合竞价
	ExchangeCallAuctionOpening    TimeStatus = TimeStatus(ExchangeCallAuction | TimeStatus(MaskOpening))           // 早盘集合竞价
	ExchangeCallAuctionOpenPhase1 TimeStatus = TimeStatus(ExchangeCallAuctionOpening | TimeStatus(MaskCancelable)) // 9:15~9:20，开盘集合竞价，可撤单
	ExchangeCallAuctionOpenPhase2 TimeStatus = ExchangeCallAuctionOpening                                          // 9:20~9:25，开盘集合竞价，不可撤单
	ExchangeCallAuctionClosePhase TimeStatus = TimeStatus(ExchangeCallAuction | TimeStatus(MaskClosing))           // 14:57~15:00，收盘集合竞价，不可撤单
	ExchangeHaltTrading           TimeStatus = TimeStatus(MaskActive | MaskHalt)                                   // 市场活跃但暂停交易（如临时停牌、熔断等）
)

// ======================================================================
// 辅助判断函数
// ======================================================================

func IsMarketClosed(status TimeStatus) bool {
	return status == ExchangeClosing
}

func IsMarketSuspended(status TimeStatus) bool {
	return status == ExchangeSuspend
}

func IsTradingHalted(status TimeStatus) bool {
	return (status & TimeStatus(MaskHalt)) != 0
}

func IsMarketActive(status TimeStatus) bool {
	return (status & TimeStatus(MaskActive)) != 0
}

func IsInContinuousTrading(status TimeStatus) bool {
	return (status & TimeStatus(MaskTrading)) != 0
}

func IsInCallAuction(status TimeStatus) bool {
	return (status & TimeStatus(MaskCallAuction)) != 0
}

func IsCallAuctionOpenPhase(status TimeStatus) bool {
	return (status & (ExchangeCallAuction | TimeStatus(MaskOpening))) == (ExchangeCallAuction | TimeStatus(MaskOpening))
}

func IsCallAuctionClosePhase(status TimeStatus) bool {
	return (status & (ExchangeCallAuction | TimeStatus(MaskClosing))) == (ExchangeCallAuction | TimeStatus(MaskClosing))
}

func IsOrderCancelable(status TimeStatus) bool {
	return (status & TimeStatus(MaskCancelable)) != 0
}

func IsTradingDisabled(status TimeStatus) bool {
	return status == ExchangeClosing || status == ExchangeSuspend || (status&TimeStatus(MaskHalt)) != 0
}

// TimeRange 交易时段, 左闭右开区间
type TimeRange struct {
	Begin  Timestamp
	End    Timestamp
	Status TimeStatus
}

func NewTimeRange(begin, end Timestamp, status TimeStatus) TimeRange {
	return TimeRange{
		Begin:  begin.Floor(),
		End:    end.Ceil(),
		Status: status,
	}
}

// In 判断是否在本交易时段
func (tr *TimeRange) In(ts Timestamp) (TimeStatus, bool) {
	if tr.Begin.LessOrEqual(ts) && ts.Less(tr.End) {
		return tr.Status, true
	}
	return 0, false
}

func (tr *TimeRange) String() string {
	return tr.Begin.ToString(LayoutSession) + "~" + tr.End.ToString(LayoutSession)
}

// TradingSession 交易会话
type TradingSession struct {
	Sessions      []TimeRange
	EarliestStart Timestamp
	LatestEnd     Timestamp
}

func NewTradingSession(sessions ...TimeRange) *TradingSession {
	ts := &TradingSession{
		Sessions: sessions,
	}
	ts.updateTimeBounds()
	return ts
}

func (ts *TradingSession) updateTimeBounds() {
	if len(ts.Sessions) == 0 {
		ts.EarliestStart = NewTimestamp(math.MaxInt64)
		ts.LatestEnd = NewTimestamp(math.MinInt64)
		return
	}
	ts.EarliestStart = NewTimestamp(math.MaxInt64)
	ts.LatestEnd = NewTimestamp(math.MinInt64)
	for _, session := range ts.Sessions {
		if session.Begin.Less(ts.EarliestStart) {
			ts.EarliestStart = session.Begin
		}
		if session.End.Greater(ts.LatestEnd) {
			ts.LatestEnd = session.End
		}
	}
}

func (ts *TradingSession) AddSession(tr TimeRange) {
	ts.Sessions = append(ts.Sessions, tr)
	ts.updateTimeBounds()
}

// In 判断是否在任何交易时段内
func (ts *TradingSession) In(t Timestamp) TimeStatus {
	for _, session := range ts.Sessions {
		if status, ok := session.In(t); ok {
			return status
		}
	}
	// 全天交易开始前
	if t.Less(ts.EarliestStart) {
		return ExchangePreMarket
	}
	// 全天交易结束前, 则会休市
	if t.Less(ts.LatestEnd) {
		return ExchangeHaltTrading
	}
	// 不在任何交易时段内, 返回已收盘
	return ExchangeClosing
}

func (ts *TradingSession) IsTradingNotStarted(t Timestamp) bool {
	return t.Less(ts.EarliestStart)
}

func (ts *TradingSession) IsTradingEnded(t Timestamp) bool {
	return t.Greater(ts.LatestEnd)
}

var (
	tsTodaySession     *TradingSession
	tsTodaySessionOnce sync.Once
)

// InitSession 初始化当日的交易会话时段
func InitSession() *TradingSession {
	now := MidnightTimestamp()
	tr1 := NewTimeRange(now.Offset(9, 15, 0, 0), now.Offset(9, 20, 0, 0), ExchangeCallAuctionOpenPhase1)
	tr2 := NewTimeRange(now.Offset(9, 20, 0, 0), now.Offset(9, 25, 0, 0), ExchangeCallAuctionOpenPhase2)
	tr3 := NewTimeRange(now.Offset(9, 25, 0, 0), now.Offset(9, 29, 0, 0), ExchangeSuspend)
	tr4 := NewTimeRange(now.Offset(9, 30, 0, 0), now.Offset(11, 29, 0, 0), ExchangeTrading)
	tr5 := NewTimeRange(now.Offset(13, 0, 0, 0), now.Offset(14, 56, 0, 0), ExchangeTrading)
	tr6 := NewTimeRange(now.Offset(14, 57, 0, 0), now.Offset(15, 0, 0, 0), ExchangeCallAuctionClosePhase)
	return NewTradingSession(tr1, tr2, tr3, tr4, tr5, tr6)
}

func GetTodaySession() *TradingSession {
	tsTodaySessionOnce.Do(func() {
		tsTodaySession = InitSession()
	})
	return tsTodaySession
}

// RuntimeStatus 运行时状态机
type RuntimeStatus struct {
	BeforeLastTradeDay bool       // 最后交易日前
	IsHoliday          bool       // 是否节假日休市
	BeforeInitTime     bool       // 初始化时间前
	CacheAfterInitTime bool       // 缓存在初始化时间之后
	UpdateInRealTime   bool       // 是否可以实时更新
	Status             TimeStatus // 当前状态
}

// CheckTradingTimestamp 检查运行时交易状态
func CheckTradingTimestamp(lastModified *Timestamp) RuntimeStatus {
	rs := RuntimeStatus{
		Status: ExchangeClosing,
	}
	now := NowTimestamp()
	var ts Timestamp
	if lastModified != nil {
		ts = *lastModified
	} else {
		ts = now
	}

	// TODO: 需要实现 Calendar 模块的 LastTradingDay
	// lastDay := LastTradingDay(GetTodayInit())
	// 暂时使用 Today 作为 LastTradingDay 的占位符
	lastDay := GetTodayInit()

	// 1. timestamp before last trading day
	if ts.Less(lastDay) {
		rs.BeforeLastTradeDay = true
		return rs
	}

	// 2. if today != last_day => holiday
	today := now
	if !today.IsSameDate(lastDay) {
		rs.IsHoliday = true
		return rs
	}

	// 3. before init
	if ts.Less(GetTodayInit()) {
		rs.BeforeInitTime = true
		return rs
	}
	rs.Status = ExchangePreMarket

	rs.CacheAfterInitTime = true

	// 5. trading not started
	session := GetTodaySession()
	if session.IsTradingNotStarted(ts) {
		return rs
	}

	rs.UpdateInRealTime = true

	rs.Status = session.In(ts)
	if IsTradingDisabled(rs.Status) {
		rs.UpdateInRealTime = false
	}
	return rs
}

// CanInitialize 判断是否可以初始化数据（等价于 C++ 中 can_initialize）
func CanInitialize(lastModified *Timestamp) bool {
	rs := CheckTradingTimestamp(lastModified)
	if rs.BeforeLastTradeDay {
		return true
	}
	if rs.IsHoliday {
		return false
	}
	if rs.BeforeInitTime {
		return false
	}
	return !rs.CacheAfterInitTime
}

var (
	tsTodayInit     Timestamp
	tsTodayInitOnce sync.Once
)

func GetTodayInit() Timestamp {
	tsTodayInitOnce.Do(func() {
		now := Now()
		t := time.UnixMilli(int64(now))
		tsTodayInit = PreMarketTimestamp(t.Year(), int(t.Month()), t.Day())
	})
	return tsTodayInit
}

const LayoutSession = "15:04:05"
