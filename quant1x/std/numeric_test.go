package std

import (
	"math"
	"testing"
)

func almostEqual(a, b float64) bool {
	return math.Abs(a-b) <= 1e-12
}

func TestDecimal_BasicCases(t *testing.T) {
	tests := []struct {
		name   string
		value  float64
		digits int
		want   float64
	}{
		{"positive half away", 1.25, 1, 1.3},
		{"negative half away", -1.25, 1, -1.3},
		{"round 4th -> 3rd", 1.2345, 3, 1.235},
		{"round to int up", 1.5, 0, 2.0},
		{"round to int down", 1.4999, 0, 1.0},
		{"T9.8", 9.825, 0, 10.0},
		{"T9.825", 9.825, 2, 9.83},
		{"T0.116", 0.116, 2, 0.12},
		{"T0.11", 0.1115355659035776, 2, 0.11},
		{"T-0.11", -0.1115355659035776, 2, -0.11},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := Decimal(tt.value, tt.digits)
			if !almostEqual(got, tt.want) {
				t.Fatalf("Decimal(%v, %d) = %v; want %v", tt.value, tt.digits, got, tt.want)
			}
		})
	}
}

func TestDecimal_NaNReturnsZero(t *testing.T) {
	got := Decimal(math.NaN(), 3)
	if got != 0.0 {
		t.Fatalf("Decimal(NaN, 3) = %v; want 0.0", got)
	}
}

func TestDecimal_DigitsClamping(t *testing.T) {
	v := 1.23456789012345

	// digits > 9 should be clamped to 9
	gotHigh := Decimal(v, 15)
	wantHigh := Decimal(v, 9)
	if !almostEqual(gotHigh, wantHigh) {
		t.Fatalf("Decimal(%v, 15) = %v; want Decimal(%v,9)=%v", v, gotHigh, v, wantHigh)
	}

	// digits < 0 should be clamped to 0
	gotLow := Decimal(1.5, -2)
	wantLow := Decimal(1.5, 0)
	if !almostEqual(gotLow, wantLow) {
		t.Fatalf("Decimal(1.5, -2) = %v; want Decimal(1.5,0)=%v", gotLow, wantLow)
	}
}
