package api

import (
	"fmt"
	"strings"
	"time"
	_ "unsafe" // for go:linkname
)

// 基础时间常量
const (
	SecondsPerMinute      = 60
	SecondsPerHour        = 60 * SecondsPerMinute
	SecondsPerDay         = 24 * SecondsPerHour
	MillisecondsPerSecond = 1000
	MillisecondsPerMinute = SecondsPerMinute * MillisecondsPerSecond
	MillisecondsPerHour   = SecondsPerHour * MillisecondsPerSecond
	MillisecondsPerDay    = SecondsPerDay * MillisecondsPerSecond
)

// CurrentDateZero t日期的0点整
func CurrentDateZero(t time.Time) time.Time {
	y, m, d := t.Date()
	return time.Date(y, m, d, 0, 0, 0, 0, time.Local)
}

// TodayZero 这也是一个当日0点的用法
func TodayZero() time.Time {
	now := time.Now()
	y, m, d := now.Date()
	return time.Date(y, m, d, 0, 0, 0, 0, time.Local)
}

// SinceZeroHour t当天0点开始到t时的毫秒数
func SinceZeroHour(t time.Time) int64 {
	zero := CurrentDateZero(t)
	return t.Sub(zero).Milliseconds()
}

// Timestamp C++风格的时间戳实现，提供与C++ timestamp类相同的API
// 这是一个兼容层，基于Go的核心timestamp实现
type Timestamp struct {
	ms int64 // 毫秒数
}

// 时间格式常量
const (
	DefaultLayout    = "2006-01-02 15:04:05"
	OnlyDateLayout   = "2006-01-02"
	CacheDateLayout  = "20060102"
	OnlyTimeLayout   = "15:04:05"
	TimeLayoutWithMS = "2006-01-02 15:04:05.000"

	// 盘前时间配置
	PreMarketHour   = 9
	PreMarketMinute = 0
	PreMarketSecond = 0
)

//go:linkname now time.now
func now() (sec int64, nsec int32, mono int64)

var (
	// 获取偏移的秒数
	zoneName, offsetInSecondsEastOfUTC = time.Now().Zone()
	_                                  = zoneName
	// UTC到本地的偏移秒数
	utcToLocal = int64(offsetInSecondsEastOfUTC)
	// 本地到UTC的偏移秒数
	localToUTC = -utcToLocal
)

// Now 获取本地当前的时间戳, 毫秒数 (UTC 转 local)
func Now() int64 {
	sec, nsec, _ := now()
	sec += int64(offsetInSecondsEastOfUTC)
	milli := sec*MillisecondsPerSecond + int64(nsec)/1e6%MillisecondsPerSecond
	return milli
}

// TimeToTimestamp 获取time.Time的本地毫秒数 (UTC 转 local)
func TimeToTimestamp(t time.Time) int64 {
	utcMilliseconds := t.UnixMilli()
	milliseconds := utcMilliseconds + utcToLocal*MillisecondsPerSecond
	return milliseconds
}

// Time 本地毫秒数转time.Time (local 转 UTC)
func Time(milliseconds int64) time.Time {
	utcMilliseconds := milliseconds + localToUTC*MillisecondsPerSecond
	return time.UnixMilli(utcMilliseconds)
}

// SinceZero 从0点到milliseconds过去的毫秒数
func SinceZero(milliseconds int64) int64 {
	elapsed := milliseconds % MillisecondsPerDay
	if elapsed < 0 {
		elapsed += MillisecondsPerDay
	}
	return elapsed
}

// ZeroHour 零点整的毫秒数
func ZeroHour(milliseconds int64) int64 {
	elapsed := SinceZero(milliseconds)
	diff := milliseconds - elapsed
	return diff
}

// Since t当日0点整到t的毫秒数
func Since(t time.Time) int64 {
	milliseconds := TimeToTimestamp(t)
	elapsed := SinceZero(milliseconds)
	return elapsed
}

// Today 获取当日0点整的时间戳, 毫秒数 (UTC 转 local)
func Today() int64 {
	milliseconds := Now()
	elapsed := ZeroHour(milliseconds)
	return elapsed
}

// NewTimestamp 创建新的时间戳
func NewTimestamp(ms int64) Timestamp {
	return Timestamp{ms: ms}
}

// NewTimestampFromTime 从time.Time创建时间戳
func NewTimestampFromTime(t time.Time) Timestamp {
	return Timestamp{ms: TimeToTimestamp(t)}
}

// NewTimestampFromString 从字符串创建时间戳
func NewTimestampFromString(str string) (Timestamp, error) {
	ts, err := ParseTimestamp(str)
	return ts, err
}

// NewTimestampFromDate 从年月日时分秒毫秒创建时间戳
func NewTimestampFromDate(year, month, day, hour, minute, second, millisecond int) Timestamp {
	t := time.Date(year, time.Month(month), day, hour, minute, second, millisecond*1000000, time.Local)
	return NewTimestampFromTime(t)
}

