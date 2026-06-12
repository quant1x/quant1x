// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

package meta

import (
	"fmt"
	"regexp"
	"time"

	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/runtime"
	"github.com/quant1x/quant1x/quant1x/std"
)

// TODO: https://www.tradinghours.com/markets

// ==========================================
// 1. 权限位掩码 (全属性统一)
// ==========================================

// Permission 全球统一交易状态位掩码
// 所有状态信息用一个整数表示
// 位分配:
// - Bit 0-5: 订单操作权限
// - Bit 6-7: 状态性质 (临时/异常)
// - Bit 8-15: 预留扩展
type Permission uint8

const (
	// ========== 订单操作权限 (Bit 0-3) ==========
	PermissionNone Permission = 0 // 0b00000000

	PermissionCancel Permission = 1 << 0 // 0b00000001 - 允许撤单
	PermissionModify Permission = 1 << 1 // 0b00000010 - 允许改单
	PermissionMarket Permission = 1 << 2 // 0b00000100 - 允许市价单
	PermissionLimit  Permission = 1 << 3 // 0b00001000 - 允许限价单

	// ========== 撮合机制 (Bit 4) ==========
	PermissionMatching Permission = 1 << 4 // 0b00010000 - 匹配中

	// ========== 成交机制 (Bit 5) ==========
	PermissionFill Permission = 1 << 5 // 0b00100000 - 会产生成交记录

	// ========== 统计标志 (Bit 6) ==========
	PermissionOpen Permission = 1 << 6 // 0b01000000 - 计入交易分钟数

	// ========== 状态性质 (Bit 7) ==========
	PermissionIsTemporary Permission = 1 << 7 // 0b10000000 - 临时状态 (可自动恢复)

	// ========== 常用组合 ==========
	PermissionMatchingTransaction Permission = PermissionMatching | PermissionFill // 撮合成交, 正在撮合中, 会产生成交记录

	// 连续交易：市价 + 限价 + 可成交 + 撤单 + 改单 + 计入分钟数
	PermissionContinuousTrading Permission = PermissionMarket | PermissionLimit | PermissionCancel | PermissionModify | PermissionOpen | PermissionMatchingTransaction

	PermissionInitializing Permission = PermissionIsTemporary // 初始化阶段

	PermissionPreMarket  Permission = PermissionIsTemporary | PermissionCancel | PermissionLimit // 盘前, 允许下单、撤单, 但不允许市价单
	PermissionAfterHours Permission = PermissionIsTemporary | PermissionCancel | PermissionLimit // 盘后, 允许下单、撤单, 但不允许市价单

	// 早盘集合竞价 = POS (Pre-Opening Session)
	// 收盘竞价时段 = CAS Closing Auction Session)

	PermissionCallAuction Permission = PermissionLimit | PermissionMatching | PermissionIsTemporary // 集合竞价, 仅限价单, 临时状态 (可自动恢复)

	PermissionCallAuctionPre   Permission = PermissionCallAuction | PermissionCancel // 集合竞价, 可撤单阶段
	PermissionCallAuctionOrder Permission = PermissionCallAuction                    // 集合竞价, 不可撤单阶段
	PermissionCallAuctionFill  Permission = PermissionCallAuction | PermissionFill   // 集合竞价, 随机对盘阶段

	PermissionAcceptOrderOnly Permission = PermissionLimit // 只挂单不成交 (午间休市) - 无 MATCHING

	PermissionReadOnly Permission = PermissionCancel // 只读状态 (停牌)

	PermissionClosed Permission = PermissionNone // 完全关闭

	PermissionEmergencyHalt Permission = PermissionOpen // 紧急停牌 (市场活跃但不能撮合, 只有 OPEN 位)

	PermissionLunchBreak Permission = PermissionAcceptOrderOnly | PermissionIsTemporary // 交易日休息时段 (允许下单、撤单, 但不允许市价单)
)

// CanMatch 是否允许成交 (连续或集合竞价)
func (p Permission) CanMatch() bool {
	return p&PermissionMatching != 0
}

