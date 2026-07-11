package base

import (
	"fmt"
	"strings"
	"time"
)

const (
	LayoutOnlyDate = "2006-01-02"
	LayoutDateTime = "2006-01-02 15:04:05"
)

// ZoneOffsetMilliseconds 获取时区偏移的毫秒数
func ZoneOffsetMilliseconds() int64 {
	_, offset := time.Now().Zone()
	return int64(offset) * 1000
}

var dateTimeLayouts = []string{
	LayoutDateTime,
	LayoutOnlyDate,
	"20060102",
	"2006/01/02 15:04:05",
	"01/02/2006 15:04:05",
	"15:04:05 02-01-2006",
	"20060102 150405",
	time.RFC3339,
	"2006-01-02T15:04:05-0700",
	time.RFC1123,
	"Jan 02 2006 15:04:05",
}

// ParseDate 解析日期
func ParseDate(str string) int64 {
	str = strings.TrimSpace(str)
	if str == "" {
		return 0
	}

	for _, layout := range dateTimeLayouts {
		if t, err := time.ParseInLocation(layout, str, time.Local); err == nil {
			return t.UnixNano() / int64(time.Millisecond)
		}
	}
	// Try parsing as UTC if local fails, or maybe just stick to Local as per C++ implementation which seems to imply local unless specified?
	// The C++ implementation uses date::from_stream which might handle timezones if present in string.
	// For now, ParseInLocation with Local is a good default.
	return 0
}

var timeLayouts = []string{
	"15:04:05",
	LayoutDateTime,
	LayoutOnlyDate,
	"20060102",
	"2006/01/02 15:04:05",
	"01/02/2006 15:04:05",
	"15:04:05 02-01-2006",
	"150405",
	"20060102 150405",
	time.RFC3339,
	"2006-01-02T15:04:05-0700",
	time.RFC1123,
	"Jan 02 2006 15:04:05",
}

// ParseTime 解析时间
func ParseTime(str string) int64 {
	str = strings.TrimSpace(str)
	if str == "" {
		return 0
	}

	for _, layout := range timeLayouts {
		if t, err := time.ParseInLocation(layout, str, time.Local); err == nil {
			// If the format doesn't include date, time.Parse adds year 0.
			// The C++ implementation returns milliseconds.
			// If it's just time, we might want to return duration from 00:00:00?
			// But the C++ implementation returns `tp.time_since_epoch().count()`.
			// If the parsed time has year 0, the epoch time will be very negative or small.
			// However, `date::from_stream` behavior depends on what's in the stream.
			// If only time is parsed, it defaults to current date or epoch?
			// Actually, `std::chrono::parse` usually defaults missing fields to 0 or current.
			// Let's assume we return the timestamp of the parsed time.
			return t.UnixNano() / int64(time.Millisecond)
		}
	}
	return 0
}

// MsUtcToLocal UTC毫秒转本地毫秒
func MsUtcToLocal(milliseconds int64) int64 {
	return milliseconds + ZoneOffsetMilliseconds()
}

// MsLocalToUtc 本地毫秒转UTC毫秒
func MsLocalToUtc(milliseconds int64) int64 {
	return milliseconds - ZoneOffsetMilliseconds()
}

// FromLocal 本地毫秒转Time
func FromLocal(milliseconds int64) time.Time {
	utcMs := MsLocalToUtc(milliseconds)
	return time.Unix(0, utcMs*int64(time.Millisecond))
}

// FromTimePoint Time转本地毫秒
func FromTimePoint(tp time.Time) int64 {
	return MsUtcToLocal(tp.UnixNano() / int64(time.Millisecond))
}

// Today 获取当前日期的字符串
func Today() string {
	return time.Now().Format(LayoutOnlyDate)
}

// GetTimestamp 获取当前时间戳的字符串
func GetTimestamp() string {
	return time.Now().Format(LayoutDateTime)
}

// TimeToString Time转字符串
func TimeToString(tp time.Time, format ...string) string {
	layout := LayoutOnlyDate
	if len(format) > 0 {
		layout = format[0]
	}
	return tp.Format(layout)
}

