// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package num

import (
	"math"
	"sort"
)

// Mean 均值; 空输入为 NaN。
func Mean(values []float64) float64 {
	if len(values) == 0 {
		return Nan
	}
	return sequentialSum(values) / float64(len(values))
}

// isReducible 判断序列非空且不含 NaN。
func isReducible(values []float64) bool {
	return len(values) > 0 && !hasNaN(values)
}

// Median 中位数; 空输入或含 NaN 为 NaN。
func Median(values []float64) float64 {
	if !isReducible(values) {
		return Nan
	}
	ordered := sortFloats(values)
	middle := len(ordered) / 2
	if len(ordered)%2 == 0 {
		return (ordered[middle-1] + ordered[middle]) / 2.0
	}
	return ordered[middle]
}

// MinValue 最小值; 空输入或含 NaN 为 NaN。
func MinValue(values []float64) float64 {
	if !isReducible(values) {
		return Nan
	}
	result := math.Inf(1)
	for _, v := range values {
		if v < result {
			result = v
		}
	}
	return result
}

// MaxValue 最大值; 空输入或含 NaN 为 NaN。
func MaxValue(values []float64) float64 {
	if !isReducible(values) {
		return Nan
	}
	result := math.Inf(-1)
	for _, v := range values {
		if v > result {
			result = v
		}
	}
	return result
}

// Argmin 最小值下标; 空输入或含 NaN 为 -1, 并列取第一个。
func Argmin(values []float64) int64 {
	if !isReducible(values) {
		return -1
	}
	best := 0
	for i := 1; i < len(values); i++ {
		if values[i] < values[best] {
			best = i
		}
	}
	return int64(best)
}

// Argmax 最大值下标; 空输入或含 NaN 为 -1, 并列取第一个。
func Argmax(values []float64) int64 {
	if !isReducible(values) {
		return -1
	}
	best := 0
	for i := 1; i < len(values); i++ {
		if values[i] > values[best] {
			best = i
		}
	}
	return int64(best)
}

// Variance 总体方差; 空输入为 0。
func Variance(values []float64) float64 {
	if len(values) == 0 {
		return 0
	}
	average := Mean(values)
	total := 0.0
	for _, v := range values {
		d := v - average
		total += mulOnly(d, d)
	}
	return total / float64(len(values))
}

// Stddev 总体标准差; 空输入为 0。
func Stddev(values []float64) float64 { return math.Sqrt(Variance(values)) }

// Covariance 总体协方差; 长度不一致或空输入为 0。
func Covariance(left, right []float64) float64 {
	if len(left) != len(right) || len(left) == 0 {
		return 0
	}
	meanLeft := Mean(left)
	meanRight := Mean(right)
	total := 0.0
	for i := range left {
		total += mulOnly(left[i]-meanLeft, right[i]-meanRight)
	}
	return total / float64(len(left))
}

// ---------------------------------------------------------------------------
// 一元线性回归
// ---------------------------------------------------------------------------

// LinearRegression 返回 y = slope * t + intercept 的斜率与截距; 不足两点或含 NaN 为 NaN。
func LinearRegression(values []float64) (float64, float64) {
	count := len(values)
	if count < 2 || hasNaN(values) {
		return Nan, Nan
	}
	meanT := float64(count-1) / 2.0
	meanY := sequentialSum(values) / float64(count)
	numerator := 0.0
	denominator := 0.0
	for i, v := range values {
		offset := float64(i) - meanT
		numerator += mulOnly(offset, v-meanY)
		denominator += mulOnly(offset, offset)
	}
	slope := numerator / denominator
	intercept := meanY - slope*meanT
	return slope, intercept
}

// Slope 回归斜率。
func Slope(values []float64) float64 {
	slope, _ := LinearRegression(values)
	return slope
}

// Forcast 回归线在最后一个点上的预测值。
func Forcast(values []float64) float64 {
	slope, intercept := LinearRegression(values)
	if math.IsNaN(slope) {
		return Nan
	}
	return intercept + slope*float64(len(values)-1)
}

// ---------------------------------------------------------------------------
// 小数位与变化率
// ---------------------------------------------------------------------------