// CanCancel 是否允许撤单
func (p Permission) CanCancel() bool {
	return p&PermissionCancel != 0
}

// CanModify 是否允许改单
func (p Permission) CanModify() bool {
	return p&PermissionModify != 0
}

// CanMarketOrder 是否允许市价单
func (p Permission) CanMarketOrder() bool {
	return p&PermissionMarket != 0
}

// CanLimitOrder 是否允许限价单
func (p Permission) CanLimitOrder() bool {
	return p&PermissionLimit != 0
}

// IsSuspended 是否暂停交易 (不允许撮合)
func (p Permission) IsSuspended() bool {
	return !p.CanMatch()
}

// IsContinuousTrading 是否计入交易分钟数
func (p Permission) IsContinuousTrading() bool {
	return p&PermissionOpen != 0
}

// ======================================================================
// 时间状态枚举（使用掩码组合）
// ======================================================================

// TimeStatus 全球统一交易时间状态枚举, 使用掩码组合表示不同状态
type TimeStatus uint8

const (
	TimeStatusOpen              TimeStatus = TimeStatus(PermissionOpen)              // 开盘
	TimeStatusClosed            TimeStatus = TimeStatus(PermissionClosed)            // 当日收盘（默认状态，不可交易）
	TimeStatusPreMarket         TimeStatus = TimeStatus(PermissionPreMarket)         // 盘前（活跃但未开始交易）
	TimeStatusAfterHours        TimeStatus = TimeStatus(PermissionAfterHours)        // 盘后（活跃但已结束交易）
	TimeStatusSuspend           TimeStatus = TimeStatus(PermissionLunchBreak)        // 休市中(非活跃，不可交易)
	TimeStatusContinuousTrading TimeStatus = TimeStatus(PermissionContinuousTrading) // 连续竞价(上午/下午，可撤单)
	TimeStatusTrading           TimeStatus = TimeStatusContinuousTrading             // 连续竞价, 盘中交易别名
	TimeStatusCallAuction       TimeStatus = TimeStatus(PermissionCallAuction)       // 集合竞价(开盘/收盘)

	// 早盘集合竞价 = POS (Pre-Opening Session)
	// 收盘竞价时段 = CAS Closing Auction Session)
	TimeStatusAuctionOrderInputPeriod     TimeStatus = TimeStatusCallAuction | TimeStatus(PermissionCancel) // 集合竞价, 订单输入 阶段, 可撤单
	TimeStatusAuctionNoCancellationPeriod TimeStatus = TimeStatusCallAuction                                // 集合竞价, 不可撤销 阶段
	TimeStatusAuctionMatchingFillPeriod   TimeStatus = TimeStatusCallAuction | TimeStatus(PermissionFill)   // 集合竞价, 竞价撮合/随机对盘 阶段

	TimeStatusAuctionMatchingToOpening TimeStatus = TimeStatusCallAuction | TimeStatus(PermissionFill) // 集合竞价开盘 阶段
	TimeStatusAuctionMatchingToClosing TimeStatus = TimeStatusCallAuction | TimeStatus(PermissionFill) // 集合竞价收盘 阶段

	TimeStatusExchangeHaltTrading TimeStatus = TimeStatusOpen // 市场活跃但暂停交易(如临时停牌、熔断等)
)

// IsMarketActive 市场是否活跃 (允许下单或撤单)
func (ts TimeStatus) IsMarketActive() bool {
	return ts.HasRealtimeData()
}

// IsOpen 市场是否开盘
func (ts TimeStatus) IsOpen() bool {
	return (ts & TimeStatusOpen) == TimeStatusOpen
}

// IsContinuousTrading 是否在连续竞价阶段 (计入交易分钟数)
func (ts TimeStatus) IsContinuousTrading() bool {
	return (ts & TimeStatusContinuousTrading) == TimeStatusContinuousTrading
}

