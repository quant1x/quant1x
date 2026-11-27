package pandas

import "github.com/quant1x/quant1x/quant1x/num"

// Series 是一维列容器，对标 quant1x/pandas/series.py。
//
// 底座是 num.NdArray（float64 / bool），数值口径全部委托 quant1x/num；
// Go 无运算符重载，契约以命名方法为准（与 C++ / Rust / Zig 的方法表一致）。
type Series struct {
	name   string
	values num.NdArray
}

// NewSeries 用浮点切片构造。
func NewSeries(values []float64) Series {
	return Series{values: num.NewNdArray(values)}
}

// NewSeriesBools 用布尔切片构造。
func NewSeriesBools(values []bool) Series {
	return Series{values: num.NewNdArrayBools(values)}
}

// SeriesWithName 构造并命名。
func SeriesWithName(name string, values []float64) Series {
	return Series{name: name, values: num.NewNdArray(values)}
}

// SeriesWithNameBools 构造布尔列并命名。
func SeriesWithNameBools(name string, values []bool) Series {
	return Series{name: name, values: num.NewNdArrayBools(values)}
}

// Name 返回列名。
func (s Series) Name() string { return s.name }

// SetName 设置列名。
func (s *Series) SetName(name string) { s.name = name }

// Dtype 返回元素类型。
func (s Series) Dtype() string { return s.values.Dtype() }

// IsBool 判断是否为布尔列。
func (s Series) IsBool() bool { return s.values.IsBool() }

// Len 返回元素个数。
func (s Series) Len() int { return s.values.Len() }

// IsEmpty 判断是否为空。
func (s Series) IsEmpty() bool { return s.values.IsEmpty() }

// Values 返回 float64 视图副本（布尔转 0/1）。
func (s Series) Values() []float64 { return s.values.ToF64() }

// Bools 返回 bool 视图副本。
func (s Series) Bools() []bool { return s.values.ToBools() }

// GetF64 读取第 i 个元素。
func (s Series) GetF64(index int) (float64, bool) { return s.values.GetF64(index) }

// GetBool 读取第 i 个元素的布尔值。
func (s Series) GetBool(index int) (bool, bool) { return s.values.GetBool(index) }

// SetF64 写入第 i 个元素。
func (s Series) SetF64(index int, value float64) error { return s.values.SetF64(index, value) }

// SetBool 写入第 i 个元素的布尔值。
func (s Series) SetBool(index int, value bool) error { return s.values.SetBool(index, value) }

// Copy 返回副本（含列名）。
func (s Series) Copy() Series { return Series{name: s.name, values: s.values.Copy()} }

// Slice 返回 [start, end) 的副本，保留列名。
func (s Series) Slice(start, end int) Series {
	return Series{name: s.name, values: s.values.Slice(start, end)}
}

// Apply 逐元素变换。
func (s Series) Apply(apply func(float64) float64) Series {
	data := s.values.ToF64()
	out := make([]float64, len(data))
	for i, v := range data {
		out[i] = apply(v)
	}
	return Series{name: s.name, values: num.NewNdArray(out)}
}

// Shift 向前取 n 周期前的值，越界补 NaN。
func (s Series) Shift(periods int64) Series {
	return Series{name: s.name, values: s.values.Shift(periods)}
}

// Diff 差分。
func (s Series) Diff(periods int64) Series {
	return Series{name: s.name, values: s.values.Diff(periods)}
}

// Cumsum 累计和。
func (s Series) Cumsum() Series { return Series{name: s.name, values: s.values.Cumsum()} }

// FillNa 缺失值填充。
func (s Series) FillNa(value float64) Series {
	return Series{name: s.name, values: s.values.FillNa(value)}
}

// Add 逐元素加。
func (s Series) Add(other Series) Series {
	return Series{name: s.name, values: s.values.Add(other.values)}
}

// Sub 逐元素减。
func (s Series) Sub(other Series) Series {
	return Series{name: s.name, values: s.values.Sub(other.values)}
}

// Mul 逐元素乘。
func (s Series) Mul(other Series) Series {
	return Series{name: s.name, values: s.values.Mul(other.values)}
}

// Div 逐元素除。
func (s Series) Div(other Series) Series {
	return Series{name: s.name, values: s.values.Div(other.values)}
}

// AddScalar 逐元素加标量。
func (s Series) AddScalar(value float64) Series {
	return Series{name: s.name, values: s.values.AddScalar(value)}
}

// SubScalar 逐元素减标量。
func (s Series) SubScalar(value float64) Series {
	return Series{name: s.name, values: s.values.SubScalar(value)}
}

// MulScalar 逐元素乘标量。
func (s Series) MulScalar(value float64) Series {
	return Series{name: s.name, values: s.values.MulScalar(value)}
}

// DivScalar 逐元素除标量。
func (s Series) DivScalar(value float64) Series {
	return Series{name: s.name, values: s.values.DivScalar(value)}
}

