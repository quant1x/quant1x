package meta

import (
	"net/http"
	"os"
	"testing"
	"time"

	"github.com/quant1x/quant1x/quant1x/core"
)

// TestCalendarMarkerReal calls updateCalendar() against the real sina URL and
// verifies the calendar.updated marker is created with a recent mtime.
func TestCalendarMarkerReal(t *testing.T) {
	meta := core.GetMetaPath()
	if err := os.MkdirAll(meta, 0o755); err != nil {
		t.Fatalf("mkdir meta: %v", err)
	}

	marker := calendarMarkerFilename()
	_ = os.Remove(marker)
	// remove existing cache to force a fresh download (avoid conditional GET 304)
	_ = os.Remove(CalendarFilename())

	if err := updateCalendar(); err != nil {
		t.Fatalf("updateCalendar failed: %v", err)
	}

	fi, err := os.Stat(marker)
	if err != nil {
		t.Fatalf("marker file missing: %v", err)
	}

	// mtime should be recent (within 48 hours)
	if time.Since(fi.ModTime()) > 48*time.Hour {
		t.Fatalf("marker mtime too old: %v", fi.ModTime())
	}

	// verify cache mtime comes from remote Last-Modified
	// fetch HEAD to get Last-Modified header
	resp, err := http.Head("https://finance.sina.com.cn/realstock/company/klc_td_sh.txt")
	if err == nil {
		lm := resp.Header.Get("Last-Modified")
		t.Logf("remote Last-Modified: %s", lm)
		if lm != "" {
			if rt, err := http.ParseTime(lm); err == nil {
				// compare integer seconds to avoid sub-second filesystem differences
				cfi, err := os.Stat(CalendarFilename())
				if err == nil {
					cacheSecs := cfi.ModTime().Unix()
					remoteSecs := rt.Unix()
					t.Logf("cache mtime: %v -> %v", cacheSecs, time.Unix(cacheSecs, 0))
					t.Logf("remote mtime: %v -> %v", remoteSecs, time.Unix(remoteSecs, 0))
					if cacheSecs != remoteSecs {
						t.Fatalf("cache mtime (%v) != remote Last-Modified (%v)", cacheSecs, remoteSecs)
					}
				}
			}
		}
	} else {
		t.Logf("failed to fetch remote headers: %v", err)
	}

	// cleanup
	//_ = os.Remove(marker)
	//_ = os.Remove(filepath.Join(meta, "calendar"))
}

func TestTradingDayQueries(t *testing.T) {
	date := func(day int) Timestamp {
		return NewTimestampFromTime(time.Date(2026, time.November, day, PreMarketHour, 0, 0, 0, time.Local))
	}
	dates := []Timestamp{date(2), date(3), date(5)}
	afterSecond := dates[1].Offset(1, 0, 0, 0)
	beforeSecond := dates[1].Offset(-1, 0, 0, 0)

	tests := []struct {
		name string
		got  Timestamp
		want Timestamp
	}{
		{"last after open", lastTradingDay(dates, dates[1], afterSecond), dates[1]},
		{"last before open", lastTradingDay(dates, dates[1], beforeSecond), dates[0]},
		{"previous exact day", prevTradingDay(dates, dates[2], afterSecond), dates[1]},
		{"next before open", nextTradingDay(dates, dates[1], beforeSecond), dates[1]},
		{"next after open", nextTradingDay(dates, dates[1], afterSecond), dates[2]},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if tt.got.Value() != tt.want.Value() {
				t.Fatalf("got %s, want %s", tt.got, tt.want)
			}
		})
	}
}

func TestTradingDateRangesAreInclusive(t *testing.T) {
	date := func(day int) Timestamp {
		return NewTimestampFromTime(time.Date(2026, time.November, day, PreMarketHour, 0, 0, 0, time.Local))
	}
	timestamps := []Timestamp{date(2), date(3), date(5)}
	got := tradingDateRange(timestamps, timestamps[0], timestamps[2], false, timestamps[1])
	if len(got) != 3 {
		t.Fatalf("inclusive date range returned %d dates, want 3", len(got))
	}
	got = tradingDateRange(timestamps, timestamps[0], timestamps[2], true, timestamps[1])
	if len(got) != 2 || got[1].Value() != timestamps[1].Value() {
		t.Fatalf("skip-today range = %v, want first two dates", got)
	}

	strings := []string{"2026-11-02", "2026-11-03", "2026-11-05"}
	stringRange := tradingDateStringRange(strings, strings[0], strings[2], false, strings[1])
	if len(stringRange) != 3 || stringRange[2] != strings[2] {
		t.Fatalf("inclusive string range = %v, want all dates", stringRange)
	}
}

func TestFixTradeDateUsesPythonFormat(t *testing.T) {
	got, err := FixTradeDate("2026-11-03", "%Y%m%d")
	if err != nil {
		t.Fatalf("FixTradeDate returned an error: %v", err)
	}
	if got != "20261103" {
		t.Fatalf("FixTradeDate = %q, want %q", got, "20261103")
	}
}