// IsTradingDisabled 是否禁止交易 (不允许下单或成交)
func (ts TimeStatus) IsTradingDisabled() bool {
	return (ts & TimeStatus(PermissionMatching)) == 0
}

// HasRealtimeData 是否有实时数据
func (ts TimeStatus) HasRealtimeData() bool {
	return ts&TimeStatus(PermissionMatching) != 0
}

// ======================================================================
// TimeRange 时间范围
// ======================================================================

// TimeRange 时间范围, 用~或-间隔HH-MM-SS
type TimeRange struct {
	Begin  Timestamp
	End    Timestamp
	Status TimeStatus
	Reg    Region
}

// NewTimeRange 创建新的 TimeRange
func NewTimeRange(timeRange string, status TimeStatus, reg Region) (*TimeRange, error) {
	tr := &TimeRange{
		Begin:  ZeroTimestamp(),
		End:    ZeroTimestamp(),
		Status: status,
		Reg:    reg,
	}

	zoneOffsetHours := std.GetTimezoneOffsetStandard(reg.Timezone(), time.Local.String()) * -1

	timeRange = regexp.MustCompile(`\s+`).ReplaceAllString(timeRange, "")

	// 支持 ~ 或 - 分隔
	re := regexp.MustCompile(`[~-]`)
	parts := re.Split(timeRange, -1)
	if len(parts) != 2 {
		return nil, fmt.Errorf("非法的时间格式: %s", timeRange)
	}

	// 时间排序
	beginStr := parts[0]
	endStr := parts[1]

	beginTS, err := ParseTimeOnly(beginStr)
	if err != nil {
		return nil, fmt.Errorf("解析开始时间失败: %w", err)
	}
	endTS, err := ParseTimeOnly(endStr)
	if err != nil {
		return nil, fmt.Errorf("解析结束时间失败: %w", err)
	}

	tr.Begin = beginTS.Offset(zoneOffsetHours, 0, 0, 0)
	tr.End = endTS.Offset(zoneOffsetHours, 0, 0, 0)

	if tr.Begin.Greater(tr.End) {
		tr.Begin, tr.End = tr.End, tr.Begin
	}

	return tr, nil
}

// InRange 是否在本交易时段
func (tr *TimeRange) InRange(timestamp interface{}) (TimeStatus, bool) {
	var ts Timestamp

	switch v := timestamp.(type) {
	case string:
		if v == "" {
			now := time.Now()
			ts, _ = ParseTimeOnly(now.Format("15:04:05"))
		} else {
			var err error
			ts, err = ParseTimeOnly(v)
			if err != nil {
				return 0, false
			}
		}
	case Timestamp:
		ts = v
	default:
		ts = NowTimestamp()
	}

	// 左闭右开
	if tr.Begin.LessOrEqual(ts) && ts.Less(tr.End) {
		return tr.Status, true
	}
	return 0, false
}

// IsTrading 是否连续竞价交易中
func (tr *TimeRange) IsTrading(timestamp interface{}) bool {
	status, ok := tr.InRange(timestamp)
	if !ok {
		return false
	}
	return (status & TimeStatusTrading) == TimeStatusTrading
}

// IsValid 时段是否有效
func (tr *TimeRange) IsValid() bool {
	return !tr.Begin.IsEmpty() && !tr.End.IsEmpty()
}

// IsSessionPre 是否盘前
func (tr *TimeRange) IsSessionPre(timestamp interface{}) bool {
	var ts Timestamp

	switch v := timestamp.(type) {
	case string:
		if v == "" {
			now := time.Now()
			ts, _ = ParseTimeOnly(now.Format("15:04:05"))
		} else {
			var err error
			ts, err = ParseTimeOnly(v)
			if err != nil {
				return false
			}
		}
	case Timestamp:
		ts = v
	default:
		ts = NowTimestamp()
	}

	return ts.Less(tr.Begin)
}

// IsSessionReg 是否盘中
func (tr *TimeRange) IsSessionReg(timestamp interface{}) bool {
	return tr.IsTrading(timestamp)
}

