// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

// Package formula 是通达信 / 同花顺公式函数, 语义以 quant1x/formula/formula.py 为准。
//
// 与 C++ formula.h、Rust formula/mod.rs 逐函数对应; 容器用 []float64 / []bool,
// 滚动与归约复用内部 quant1x/num。Go 导出名用 CamelCase, 与其它语言的 snake_case 一一对应。
package formula

import (
	"math"

	"github.com/quant1x/quant1x/quant1x/num"
)

// Series 是数值序列。
type Series = []float64

// Bools 是布尔序列。
type Bools = []bool

// Nan 缺失值。
func Nan() float64 { return math.NaN() }

// IsNaN 判断缺失。
func IsNaN(v float64) bool { return math.IsNaN(v) }

// IsTrue 判断真值。
func IsTrue(v float64) bool { return !math.IsNaN(v) && v != 0 }

// MakeSeries 构造全 NaN 序列。
func MakeSeries(n int) Series {
	out := make(Series, n)
	for i := range out {
		out[i] = math.NaN()
	}
	return out
}

// Filled 构造指定值序列。
func Filled(n int, v float64) Series {
	out := make(Series, n)
	for i := range out {
		out[i] = v
	}
	return out
}

// AsSeries 布尔转数值。
func AsSeries(values Bools) Series {
	out := make(Series, len(values))
	for i, v := range values {
		if v {
			out[i] = 1
		}
	}
	return out
}

// AsBools 数值转布尔。
func AsBools(values Series) Bools {
	out := make(Bools, len(values))
	for i, v := range values {
		out[i] = IsTrue(v)
	}
	return out
}

func rightAt(values Series, index int) float64 {
	if index < len(values) {
		return values[index]
	}
	return math.NaN()
}

// ---------------------------------------------------------------------------
// 逐元素
// ---------------------------------------------------------------------------

// Abs 绝对值。
func Abs(x Series) Series {
	out := make(Series, len(x))
	for i, v := range x {
		out[i] = math.Abs(v)
	}
	return out
}

// Sqrt 平方根。
func Sqrt(x Series) Series {
	out := make(Series, len(x))
	for i, v := range x {
		out[i] = math.Sqrt(v)
	}
	return out
}

// Log 自然对数。
func Log(x Series) Series {
	out := make(Series, len(x))
	for i, v := range x {
		out[i] = math.Log(v)
	}
	return out
}

// Pow 幂。
func Pow(x Series, exponent float64) Series {
	out := make(Series, len(x))
	for i, v := range x {
		out[i] = math.Pow(v, exponent)
	}
	return out
}

func binary(left, right Series, op func(a, b float64) float64) Series {
	out := make(Series, len(left))
	for i, v := range left {
		out[i] = op(v, rightAt(right, i))
	}
	return out
}

// Add 逐元素加。Sub 减。Mul 乘。Div 除。
func Add(l, r Series) Series { return binary(l, r, func(a, b float64) float64 { return a + b }) }
func Sub(l, r Series) Series { return binary(l, r, func(a, b float64) float64 { return a - b }) }
func Mul(l, r Series) Series { return binary(l, r, func(a, b float64) float64 { return a * b }) }
func Div(l, r Series) Series { return binary(l, r, func(a, b float64) float64 { return a / b }) }

func scalar(x Series, v float64, op func(a, b float64) float64) Series {
	out := make(Series, len(x))
	for i, item := range x {
		out[i] = op(item, v)
	}
	return out
}

// AddScalar / SubScalar / MulScalar / DivScalar 序列与标量四则。
func AddScalar(x Series, v float64) Series {
	return scalar(x, v, func(a, b float64) float64 { return a + b })
}
func SubScalar(x Series, v float64) Series {
	return scalar(x, v, func(a, b float64) float64 { return a - b })
}
func MulScalar(x Series, v float64) Series {
	return scalar(x, v, func(a, b float64) float64 { return a * b })
}
func DivScalar(x Series, v float64) Series {
	return scalar(x, v, func(a, b float64) float64 { return a / b })
}

