package meta

import (
	"encoding/csv"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"github.com/quant1x/quant1x/quant1x/core"
	"github.com/quant1x/quant1x/quant1x/data/meta/sina"
	"github.com/quant1x/quant1x/quant1x/runtime"

	"time"
)

// CalendarFilename returns full path to calendar cache file under meta directory.
// Previously this helper lived in package `meta`; migrated here to keep
// exchange-specific cache layout nearby.
func CalendarFilename() string {
	return filepath.Join(core.GetMetaPath(), "calendar")
}

// calendarMarkerFilename returns the path to the marker file used to record
// the last successful calendar update. Kept private to this package.
func calendarMarkerFilename() string {
	return filepath.Join(core.GetMetaPath(), "calendar.updated")
}

const (
	sinaCalendarURL     = "https://finance.sina.com.cn/realstock/company/klc_td_sh.txt"
	calendarMissingDate = "1992-05-04"
)

var (
	// in-memory caches
	globalCalendarsString    []string
	globalCalendarsTimestamp []Timestamp
	// Initialize a default RollingOnce using the central `runtime` package so call sites can invoke it directly
	// and behavior matches C++'s global_calendar_once->Do(...).
	calendarRollingOnce = runtime.RollingOnceDaily(PreMarketHour, PreMarketMinute)
)

// preprocess JS-like response text (strip assignment, trailing semicolon, and quotes)
// Name aligned with C++ `preprocess` for 1:1 parity.
func preprocess(text string) string {
	s := text
	if pos := strings.Index(s, "="); pos != -1 {
		s = s[pos+1:]
	}
	if pos := strings.Index(s, ";"); pos != -1 {
		s = s[:pos]
	}
	s = strings.ReplaceAll(s, "\"", "")
	return s
}

// decode mirrors the C++ detail::decode: preprocess then use the sina finance
// decoder and extract the "date" field from decoded records preserving order.
func decode(text string) []string {
	pre := preprocess(text)
	dec := sina.NewFinanceDecoder(pre)
	raw := dec.Decode()

	dates := make([]string, 0)
	switch v := raw.(type) {
	case []map[string]string:
		for _, m := range v {
			if d, ok := m["date"]; ok && strings.TrimSpace(d) != "" {
				dates = append(dates, d)
			}
		}
	case []map[string]any:
		for _, m := range v {
			if ai, ok := m["date"]; ok {
				if ds, ok2 := ai.(string); ok2 && strings.TrimSpace(ds) != "" {
					dates = append(dates, ds)
				}
			}
		}
	case []string:
		dates = append(dates, v...)
	default:
		// no-op, return empty/nil to indicate nothing decoded
	}

	if len(dates) == 0 {
		return nil
	}
	return dates
}