// IsSessionPost 是否盘后
func (tr *TimeRange) IsSessionPost(timestamp interface{}) bool {
	var ts Timestamp

	switch v := timestamp.(type) {
	case string:
		if v == "" {
			now := time.Now()
			ts, _ = ParseTimeOnly(now.Format("15:04:05"))
		} else {
			var err error
			ts, err = ParseTimeOnly(v)
			if err != nil {
				return false
			}
		}
	case Timestamp:
		ts = v
	default:
		ts = NowTimestamp()
	}

	return ts.GreaterOrEqual(tr.End)
}

// GetDurationMinutes 计算时段总时长 (分钟)
func (tr *TimeRange) GetDurationMinutes() int {
	startMinutes := tr.Begin.Value() / 60000
	endMinutes := tr.End.Value() / 60000

	if endMinutes > startMinutes {
		return int(endMinutes - startMinutes)
	}
	return (24*60 - int(startMinutes)) + int(endMinutes)
}

// GetElapsedMinutes 时段已经开始多少分钟
func (tr *TimeRange) GetElapsedMinutes(currentTime Timestamp) int {
	current := min(currentTime.Value(), tr.End.Value())
	start := min(tr.Begin.Value(), currentTime.Value())

	currentMinutes := current / 60000
	startMinutes := start / 60000

	if currentMinutes >= startMinutes {
		return int(currentMinutes - startMinutes)
	}
	return 0
}

// ======================================================================
// TradingSession 交易时段
// ======================================================================

// TradingSession 交易时段
type TradingSession struct {
	Sessions      []*TimeRange
	EarliestStart Timestamp
	LatestEnd     Timestamp
	ClosingTime   Timestamp
}

// NewTradingSession 创建新的 TradingSession
func NewTradingSession(ranges ...*TimeRange) *TradingSession {
	ts := &TradingSession{
		Sessions:      ranges,
		EarliestStart: ZeroTimestamp(),
		LatestEnd:     ZeroTimestamp(),
		ClosingTime:   ZeroTimestamp(),
	}
	ts.updateTimeBounds()
	return ts
}

// NewTradingSessionFromString 从字符串创建 TradingSession
func NewTradingSessionFromString(timeRangeStr string) (*TradingSession, error) {
	re := regexp.MustCompile(`,\s*`)
	parts := re.Split(timeRangeStr, -1)

	var sessions []*TimeRange
	for _, v := range parts {
		v = regexp.MustCompile(`\s+`).ReplaceAllString(v, "")
		tr, err := NewTimeRange(v, TimeStatusTrading, RegionCN)
		if err != nil {
			return nil, err
		}
		sessions = append(sessions, tr)
	}

	return NewTradingSession(sessions...), nil
}

// updateTimeBounds 更新交易时段的时间边界
func (ts *TradingSession) updateTimeBounds() {
	if len(ts.Sessions) == 0 {
		earliest, _ := ParseTimeOnly("23:59:59")
		latest, _ := ParseTimeOnly("00:00:00")
		closing, _ := ParseTimeOnly("00:00:00")
		ts.EarliestStart = earliest
		ts.LatestEnd = latest
		ts.ClosingTime = closing
		return
	}

	earliest, _ := ParseTimeOnly("23:59:59")
	latest, _ := ParseTimeOnly("00:00:00")
	closing, _ := ParseTimeOnly("00:00:00")
	ts.EarliestStart = earliest
	ts.LatestEnd = latest
	ts.ClosingTime = closing

	for _, session := range ts.Sessions {
		if session.Begin.Less(ts.EarliestStart) {
			ts.EarliestStart = session.Begin
		}
		if session.End.Greater(ts.LatestEnd) {
			ts.LatestEnd = session.End
			if session.Status.IsOpen() {
				ts.ClosingTime = session.End
			}
		}
	}
}

// AddSession 添加交易时段
func (ts *TradingSession) AddSession(range_ *TimeRange) {
	ts.Sessions = append(ts.Sessions, range_)
	ts.updateTimeBounds()
}

