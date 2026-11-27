// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

// Package num 是数值规格层: 与 quant1x/num/*.py 的 Python 实现同语义的 Go 对齐实现。
//
// 口径见 quant1x/num/num.md: 一维 float64 序列、NaN 缺失、顺序累加、完整窗口、
// 总体方差、均值中心化回归、半程远离零。容器用 []float64 / []bool, 全程显式循环。
package num

import (
	"math"
	"sort"
)

// Nan 是缺失值。
var Nan = math.NaN()

// mulOnly 只做一次乘法并返回; noinline 阻止编译器把 a*b + c 收缩成 FMA,
// 保证 GOAMD64>=v3 下与 Python / C++ / Rust 逐位一致。
//
//go:noinline
func mulOnly(a, b float64) float64 { return a * b }

// IsNaN 判断缺失。
func IsNaN(value float64) bool { return math.IsNaN(value) }

// IsTrue 判断真值: 非零且非 NaN。
func IsTrue(value float64) bool { return !math.IsNaN(value) && value != 0 }

// DefaultLike 生成与输入等长的全 NaN 序列。
func DefaultLike(values []float64) []float64 {
	out := make([]float64, len(values))
	for i := range out {
		out[i] = Nan
	}
	return out
}

// ToScalar 取最后一个值; 空序列返回 NaN。
func ToScalar(values []float64) float64 {
	if len(values) == 0 {
		return Nan
	}
	return values[len(values)-1]
}

// ConstantWindow 把常量窗口展开成与序列等长的数组。
func ConstantWindow(value int64, length int) []int64 {
	out := make([]int64, length)
	for i := range out {
		out[i] = value
	}
	return out
}

// SeriesWindow 把浮点窗口序列转成 int64, NaN 记为 -1。
func SeriesWindow(values []float64) []int64 {
	out := make([]int64, len(values))
	for i, value := range values {
		if math.IsNaN(value) {
			out[i] = -1
		} else {
			out[i] = int64(math.Round(value))
		}
	}
	return out
}

// ---------------------------------------------------------------------------
// 逐元素运算
// ---------------------------------------------------------------------------

// rightAt 取二元算子右操作数的值, 越界为 NaN。
func rightAt(values []float64, index int) float64 {
	if index < len(values) {
		return values[index]
	}
	return Nan
}

// Abs 求绝对值。
func Abs(x []float64) []float64 {
	out := make([]float64, len(x))
	for i, v := range x {
		out[i] = math.Abs(v)
	}
	return out
}

// Sqrt 求平方根, 负数得 NaN。
func Sqrt(x []float64) []float64 {
	out := make([]float64, len(x))
	for i, v := range x {
		out[i] = math.Sqrt(v)
	}
	return out
}

// Log 求自然对数, 非正数得 NaN。
func Log(x []float64) []float64 {
	out := make([]float64, len(x))
	for i, v := range x {
		out[i] = math.Log(v)
	}
	return out
}

// Pow 求幂。
func Pow(x []float64, exponent float64) []float64 {
	out := make([]float64, len(x))
	for i, v := range x {
		out[i] = math.Pow(v, exponent)
	}
	return out
}

// binary 逐元素二元运算, 按第一个操作数的长度对齐。
func binary(left, right []float64, op func(a, b float64) float64) []float64 {
	out := make([]float64, len(left))
	for i, v := range left {
		out[i] = op(v, rightAt(right, i))
	}
	return out
}

// Add 逐元素加。
func Add(left, right []float64) []float64 {
	return binary(left, right, func(a, b float64) float64 { return a + b })
}

// Sub 逐元素减。
func Sub(left, right []float64) []float64 {
	return binary(left, right, func(a, b float64) float64 { return a - b })
}

// Mul 逐元素乘。
func Mul(left, right []float64) []float64 {
	return binary(left, right, func(a, b float64) float64 { return a * b })
}

// Div 逐元素除, 除零得 ±inf 或 NaN。
func Div(left, right []float64) []float64 {
	return binary(left, right, func(a, b float64) float64 { return a / b })
}

// AddScalar 序列与标量加。
func AddScalar(x []float64, value float64) []float64 {
	out := make([]float64, len(x))
	for i, v := range x {
		out[i] = v + value
	}
	return out
}

// SubScalar 序列与标量减。
func SubScalar(x []float64, value float64) []float64 {
	out := make([]float64, len(x))
	for i, v := range x {
		out[i] = v - value
	}
	return out
}

// MulScalar 序列与标量乘。
func MulScalar(x []float64, value float64) []float64 {
	out := make([]float64, len(x))
	for i, v := range x {
		out[i] = v * value
	}
	return out
}

// DivScalar 序列与标量除。
func DivScalar(x []float64, value float64) []float64 {
	out := make([]float64, len(x))
	for i, v := range x {
		out[i] = v / value
	}
	return out
}

