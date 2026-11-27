// Package num 的一维数组容器，对标 numpy.ndarray。
//
// 研究可以在 Python 里随意做，但模型与因子要上生产，必须能落到 C++/Rust/Go/Zig。
// NdArray 是这层契约在 Go 侧的实现：只保留上生产必需的子集，方法表与
// quant1x/num/ndarray.md 一致（Go 无运算符重载，所以一律用命名方法）。
//
// 数值口径全部委托本包的自由函数，容器不另写算法。
package num

import "fmt"

// NdArray 是一维 float64 / bool 数组。
//
// 两个切片只有一个是有效的：浮点数组用 floats，布尔数组用 bools。
type NdArray struct {
	floats []float64
	bools  []bool
}

// NewNdArray 用浮点切片构造。
func NewNdArray(values []float64) NdArray {
	data := make([]float64, len(values))
	copy(data, values)
	return NdArray{floats: data}
}

// NewNdArrayBools 用布尔切片构造。
func NewNdArrayBools(values []bool) NdArray {
	data := make([]bool, len(values))
	copy(data, values)
	return NdArray{bools: data}
}

// NdArrayZeros 返回全零数组。
func NdArrayZeros(length int) NdArray { return NdArray{floats: Zeros(length)} }

// NdArrayOnes 返回全一数组。
func NdArrayOnes(length int) NdArray { return NdArray{floats: Ones(length)} }

// NdArrayFull 返回定值数组。
func NdArrayFull(length int, value float64) NdArray {
	data := make([]float64, length)
	for i := range data {
		data[i] = value
	}
	return NdArray{floats: data}
}

// NdArrayArange 返回等差序列。
func NdArrayArange(start, stop, step float64) NdArray {
	return NdArray{floats: Arange(start, stop, step)}
}

// NdArrayLinspace 返回等分序列。
func NdArrayLinspace(start, stop float64, count int) NdArray {
	return NdArray{floats: Linspace(start, stop, count)}
}

// Dtype 返回元素类型: "float64" 或 "bool"。
func (a NdArray) Dtype() string {
	if a.IsBool() {
		return "bool"
	}
	return "float64"
}

// IsBool 判断是否为布尔数组。
func (a NdArray) IsBool() bool { return a.floats == nil && a.bools != nil }

// Len 返回元素个数。
func (a NdArray) Len() int {
	if a.IsBool() {
		return len(a.bools)
	}
	return len(a.floats)
}

// IsEmpty 判断是否为空数组。
func (a NdArray) IsEmpty() bool { return a.Len() == 0 }

// ToF64 返回 float64 视图（布尔转 0/1），返回副本。
func (a NdArray) ToF64() []float64 {
	if !a.IsBool() {
		out := make([]float64, len(a.floats))
		copy(out, a.floats)
		return out
	}
	out := make([]float64, len(a.bools))
	for i, v := range a.bools {
		if v {
			out[i] = 1.0
		}
	}
	return out
}

// ToBools 返回 bool 视图（非零为真），返回副本。
func (a NdArray) ToBools() []bool {
	if a.IsBool() {
		out := make([]bool, len(a.bools))
		copy(out, a.bools)
		return out
	}
	return ToBools(a.floats)
}

// GetF64 读取第 i 个元素（布尔数组按 0/1 返回）。
func (a NdArray) GetF64(index int) (float64, bool) {
	if index < 0 || index >= a.Len() {
		return 0, false
	}
	if a.IsBool() {
		if a.bools[index] {
			return 1.0, true
		}
		return 0.0, true
	}
	return a.floats[index], true
}

// GetBool 读取第 i 个元素的布尔值。
func (a NdArray) GetBool(index int) (bool, bool) {
	if index < 0 || index >= a.Len() {
		return false, false
	}
	if a.IsBool() {
		return a.bools[index], true
	}
	return !IsNaN(a.floats[index]) && a.floats[index] != 0, true
}

// SetF64 写入第 i 个元素（布尔数组忽略）。
func (a NdArray) SetF64(index int, value float64) error {
	if a.IsBool() {
		return fmt.Errorf("NdArray: 布尔数组不能写入 float64")
	}
	if index < 0 || index >= len(a.floats) {
		return fmt.Errorf("NdArray: 下标越界 %d", index)
	}
	a.floats[index] = value
	return nil
}

// SetBool 写入第 i 个元素的布尔值（浮点数组忽略）。
func (a NdArray) SetBool(index int, value bool) error {
	if !a.IsBool() {
		return fmt.Errorf("NdArray: 浮点数组不能写入 bool")
	}
	if index < 0 || index >= len(a.bools) {
		return fmt.Errorf("NdArray: 下标越界 %d", index)
	}
	a.bools[index] = value
	return nil
}

// Copy 返回副本。
func (a NdArray) Copy() NdArray {
	if a.IsBool() {
		return NewNdArrayBools(a.bools)
	}
	return NewNdArray(a.floats)
}