// CheckStatus 判断当前时间的状态
func (ts *TradingSession) CheckStatus(timestamp interface{}) TimeStatus {
	var tsTime Timestamp

	switch v := timestamp.(type) {
	case string:
		if v == "" {
			now := time.Now()
			tsTime, _ = ParseTimeOnly(now.Format("15:04:05"))
		} else {
			var err error
			tsTime, err = ParseTimeOnly(v)
			if err != nil {
				return TimeStatusClosed
			}
		}
	case Timestamp:
		tsTime = v
	default:
		tsTime = NowTimestamp()
	}

	for _, session := range ts.Sessions {
		if status, ok := session.InRange(tsTime); ok {
			return status
		}
	}

	// 不在任何交易时段内, 进一步判断是盘前、盘后还是休市

	// 全天交易开始前
	if tsTime.Less(ts.EarliestStart) {
		return TimeStatusPreMarket
	}

	// 全天交易结束前, 则会休市 (例如中午休市)
	if tsTime.Less(ts.LatestEnd) {
		return TimeStatusExchangeHaltTrading
	}

	// 不在任何交易时段内, 返回已收盘
	return TimeStatusClosed
}

// IsTrading 是否交易中
func (ts *TradingSession) IsTrading(timestamp interface{}) bool {
	status := ts.CheckStatus(timestamp)
	return (status & TimeStatusTrading) == TimeStatusTrading
}

// IsValid 时段是否有效
func (ts *TradingSession) IsValid() bool {
	for _, item := range ts.Sessions {
		if !item.IsValid() {
			return false
		}
	}
	return true
}

// IsTradingNotStarted 交易是否尚未开始
func (ts *TradingSession) IsTradingNotStarted(timestamp interface{}) bool {
	var tsTime Timestamp

	switch v := timestamp.(type) {
	case string:
		if v == "" {
			now := time.Now()
			tsTime, _ = ParseTimeOnly(now.Format("15:04:05"))
		} else {
			var err error
			tsTime, err = ParseTimeOnly(v)
			if err != nil {
				return false
			}
		}
	case Timestamp:
		tsTime = v
	default:
		tsTime = NowTimestamp()
	}

	return tsTime.Less(ts.EarliestStart)
}

// IsTradingEnded 交易是否已结束
func (ts *TradingSession) IsTradingEnded(timestamp interface{}) bool {
	var tsTime Timestamp

	switch v := timestamp.(type) {
	case string:
		if v == "" {
			now := time.Now()
			tsTime, _ = ParseTimeOnly(now.Format("15:04:05"))
		} else {
			var err error
			tsTime, err = ParseTimeOnly(v)
			if err != nil {
				return false
			}
		}
	case Timestamp:
		tsTime = v
	default:
		tsTime = NowTimestamp()
	}

	return tsTime.Greater(ts.LatestEnd)
}

// Minutes 计算当前时间距离最近的交易时间的分钟数
func (ts *TradingSession) Minutes(timestamp interface{}) int {
	var tsTime Timestamp

	switch v := timestamp.(type) {
	case string:
		if v == "" {
			now := time.Now()
			tsTime, _ = ParseTimeOnly(now.Format("15:04:05"))
		} else {
			var err error
			tsTime, err = ParseTimeOnly(v)
			if err != nil {
				return 0
			}
		}
	case Timestamp:
		tsTime = v
	default:
		tsTime = NowTimestamp()
	}

	total := 0
	for _, tr := range ts.Sessions {
		if tr.Status.IsOpen() {
			total += tr.GetElapsedMinutes(tsTime)
		}
	}
	return total
}

// GetTradingMinutes 当日可交易时段总时长 (分钟)
func (ts *TradingSession) GetTradingMinutes() int {
	total := 0
	for _, tr := range ts.Sessions {
		if tr.Status.IsOpen() {
			total += tr.GetDurationMinutes()
		}
	}
	return total
}

// ======================================================================
// 各市场交易时段初始化函数
// ======================================================================

