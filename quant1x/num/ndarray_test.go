// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package num

import (
	"math"
	"testing"
)

func approxNd(t *testing.T, got, want float64) {
	t.Helper()
	if math.IsNaN(got) && math.IsNaN(want) {
		return
	}
	if math.Abs(got-want) > 1e-12 {
		t.Errorf("期望 %v, 实际 %v", want, got)
	}
}

func approxSlice(t *testing.T, got, want []float64) {
	t.Helper()
	if len(got) != len(want) {
		t.Fatalf("长度 %d vs %d", len(got), len(want))
	}
	for i := range want {
		approxNd(t, got[i], want[i])
	}
}

// TestNdArrayConstruct 覆盖构造、属性、切片与类型转换。
func TestNdArrayConstruct(t *testing.T) {
	a := NewNdArray([]float64{1, 2, 3, 4, 5})
	if a.Dtype() != "float64" || a.Len() != 5 || a.IsEmpty() {
		t.Fatalf("属性错误: %s %d %v", a.Dtype(), a.Len(), a.IsEmpty())
	}

	part := a.Slice(1, 3)
	approxSlice(t, part.ToF64(), []float64{2, 3})

	flags := a.GtScalar(3)
	if flags.Dtype() != "bool" {
		t.Fatalf("比较结果应为 bool, 实际 %s", flags.Dtype())
	}
	floats, err := flags.Astype("float64")
	if err != nil {
		t.Fatal(err)
	}
	approxSlice(t, floats.ToF64(), []float64{0, 0, 0, 1, 1})

	if _, err := flags.Astype("int64"); err == nil {
		t.Fatal("不支持的 dtype 应报错")
	}

	approxSlice(t, NdArrayZeros(3).ToF64(), []float64{0, 0, 0})
	approxSlice(t, NdArrayOnes(3).ToF64(), []float64{1, 1, 1})
	approxSlice(t, NdArrayFull(2, 7).ToF64(), []float64{7, 7})
	approxSlice(t, NdArrayArange(0, 5, 1).ToF64(), []float64{0, 1, 2, 3, 4})
	approxSlice(t, NdArrayLinspace(0, 1, 3).ToF64(), []float64{0, 0.5, 1})

	value, ok := a.GetF64(2)
	if !ok || value != 3 {
		t.Fatalf("GetF64 错误: %v %v", value, ok)
	}
	if err := a.SetF64(0, 9); err != nil {
		t.Fatal(err)
	}
	approxNd(t, a.ToF64()[0], 9)
}

// TestNdArrayArithmetic 覆盖四则、极值与一元数学。
func TestNdArrayArithmetic(t *testing.T) {
	left := NewNdArray([]float64{1, 2, 3})
	right := NewNdArray([]float64{4, 5, 6})

	approxSlice(t, left.Add(right).ToF64(), []float64{5, 7, 9})
	approxSlice(t, left.Sub(right).ToF64(), []float64{-3, -3, -3})
	approxSlice(t, left.Mul(right).ToF64(), []float64{4, 10, 18})
	approxSlice(t, left.Div(right).ToF64(), []float64{0.25, 0.4, 0.5})

	approxSlice(t, left.AddScalar(1).ToF64(), []float64{2, 3, 4})
	approxSlice(t, left.MulScalar(2).ToF64(), []float64{2, 4, 6})
	approxSlice(t, left.Maximum(NdArrayFull(3, 2.5)).ToF64(), []float64{2.5, 2.5, 3})
	approxSlice(t, left.Minimum(NdArrayFull(3, 2.5)).ToF64(), []float64{1, 2, 2.5})

	approxSlice(t, NewNdArray([]float64{-1, 4}).Abs().ToF64(), []float64{1, 4})
	approxSlice(t, NewNdArray([]float64{4, 9}).Sqrt().ToF64(), []float64{2, 3})
	approxNd(t, NewNdArray([]float64{2}).Pow(3).ToF64()[0], 8)
	approxSlice(t, left.Neg().ToF64(), []float64{-1, -2, -3})
}