// updateCalendar downloads calendar data from Sina and caches it to disk.
func updateCalendar() error {
	fname := CalendarFilename()
	// attempt conditional GET if file exists
	var modtime time.Time
	if fi, err := os.Stat(fname); err == nil {
		modtime = fi.ModTime()
	}

	client := &http.Client{Timeout: 15 * time.Second}
	req, err := http.NewRequest(http.MethodGet, sinaCalendarURL, nil)
	if err != nil {
		return err
	}
	if !modtime.IsZero() {
		req.Header.Set("If-Modified-Since", modtime.UTC().Format(http.TimeFormat))
	}
	resp, err := client.Do(req)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode == http.StatusNotModified {
		return nil
	}
	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("http status %d", resp.StatusCode)
	}

	bodyBytes, err := io.ReadAll(resp.Body)
	if err != nil {
		return err
	}
	body := string(bodyBytes)

	// decode using the package-local helper which mirrors C++ detail::decode
	dates := decode(body)

	// ensure missing date present
	// Insert missing date in-place (do NOT re-sort) to preserve decoder order,
	// matching the C++ behavior which does not explicitly sort the decoded list.
	idx := sort.SearchStrings(dates, calendarMissingDate)
	if idx == len(dates) || dates[idx] != calendarMissingDate {
		// insert at idx
		dates = append(dates, "")
		copy(dates[idx+1:], dates[idx:])
		dates[idx] = calendarMissingDate
	}

	// write CSV cache
	if err := os.MkdirAll(filepath.Dir(fname), 0o755); err != nil {
		return err
	}
	f, err := os.Create(fname)
	if err != nil {
		return err
	}
	defer f.Close()
	w := csv.NewWriter(f)
	_ = w.Write([]string{"date", "source"})
	for _, d := range dates {
		_ = w.Write([]string{d, "sina"})
	}
	w.Flush()
	if err := w.Error(); err != nil {
		return err
	}

	// set file mtime if Last-Modified provided
	if lm := resp.Header.Get("Last-Modified"); lm != "" {
		if t, e := http.ParseTime(lm); e == nil {
			_ = os.Chtimes(fname, t, t)
		}
	}

	// create/update marker file next to calendar cache so callers can
	// decide whether a remote update is necessary without hitting the
	// network. Use the local completion time (now) as the marker mtime —
	// the marker indicates when we last successfully updated the cache,
	// not when the remote resource claims to have been last-modified.
	marker := calendarMarkerFilename()
	markerTime := time.Now()
	if mf, err := os.Create(marker); err == nil {
		_ = mf.Close()
		_ = os.Chtimes(marker, markerTime, markerTime)
	}
	return nil
}

// lazyLoadCalendar ensures the calendar cache exists and loads it into memory.
func lazyLoadCalendar() {
	fname := CalendarFilename()
	// marker file lives next to calendar cache
	marker := calendarMarkerFilename()

	// Determine today's pre-market timestamp via session.GetTodayInit()
	tsTodayInit := GetTodayInit()
	//nowTs := NowTimestamp()

	ensureUpdated := false
	tsMarkerModified, err := GetFilenameModifiedTime(marker)
	if err != nil {
		// marker missing: ensure update
		ensureUpdated = true
	} else {
		// if marker modified time < today's pre-market time, ensure update
		if tsMarkerModified.Less(tsTodayInit) {
			ensureUpdated = true
		}
	}

	if ensureUpdated {
		_ = updateCalendar()
	}
	f, err := os.Open(fname)
	if err != nil {
		return
	}
	defer f.Close()
	r := csv.NewReader(f)
	// read header
	if _, err := r.Read(); err != nil {
		return
	}
	var ss []string
	var ts []Timestamp
	for {
		rec, err := r.Read()
		if err == io.EOF {
			break
		}
		if err != nil || len(rec) == 0 {
			continue
		}
		date := strings.TrimSpace(rec[0])
		if date == "" {
			continue
		}
		ss = append(ss, date)
		if t, e := ParseTimestamp(date); e == nil {
			ts = append(ts, t.PreMarketTime())
		}
	}
	globalCalendarsString = ss
	globalCalendarsTimestamp = ts
}

// get_calendar_list returns the in-memory calendar (loads on first call).
// Name aligned with C++ `get_calendar_list` for 1:1 parity.
func get_calendar_list() []string {
	// Ensure calendar is loaded; use persistent RollingOnce (parity with C++ global_calendar_once).
	calendarRollingOnce.Do(lazyLoadCalendar)
	if len(globalCalendarsString) == 0 {
		panic("exchange calendar is empty")
	}
	return append([]string(nil), globalCalendarsString...)
}

// #sym:get_calendar_list
// GetCalendarList returns all trading dates as YYYY-MM-DD strings.
func GetCalendarList() []string {
	return get_calendar_list()
}

// #sym:get_calendar_timestamps
// GetCalendarTimestamps returns a copy of the trading calendar timestamps.
func GetCalendarTimestamps() []Timestamp {
	calendarRollingOnce.Do(lazyLoadCalendar)
	return append([]Timestamp(nil), globalCalendarsTimestamp...)
}