// ZeroTimestamp 零值C++风格时间戳 - 对应C++的zero()
func ZeroTimestamp() Timestamp {
	return Timestamp{ms: 0}
}

// NowTimestamp 当前C++风格时间戳 - 对应C++的now()
func NowTimestamp() Timestamp {
	return Timestamp{ms: Now()}
}

// PreMarketTimestamp 盘前初始化C++风格时间戳 - 对应C++的pre_market_time(int year, int month, int day)
func PreMarketTimestamp(year, month, day int) Timestamp {
	return NewTimestampFromDate(year, month, day, PreMarketHour, PreMarketMinute, PreMarketSecond, 0)
}

// Value 获取毫秒数 - 对应C++的value()
func (t Timestamp) Value() int64 {
	return t.ms
}

// StartOfDay 获取当天零点时间戳 - 对应C++的start_of_day()
func (t Timestamp) StartOfDay() Timestamp {
	return Timestamp{ms: t.ms - (t.ms % MillisecondsPerDay)}
}

// MidnightTimestamp 当天零点整 - 对应C++的midnight()
func MidnightTimestamp() Timestamp {
	ts := Now()
	return Timestamp{ms: ts - ts%MillisecondsPerDay}
}

// Today 当天指定时间 - 对应C++的today(int hour, int minute, int second, int millisecond)
func (t Timestamp) Today(hour, minute, second, millisecond int) Timestamp {
	ts := t.StartOfDay().Value()
	ts += int64(hour) * MillisecondsPerHour
	ts += int64(minute) * MillisecondsPerMinute
	ts += int64(second) * MillisecondsPerSecond
	ts += int64(millisecond)
	return Timestamp{ms: ts}
}

// Since 从当天零点开始的指定时间 - 对应C++的since(int hour, int minute, int second, int millisecond)
func (t Timestamp) Since(hour, minute, second, millisecond int) Timestamp {
	return t.Today(hour, minute, second, millisecond)
}

// Offset 偏移指定时间 - 对应C++的offset(int hour, int minute, int second, int millisecond)
func (t Timestamp) Offset(hour, minute, second, millisecond int) Timestamp {
	ts := t.Value()
	ts += int64(hour) * MillisecondsPerHour
	ts += int64(minute) * MillisecondsPerMinute
	ts += int64(second) * MillisecondsPerSecond
	ts += int64(millisecond)
	return Timestamp{ms: ts}
}

// PreMarketTime 当天盘前时间 - 对应C++的pre_market_time()
func (t Timestamp) PreMarketTime() Timestamp {
	return t.Since(PreMarketHour, PreMarketMinute, PreMarketSecond, 0)
}

// Floor 调整时间戳到0秒0毫秒 - 对应C++的floor()
func (t Timestamp) Floor() Timestamp {
	ts := t.Value()
	ts -= (ts % MillisecondsPerMinute)
	return Timestamp{ms: ts}
}

// Ceil 调整时间戳到59秒999毫秒 - 对应C++的ceil()
func (t Timestamp) Ceil() Timestamp {
	ts := t.Value()
	ts = ts - (ts % MillisecondsPerMinute) + (MillisecondsPerMinute - 1)
	return Timestamp{ms: ts}
}

// Extract 提取年月日 - 对应C++的extract()
func (t Timestamp) Extract() (year, month, day int) {
	timeVal := Time(t.ms)
	year, m, day := timeVal.Date()
	month = int(m)
	return
}

// ToString 格式化为字符串 - 对应C++的toString(const std::string &layout)
func (t Timestamp) ToString(layout ...string) string {
	var formatLayout string
	if len(layout) > 0 {
		formatLayout = layout[0]
	} else {
		formatLayout = TimeLayoutWithMS
	}

	timeVal := Time(t.ms)
	if strings.Contains(formatLayout, ".000") {
		// 包含毫秒的格式
		return timeVal.Format(formatLayout)
	}
	return timeVal.Format(formatLayout)
}

// ToStringAsTimeInSeconds 以秒为单位格式化 - 对应C++的toStringAsTimeInSeconds()
func (t Timestamp) ToStringAsTimeInSeconds(layout ...string) string {
	var formatLayout string
	if len(layout) > 0 {
		formatLayout = layout[0]
	} else {
		formatLayout = OnlyTimeLayout
	}

	timeVal := Time(t.ms)
	// 截断到秒
	timeVal = timeVal.Truncate(time.Second)
	return timeVal.Format(formatLayout)
}

// OnlyDate 返回日期 - 对应C++的only_date()
func (t Timestamp) OnlyDate() string {
	return t.ToString(OnlyDateLayout)
}

// CacheDate 返回缓存日期格式 - 对应C++的cache_date()
func (t Timestamp) CacheDate() string {
	return t.ToString(CacheDateLayout)
}

// OnlyTime 返回时间 - 对应C++的only_time()
func (t Timestamp) OnlyTime() string {
	return t.ToStringAsTimeInSeconds(OnlyTimeLayout)
}

// YYYYMMDD 返回整型日期 - 对应C++的yyyymmdd()
func (t Timestamp) YYYYMMDD() uint32 {
	y, m, d := t.Extract()
	return uint32(y*10000 + m*100 + d)
}

