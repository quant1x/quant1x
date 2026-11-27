// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package num

import (
	"encoding/csv"
	"math"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"testing"
)

func parseFloat(text string) float64 {
	if text == "" || text == "nan" {
		return math.NaN()
	}
	value, err := strconv.ParseFloat(text, 64)
	if err != nil {
		return math.NaN()
	}
	return value
}

func parseSeries(text string) []float64 {
	if text == "" {
		return []float64{}
	}
	tokens := strings.Split(text, "|")
	out := make([]float64, len(tokens))
	for i, token := range tokens {
		out[i] = parseFloat(token)
	}
	return out
}

func parseInts(text string) []int64 {
	if text == "" {
		return []int64{}
	}
	tokens := strings.Split(text, "|")
	out := make([]int64, len(tokens))
	for i, token := range tokens {
		value, _ := strconv.ParseFloat(token, 64)
		out[i] = int64(value)
	}
	return out
}

func same(left, right float64) bool {
	return (math.IsNaN(left) && math.IsNaN(right)) || left == right
}

func boolToFloat(value bool) float64 {
	if value {
		return 1
	}
	return 0
}

func compareSeries(t *testing.T, function string, actual, expected []float64) {
	t.Helper()
	// Go 的 math.Log / math.Pow 与 NumPy 的 libm 实现相差至多 1 ULP, 这两类按相对误差比较;
	// 纯算术、归约与滚动逐位相等。
	tolerant := function == "log" || function == "pow"
	if len(actual) != len(expected) {
		t.Errorf("%s: 长度 %d vs %d", function, len(actual), len(expected))
		return
	}
	for i := range actual {
		if same(actual[i], expected[i]) {
			continue
		}
		if tolerant && math.Abs(actual[i]-expected[i]) <= 1e-12*math.Abs(expected[i]) {
			continue
		}
		t.Errorf("%s [%d]: %v vs %v", function, i, actual[i], expected[i])
		return
	}
}

func compareBools(t *testing.T, function string, actual []bool, expected []float64) {
	t.Helper()
	if len(actual) != len(expected) {
		t.Errorf("%s: 长度 %d vs %d", function, len(actual), len(expected))
		return
	}
	for i := range actual {
		if actual[i] != (expected[i] != 0) {
			t.Errorf("%s [%d]: %v vs %v", function, i, actual[i], expected[i])
			return
		}
	}
}