// Decimal 单值保留小数位 (半程远离零, NaN 按 0 参与舍入)。
func Decimal(value float64, digits int) float64 {
	places := digits
	if places < 0 {
		places = 0
	}
	input := value
	if math.IsNaN(input) {
		input = 0
	}
	half := 5.0
	if math.Signbit(input) {
		half = -5.0
	}
	scale := math.Pow(10, float64(places))
	wider := math.Pow(10, float64(places+1))
	return math.Trunc((input*wider+half)/10.0) / scale
}

// DecimalSeries 对序列逐元素保留小数位。
func DecimalSeries(values []float64, digits int) []float64 {
	out := make([]float64, len(values))
	for i, v := range values {
		out[i] = Decimal(v, digits)
	}
	return out
}

// ChangeRate 变化率 current / base。
func ChangeRate(base, current float64) float64 { return current / base }

// NetChangeRate 净变化率百分比 (current / base - 1) * 100。
func NetChangeRate(base, current float64) float64 { return (ChangeRate(base, current) - 1.0) * 100.0 }

// ---------------------------------------------------------------------------
// 对齐与重复
//
// 跨语言参考算法: Python spec 自身不调用 align / padTo / repeat, 应用层请用
// quant1x/pandas 的容器对齐; 这里只为与 Python 的对齐口径逐位对照而存在.
// ---------------------------------------------------------------------------

// Align 对齐到指定长度: 超长截断, 不足补 fillValue。
func Align(values []float64, length int, fillValue float64) []float64 {
	out := make([]float64, length)
	for i := range out {
		out[i] = fillValue
	}
	count := len(values)
	if count > length {
		count = length
	}
	copy(out, values[:count])
	return out
}

// PadTo 补齐到指定长度 (与 Align 同义)。
func PadTo(values []float64, length int, fillValue float64) []float64 {
	return Align(values, length, fillValue)
}

// Repeat 平铺到指定长度: 序列循环重复, 空序列补 NaN。
func Repeat(values []float64, length int) []float64 {
	out := make([]float64, length)
	for i := range out {
		out[i] = Nan
	}
	if len(values) == 0 {
		return out
	}
	for i := 0; i < length; i++ {
		out[i] = values[i%len(values)]
	}
	return out
}

// ---------------------------------------------------------------------------
// 形状与构造
// ---------------------------------------------------------------------------

// Shape 形状 (一维固定为 {length})。
func Shape(values []float64) []int { return []int{len(values)} }

// Concat1d 一维拼接。
func Concat1d(parts ...[]float64) []float64 {
	out := make([]float64, 0)
	for _, part := range parts {
		out = append(out, part...)
	}
	return out
}

// Transpose2d 二维转置; 非矩形输入按最短行截断。
func Transpose2d(values [][]float64) [][]float64 {
	if len(values) == 0 {
		return nil
	}
	columns := len(values[0])
	for _, row := range values {
		if len(row) < columns {
			columns = len(row)
		}
	}
	out := make([][]float64, columns)
	for column := 0; column < columns; column++ {
		row := make([]float64, len(values))
		for i, value := range values {
			row[i] = value[column]
		}
		out[column] = row
	}
	return out
}

// Zeros 全零序列。
func Zeros(length int) []float64 { return make([]float64, length) }

// Ones 全一序列。
func Ones(length int) []float64 {
	out := make([]float64, length)
	for i := range out {
		out[i] = 1
	}
	return out
}

// Arange 等差序列 [start, stop)。
func Arange(start, stop, step float64) []float64 {
	out := make([]float64, 0)
	if step == 0 {
		return out
	}
	value := start
	if step > 0 {
		for value < stop {
			out = append(out, value)
			value += step
		}
	} else {
		for value > stop {
			out = append(out, value)
			value += step
		}
	}
	return out
}

// Linspace 等分序列, 含两端。
func Linspace(start, stop float64, count int) []float64 {
	if count <= 0 {
		return []float64{}
	}
	if count == 1 {
		return []float64{start}
	}
	out := make([]float64, count)
	step := (stop - start) / float64(count-1)
	for i := 0; i < count; i++ {
		out[i] = start + step*float64(i)
	}
	return out
}

var _ = sort.Float64s