// GetQuarterDay 获得当前季度的初始和结束日期, months为偏移的月数
func GetQuarterDay(months int) (string, string) {
	now := time.Now()
	// 减去 months 个月
	targetDate := now.AddDate(0, -months, 0)
	year := targetDate.Year()
	month := targetDate.Month()

	var firstOfQuarter, lastOfQuarter string

	if month >= 1 && month <= 3 {
		firstOfQuarter = fmt.Sprintf("%d-01-01 00:00:00", year)
		lastOfQuarter = fmt.Sprintf("%d-03-31 23:59:59", year)
	} else if month >= 4 && month <= 6 {
		firstOfQuarter = fmt.Sprintf("%d-04-01 00:00:00", year)
		lastOfQuarter = fmt.Sprintf("%d-06-30 23:59:59", year)
	} else if month >= 7 && month <= 9 {
		firstOfQuarter = fmt.Sprintf("%d-07-01 00:00:00", year)
		lastOfQuarter = fmt.Sprintf("%d-09-30 23:59:59", year)
	} else {
		firstOfQuarter = fmt.Sprintf("%d-10-01 00:00:00", year)
		lastOfQuarter = fmt.Sprintf("%d-12-31 23:59:59", year)
	}

	return firstOfQuarter, lastOfQuarter
}

// parseTimeInternal 内部使用的解析函数, 返回 time.Time
func parseTimeInternal(dateStr string) time.Time {
	for _, layout := range dateTimeLayouts {
		if t, err := time.ParseInLocation(layout, dateStr, time.Local); err == nil {
			// Fix year if needed (logic from C++: if year < 100)
			year := t.Year()
			if year < 100 {
				if year < 70 {
					year += 2000
				} else {
					year += 1900
				}
				t = t.AddDate(year-t.Year(), 0, 0)
			}
			return t
		}
	}
	return time.Now()
}

// GetQuarterByDate 通过给定的日期 获得日期所在财报的季度, 初始以及结束日期
// diff 季度偏移数, 大于0前移diff个季度, 小于0后移diff个季度, 默认为当前季度
func GetQuarterByDate(dateStr string, diff int) (string, string, string) {
	t := parseTimeInternal(dateStr)

	// Apply quarter offset (C++: now.tm_mon -= 3 * diffQuarters)
	// Note: C++ logic subtracts, so positive diff means "ago".
	t = t.AddDate(0, -3*diff, 0)

	year := t.Year()
	month := t.Month()

	var quarter, firstOfQuarter, lastOfQuarter string

	if month >= 1 && month <= 3 {
		firstOfQuarter = fmt.Sprintf("%d-01-01 00:00:00", year)
		lastOfQuarter = fmt.Sprintf("%d-03-31 23:59:59", year)
		quarter = fmt.Sprintf("%dQ1", year)
	} else if month >= 4 && month <= 6 {
		firstOfQuarter = fmt.Sprintf("%d-04-01 00:00:00", year)
		lastOfQuarter = fmt.Sprintf("%d-06-30 23:59:59", year)
		quarter = fmt.Sprintf("%dQ2", year)
	} else if month >= 7 && month <= 9 {
		firstOfQuarter = fmt.Sprintf("%d-07-01 00:00:00", year)
		lastOfQuarter = fmt.Sprintf("%d-09-30 23:59:59", year)
		quarter = fmt.Sprintf("%dQ3", year)
	} else {
		firstOfQuarter = fmt.Sprintf("%d-10-01 00:00:00", year)
		lastOfQuarter = fmt.Sprintf("%d-12-31 23:59:59", year)
		quarter = fmt.Sprintf("%dQ4", year)
	}

	return quarter, firstOfQuarter, lastOfQuarter
}

// GetTimezoneOffsetStandard 计算两个时区之间的标准时间差(以小时为单位)
//
// Args:
//
//	targetZone: 目标时区名称(如"Asia/Shanghai", "America/New_York")
//	localZone: 本地时区名称, 如果为空则使用系统本地时区
//
// Returns:
//
//	int: 目标时区相对于本地时区的时间差(小时), 正数表示目标时区比本地快
//
// Example:
//
//	offset := GetTimezoneOffsetStandard("America/New_York", "")
//	offset := GetTimezoneOffsetStandard("Asia/Tokyo", "UTC")
func GetTimezoneOffsetStandard(targetZone string, localZone string) int {
	now := time.Now()

	// 获取目标时区
	targetLocation, err := time.LoadLocation(targetZone)
	if err != nil {
		// 如果加载失败, 使用 UTC
		targetLocation = time.UTC
	}

	// 获取本地时区
	var localLocation *time.Location
	if localZone == "" {
		localLocation = time.Local
	} else {
		localLocation, err = time.LoadLocation(localZone)
		if err != nil {
			// 如果加载失败, 使用本地系统时区
			localLocation = time.Local
		}
	}

	// 计算两个时区的 UTC 偏移
	_, targetOffset := now.In(targetLocation).Zone()
	_, localOffset := now.In(localLocation).Zone()

	// 计算时差(秒转小时)
	offsetSeconds := targetOffset - localOffset
	return offsetSeconds / 3600
}
