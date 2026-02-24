package meta

import (
	"net/http"
	"os"
	"testing"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/core"
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