// TestNdArrayLogic 覆盖比较与逻辑。
func TestNdArrayLogic(t *testing.T) {
	a := NewNdArray([]float64{1, 2, 3})
	b := NewNdArray([]float64{2, 2, 2})

	if !sameBools(a.Gt(b).ToBools(), []bool{false, false, true}) {
		t.Error("Gt 错误")
	}
	if !sameBools(a.Gte(b).ToBools(), []bool{false, true, true}) {
		t.Error("Gte 错误")
	}
	if !sameBools(a.Equal(b).ToBools(), []bool{false, true, false}) {
		t.Error("Equal 错误")
	}
	if !sameBools(a.NotEqual(b).ToBools(), []bool{true, false, true}) {
		t.Error("NotEqual 错误")
	}

	left := NewNdArrayBools([]bool{true, true, false})
	right := NewNdArrayBools([]bool{true, false, false})
	if !sameBools(left.LogicAnd(right).ToBools(), []bool{true, false, false}) {
		t.Error("LogicAnd 错误")
	}
	if !sameBools(left.LogicOr(right).ToBools(), []bool{true, true, false}) {
		t.Error("LogicOr 错误")
	}
	if !sameBools(left.LogicNot().ToBools(), []bool{false, false, true}) {
		t.Error("LogicNot 错误")
	}
}

// TestNdArrayReduce 覆盖归约。
func TestNdArrayReduce(t *testing.T) {
	a := NewNdArray([]float64{1, 2, 3, 4})
	approxNd(t, a.Sum(), 10)
	approxNd(t, a.Mean(), 2.5)
	approxNd(t, a.Median(), 2.5)
	approxNd(t, a.MinValue(), 1)
	approxNd(t, a.MaxValue(), 4)
	if a.Argmin() != 0 || a.Argmax() != 3 {
		t.Errorf("极值下标错误: %d %d", a.Argmin(), a.Argmax())
	}
	approxNd(t, a.Variance(), 1.25)
	approxNd(t, a.Stddev(), math.Sqrt(1.25))
	approxSlice(t, a.Cumsum().ToF64(), []float64{1, 3, 6, 10})

	empty := NewNdArray(nil)
	if !math.IsNaN(empty.Sum()) && empty.Sum() != 0 {
		t.Errorf("空数组求和口径异常: %v", empty.Sum())
	}
	if empty.Argmin() != -1 {
		t.Errorf("空数组 argmin 应为 -1, 实际 %d", empty.Argmin())
	}
}

// TestNdArrayShiftRolling 覆盖移位、差分与滚动、EWM。
func TestNdArrayShiftRolling(t *testing.T) {
	a := NewNdArray([]float64{1, 2, 3})

	shifted := a.Shift(1).ToF64()
	if !math.IsNaN(shifted[0]) {
		t.Errorf("移位越界应为 NaN, 实际 %v", shifted[0])
	}
	approxSlice(t, shifted[1:], []float64{1, 2})

	approxSlice(t, a.Diff(1).ToF64()[1:], []float64{1, 1})
	approxSlice(t, NewNdArray([]float64{1, Nan, 3}).FillNa(0).ToF64(), []float64{1, 0, 3})

	series := NewNdArray([]float64{1, 2, 3, 4})
	sums := series.RollingSum(2).ToF64()
	if !math.IsNaN(sums[0]) {
		t.Errorf("窗口不足应为 NaN, 实际 %v", sums[0])
	}
	approxSlice(t, sums[1:], []float64{3, 5, 7})
	approxSlice(t, series.RollingMean(2).ToF64()[1:], []float64{1.5, 2.5, 3.5})
	approxNd(t, series.RollingMax(2).ToF64()[3], 4)
	approxNd(t, series.RollingMin(2).ToF64()[3], 3)

	ewm := NewNdArray([]float64{1, 2, 3, 4}).EwmMean(0.3).ToF64()
	approxNd(t, ewm[0], 1)
	if ewm[3] <= ewm[2] {
		t.Errorf("EWM 应单调上升: %v", ewm)
	}
}

func sameBools(got, want []bool) bool {
	if len(got) != len(want) {
		return false
	}
	for i := range want {
		if got[i] != want[i] {
			return false
		}
	}
	return true
}
