// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package formula

import (
	"encoding/csv"
	"math"
	"os"
	"path/filepath"
	"strconv"
	"testing"
)

func parseDouble(text string) float64 {
	if text == "" || text == "nan" {
		return math.NaN()
	}
	v, err := strconv.ParseFloat(text, 64)
	if err != nil {
		return math.NaN()
	}
	return v
}

type row struct {
	label    string
	name     string
	a        float64
	b        float64
	c        float64
	w        int64
	hasW     bool
	n        float64
	hasN     bool
	m        float64
	hasB     bool
	expected float64
}

func column(rows []row, pick func(row) float64) Series {
	out := make(Series, len(rows))
	for i, r := range rows {
		out[i] = pick(r)
	}
	return out
}

func windowOf(rows []row, hasWindow bool, ni int64) []int64 {
	if hasWindow {
		out := make([]int64, len(rows))
		for i, r := range rows {
			out[i] = r.w
		}
		return out
	}
	return constantWindow(ni, len(rows))
}

func evaluate(label string, rows []row) Series {
	a := column(rows, func(r row) float64 { return r.a })
	b := column(rows, func(r row) float64 { return r.b })
	c := column(rows, func(r row) float64 { return r.c })
	n := rows[0].n
	m := rows[0].m
	ni := int64(n)
	mi := int64(m)
	hasWindow := rows[0].hasW
	hasB := rows[0].hasB
	hasN := rows[0].hasN

	switch label {
	case "abs":
		return Abs(a)
	case "sqrt":
		return Sqrt(a)
	case "ln":
		return Log(a)
	case "pow":
		return Pow(a, n)
	case "max":
		return Maximum(a, b)
	case "min":
		return Minimum(a, b)
	case "if":
		return IfSelect(AsBools(a), b, c)
	case "ref":
		if hasWindow {
			return RefWindow(a, windowOf(rows, hasWindow, ni))
		}
		return Ref(a, ni)
	case "diff":
		return Diff(a, ni)
	case "sum":
		return Sum(a, windowOf(rows, hasWindow, ni))
	case "const":
		return Const(a)
	case "hhv":
		if hasWindow {
			return HhvWindow(a, windowOf(rows, hasWindow, ni))
		}
		return Hhv(a, ni)
	case "llv":
		if hasWindow {
			return LlvWindow(a, windowOf(rows, hasWindow, ni))
		}
		return Llv(a, ni)
	case "hhvbars":
		return Hhvbars(a, ni)
	case "llvbars":
		return Llvbars(a, ni)
	case "ma":
		if hasWindow {
			return MaWindow(a, windowOf(rows, hasWindow, ni))
		}
		return Ma(a, ni)
	case "ema":
		return Ema(a, ni)
	case "sma":
		return Sma(a, ni, mi)
	case "wma":
		return Wma(a, ni)
	case "std":
		return Std(a, ni)
	case "avedev":
		if hasWindow {
			return AvedevWindow(a, windowOf(rows, hasWindow, ni))
		}
		return Avedev(a, ni)
	case "slope":
		return Slope(a, ni)
	case "forcast":
		if hasWindow {
			return ForcastWindow(a, windowOf(rows, hasWindow, ni))
		}
		return Forcast(a, ni)
	case "dma":
		if hasB {
			return DmaSeries(a, b)
		}
		return Dma(a, n)
	case "last":
		return AsSeries(Last(AsBools(a), ni, mi))
	case "count":
		return Count(AsBools(a), ni)
	case "every":
		return AsSeries(Every(AsBools(a), ni))
	case "exist":
		return AsSeries(Exist(AsBools(a), ni))
	case "filter":
		return Filter(AsBools(a), ni)
	case "barslast":
		return Barslast(AsBools(a))
	case "barslastcount":
		return Barslastcount(AsBools(a))
	case "barssincen":
		return Barssincen(AsBools(a), ni)
	case "cross":
		return AsSeries(Cross(a, b))
	case "longcross":
		return AsSeries(Longcross(a, b, ni))
	case "valuewhen":
		return Valuewhen(AsBools(a), b)
	case "between":
		return AsSeries(Between(a, n, m))
	case "toprange":
		return Toprange(a)
	case "lowrange":
		return Lowrange(a)
	case "rsi":
		period := int64(24)
		if hasN {
			period = ni
		}
		return Rsi(a, period)
	case "ztprice":
		return Ztprice(a, n)
	case "macd.dif":
		return first(Macd(a, 12, 26, 9))
	case "macd.dea":
		return second(Macd(a, 12, 26, 9))
	case "macd.macd":
		return third(Macd(a, 12, 26, 9))
	case "kdj.k":
		return first(Kdj(a, b, c, 9, 3, 3))
	case "kdj.d":
		return second(Kdj(a, b, c, 9, 3, 3))
	case "kdj.j":
		return third(Kdj(a, b, c, 9, 3, 3))
	case "boll.upper":
		return first(Boll(a, 20, 2.0))
	case "boll.mid":
		return second(Boll(a, 20, 2.0))
	case "boll.lower":
		return third(Boll(a, 20, 2.0))
	default:
		return nil
	}
}

func first(a, _, _ Series) Series  { return a }
func second(_, b, _ Series) Series { return b }
func third(_, _, c Series) Series  { return c }

func TestFormulaParity(t *testing.T) {
	path := filepath.Join("..", "..", "tests", "fixtures", "formula", "cases.csv")
	stream, err := os.Open(path)
	if err != nil {
		t.Fatalf("打开 fixture 失败: %v", err)
	}
	defer stream.Close()
	records, err := csv.NewReader(stream).ReadAll()
	if err != nil {
		t.Fatalf("读取 CSV 失败: %v", err)
	}

	var groups [][]row
	for _, fields := range records[1:] {
		if len(fields) != 11 {
			t.Fatalf("列数错误: %v", fields)
		}
		r := row{
			label: fields[0],
			name:  fields[1],
			a:     parseDouble(fields[3]),
			b:     parseDouble(fields[4]),
			c:     parseDouble(fields[5]),
			hasB:  fields[4] != "",
			hasN:  fields[8] != "",
			hasW:  fields[7] != "",
		}
		if fields[7] != "" {
			r.w = int64(parseDouble(fields[7]))
		}
		if fields[8] != "" {
			r.n = parseDouble(fields[8])
		}
		if fields[9] != "" {
			r.m = parseDouble(fields[9])
		}
		r.expected = parseDouble(fields[10])
		if len(groups) > 0 {
			last := groups[len(groups)-1]
			if last[0].label == r.label && last[0].name == r.name {
				groups[len(groups)-1] = append(last, r)
				continue
			}
		}
		groups = append(groups, []row{r})
	}

	for _, g := range groups {
		label := g[0].label
		actual := evaluate(label, g)
		if actual == nil {
			t.Errorf("未覆盖算子: %s", label)
			continue
		}
		expected := make([]float64, len(g))
		for i, r := range g {
			expected[i] = r.expected
		}
		tolerant := label == "ln" || label == "pow"
		if len(actual) != len(expected) {
			t.Errorf("%s/%s: 长度 %d vs %d", label, g[0].name, len(actual), len(expected))
			continue
		}
		for i := range actual {
			ok := (math.IsNaN(actual[i]) && math.IsNaN(expected[i])) || actual[i] == expected[i]
			if !ok && tolerant && math.Abs(actual[i]-expected[i]) <= 1e-12*math.Abs(expected[i]) {
				ok = true
			}
			if !ok {
				t.Errorf("%s/%s [%d]: %v vs %v", label, g[0].name, i, actual[i], expected[i])
				break
			}
		}
	}
}