// InitCNSession 初始化当日的交易会话时段 (A股)
func InitCNSession() (*TradingSession, error) {
	tr1, err := NewTimeRange("09:15:00 ~ 09:20:00", TimeStatusAuctionOrderInputPeriod, RegionCN)
	if err != nil {
		return nil, err
	}
	tr2, err := NewTimeRange("09:20:00 ~ 09:25:00", TimeStatusAuctionMatchingToOpening, RegionCN)
	if err != nil {
		return nil, err
	}
	tr3, err := NewTimeRange("09:25:00 ~ 09:30:00", TimeStatusSuspend, RegionCN)
	if err != nil {
		return nil, err
	}
	tr4, err := NewTimeRange("09:30:00 ~ 11:30:00", TimeStatusTrading, RegionCN)
	if err != nil {
		return nil, err
	}
	tr5, err := NewTimeRange("13:00:00 ~ 14:57:00", TimeStatusTrading, RegionCN)
	if err != nil {
		return nil, err
	}
	tr6, err := NewTimeRange("14:57:00 ~ 15:00:00", TimeStatusAuctionMatchingToClosing|TimeStatus(PermissionOpen), RegionCN)
	if err != nil {
		return nil, err
	}

	return NewTradingSession(tr1, tr2, tr3, tr4, tr5, tr6), nil
}

// InitHKSession 初始化当日的交易会话时段 (港股)
func InitHKSession() (*TradingSession, error) {
	// 1. 输入买卖盘时段：上午9:00-9:15
	tr1, _ := NewTimeRange("09:00:00 ~ 09:15:00", TimeStatusAuctionOrderInputPeriod, RegionHK)
	// 2. 不可取消时段：上午9:15-9:20
	tr2, _ := NewTimeRange("09:15:00 ~ 09:20:00", TimeStatusAuctionNoCancellationPeriod, RegionHK)
	// 3. 随机对盘时段：上午9:20-9:22
	tr3, _ := NewTimeRange("09:20:00 ~ 09:22:00", TimeStatusAuctionMatchingToOpening, RegionHK)
	// 4. 暂停时段：完成对盘后-上午9:30
	tr4, _ := NewTimeRange("09:22:00 ~ 09:30:00", TimeStatusSuspend, RegionHK)
	tr5, _ := NewTimeRange("09:30:00 ~ 12:00:00", TimeStatusContinuousTrading, RegionHK)
	tr6, _ := NewTimeRange("12:00:00 ~ 13:00:00", TimeStatusSuspend, RegionHK)
	tr7, _ := NewTimeRange("13:00:00 ~ 16:00:00", TimeStatusContinuousTrading, RegionHK)
	// 收盘竞价 - 参考价定价阶段(Reference Price) (16:00-16:01)
	tr8, _ := NewTimeRange("16:00:00 ~ 16:01:00", TimeStatusAuctionOrderInputPeriod, RegionHK)
	// 收盘竞价 - 输入订单阶段 (16:01-16:06)
	tr9, _ := NewTimeRange("16:01:00 ~ 16:06:00", TimeStatusAuctionOrderInputPeriod, RegionHK)
	// 收盘竞价 - 不可撤销阶段 (16:06-16:08)
	tr10, _ := NewTimeRange("16:06:00 ~ 16:08:00", TimeStatusAuctionNoCancellationPeriod, RegionHK)
	// 收盘竞价 - 随机收盘 (16:06-16:10)
	tr11, _ := NewTimeRange("16:06:00 ~ 16:10:00", TimeStatusAuctionMatchingToClosing, RegionHK)

	return NewTradingSession(tr1, tr2, tr3, tr4, tr5, tr6, tr7, tr8, tr9, tr10, tr11), nil
}

// InitUSSession 初始化当日的交易会话时段 (美股)
func InitUSSession() (*TradingSession, error) {
	tr1, _ := NewTimeRange("04:00:00 ~ 09:30:00", TimeStatusPreMarket, RegionUS)
	tr2, _ := NewTimeRange("09:30:00 ~ 16:00:00", TimeStatusTrading, RegionUS)
	tr3, _ := NewTimeRange("16:00:00 ~ 20:00:00", TimeStatusAfterHours, RegionUS)

	return NewTradingSession(tr1, tr2, tr3), nil
}

