package std

import "math"

// Decimal rounds a floating-point number to the specified number of decimal
// places using the same branchless algorithm as the C++ `numerics::decimal`.
// digits is clamped to [0,9]. NaN returns 0.0.
func Decimal(value float64, digits int) float64 {
	if math.IsNaN(value) {
		return 0.0
	}
	if digits < 0 {
		digits = 0
	}
	if digits > 9 {
		digits = 9
	}

	kPowersOf10 := [...]float64{1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10}

	half := math.Copysign(5.0, value)
	nj1 := kPowersOf10[digits+1]
	scaled := value*nj1 + half
	truncated := math.Trunc(scaled / 10.0)
	return truncated / (nj1 / 10.0)
}
