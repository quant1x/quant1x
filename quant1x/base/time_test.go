package base

import (
	"testing"
	"time"
)

func TestZoneOffsetMilliseconds(t *testing.T) {
	offset := ZoneOffsetMilliseconds()
	_, zoneOffset := time.Now().Zone()
	expected := int64(zoneOffset) * 1000
	if offset != expected {
		t.Errorf("ZoneOffsetMilliseconds() = %v, want %v", offset, expected)
	}
}

func TestParseDate(t *testing.T) {
	tests := []struct {
		input    string
		expected int64 // We might not be able to match exact int64 due to timezone, so we'll check if it's non-zero for valid inputs
		isValid  bool
	}{
		{"2023-05-15 14:30:00", 0, true},
		{"2023-05-15", 0, true},
		{"20230515", 0, true},
		{"invalid", 0, false},
		{"", 0, false},
	}

	for _, tt := range tests {
		got := ParseDate(tt.input)
		if tt.isValid && got == 0 {
			t.Errorf("ParseDate(%q) = 0, want non-zero", tt.input)
		}
		if !tt.isValid && got != 0 {
			t.Errorf("ParseDate(%q) = %v, want 0", tt.input, got)
		}
	}
}

func TestParseTime(t *testing.T) {
	tests := []struct {
		input   string
		isValid bool
	}{
		{"14:30:00", true},
		{"2023-05-15 14:30:00", true},
		{"invalid", false},
		{"", false},
	}

	for _, tt := range tests {
		got := ParseTime(tt.input)
		if tt.isValid && got == 0 {
			t.Errorf("ParseTime(%q) = 0, want non-zero", tt.input)
		}
		if !tt.isValid && got != 0 {
			t.Errorf("ParseTime(%q) = %v, want 0", tt.input, got)
		}
	}
}

func TestMsUtcToLocalAndBack(t *testing.T) {
	now := time.Now().UnixNano() / int64(time.Millisecond)
	local := MsUtcToLocal(now)
	utc := MsLocalToUtc(local)

	if utc != now {
		t.Errorf("MsLocalToUtc(MsUtcToLocal(%v)) = %v, want %v", now, utc, now)
	}
}

func TestFromLocal(t *testing.T) {
	// This is a bit tricky to test exactly without duplicating logic,
	// but we can check round trip consistency or specific known values if we fix timezone.
	// For now, let's just ensure it returns a valid time.
	ms := int64(1684132200000) // 2023-05-15 14:30:00 UTC approx
	tm := FromLocal(ms)
	if tm.IsZero() {
		t.Error("FromLocal returned zero time")
	}
}

func TestFromTimePoint(t *testing.T) {
	now := time.Now()
	ms := FromTimePoint(now)
	// FromTimePoint returns local milliseconds.
	// Let's convert it back to UTC ms and compare with now.UnixMilli()
	utcMs := MsLocalToUtc(ms)

	// Allow small difference due to precision loss (nanoseconds to milliseconds)
	diff := utcMs - now.UnixMilli()
	if diff < -1 || diff > 1 {
		t.Errorf("FromTimePoint round trip diff = %v, want 0", diff)
	}
}

func TestToday(t *testing.T) {
	got := Today()
	expected := time.Now().Format(LayoutOnlyDate)
	if got != expected {
		t.Errorf("Today() = %v, want %v", got, expected)
	}
}

func TestGetTimestamp(t *testing.T) {
	got := GetTimestamp()
	if len(got) == 0 {
		t.Error("GetTimestamp() returned empty string")
	}
	// Basic format check
	_, err := time.ParseInLocation(LayoutDateTime, got, time.Local)
	if err != nil {
		t.Errorf("GetTimestamp() returned invalid format: %v", got)
	}
}

func TestTimeToString(t *testing.T) {
	now := time.Now()

	// Test default format (YYYY-MM-DD)
	got := TimeToString(now)
	expected := now.Format(LayoutOnlyDate)
	if got != expected {
		t.Errorf("TimeToString() default = %v, want %v", got, expected)
	}

	// Test with specific format
	gotFull := TimeToString(now, LayoutDateTime)
	expectedFull := now.Format(LayoutDateTime)
	if gotFull != expectedFull {
		t.Errorf("TimeToString(full) = %v, want %v", gotFull, expectedFull)
	}
}

func TestGetQuarterDay(t *testing.T) {
	// Test with 0 months (current quarter)
	// We can't easily predict exact output without mocking time,
	// but we can check the format and basic logic.

	// Let's try to verify logic by manually calculating for a fixed date if we could inject time,
	// but since we can't, we'll check format.
	start, end := GetQuarterDay(0)

	if len(start) != 19 || len(end) != 19 {
		t.Errorf("GetQuarterDay(0) returned invalid length strings: %q, %q", start, end)
	}

	// Check if end is after start
	if start >= end {
		t.Errorf("GetQuarterDay(0) start %q should be before end %q", start, end)
	}
}

func TestGetQuarterByDate(t *testing.T) {
	tests := []struct {
		dateStr     string
		diff        int
		wantQuarter string
		wantStart   string
		wantEnd     string
	}{
		{
			dateStr:     "2023-05-15",
			diff:        0,
			wantQuarter: "2023Q2",
			wantStart:   "2023-04-01 00:00:00",
			wantEnd:     "2023-06-30 23:59:59",
		},
		{
			dateStr:     "2023-01-15",
			diff:        0,
			wantQuarter: "2023Q1",
			wantStart:   "2023-01-01 00:00:00",
			wantEnd:     "2023-03-31 23:59:59",
		},
		{
			dateStr:     "2023-05-15",
			diff:        1, // 1 quarter ago -> 2023Q1
			wantQuarter: "2023Q1",
			wantStart:   "2023-01-01 00:00:00",
			wantEnd:     "2023-03-31 23:59:59",
		},
		{
			dateStr:     "2023-01-15",
			diff:        1, // 1 quarter ago -> 2022Q4
			wantQuarter: "2022Q4",
			wantStart:   "2022-10-01 00:00:00",
			wantEnd:     "2022-12-31 23:59:59",
		},
	}

	for _, tt := range tests {
		q, s, e := GetQuarterByDate(tt.dateStr, tt.diff)
		if q != tt.wantQuarter {
			t.Errorf("GetQuarterByDate(%q, %d) quarter = %v, want %v", tt.dateStr, tt.diff, q, tt.wantQuarter)
		}
		if s != tt.wantStart {
			t.Errorf("GetQuarterByDate(%q, %d) start = %v, want %v", tt.dateStr, tt.diff, s, tt.wantStart)
		}
		if e != tt.wantEnd {
			t.Errorf("GetQuarterByDate(%q, %d) end = %v, want %v", tt.dateStr, tt.diff, e, tt.wantEnd)
		}
	}
}