// ScalarAdd / ScalarSub / ScalarMul / ScalarDiv 标量与序列四则。
func ScalarAdd(v float64, x Series) Series {
	return scalar(x, v, func(a, b float64) float64 { return b + a })
}
func ScalarSub(v float64, x Series) Series {
	return scalar(x, v, func(a, b float64) float64 { return b - a })
}
func ScalarMul(v float64, x Series) Series {
	return scalar(x, v, func(a, b float64) float64 { return b * a })
}
func ScalarDiv(v float64, x Series) Series {
	return scalar(x, v, func(a, b float64) float64 { return b / a })
}

// Maximum / Minimum 逐元素取极值, 任一侧 NaN 则 NaN。
func Maximum(l, r Series) Series {
	return binary(l, r, func(a, b float64) float64 {
		if math.IsNaN(a) || math.IsNaN(b) {
			return math.NaN()
		}
		return math.Max(a, b)
	})
}

// Minimum 逐元素取小。
func Minimum(l, r Series) Series {
	return binary(l, r, func(a, b float64) float64 {
		if math.IsNaN(a) || math.IsNaN(b) {
			return math.NaN()
		}
		return math.Min(a, b)
	})
}

// MaximumScalar / MinimumScalar 序列与标量取极值。
func MaximumScalar(x Series, v float64) Series { return Maximum(x, Filled(len(x), v)) }
func MinimumScalar(x Series, v float64) Series { return Minimum(x, Filled(len(x), v)) }

func compareSeries(l, r Series, op func(a, b float64) bool) Bools {
	out := make(Bools, len(l))
	for i, v := range l {
		out[i] = op(v, rightAt(r, i))
	}
	return out
}

// Greater / GreaterEqual / Less / LessEqual / Equal / NotEqual 比较。
func Greater(l, r Series) Bools { return compareSeries(l, r, func(a, b float64) bool { return a > b }) }
func GreaterEqual(l, r Series) Bools {
	return compareSeries(l, r, func(a, b float64) bool { return a >= b })
}
func Less(l, r Series) Bools { return compareSeries(l, r, func(a, b float64) bool { return a < b }) }
func LessEqual(l, r Series) Bools {
	return compareSeries(l, r, func(a, b float64) bool { return a <= b })
}
func Equal(l, r Series) Bools { return compareSeries(l, r, func(a, b float64) bool { return a == b }) }
func NotEqual(l, r Series) Bools {
	return compareSeries(l, r, func(a, b float64) bool { return a != b })
}

// GreaterScalar / GreaterEqualScalar / LessScalar / LessEqualScalar / EqualScalar 标量比较。
func GreaterScalar(x Series, v float64) Bools      { return Greater(x, Filled(len(x), v)) }
func GreaterEqualScalar(x Series, v float64) Bools { return GreaterEqual(x, Filled(len(x), v)) }
func LessScalar(x Series, v float64) Bools         { return Less(x, Filled(len(x), v)) }
func LessEqualScalar(x Series, v float64) Bools    { return LessEqual(x, Filled(len(x), v)) }
func EqualScalar(x Series, v float64) Bools        { return Equal(x, Filled(len(x), v)) }

// LogicAnd / LogicOr / LogicNot 逻辑运算。
func LogicAnd(l, r Bools) Bools {
	n := len(l)
	if len(r) > n {
		n = len(r)
	}
	out := make(Bools, n)
	for i := 0; i < n; i++ {
		out[i] = (i < len(l) && l[i]) && (i < len(r) && r[i])
	}
	return out
}

// LogicOr 逻辑或。
func LogicOr(l, r Bools) Bools {
	n := len(l)
	if len(r) > n {
		n = len(r)
	}
	out := make(Bools, n)
	for i := 0; i < n; i++ {
		out[i] = (i < len(l) && l[i]) || (i < len(r) && r[i])
	}
	return out
}