// Slice 返回 [start, end) 的副本; 越界自动收敛。
func (a NdArray) Slice(start, end int) NdArray {
	length := a.Len()
	if end > length {
		end = length
	}
	if start < 0 {
		start = 0
	}
	if start > end {
		start = end
	}
	if a.IsBool() {
		return NewNdArrayBools(a.bools[start:end])
	}
	return NewNdArray(a.floats[start:end])
}

// Astype 类型转换, 仅支持 "float64" / "bool"。
func (a NdArray) Astype(dtype string) (NdArray, error) {
	switch dtype {
	case "float64":
		return NewNdArray(a.ToF64()), nil
	case "bool":
		return NewNdArrayBools(a.ToBools()), nil
	default:
		return NdArray{}, fmt.Errorf("NdArray: 不支持的 dtype %q", dtype)
	}
}

// 四则: 与另一个数组逐个元素对齐（按左侧长度, 右侧不足补 NaN）。

// Add 逐元素加。
func (a NdArray) Add(other NdArray) NdArray { return NewNdArray(Add(a.ToF64(), other.ToF64())) }

// Sub 逐元素减。
func (a NdArray) Sub(other NdArray) NdArray { return NewNdArray(Sub(a.ToF64(), other.ToF64())) }

// Mul 逐元素乘。
func (a NdArray) Mul(other NdArray) NdArray { return NewNdArray(Mul(a.ToF64(), other.ToF64())) }

// Div 逐元素除。
func (a NdArray) Div(other NdArray) NdArray { return NewNdArray(Div(a.ToF64(), other.ToF64())) }

// AddScalar 逐元素加标量。
func (a NdArray) AddScalar(value float64) NdArray { return NewNdArray(AddScalar(a.ToF64(), value)) }

// SubScalar 逐元素减标量。
func (a NdArray) SubScalar(value float64) NdArray { return NewNdArray(SubScalar(a.ToF64(), value)) }

// MulScalar 逐元素乘标量。
func (a NdArray) MulScalar(value float64) NdArray { return NewNdArray(MulScalar(a.ToF64(), value)) }

// DivScalar 逐元素除标量。
func (a NdArray) DivScalar(value float64) NdArray { return NewNdArray(DivScalar(a.ToF64(), value)) }

// Maximum 逐元素取大。
func (a NdArray) Maximum(other NdArray) NdArray {
	return NewNdArray(Maximum(a.ToF64(), other.ToF64()))
}

// Minimum 逐元素取小。
func (a NdArray) Minimum(other NdArray) NdArray {
	return NewNdArray(Minimum(a.ToF64(), other.ToF64()))
}

// MaximumScalar 逐元素与标量取大。
func (a NdArray) MaximumScalar(value float64) NdArray {
	return NewNdArray(MaximumScalar(a.ToF64(), value))
}

// MinimumScalar 逐元素与标量取小。
func (a NdArray) MinimumScalar(value float64) NdArray {
	return NewNdArray(MinimumScalar(a.ToF64(), value))
}

// Abs 绝对值。
func (a NdArray) Abs() NdArray { return NewNdArray(Abs(a.ToF64())) }

// Sqrt 平方根。
func (a NdArray) Sqrt() NdArray { return NewNdArray(Sqrt(a.ToF64())) }

// Log 自然对数。
func (a NdArray) Log() NdArray { return NewNdArray(Log(a.ToF64())) }

// Pow 幂。
func (a NdArray) Pow(exponent float64) NdArray { return NewNdArray(Pow(a.ToF64(), exponent)) }

// Neg 取负。
func (a NdArray) Neg() NdArray { return NewNdArray(MulScalar(a.ToF64(), -1.0)) }

// 比较: 返回布尔数组。

// Gt 大于。
func (a NdArray) Gt(other NdArray) NdArray { return NewNdArrayBools(Gt(a.ToF64(), other.ToF64())) }

// Gte 大于等于。
func (a NdArray) Gte(other NdArray) NdArray { return NewNdArrayBools(Gte(a.ToF64(), other.ToF64())) }

// Lt 小于。
func (a NdArray) Lt(other NdArray) NdArray { return NewNdArrayBools(Lt(a.ToF64(), other.ToF64())) }

// Lte 小于等于。
func (a NdArray) Lte(other NdArray) NdArray { return NewNdArrayBools(Lte(a.ToF64(), other.ToF64())) }

// Equal 相等。
func (a NdArray) Equal(other NdArray) NdArray {
	return NewNdArrayBools(Equal(a.ToF64(), other.ToF64()))
}

// NotEqual 不等。
func (a NdArray) NotEqual(other NdArray) NdArray {
	return NewNdArrayBools(NotEqual(a.ToF64(), other.ToF64()))
}

// GtScalar 大于标量。
func (a NdArray) GtScalar(value float64) NdArray { return a.Gt(NdArrayFull(a.Len(), value)) }