// Gt 大于。
func (s Series) Gt(other Series) Series {
	return Series{name: s.name, values: s.values.Gt(other.values)}
}

// Gte 大于等于。
func (s Series) Gte(other Series) Series {
	return Series{name: s.name, values: s.values.Gte(other.values)}
}

// Lt 小于。
func (s Series) Lt(other Series) Series {
	return Series{name: s.name, values: s.values.Lt(other.values)}
}

// Lte 小于等于。
func (s Series) Lte(other Series) Series {
	return Series{name: s.name, values: s.values.Lte(other.values)}
}

// Equal 相等。
func (s Series) Equal(other Series) Series {
	return Series{name: s.name, values: s.values.Equal(other.values)}
}

// NotEqual 不等。
func (s Series) NotEqual(other Series) Series {
	return Series{name: s.name, values: s.values.NotEqual(other.values)}
}

// GtScalar 大于标量。
func (s Series) GtScalar(value float64) Series {
	return Series{name: s.name, values: s.values.GtScalar(value)}
}

// GteScalar 大于等于标量。
func (s Series) GteScalar(value float64) Series {
	return Series{name: s.name, values: s.values.GteScalar(value)}
}

// LtScalar 小于标量。
func (s Series) LtScalar(value float64) Series {
	return Series{name: s.name, values: s.values.LtScalar(value)}
}

// LteScalar 小于等于标量。
func (s Series) LteScalar(value float64) Series {
	return Series{name: s.name, values: s.values.LteScalar(value)}
}

// EqualScalar 等于标量。
func (s Series) EqualScalar(value float64) Series {
	return Series{name: s.name, values: s.values.EqualScalar(value)}
}

// NotEqualScalar 不等于标量。
func (s Series) NotEqualScalar(value float64) Series {
	return Series{name: s.name, values: s.values.NotEqualScalar(value)}
}

// And 逻辑与。
func (s Series) And(other Series) Series {
	return Series{name: s.name, values: s.values.LogicAnd(other.values)}
}

// Or 逻辑或。
func (s Series) Or(other Series) Series {
	return Series{name: s.name, values: s.values.LogicOr(other.values)}
}

// Not 逻辑非。
func (s Series) Not() Series { return Series{name: s.name, values: s.values.LogicNot()} }

// Sum 求和。
func (s Series) Sum() float64 { return s.values.Sum() }

// Mean 均值。
func (s Series) Mean() float64 { return s.values.Mean() }

// Median 中位数。
func (s Series) Median() float64 { return s.values.Median() }

// MinValue 最小值。
func (s Series) MinValue() float64 { return s.values.MinValue() }

// MaxValue 最大值。
func (s Series) MaxValue() float64 { return s.values.MaxValue() }

// Argmin 最小值下标。
func (s Series) Argmin() int64 { return s.values.Argmin() }

// Argmax 最大值下标。
func (s Series) Argmax() int64 { return s.values.Argmax() }

// Variance 总体方差。
func (s Series) Variance() float64 { return s.values.Variance() }

// Stddev 总体标准差。
func (s Series) Stddev() float64 { return s.values.Stddev() }

// Rolling 绑定滚动窗口。
func (s Series) Rolling(window int64) Rolling { return Rolling{series: s, window: window} }

// Ewm 绑定指数加权系数。
func (s Series) Ewm(alpha float64) Ewm { return Ewm{series: s, alpha: alpha} }

// EwmSpan 由周期换算 alpha 后绑定。
func (s Series) EwmSpan(span int64) Ewm {
	period := 1.0
	if span > 0 {
		period = float64(span)
	}
	return Ewm{series: s, alpha: 2.0 / (period + 1.0)}
}

// Rolling 是滚动窗口句柄。
type Rolling struct {
	series Series
	window int64
}

// Sum 滚动求和。
func (r Rolling) Sum() Series {
	return Series{name: r.series.name, values: r.series.values.RollingSum(r.window)}
}

// Mean 滚动均值。
func (r Rolling) Mean() Series {
	return Series{name: r.series.name, values: r.series.values.RollingMean(r.window)}
}

// Stddev 滚动总体标准差。
func (r Rolling) Stddev() Series {
	return Series{name: r.series.name, values: r.series.values.RollingStd(r.window)}
}

// MinValue 滚动最小值。
func (r Rolling) MinValue() Series {
	return Series{name: r.series.name, values: r.series.values.RollingMin(r.window)}
}

// MaxValue 滚动最大值。
func (r Rolling) MaxValue() Series {
	return Series{name: r.series.name, values: r.series.values.RollingMax(r.window)}
}

// Apply 逐窗口归约。
func (r Rolling) Apply(apply func([]float64) float64) Series {
	return Series{name: r.series.name, values: r.series.values.RollingApply(r.window, apply)}
}

// Ewm 是指数加权句柄。
type Ewm struct {
	series Series
	alpha  float64
}

// Mean 指数加权均值。
func (e Ewm) Mean() Series {
	return Series{name: e.series.name, values: e.series.values.EwmMean(e.alpha)}
}
