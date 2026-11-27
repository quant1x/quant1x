// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package num

import "math"

// sequentialSum 从左到右顺序累加。
func sequentialSum(values []float64) float64 {
	total := 0.0
	for _, v := range values {
		total += v
	}
	return total
}

// hasNaN 判断序列是否含 NaN。
func hasNaN(values []float64) bool {
	for _, v := range values {
		if math.IsNaN(v) {
			return true
		}
	}
	return false
}

// block 是有效窗口的位置范围。
type block struct {
	valid bool
	begin int
	end   int
}

// blocks 枚举窗口: 完整窗口才计算。
func blocks(x []float64, window []int64) []block {
	out := make([]block, len(x))
	for i := range x {
		size := window[i]
		if size <= 0 || int(size) > i+1 {
			out[i] = block{}
			continue
		}
		begin := i + 1 - int(size)
		out[i] = block{valid: true, begin: begin, end: i + 1}
	}
	return out
}

// Rolling 返回每个位置的窗口; 无效位置为空序列。
func Rolling(x []float64, window []int64) [][]float64 {
	bs := blocks(x, window)
	out := make([][]float64, len(x))
	for i, b := range bs {
		if !b.valid {
			continue
		}
		out[i] = append([]float64(nil), x[b.begin:b.end]...)
	}
	return out
}

// RollingApply 逐窗口归约; 无效窗口或含 NaN 的窗口为 NaN。
func RollingApply(x []float64, window []int64, apply func([]float64) float64) []float64 {
	out := DefaultLike(x)
	bs := blocks(x, window)
	for i, b := range bs {
		if !b.valid {
			continue
		}
		chunk := x[b.begin:b.end]
		if hasNaN(chunk) {
			continue
		}
		out[i] = apply(chunk)
	}
	return out
}

// RollingSum 滚动求和。
func RollingSum(x []float64, window []int64) []float64 {
	return RollingApply(x, window, sequentialSum)
}

// RollingMean 滚动均值。
func RollingMean(x []float64, window []int64) []float64 {
	return RollingApply(x, window, func(b []float64) float64 {
		return sequentialSum(b) / float64(len(b))
	})
}

// RollingStd 滚动总体标准差。
func RollingStd(x []float64, window []int64) []float64 {
	return RollingApply(x, window, func(b []float64) float64 {
		count := float64(len(b))
		average := sequentialSum(b) / count
		total := 0.0
		for _, v := range b {
			d := v - average
			total += mulOnly(d, d)
		}
		return math.Sqrt(total / count)
	})
}

// RollingMin 滚动最小值。
func RollingMin(x []float64, window []int64) []float64 {
	return RollingApply(x, window, func(b []float64) float64 {
		result := b[0]
		for _, v := range b[1:] {
			if v < result {
				result = v
			}
		}
		return result
	})
}

// RollingMax 滚动最大值。
func RollingMax(x []float64, window []int64) []float64 {
	return RollingApply(x, window, func(b []float64) float64 {
		result := b[0]
		for _, v := range b[1:] {
			if v > result {
				result = v
			}
		}
		return result
	})
}

// RollingAvedev 滚动平均绝对偏差。
func RollingAvedev(x []float64, window []int64) []float64 {
	return RollingApply(x, window, func(b []float64) float64 {
		count := float64(len(b))
		average := sequentialSum(b) / count
		total := 0.0
		for _, v := range b {
			total += math.Abs(v - average)
		}
		return total / count
	})
}

// RollingHhvbars 最高值到窗口末端的周期数, 并列取最近。
func RollingHhvbars(x []float64, window []int64) []float64 {
	return RollingApply(x, window, func(b []float64) float64 {
		position := 0
		for i := 1; i < len(b); i++ {
			if b[i] >= b[position] {
				position = i
			}
		}
		return float64(len(b) - 1 - position)
	})
}

// RollingLlvbars 最低值到窗口末端的周期数, 并列取最近。
func RollingLlvbars(x []float64, window []int64) []float64 {
	return RollingApply(x, window, func(b []float64) float64 {
		position := 0
		for i := 1; i < len(b); i++ {
			if b[i] <= b[position] {
				position = i
			}
		}
		return float64(len(b) - 1 - position)
	})
}

// EwmMean 指数加权均值: y_t = (1-alpha)*y_{t-1} + alpha*x_t, 首值作种子, NaN 跳过。
func EwmMean(values []float64, alpha float64) []float64 {
	out := DefaultLike(values)
	decay := 1.0 - alpha
	previous := Nan
	seeded := false
	for i, current := range values {
		if math.IsNaN(current) {
			if seeded {
				out[i] = previous
			}
			continue
		}
		if !seeded {
			previous = current
			seeded = true
			out[i] = previous
			continue
		}
		weightedPrevious := mulOnly(decay, previous)
		weightedCurrent := mulOnly(alpha, current)
		previous = weightedPrevious + weightedCurrent
		out[i] = previous
	}
	return out
}

// Shift 向后移位: 取 N 周期前的值, 越界为 NaN。
func Shift(values []float64, periods int64) []float64 {
	out := DefaultLike(values)
	n := len(values)
	if periods == 0 {
		copy(out, values)
	} else if periods > 0 {
		offset := int(periods)
		if offset < n {
			for i := offset; i < n; i++ {
				out[i] = values[i-offset]
			}
		}
	} else {
		offset := int(-periods)
		if offset < n {
			for i := 0; i < n-offset; i++ {
				out[i] = values[i+offset]
			}
		}
	}
	return out
}

// Ref 引用 N 周期前的值。
func Ref(values []float64, periods int64) []float64 { return Shift(values, periods) }

// Diff N 周期差分。
func Diff(values []float64, periods int64) []float64 {
	lagged := Shift(values, periods)
	out := make([]float64, len(values))
	for i := range values {
		out[i] = values[i] - lagged[i]
	}
	return out
}

// Cumsum 累计和, 从左到右顺序累加。
func Cumsum(values []float64) []float64 {
	out := make([]float64, len(values))
	total := 0.0
	for i, v := range values {
		total += v
		out[i] = total
	}
	return out
}

// Sum 求和 (整段顺序累加), 空输入为 0。
func Sum(values []float64) float64 { return sequentialSum(values) }

// SumValue 求和 (与 Sum 同义)。
func SumValue(values []float64) float64 { return sequentialSum(values) }

// Fill 把 NaN 替换为指定值。
func Fill(values []float64, value float64) []float64 {
	out := make([]float64, len(values))
	for i, v := range values {
		if math.IsNaN(v) {
			out[i] = value
		} else {
			out[i] = v
		}
	}
	return out
}

// FillNa 填充缺失值 (与 Fill 同义)。
func FillNa(values []float64, value float64) []float64 { return Fill(values, value) }

// IsNa 逐元素判断是否为 NaN。
func IsNa(values []float64) []bool {
	out := make([]bool, len(values))
	for i, v := range values {
		out[i] = math.IsNaN(v)
	}
	return out
}