// #sym:load_calendar
// LoadCalendar loads and returns the trading calendar timestamps.
func LoadCalendar() []Timestamp {
	return GetCalendarTimestamps()
}

// #sym:load_calendar_series
// LoadCalendarSeries returns the trading calendar as date strings.
func LoadCalendarSeries() []string {
	return GetCalendarList()
}

// #sym:get_today
// GetToday returns the current date in YYYY-MM-DD format.
func GetToday() string {
	return NowTimestamp().OnlyDate()
}

// #sym:fix_trade_date
// FixTradeDate parses a YYYY-MM-DD date and formats it with a strftime format.
func FixTradeDate(date string, format ...string) (string, error) {
	if date == "" {
		return date, nil
	}
	parsed, err := time.ParseInLocation(OnlyDateLayout, date, time.Local)
	if err != nil {
		return "", fmt.Errorf("invalid trade date %q: %w", date, err)
	}
	outputFormat := "%Y-%m-%d"
	if len(format) > 0 && format[0] != "" {
		outputFormat = format[0]
	}
	outputLayout, err := strftimeLayout(outputFormat)
	if err != nil {
		return "", err
	}
	return parsed.Format(outputLayout), nil
}

func strftimeLayout(format string) (string, error) {
	var layout strings.Builder
	for i := 0; i < len(format); i++ {
		if format[i] != '%' {
			layout.WriteByte(format[i])
			continue
		}
		i++
		if i >= len(format) {
			return "", fmt.Errorf("invalid trailing %% in date format")
		}
		switch format[i] {
		case '%':
			layout.WriteByte('%')
		case 'Y':
			layout.WriteString("2006")
		case 'y':
			layout.WriteString("06")
		case 'm':
			layout.WriteString("01")
		case 'd':
			layout.WriteString("02")
		case 'e':
			layout.WriteString("_2")
		case 'H':
			layout.WriteString("15")
		case 'I':
			layout.WriteString("03")
		case 'M':
			layout.WriteString("04")
		case 'S':
			layout.WriteString("05")
		case 'b':
			layout.WriteString("Jan")
		case 'B':
			layout.WriteString("January")
		case 'a':
			layout.WriteString("Mon")
		case 'A':
			layout.WriteString("Monday")
		case 'p':
			layout.WriteString("PM")
		case 'j':
			layout.WriteString("002")
		case 'z':
			layout.WriteString("-0700")
		case 'Z':
			layout.WriteString("MST")
		default:
			return "", fmt.Errorf("unsupported date format directive %%%c", format[i])
		}
	}
	return layout.String(), nil
}

func get_date_range(begin, end string, skipToday bool) []string {
	return GetDateRange(begin, end, skipToday)
}

// #sym:get_date_range
// GetDateRange returns trading date strings within the inclusive date range.
func GetDateRange(begin, end string, skipTodayOption ...bool) []string {
	skipToday := len(skipTodayOption) > 0 && skipTodayOption[0]
	return tradingDateStringRange(get_calendar_list(), begin, end, skipToday, GetToday())
}

func tradingDateStringRange(list []string, begin, end string, skipToday bool, today string) []string {
	if begin > end {
		return nil
	}
	if len(list) == 0 {
		return nil
	}
	is := sort.SearchStrings(list, begin)
	ie := sort.Search(len(list), func(i int) bool { return list[i] > end })
	if skipToday {
		if ie > 0 {
			lastDay := list[ie-1]
			if lastDay > today || lastDay > end {
				ie--
			}
		}
	}
	if is >= ie {
		return nil
	}
	return append([]string(nil), list[is:ie]...)
}