// LogicNot 逻辑非。
func LogicNot(x Bools) Bools {
	out := make(Bools, len(x))
	for i, v := range x {
		out[i] = !v
	}
	return out
}

// IfSelect 条件选择 (序列/序列)。
func IfSelect(cond Bools, t, fv Series) Series {
	out := make(Series, len(cond))
	for i := range cond {
		if cond[i] {
			out[i] = rightAt(t, i)
		} else {
			out[i] = rightAt(fv, i)
		}
	}
	return out
}

// IfSelectScalar 条件选择 (标量/标量)。
func IfSelectScalar(cond Bools, t, fv float64) Series {
	out := make(Series, len(cond))
	for i, flag := range cond {
		if flag {
			out[i] = t
		} else {
			out[i] = fv
		}
	}
	return out
}

// IfSelectFalse 条件选择 (真值序列, 假值标量)。
func IfSelectFalse(cond Bools, t Series, fv float64) Series {
	return IfSelect(cond, t, Filled(len(cond), fv))
}

// IfSelectTrue 条件选择 (真值标量, 假值序列)。
func IfSelectTrue(cond Bools, t float64, fv Series) Series {
	return IfSelect(cond, Filled(len(cond), t), fv)
}

// IfSelectBools 条件选择 (布尔/布尔)。
func IfSelectBools(cond, t, fv Bools) Bools {
	out := make(Bools, len(cond))
	for i := range cond {
		if cond[i] {
			out[i] = i < len(t) && t[i]
		} else {
			out[i] = i < len(fv) && fv[i]
		}
	}
	return out
}

// SeriesToWindow 数值序列转窗口, NaN 记为 -1, 其余四舍五入。
func SeriesToWindow(values Series) []int64 { return num.SeriesWindow(values) }

// ---------------------------------------------------------------------------
// 移位 / 差分 / 累计
// ---------------------------------------------------------------------------

// Ref 取 N 周期前的值。
func Ref(x Series, periods int64) Series {
	out := MakeSeries(len(x))
	for i := range x {
		if periods >= 0 {
			if int(periods) > i {
				continue
			}
			out[i] = x[i-int(periods)]
		} else {
			offset := int(-periods)
			if i+offset >= len(x) {
				continue
			}
			out[i] = x[i+offset]
		}
	}
	return out
}

// RefWindow 取逐位置窗口前的值。
func RefWindow(x Series, window []int64) Series {
	out := MakeSeries(len(x))
	for i := range x {
		p := window[i]
		if p >= 0 {
			if int(p) > i {
				continue
			}
			out[i] = x[i-int(p)]
		} else {
			offset := int(-p)
			if i+offset >= len(x) {
				continue
			}
			out[i] = x[i+offset]
		}
	}
	return out
}

// Diff N 周期差分。
func Diff(x Series, periods int64) Series { return Sub(x, Ref(x, periods)) }

// Cumsum 累计和。
func Cumsum(x Series) Series { return num.Cumsum(x) }

// Sum 求 N 周期累计和, 窗口序列 size<=0 表示从首个值开始累计。
func Sum(x Series, window []int64) Series {
	out := MakeSeries(len(x))
	for i := range x {
		size := window[i]
		if size > 0 {
			if int(size) > i+1 {
				continue
			}
			chunk := x[i+1-int(size) : i+1]
			if hasNaN(chunk) {
				continue
			}
			out[i] = num.Sum(chunk)
		} else {
			chunk := x[:i+1]
			if hasNaN(chunk) {
				continue
			}
			out[i] = num.Sum(chunk)
		}
	}
	return out
}

// Const 常量序列: 用最后一个值铺满。
func Const(x Series) Series {
	if len(x) == 0 {
		return Series{}
	}
	return Filled(len(x), x[len(x)-1])
}

func hasNaN(values []float64) bool {
	for _, v := range values {
		if math.IsNaN(v) {
			return true
		}
	}
	return false
}