// ======================================================================
// 全局单例管理
// ======================================================================

var (
	tradingHoursMap     = make(map[string]*TradingSession)
	tradingHoursDefault *TradingSession
	tradingHoursOnce    = runtime.RollingOnceFromSpec(data.CnCronExprDailyInit) // 每天9:00重置
)

// GetTradingHoursMap 获取交易时段映射
func GetTradingHoursMap() map[string]*TradingSession {
	return tradingHoursMap
}

// SetTradingHours 设置指定市场的交易时段
func SetTradingHours(market string, session *TradingSession) {
	tradingHoursMap[market] = session
}

// InitTradingHours 初始化各市场交易时段
func InitTradingHours() error {
	cnSession, err := InitCNSession()
	if err != nil {
		return err
	}
	hkSession, err := InitHKSession()
	if err != nil {
		return err
	}
	usSession, err := InitUSSession()
	if err != nil {
		return err
	}

	tradingHoursMap["cn"] = cnSession
	tradingHoursMap["hk"] = hkSession
	tradingHoursMap["us"] = usSession
	tradingHoursDefault = cnSession

	return nil
}

// LatestSessionByExchange 获取指定交易所当天的交易时段信息
func LatestSessionByExchange(exchange Exchange) *TradingSession {
	tradingHoursOnce.Do(func() {
		InitTradingHours()
	})

	key := exchange.Region().String()
	session, ok := tradingHoursMap[key]
	if !ok || session == nil {
		session = tradingHoursDefault
	}
	return session
}

// ======================================================================
// RuntimeStatus 运行时状态
// ======================================================================

// RuntimeStatus 运行时状态
type RuntimeStatus struct {
	BeforeLastTradeDay bool       // 最后交易日前
	IsHoliday          bool       // 是否节假日休市
	BeforeInitTime     bool       // 初始化时间前
	CacheAfterInitTime bool       // 缓存在初始化时间之后
	UpdateInRealTime   bool       // 是否可以实时更新
	Status             TimeStatus // 当前状态
}

// CheckTradingTimestamp 检查交易时间戳状态
func CheckTradingTimestamp(exchange Exchange, lastModified *Timestamp) RuntimeStatus {
	rs := RuntimeStatus{
		Status: TimeStatusClosed,
	}

	now := NowTimestamp()
	var ts Timestamp
	if lastModified != nil {
		ts = *lastModified
	} else {
		ts = now
	}

	lastDay := LastTradingDay(now)

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
	todayInit := GetTodayInit()
	if ts.Less(todayInit) {
		rs.BeforeInitTime = true
		return rs
	}

	rs.Status = TimeStatusPreMarket
	rs.CacheAfterInitTime = true

	// 5. trading not started
	session := LatestSessionByExchange(exchange)
	if session.IsTradingNotStarted(ts) {
		return rs
	}

	rs.UpdateInRealTime = true

	rs.Status = session.CheckStatus(ts)
	if rs.Status.IsTradingDisabled() {
		rs.UpdateInRealTime = false
	}
	return rs
}

// ======================================================================
// 辅助函数
// ======================================================================

var (
	tsTodayInit     Timestamp
	tsTodayInitOnce = runtime.RollingOnceFromSpec(data.CnCronExprDailyInit) // 每天9:00重置
)

// GetTodayInit 获取今日初始化时间
func GetTodayInit() Timestamp {
	tsTodayInitOnce.Do(func() {
		now := NowTimestamp()
		tsTodayInit = now.PreMarketTime()
	})
	return tsTodayInit
}

// CanInitialize 判断是否可以初始化
func CanInitialize(exchange Exchange, lastModified *Timestamp) bool {
	rs := CheckTradingTimestamp(exchange, lastModified)
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
