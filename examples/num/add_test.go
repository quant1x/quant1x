package num

import (
	"slices"
	"testing"

	"gitee.com/quant1x/num/labs"
)

func TestAdd(t *testing.T) {
	type args struct {
		x any
		y any
	}
	type testCase struct {
		Name     string
		Args     args
		Want     any
		TestFunc func(v any) any
	}
	tests := []testCase{
		{
			Name: "float64",
			Args: args{
				x: []float64{-0.1, 1.0, -2.00, -3},
				y: []float64{-0.1, 1.0, -2.00, -3},
			},
			Want: []float64{-0.2, 2.0, -4.00, -6},
			TestFunc: func(v any) any {
				vs := v.(args)
				return Add(vs.x.([]float64), vs.y.([]float64))
			},
		},
		{
			Name: "float64-no-align-left",
			Args: args{
				x: []float64{-0.1, 1.0, -2.00},
				y: []float64{-0.1, 1.0, -2.00, -3},
			},
			Want: []float64{-0.2, 2.0, -4.00, 0},
			TestFunc: func(v any) any {
				vs := v.(args)
				return Add(vs.x.([]float64), vs.y.([]float64))
			},
		},
		{
			Name: "float64-no-align-right",
			Args: args{
				x: []float64{-0.1, 1.0, -2.00, -3},
				y: []float64{-0.1, 1.0, -2.00},
			},
			Want: []float64{-0.2, 2.0, -4.00, 0},
			TestFunc: func(v any) any {
				vs := v.(args)
				return Add(vs.x.([]float64), vs.y.([]float64))
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.Name, func(t *testing.T) {
			if got := tt.TestFunc(tt.Args); !labs.DeepEqual(got, tt.Want) {
				t.Errorf("Add() = %v, want %v", got, tt.Want)
			}
		})
	}
}

func BenchmarkAdd_init(b *testing.B) {
	testalignOnce.Do(initTestData)
}

func BenchmarkAdd_release(b *testing.B) {
	testalignOnce.Do(initTestData)
	x := slices.Clone(testDataFloat64)
	y := slices.Clone(testDataFloat64y)
	for n := 0; n < b.N; n++ {
		_ = Add(x, y)
	}
}

func BenchmarkAdd_v1(b *testing.B) {
	testalignOnce.Do(initTestData)
	x := slices.Clone(testDataFloat64)
	y := slices.Clone(testDataFloat64y)
	for n := 0; n < b.N; n++ {
		_ = v1Add(x, y)
	}
}

func BenchmarkAdd_v2(b *testing.B) {
	testalignOnce.Do(initTestData)
	x := slices.Clone(testDataFloat64)
	y := slices.Clone(testDataFloat64y)
	for n := 0; n < b.N; n++ {
		_ = v2Add(x, y)
	}
}

func BenchmarkAdd_v3(b *testing.B) {
	testalignOnce.Do(initTestData)
	x := slices.Clone(testDataFloat64)
	y := slices.Clone(testDataFloat64y)
	for n := 0; n < b.N; n++ {
		_ = v3Add(x, y)
	}
}

func BenchmarkAdd_v4(b *testing.B) {
	testalignOnce.Do(initTestData)
	x := slices.Clone(testDataFloat64)
	y := slices.Clone(testDataFloat64y)
	for n := 0; n < b.N; n++ {
		_ = v4Add(x, y)
	}
}

func BenchmarkAdd_v5(b *testing.B) {
	testalignOnce.Do(initTestData)
	x := slices.Clone(testDataFloat64)
	y := slices.Clone(testDataFloat64y)
	for n := 0; n < b.N; n++ {
		_ = v5Add(x, y)
	}
}
