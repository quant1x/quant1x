package pandas

import (
	"math"
	"testing"

	"github.com/quant1x/quant1x/quant1x/num"
)

func approxSeries(t *testing.T, got []float64, want ...float64) {
	t.Helper()
	if len(got) != len(want) {
		t.Fatalf("长度 %d vs %d", len(got), len(want))
	}
	for i := range want {
		if math.IsNaN(got[i]) && math.IsNaN(want[i]) {
			continue
		}
		if math.Abs(got[i]-want[i]) > 1e-12 {
			t.Errorf("[%d] 期望 %v, 实际 %v", i, want[i], got[i])
		}
	}
}

func approxSeriesValue(t *testing.T, got, want float64) {
	t.Helper()
	if math.IsNaN(got) && math.IsNaN(want) {
		return
	}
	if math.Abs(got-want) > 1e-12 {
		t.Errorf("期望 %v, 实际 %v", want, got)
	}
}

// TestSeriesBasic 覆盖构造、属性、取值与切片。
func TestSeriesBasic(t *testing.T) {
	s := SeriesWithName("close", []float64{1, 2, 3, 4})
	if s.Name() != "close" || s.Len() != 4 || s.Dtype() != "float64" {
		t.Fatalf("属性错误: %q %d %s", s.Name(), s.Len(), s.Dtype())
	}

	value, ok := s.GetF64(2)
	if !ok || value != 3 {
		t.Fatalf("GetF64 错误: %v %v", value, ok)
	}
	if err := s.SetF64(0, 9); err != nil {
		t.Fatal(err)
	}
	if v, _ := s.GetF64(0); v != 9 {
		t.Errorf("SetF64 未生效: %v", v)
	}

	part := s.Slice(1, 3)
	if part.Name() != "close" {
		t.Errorf("切片应保留列名: %q", part.Name())
	}
	approxSeries(t, part.Values(), 2, 3)

	flags := s.GtScalar(2)
	if !flags.IsBool() || flags.Dtype() != "bool" {
		t.Fatal("比较结果应为布尔列")
	}
	if got := flags.Bools(); len(got) != 4 || !got[0] || got[1] || !got[2] || !got[3] {
		t.Errorf("GtScalar 错误: %v", got)
	}
}

// TestSeriesArithmetic 覆盖四则与标量。
func TestSeriesArithmetic(t *testing.T) {
	a := NewSeries([]float64{1, 2, 3})
	b := NewSeries([]float64{4, 5, 6})

	approxSeries(t, a.Add(b).Values(), 5, 7, 9)
	approxSeries(t, a.Sub(b).Values(), -3, -3, -3)
	approxSeries(t, a.Mul(b).Values(), 4, 10, 18)
	approxSeries(t, b.Div(a).Values(), 4, 2.5, 2)
	approxSeries(t, a.AddScalar(1).Values(), 2, 3, 4)
	approxSeries(t, a.MulScalar(2).Values(), 2, 4, 6)
}

// TestSeriesLogic 覆盖比较与逻辑。
func TestSeriesLogic(t *testing.T) {
	a := NewSeries([]float64{1, 2, 3})
	b := NewSeries([]float64{2, 2, 2})

	if got := a.Gt(b).Bools(); got[0] || got[1] || !got[2] {
		t.Errorf("Gt 错误: %v", got)
	}
	if got := a.Equal(b).Bools(); got[0] || !got[1] || got[2] {
		t.Errorf("Equal 错误: %v", got)
	}

	left := NewSeriesBools([]bool{true, true, false})
	right := NewSeriesBools([]bool{true, false, false})
	if got := left.And(right).Bools(); !got[0] || got[1] || got[2] {
		t.Errorf("And 错误: %v", got)
	}
	if got := left.Or(right).Bools(); !got[0] || !got[1] || got[2] {
		t.Errorf("Or 错误: %v", got)
	}
	if got := left.Not().Bools(); got[0] || got[1] || !got[2] {
		t.Errorf("Not 错误: %v", got)
	}
}

// TestSeriesReduce 覆盖归约。
func TestSeriesReduce(t *testing.T) {
	s := NewSeries([]float64{1, 2, 3, 4})
	approxSeriesValue(t, s.Sum(), 10)
	approxSeriesValue(t, s.Mean(), 2.5)
	approxSeriesValue(t, s.Median(), 2.5)
	approxSeriesValue(t, s.MinValue(), 1)
	approxSeriesValue(t, s.MaxValue(), 4)
	if s.Argmin() != 0 || s.Argmax() != 3 {
		t.Errorf("极值下标错误: %d %d", s.Argmin(), s.Argmax())
	}
	approxSeriesValue(t, s.Variance(), 1.25)
	approxSeriesValue(t, s.Stddev(), math.Sqrt(1.25))
	approxSeries(t, s.Cumsum().Values(), 1, 3, 6, 10)
}

// TestSeriesShiftRolling 覆盖移位、填充、滚动与 EWM。
func TestSeriesShiftRolling(t *testing.T) {
	s := NewSeries([]float64{1, 2, 3})

	shifted := s.Shift(1).Values()
	if !math.IsNaN(shifted[0]) {
		t.Errorf("移位越界应为 NaN, 实际 %v", shifted[0])
	}
	approxSeries(t, shifted[1:], 1, 2)
	approxSeries(t, s.Diff(1).Values()[1:], 1, 1)
	approxSeries(t, NewSeries([]float64{1, num.Nan, 3}).FillNa(0).Values(), 1, 0, 3)

	series := NewSeries([]float64{1, 2, 3, 4})
	sums := series.Rolling(2).Sum().Values()
	if !math.IsNaN(sums[0]) {
		t.Errorf("窗口不足应为 NaN, 实际 %v", sums[0])
	}
	approxSeries(t, sums[1:], 3, 5, 7)
	approxSeries(t, series.Rolling(2).Mean().Values()[1:], 1.5, 2.5, 3.5)
	approxSeriesValue(t, series.Rolling(2).MaxValue().Values()[3], 4)
	approxSeriesValue(t, series.Rolling(2).MinValue().Values()[3], 3)

	ewm := series.Ewm(0.3).Mean().Values()
	approxSeriesValue(t, ewm[0], 1)
	if ewm[3] <= ewm[2] {
		t.Errorf("EWM 应单调上升: %v", ewm)
	}
	span := series.EwmSpan(3).Mean().Values()
	if len(span) != 4 || math.IsNaN(span[3]) {
		t.Errorf("EwmSpan 输出异常: %v", span)
	}
}

// TestSeriesApply 覆盖逐元素与逐窗口自定义函数。
func TestSeriesApply(t *testing.T) {
	s := NewSeries([]float64{1, 2, 3})
	approxSeries(t, s.Apply(func(v float64) float64 { return v * 2 }).Values(), 2, 4, 6)

	windowed := s.Rolling(2).Apply(func(values []float64) float64 {
		total := 0.0
		for _, v := range values {
			total += v
		}
		return total
	}).Values()
	if !math.IsNaN(windowed[0]) {
		t.Errorf("窗口不足应为 NaN, 实际 %v", windowed[0])
	}
	approxSeries(t, windowed[1:], 3, 5)

	copied := s.Copy()
	if err := copied.SetF64(0, 100); err != nil {
		t.Fatal(err)
	}
	if v, _ := s.GetF64(0); v != 1 {
		t.Errorf("Copy 应为深拷贝, 原列被改动: %v", v)
	}
}
