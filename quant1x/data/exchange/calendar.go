package exchange

import (
	"encoding/csv"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"gitee.com/quant1x/quant1x/quant1x/core"
	"gitee.com/quant1x/quant1x/quant1x/exchange/sina"
	"gitee.com/quant1x/quant1x/quant1x/runtime"

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

func get_date_range(begin, end string, skipToday bool) []string {
	if begin > end {
		return nil
	}
	list := get_calendar_list()
	is := sort.SearchStrings(list, begin)
	ie := sort.SearchStrings(list, end)

	// Follow C++ ordering: handle skipToday first, else adjust ie to <= end
	if skipToday {
		if ie < len(list) {
			today := NowTimestamp().OnlyDate()
			lastDay := list[ie]
			if lastDay > today || lastDay > end {
				ie--
			}
		}
	} else {
		for ie >= 0 && ie < len(list) && list[ie] > end {
			ie--
		}
	}

	if is < 0 || ie < 0 || is > ie || ie >= len(list) {
		return nil
	}
	return append([]string(nil), list[is:ie+1]...)
}

func DateRange(begin, end Timestamp, skipToday bool) []Timestamp {
	// Mirror C++: operate directly on the timestamp cache (pre-market times)
	calendarRollingOnce.Do(lazyLoadCalendar)
	tradeDates := globalCalendarsTimestamp
	if len(tradeDates) == 0 {
		return nil
	}

	n := len(tradeDates)

	// lower_bound: first >= begin
	lower := sort.Search(n, func(i int) bool { return !tradeDates[i].Less(begin) })

	// upper_bound: first > end
	upper := sort.Search(n, func(i int) bool { return tradeDates[i].Greater(end) })

	if skipToday && upper < n {
		today := NowTimestamp().PreMarketTime()
		// if candidate at upper is > today or > end, step back
		if tradeDates[upper].Greater(today) || tradeDates[upper].Greater(end) {
			if upper > 0 {
				upper--
			}
		}
	} else {
		// adjust upper to the last index <= end
		for upper > 0 && tradeDates[upper-1].Greater(end) {
			upper--
		}
	}

	// validity checks (mirror C++ conditions)
	if lower >= upper || lower == n || upper == 0 {
		return nil
	}

	// return half-open [lower, upper)
	out := make([]Timestamp, upper-lower)
	copy(out, tradeDates[lower:upper])
	return out
}

// LastTradingDay returns the most recent trading day <= date
func LastTradingDay(t Timestamp) Timestamp {
	// Ensure calendar is loaded (persistent RollingOnce, parity with C++ global_calendar_once).
	calendarRollingOnce.Do(lazyLoadCalendar)
	if len(globalCalendarsTimestamp) == 0 {
		return NowTimestamp().PreMarketTime()
	}
	// Follow C++ logic: find upper_bound (first > t), then step back to <= t.
	n := len(globalCalendarsTimestamp)
	i := sort.Search(n, func(i int) bool { return globalCalendarsTimestamp[i].Greater(t) })
	if i != 0 {
		i-- // now globalCalendarsTimestamp[i] <= t
	}

	lastTimestamp := globalCalendarsTimestamp[i]
	currentTimestamp := NowTimestamp()
	// If current time is before lastTimestamp (盘前), move to previous trading day if possible
	if currentTimestamp.Less(lastTimestamp) && i != 0 {
		i--
	}
	return globalCalendarsTimestamp[i]
}