// #sym:date_range
func DateRange(begin, end Timestamp, skipTodayOption ...bool) []Timestamp {
	skipToday := len(skipTodayOption) > 0 && skipTodayOption[0]
	return tradingDateRange(GetCalendarTimestamps(), begin, end, skipToday, NowTimestamp().PreMarketTime())
}

// DateRangeToNow returns trading dates from begin through the current time.
func DateRangeToNow(begin Timestamp, skipToday ...bool) []Timestamp {
	end := NowTimestamp()
	if len(skipToday) > 0 {
		return DateRange(begin, end, skipToday[0])
	}
	return DateRange(begin, end)
}

func tradingDateRange(tradeDates []Timestamp, begin, end Timestamp, skipToday bool, today Timestamp) []Timestamp {
	if len(tradeDates) == 0 {
		return nil
	}
	if begin.Greater(end) {
		return nil
	}
	n := len(tradeDates)
	lower := sort.Search(n, func(i int) bool { return !tradeDates[i].Less(begin) })
	upper := sort.Search(n, func(i int) bool { return tradeDates[i].Greater(end) })
	if skipToday && upper > 0 {
		lastInRange := tradeDates[upper-1]
		if lastInRange.Greater(today) || lastInRange.Greater(end) {
			upper--
		}
	}
	if lower >= upper {
		return nil
	}
	return append([]Timestamp(nil), tradeDates[lower:upper]...)
}

func lastTradingDay(tradeDates []Timestamp, date, current Timestamp) Timestamp {
	if len(tradeDates) == 0 {
		return date
	}
	i := sort.Search(len(tradeDates), func(i int) bool { return tradeDates[i].Greater(date) })
	if i != 0 {
		i--
	}
	if current.Less(tradeDates[i]) && i != 0 {
		i--
	}
	return tradeDates[i]
}

func prevTradingDay(tradeDates []Timestamp, date, current Timestamp) Timestamp {
	if len(tradeDates) == 0 {
		return date
	}
	i := sort.Search(len(tradeDates), func(i int) bool { return !tradeDates[i].Less(date) })
	if i > 0 {
		i--
	}
	if current.Less(tradeDates[i]) && i > 0 {
		i--
	}
	return tradeDates[i]
}

func nextTradingDay(tradeDates []Timestamp, date, current Timestamp) Timestamp {
	if len(tradeDates) == 0 {
		return date
	}
	i := sort.Search(len(tradeDates), func(i int) bool { return !tradeDates[i].Less(date) })
	if i >= len(tradeDates) {
		return tradeDates[len(tradeDates)-1]
	}
	if !current.Less(tradeDates[i]) && i < len(tradeDates)-1 {
		i++
	}
	return tradeDates[i]
}

func calendarQueryArguments(args []Timestamp) (Timestamp, Timestamp) {
	date := NowTimestamp().PreMarketTime()
	current := NowTimestamp()
	if len(args) > 0 && args[0].Value() != 0 {
		date = args[0]
	}
	if len(args) > 1 && args[1].Value() != 0 {
		current = args[1]
	}
	return date, current
}

// #sym:last_trading_day
// LastTradingDay returns the most recent trading day not later than date.
// With no arguments, the current date and time are used. An optional second
// timestamp supplies the current time for deterministic pre-market checks.
func LastTradingDay(args ...Timestamp) Timestamp {
	date, current := calendarQueryArguments(args)
	return lastTradingDay(GetCalendarTimestamps(), date, current)
}

// #sym:prev_trading_day
// PrevTradingDay returns the trading day immediately before date.
func PrevTradingDay(args ...Timestamp) Timestamp {
	date, current := calendarQueryArguments(args)
	return prevTradingDay(GetCalendarTimestamps(), date, current)
}

// #sym:next_trading_day
// NextTradingDay returns the next trading day, considering whether today's
// pre-market time has already passed.
func NextTradingDay(args ...Timestamp) Timestamp {
	date, current := calendarQueryArguments(args)
	return nextTradingDay(GetCalendarTimestamps(), date, current)
}