// Maximum 逐元素取大, 任一侧 NaN 则结果 NaN。
func Maximum(left, right []float64) []float64 {
	return binary(left, right, func(a, b float64) float64 {
		if math.IsNaN(a) || math.IsNaN(b) {
			return Nan
		}
		return math.Max(a, b)
	})
}

// Minimum 逐元素取小, 任一侧 NaN 则结果 NaN。
func Minimum(left, right []float64) []float64 {
	return binary(left, right, func(a, b float64) float64 {
		if math.IsNaN(a) || math.IsNaN(b) {
			return Nan
		}
		return math.Min(a, b)
	})
}

// MaximumScalar 序列与标量取大。
func MaximumScalar(x []float64, value float64) []float64 {
	out := make([]float64, len(x))
	for i, v := range x {
		if math.IsNaN(v) {
			out[i] = Nan
		} else {
			out[i] = math.Max(v, value)
		}
	}
	return out
}

// MinimumScalar 序列与标量取小。
func MinimumScalar(x []float64, value float64) []float64 {
	out := make([]float64, len(x))
	for i, v := range x {
		if math.IsNaN(v) {
			out[i] = Nan
		} else {
			out[i] = math.Min(v, value)
		}
	}
	return out
}

// Where 条件选择: 按最长对齐, 条件缺位视为假、取值缺位补 NaN。
func Where(condition []bool, trueValue, falseValue []float64) []float64 {
	length := len(condition)
	if len(trueValue) > length {
		length = len(trueValue)
	}
	if len(falseValue) > length {
		length = len(falseValue)
	}
	out := make([]float64, length)
	for i := 0; i < length; i++ {
		flag := i < len(condition) && condition[i]
		if flag {
			out[i] = rightAt(trueValue, i)
		} else {
			out[i] = rightAt(falseValue, i)
		}
	}
	return out
}

// ---------------------------------------------------------------------------
// 逻辑与比较
// ---------------------------------------------------------------------------

func compare(left, right []float64, op func(a, b float64) bool) []bool {
	out := make([]bool, len(left))
	for i, v := range left {
		out[i] = op(v, rightAt(right, i))
	}
	return out
}

// Gt 大于。
func Gt(left, right []float64) []bool {
	return compare(left, right, func(a, b float64) bool { return a > b })
}

// Gte 大于等于。
func Gte(left, right []float64) []bool {
	return compare(left, right, func(a, b float64) bool { return a >= b })
}

// Lt 小于。
func Lt(left, right []float64) []bool {
	return compare(left, right, func(a, b float64) bool { return a < b })
}

// Lte 小于等于。
func Lte(left, right []float64) []bool {
	return compare(left, right, func(a, b float64) bool { return a <= b })
}

// Equal 相等。
func Equal(left, right []float64) []bool {
	return compare(left, right, func(a, b float64) bool { return a == b })
}

// NotEqual 不等。
func NotEqual(left, right []float64) []bool {
	return compare(left, right, func(a, b float64) bool { return a != b })
}

// ToBools 数值转布尔: 非零且非 NaN 为真。
func ToBools(values []float64) []bool {
	out := make([]bool, len(values))
	for i, v := range values {
		out[i] = IsTrue(v)
	}
	return out
}

// LogicAnd 逐元素逻辑与, 不足部分视为假。
func LogicAnd(left, right []bool) []bool {
	length := len(left)
	if len(right) > length {
		length = len(right)
	}
	out := make([]bool, length)
	for i := 0; i < length; i++ {
		lhs := i < len(left) && left[i]
		rhs := i < len(right) && right[i]
		out[i] = lhs && rhs
	}
	return out
}

// LogicOr 逐元素逻辑或, 不足部分视为假。
func LogicOr(left, right []bool) []bool {
	length := len(left)
	if len(right) > length {
		length = len(right)
	}
	out := make([]bool, length)
	for i := 0; i < length; i++ {
		lhs := i < len(left) && left[i]
		rhs := i < len(right) && right[i]
		out[i] = lhs || rhs
	}
	return out
}

// LogicNot 逐元素逻辑非。
func LogicNot(values []bool) []bool {
	out := make([]bool, len(values))
	for i, v := range values {
		out[i] = !v
	}
	return out
}

// AllOf 是否全真。
func AllOf(values []bool) bool {
	for _, v := range values {
		if !v {
			return false
		}
	}
	return true
}

// AnyOf 是否存在真值。
func AnyOf(values []bool) bool {
	for _, v := range values {
		if v {
			return true
		}
	}
	return false
}

// CountTrue 统计真值个数。
func CountTrue(values []bool) int {
	count := 0
	for _, v := range values {
		if v {
			count++
		}
	}
	return count
}

// sortFloats 复制后升序排序。
func sortFloats(values []float64) []float64 {
	out := make([]float64, len(values))
	copy(out, values)
	sort.Float64s(out)
	return out
}