// IsEmpty 是否为空（零值） - 对应C++的empty()
func (t Timestamp) IsEmpty() bool {
	return t.Value() == 0
}

// IsSameDate 检查是否同一天 - 对应C++的is_same_date()
func (t Timestamp) IsSameDate(other Timestamp) bool {
	day1 := t.ms / MillisecondsPerDay
	day2 := other.ms / MillisecondsPerDay
	return day1 == day2
}

// ParseTimestamp 解析日期时间字符串 - 主要用于日期格式，也支持包含时间 - 对应C++的parse()
func ParseTimestamp(str string) (Timestamp, error) {
	// 尝试多种格式解析，偏向日期格式
	formats := []string{
		"2006-01-02 15:04:05.000",       // 完整日期时间+毫秒
		"2006-01-02 15:04:05",           // 完整日期时间
		"2006-01-02",                    // 仅日期
		"20060102",                      // 紧凑日期格式
		"2006/01/02 15:04:05",           // 斜杠分隔的日期时间
		"01/02/2006 15:04:05",           // 美式日期时间
		"15:04:05 02-01-2006",           // 时间在前的格式
		"20060102 150405",               // 紧凑日期时间
		"2006-01-02T15:04:05Z",          // ISO 8601 UTC
		"2006-01-02T15:04:05Z07:00",     // ISO 8601 with timezone
		"Mon, 02 Jan 2006 15:04:05 MST", // RFC 1123
		"Jan 02 2006 15:04:05",          // 月份名格式
	}

	for _, format := range formats {
		if t, err := time.ParseInLocation(format, str, time.Local); err == nil {
			return NewTimestampFromTime(t), nil
		}
	}

	return ZeroTimestamp(), fmt.Errorf("unable to parse date string: %s", str)
}

// ParseTimeOnly 解析时间字符串 - 主要用于时间格式，但也兼容完整日期时间 - 对应C++的parse_time()
func ParseTimeOnly(str string) (Timestamp, error) {
	// 设计目的：用户关注时分秒时使用，但不限制输入格式
	// 既支持纯时间，也支持包含日期的格式
	timeFormats := []string{
		"15:04:05",                      // 纯时间
		"2006-01-02 15:04:05",           // 完整日期时间
		"2006-01-02",                    // 仅日期
		"20060102",                      // 紧凑日期
		"2006/01/02 15:04:05",           // 斜杠日期时间
		"01/02/2006 15:04:05",           // 美式日期时间
		"15:04:05 02-01-2006",           // 时间在前
		"150405",                        // 紧凑时间
		"20060102 150405",               // 紧凑日期时间
		"2006-01-02T15:04:05Z",          // ISO 8601 UTC
		"2006-01-02T15:04:05Z07:00",     // ISO 8601 with timezone
		"Mon, 02 Jan 2006 15:04:05 MST", // RFC 1123
		"Jan 02 2006 15:04:05",          // 月份名格式
	}

	for _, format := range timeFormats {
		if t, err := time.ParseInLocation(format, str, time.Local); err == nil {
			return NewTimestampFromTime(t), nil
		}
	}

	return ZeroTimestamp(), fmt.Errorf("unable to parse time string: %s", str)
}

// String 字符串表示 - 对应C++的operator<<
func (t Timestamp) String() string {
	return t.ToString()
}

// 比较方法 - 对应C++的各种比较运算符

// Equal 等于比较 - 对应C++的operator==
func (t Timestamp) Equal(other Timestamp) bool {
	return t.ms == other.ms
}

// NotEqual 不等于比较 - 对应C++的operator!=
func (t Timestamp) NotEqual(other Timestamp) bool {
	return t.ms != other.ms
}

// Less 小于比较 - 对应C++的operator<
func (t Timestamp) Less(other Timestamp) bool {
	return t.ms < other.ms
}

// Greater 大于比较 - 对应C++的operator>
func (t Timestamp) Greater(other Timestamp) bool {
	return t.ms > other.ms
}

// LessOrEqual 小于等于比较 - 对应C++的operator<=
func (t Timestamp) LessOrEqual(other Timestamp) bool {
	return t.ms <= other.ms
}

// GreaterOrEqual 大于等于比较 - 对应C++的operator>=
func (t Timestamp) GreaterOrEqual(other Timestamp) bool {
	return t.ms >= other.ms
}

// UnixMilli 转换为Unix毫秒时间戳
func (t Timestamp) UnixMilli() int64 {
	// 使用现有的Time函数转换，然后获取Unix毫秒
	return t.ToTime().UnixMilli()
}

// ToTime 转换为time.Time
func (t Timestamp) ToTime() time.Time {
	return Time(t.ms)
}

// AsTimestamp 转换为Go原生int64时间戳
func (t Timestamp) AsTimestamp() int64 {
	return t.ms
}

// FromTimestamp 从Go原生int64时间戳创建
func FromTimestamp(ts int64) Timestamp {
	return Timestamp{ms: ts}
}
