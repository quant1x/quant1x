package std

import (
	"testing"
	"time"
)

func TestCronMakeCron(t *testing.T) {
	tests := []struct {
		expr    string
		wantErr bool
	}{
		{"* * * * * *", false},
		{"0 0 12 * * *", false},
		{"0 30 9-17 * * MON-FRI", false},
		{"*/5 * * * * *", false},
		{"1/5 * * * * *", false},
		{"invalid", true},
		{"* * * * *", true}, // 5 fields
	}

	for _, tt := range tests {
		_, err := MakeCron(tt.expr)
		if (err != nil) != tt.wantErr {
			t.Errorf("MakeCron(%q) error = %v, wantErr %v", tt.expr, err, tt.wantErr)
		}
	}
}

func TestCronNext(t *testing.T) {
	// Fixed time for testing: 2023-01-01 00:00:00 (Sunday)
	start := time.Date(2023, 1, 1, 0, 0, 0, 0, time.UTC)

	tests := []struct {
		expr     string
		expected time.Time
	}{
		{"* * * * * *", start.Add(time.Second)},
		{"0 0 12 * * *", time.Date(2023, 1, 1, 12, 0, 0, 0, time.UTC)},
		{"0 30 9 * * *", time.Date(2023, 1, 1, 9, 30, 0, 0, time.UTC)},
		{"0 0 0 2 * *", time.Date(2023, 1, 2, 0, 0, 0, 0, time.UTC)}, // Next day
		{"1/5 * * * * *", start.Add(time.Second)},
	}

	for _, tt := range tests {
		c, err := MakeCron(tt.expr)
		if err != nil {
			t.Errorf("MakeCron(%q) failed: %v", tt.expr, err)
			continue
		}
		got := c.Next(start)
		if !got.Equal(tt.expected) {
			t.Errorf("Next(%q) = %v, want %v", tt.expr, got, tt.expected)
		}
	}
}

func TestCronNextComplex(t *testing.T) {
	// 2023-01-01 is Sunday.
	start := time.Date(2023, 1, 1, 0, 0, 0, 0, time.UTC)

	// Every Monday at 10:00:00
	// 2023-01-02 is Monday.
	expr := "0 0 10 * * MON"
	expected := time.Date(2023, 1, 2, 10, 0, 0, 0, time.UTC)

	c, err := MakeCron(expr)
	if err != nil {
		t.Fatalf("MakeCron failed: %v", err)
	}

	got := c.Next(start)
	if !got.Equal(expected) {
		t.Errorf("Next(%q) = %v, want %v", expr, got, expected)
	}
}

func TestCronTraits(t *testing.T) {
	// Oracle: 0-11 months. Jan=0.
	// Standard: 1-12 months. Jan=1.

	// Test Oracle Traits
	// "0 0 0 1 0 *" -> 1st of Jan (Month 0)
	expr := "0 0 0 1 0 *"
	c, err := MakeCronWithTraits(expr, OracleTraits)
	if err != nil {
		t.Fatalf("MakeCronWithTraits failed: %v", err)
	}

	start := time.Date(2023, 1, 1, 0, 0, 0, 0, time.UTC)
	// Should match immediately if start is Jan 1st?
	// Next() always returns time > start if start matches?
	// My implementation: if !next.After(t) { next = next.Add(time.Second) }
	// So if start matches, it returns start + 1s (if seconds match).
	// Here seconds=0. start seconds=0.
	// So it will look for next occurrence.
	// Next occurrence of "0 0 0 1 0 *" is next year Jan 1st.

	// Let's start from Dec 31 2022.
	start = time.Date(2022, 12, 31, 23, 59, 59, 0, time.UTC)
	expected := time.Date(2023, 1, 1, 0, 0, 0, 0, time.UTC)

	got := c.Next(start)
	if !got.Equal(expected) {
		t.Errorf("Oracle Next(%q) = %v, want %v", expr, got, expected)
	}

	// Test Standard Traits with same expression
	// "0 0 0 1 0 *" -> Month 0 is invalid in Standard (1-12)
	_, err = MakeCronWithTraits(expr, StandardTraits)
	if err == nil {
		t.Error("MakeCronWithTraits(Standard) should fail for month 0")
	}
}

func TestCronQuestionMark(t *testing.T) {
	// ? in DOM
	expr := "0 0 12 ? * MON"
	c, err := MakeCron(expr)
	if err != nil {
		t.Fatalf("MakeCron failed: %v", err)
	}

	// Should be treated as * for DOM
	// So every Monday at 12:00
	start := time.Date(2023, 1, 1, 0, 0, 0, 0, time.UTC)     // Sunday
	expected := time.Date(2023, 1, 2, 12, 0, 0, 0, time.UTC) // Monday

	got := c.Next(start)
	if !got.Equal(expected) {
		t.Errorf("Next(%q) = %v, want %v", expr, got, expected)
	}
}