// run 返回 (浮点结果, 布尔结果); 二者至多一个非 nil。
func run(t *testing.T, function string, a, b, c []float64, w []int64, n float64) ([]float64, []bool) {
	t.Helper()
	window := w
	if len(window) == 0 {
		window = ConstantWindow(int64(n), len(a))
	}
	first := 0.0
	if len(a) > 0 {
		first = a[0]
	}

	switch function {
	case "to_scalar":
		return []float64{ToScalar(a)}, nil
	case "default_like":
		return DefaultLike(a), nil
	case "abs":
		return Abs(a), nil
	case "sqrt":
		return Sqrt(a), nil
	case "log":
		return Log(a), nil
	case "pow":
		return Pow(a, n), nil
	case "add":
		return Add(a, b), nil
	case "sub":
		return Sub(a, b), nil
	case "mul":
		return Mul(a, b), nil
	case "div":
		return Div(a, b), nil
	case "maximum":
		return Maximum(a, b), nil
	case "minimum":
		return Minimum(a, b), nil
	case "add_scalar":
		return AddScalar(a, n), nil
	case "sub_scalar":
		return SubScalar(a, n), nil
	case "mul_scalar":
		return MulScalar(a, n), nil
	case "div_scalar":
		return DivScalar(a, n), nil
	case "maximum_scalar":
		return MaximumScalar(a, n), nil
	case "minimum_scalar":
		return MinimumScalar(a, n), nil
	case "where":
		return Where(ToBools(a), b, c), nil
	case "gt":
		return nil, Gt(a, b)
	case "gte":
		return nil, Gte(a, b)
	case "lt":
		return nil, Lt(a, b)
	case "lte":
		return nil, Lte(a, b)
	case "equal":
		return nil, Equal(a, b)
	case "not_equal":
		return nil, NotEqual(a, b)
	case "to_bools":
		return nil, ToBools(a)
	case "logic_and":
		return nil, LogicAnd(ToBools(a), ToBools(b))
	case "logic_or":
		return nil, LogicOr(ToBools(a), ToBools(b))
	case "logic_not":
		return nil, LogicNot(ToBools(a))
	case "all_of":
		return []float64{boolToFloat(AllOf(ToBools(a)))}, nil
	case "any_of":
		return []float64{boolToFloat(AnyOf(ToBools(a)))}, nil
	case "count_true":
		return []float64{float64(CountTrue(ToBools(a)))}, nil
	case "sum_value":
		return []float64{SumValue(a)}, nil
	case "mean":
		return []float64{Mean(a)}, nil
	case "median":
		return []float64{Median(a)}, nil
	case "min_value":
		return []float64{MinValue(a)}, nil
	case "max_value":
		return []float64{MaxValue(a)}, nil
	case "argmin":
		return []float64{float64(Argmin(a))}, nil
	case "argmax":
		return []float64{float64(Argmax(a))}, nil
	case "variance":
		return []float64{Variance(a)}, nil
	case "std":
		return []float64{Stddev(a)}, nil
	case "covariance":
		return []float64{Covariance(a, b)}, nil
	case "rolling_sum":
		return RollingSum(a, window), nil
	case "rolling_mean":
		return RollingMean(a, window), nil
	case "rolling_std":
		return RollingStd(a, window), nil
	case "rolling_min":
		return RollingMin(a, window), nil
	case "rolling_max":
		return RollingMax(a, window), nil
	case "rolling_avedev":
		return RollingAvedev(a, window), nil
	case "rolling_hhvbars":
		return RollingHhvbars(a, window), nil
	case "rolling_llvbars":
		return RollingLlvbars(a, window), nil
	case "ewm_mean":
		return EwmMean(a, n), nil
	case "shift":
		return Shift(a, int64(n)), nil
	case "ref":
		return Ref(a, int64(n)), nil
	case "diff":
		return Diff(a, int64(n)), nil
	case "cumsum":
		return Cumsum(a), nil
	case "fill":
		return Fill(a, n), nil
	case "fill_na":
		return FillNa(a, n), nil
	case "is_na":
		return nil, IsNa(a)
	case "align":
		return Align(a, int(n), math.NaN()), nil
	case "pad_to":
		return PadTo(a, int(n), math.NaN()), nil
	case "repeat":
		return Repeat(a, int(n)), nil
	case "linear_regression":
		slope, intercept := LinearRegression(a)
		return []float64{slope, intercept}, nil
	case "slope":
		return []float64{Slope(a)}, nil
	case "forcast":
		return []float64{Forcast(a)}, nil
	case "decimal":
		return []float64{Decimal(first, int(n))}, nil
	case "decimal_series":
		return DecimalSeries(a, int(n)), nil
	case "change_rate":
		return []float64{ChangeRate(first, b[0])}, nil
	case "net_change_rate":
		return []float64{NetChangeRate(first, b[0])}, nil
	case "shape":
		return []float64{float64(len(a))}, nil
	case "concat1d":
		return Concat1d(a, b), nil
	case "transpose2d":
		return a, nil
	case "zeros":
		return Zeros(int(n)), nil
	case "ones":
		return Ones(int(n)), nil
	case "arange":
		if len(a) > 0 {
			return Arange(a[0], b[0], c[0]), nil
		}
		return Arange(0, n, 1), nil
	case "linspace":
		return Linspace(0, 1, int(n)), nil
	default:
		t.Errorf("未覆盖算子: %s", function)
		return nil, nil
	}
}

// TestNumParity 读 cases.csv, 逐值比较 Go 与 Python 输出。
func TestNumParity(t *testing.T) {
	path := filepath.Join("..", "..", "tests", "fixtures", "num", "cases.csv")
	stream, err := os.Open(path)
	if err != nil {
		t.Fatalf("打开 fixture 失败: %v", err)
	}
	defer stream.Close()

	records, err := csv.NewReader(stream).ReadAll()
	if err != nil {
		t.Fatalf("读取 CSV 失败: %v", err)
	}
	if len(records) < 2 {
		t.Fatalf("空 fixture")
	}

	for _, record := range records[1:] {
		if len(record) != 8 {
			t.Fatalf("列数错误: %v", record)
		}
		function := record[0]
		a := parseSeries(record[2])
		b := parseSeries(record[3])
		c := parseSeries(record[4])
		w := parseInts(record[5])
		n := parseFloat(record[6])
		expected := parseSeries(record[7])
		floats, bools := run(t, function, a, b, c, w, n)
		if bools != nil {
			compareBools(t, function, bools, expected)
		} else if floats != nil {
			compareSeries(t, function, floats, expected)
		}
	}
}
