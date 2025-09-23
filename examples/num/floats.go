package num

import (
	"math"
)

// Decimal 保留小数点四舍五入
func v1Decimal(value float64, digits ...int) float64 {
	// 处理精度参数
	precision := 2
	if len(digits) > 0 {
		precision = digits[0]
		if precision < 0 {
			precision = 0
		}
	}

	// 处理特殊值
	if math.IsNaN(value) {
		return math.NaN() // 或 return 0 根据业务需求选择
	}

	// 优化核心计算逻辑
	scale := math.Pow10(precision)
	return math.Round(value*scale) / scale
}

func v2Decimal(value float64, digits ...int) float64 {
	defaultDigits := 2
	if len(digits) > 0 {
		defaultDigits = digits[0]
		if defaultDigits < 0 {
			defaultDigits = 0
		}
	}
	if math.IsNaN(value) {
		value = float64(0)
	}
	half := 0.5
	if math.Signbit(value) {
		// 如果是负值, 半数用-0.5
		half = -half
	}
	n10 := math.Pow10(defaultDigits)
	return math.Trunc((value+half/n10)*n10) / n10
}

func Decimal(value float64, digits ...int) float64 {
	defaultDigits := 2
	if len(digits) > 0 {
		defaultDigits = digits[0]
		if defaultDigits < 0 {
			defaultDigits = 0
		}
	}
	if math.IsNaN(value) {
		value = float64(0)
	}
	half := float64(5)
	if math.Signbit(value) {
		// 如果是负值, 半数用-0.5
		half = -half
	}
	n10 := math.Pow10(defaultDigits)
	nj1 := math.Pow10(defaultDigits + 1)
	return math.Trunc((value*nj1+half)/10) / n10
}