// GteScalar 大于等于标量。
func (a NdArray) GteScalar(value float64) NdArray { return a.Gte(NdArrayFull(a.Len(), value)) }

// LtScalar 小于标量。
func (a NdArray) LtScalar(value float64) NdArray { return a.Lt(NdArrayFull(a.Len(), value)) }

// LteScalar 小于等于标量。
func (a NdArray) LteScalar(value float64) NdArray { return a.Lte(NdArrayFull(a.Len(), value)) }

// EqualScalar 等于标量。
func (a NdArray) EqualScalar(value float64) NdArray { return a.Equal(NdArrayFull(a.Len(), value)) }

// NotEqualScalar 不等于标量。
func (a NdArray) NotEqualScalar(value float64) NdArray {
	return a.NotEqual(NdArrayFull(a.Len(), value))
}

// LogicAnd 逻辑与。
func (a NdArray) LogicAnd(other NdArray) NdArray {
	return NewNdArrayBools(LogicAnd(a.ToBools(), other.ToBools()))
}

// LogicOr 逻辑或。
func (a NdArray) LogicOr(other NdArray) NdArray {
	return NewNdArrayBools(LogicOr(a.ToBools(), other.ToBools()))
}

// LogicNot 逻辑非。
func (a NdArray) LogicNot() NdArray { return NewNdArrayBools(LogicNot(a.ToBools())) }

// 归约。

// Sum 求和。
func (a NdArray) Sum() float64 { return SumValue(a.ToF64()) }

// Mean 均值。
func (a NdArray) Mean() float64 { return Mean(a.ToF64()) }

// Median 中位数。
func (a NdArray) Median() float64 { return Median(a.ToF64()) }

// MinValue 最小值。
func (a NdArray) MinValue() float64 { return MinValue(a.ToF64()) }

// MaxValue 最大值。
func (a NdArray) MaxValue() float64 { return MaxValue(a.ToF64()) }

// Argmin 最小值下标, 空数组返回 -1。
func (a NdArray) Argmin() int64 { return Argmin(a.ToF64()) }

// Argmax 最大值下标, 空数组返回 -1。
func (a NdArray) Argmax() int64 { return Argmax(a.ToF64()) }

// Variance 总体方差。
func (a NdArray) Variance() float64 { return Variance(a.ToF64()) }

// Stddev 总体标准差。
func (a NdArray) Stddev() float64 { return Stddev(a.ToF64()) }

// Cumsum 累计和。
func (a NdArray) Cumsum() NdArray { return NewNdArray(Cumsum(a.ToF64())) }

// Shift 向前取 n 周期前的值, 越界补 NaN。
func (a NdArray) Shift(periods int64) NdArray { return NewNdArray(Shift(a.ToF64(), periods)) }

// Diff 差分。
func (a NdArray) Diff(periods int64) NdArray { return NewNdArray(Diff(a.ToF64(), periods)) }

// FillNa 缺失值填充。
func (a NdArray) FillNa(value float64) NdArray { return NewNdArray(FillNa(a.ToF64(), value)) }

// RollingSum 滚动求和。
func (a NdArray) RollingSum(window int64) NdArray {
	return NewNdArray(RollingSum(a.ToF64(), ConstantWindow(window, a.Len())))
}

// RollingMean 滚动均值。
func (a NdArray) RollingMean(window int64) NdArray {
	return NewNdArray(RollingMean(a.ToF64(), ConstantWindow(window, a.Len())))
}

// RollingStd 滚动总体标准差。
func (a NdArray) RollingStd(window int64) NdArray {
	return NewNdArray(RollingStd(a.ToF64(), ConstantWindow(window, a.Len())))
}

// RollingMin 滚动最小值。
func (a NdArray) RollingMin(window int64) NdArray {
	return NewNdArray(RollingMin(a.ToF64(), ConstantWindow(window, a.Len())))
}

// RollingMax 滚动最大值。
func (a NdArray) RollingMax(window int64) NdArray {
	return NewNdArray(RollingMax(a.ToF64(), ConstantWindow(window, a.Len())))
}

// RollingAvedev 滚动平均绝对偏差。
func (a NdArray) RollingAvedev(window int64) NdArray {
	return NewNdArray(RollingAvedev(a.ToF64(), ConstantWindow(window, a.Len())))
}

// RollingApply 滚动窗口自定义聚合。
func (a NdArray) RollingApply(window int64, apply func([]float64) float64) NdArray {
	return NewNdArray(RollingApply(a.ToF64(), ConstantWindow(window, a.Len()), apply))
}

// EwmMean 指数加权移动平均。
func (a NdArray) EwmMean(alpha float64) NdArray { return NewNdArray(EwmMean(a.ToF64(), alpha)) }

// String 便于调试。
func (a NdArray) String() string {
	if a.IsBool() {
		return fmt.Sprintf("NdArray(%v, dtype='bool')", a.bools)
	}
	return fmt.Sprintf("NdArray(%v, dtype='float64')", a.floats)
}
