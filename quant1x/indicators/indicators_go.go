// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

// Package indicators 实现通达信指标, 语义以 quant1x/indicators/*.py 为准。
//
// 与 C++ indicators/*.h、Rust indicators/mod.rs 逐函数对应; 输入 []schema.Bar,
// 算子走内部 quant1x/formula。
package indicators

import (
	"github.com/quant1x/quant1x/quant1x/data/schema"
	"github.com/quant1x/quant1x/quant1x/formula"
)

func column(bars []schema.Bar, pick func(schema.Bar) float64) formula.Series {
	out := make(formula.Series, len(bars))
	for i, bar := range bars {
		out[i] = pick(bar)
	}
	return out
}

// CdtdResult 抄底逃顶输出列。
type CdtdResult struct {
	Wen formula.Series
	S1  formula.Bools
	B   formula.Bools
	S2  formula.Bools
	S3  formula.Bools
}

// Cdtd 抄底逃顶。
func Cdtd(bars []schema.Bar) CdtdResult {
	close := column(bars, func(b schema.Bar) float64 { return b.Close })
	high := column(bars, func(b schema.Bar) float64 { return b.High })
	low := column(bars, func(b schema.Bar) float64 { return b.Low })

	const n1, n2, n3, n4 = 3, 9, 27, 5

	highLowSpan := formula.Sub(formula.Hhv(high, n3), formula.Llv(low, n3))
	closeLowSpan := formula.Sub(close, formula.Llv(low, n3))
	rsv1 := formula.MulScalar(formula.Div(
		formula.Sub(close, formula.Llv(low, n2)),
		formula.Sub(formula.Hhv(high, n2), formula.Llv(low, n2)),
	), 100.0)
	rsv2 := formula.MulScalar(formula.Div(closeLowSpan, highLowSpan), 100.0)

	smooth := formula.Sma(rsv2, n4, 1)
	wen := formula.Sub(formula.MulScalar(smooth, n1), formula.MulScalar(formula.Sma(smooth, n1, 1), 2.0))

	j1 := formula.Sma(rsv1, n1, 1)
	j2 := formula.Sma(j1, n1, 1)

	s1 := formula.LogicAnd(formula.Cross(j2, j1), formula.GreaterScalar(j2, 85.0))
	s2 := formula.LessEqualScalar(wen, 5.0)
	buy := formula.LogicAnd(formula.Less(wen, formula.Ref(wen, 1)), s2)
	s3 := formula.EqualScalar(formula.Count(buy, 2), 2.0)

	return CdtdResult{Wen: wen, S1: s1, B: buy, S2: s2, S3: s3}
}

// F89kResult 八九空间输出列。
type F89kResult struct {
	Zs formula.Series
	B  formula.Bools
	Dn formula.Series
}

// F89k 八九空间。
func F89k(bars []schema.Bar) F89kResult {
	close := column(bars, func(b schema.Bar) float64 { return b.Close })
	high := column(bars, func(b schema.Bar) float64 { return b.High })
	low := column(bars, func(b schema.Bar) float64 { return b.Low })
	volume := column(bars, func(b schema.Bar) float64 { return b.Volume })

	const n1 = 89
	const cs = 0.191

	high89 := formula.Hhv(high, n1)
	n7 := formula.Barslast(formula.Equal(high89, high))
	t8 := formula.LlvWindow(low, formula.SeriesToWindow(n7))
	n8 := formula.Barslast(formula.LogicAnd(formula.Equal(t8, low), formula.LessScalar(n7, n1)))
	t9 := formula.HhvWindow(high, formula.SeriesToWindow(n8))
	n9x := formula.Barslast(formula.LogicAnd(formula.Equal(t9, high), formula.Greater(n7, n8)))
	n9 := formula.IfSelect(formula.EqualScalar(n9x, 0), formula.AddScalar(n9x, 1), n9x)
	k10 := formula.LlvWindow(low, formula.SeriesToWindow(n9))
	n10 := formula.Barslast(formula.LogicAnd(formula.Equal(k10, low), formula.Greater(n8, n9)))

	male := formula.Greater(close, formula.Ref(close, 1))
	volumeUp := formula.Greater(volume, formula.Ref(volume, 1))
	volumeAverage := formula.Ma(volume, 5)
	indexVolume := formula.GreaterEqual(volume, formula.MulScalar(volumeAverage, 1.0+cs/10.0))
	generalVolume := formula.GreaterEqual(volume, formula.MulScalar(volumeAverage, 1.0+cs))
	branch := formula.IfSelectBools(formula.GreaterEqualScalar(close, 100.0), indexVolume, generalVolume)
	breakout := formula.LogicAnd(formula.LogicAnd(branch, male), volumeUp)

	dn := formula.Barslast(breakout)
	stopLoss := formula.RefWindow(low, formula.SeriesToWindow(dn))

	above := formula.LogicAnd(formula.Greater(close, stopLoss), formula.Greater(n9, n10))
	below := formula.LogicAnd(formula.Less(close, stopLoss), formula.Greater(n9, n10))
	c1Zero := formula.EqualScalar(formula.Barslast(above), 0)
	c2One := formula.EqualScalar(formula.Barslast(below), 1)
	n10Positive := formula.GreaterScalar(n10, 0)
	buy := formula.LogicAnd(formula.LogicAnd(c1Zero, c2One), n10Positive)

	return F89kResult{Zs: stopLoss, B: buy, Dn: dn}
}

// HjkResult 黄金坑输出列。
type HjkResult struct {
	B formula.Bools
}

// Hjk 黄金坑。
func Hjk(bars []schema.Bar, limitUpRatio float64) HjkResult {
	close := column(bars, func(b schema.Bar) float64 { return b.Close })
	high := column(bars, func(b schema.Bar) float64 { return b.High })
	volume := column(bars, func(b schema.Bar) float64 { return b.Volume })

	previousClose := formula.Ref(close, 1)
	limitPrice := formula.Ztprice(previousClose, limitUpRatio)
	volumeRatio := formula.Div(volume, formula.Ref(volume, 1))

	doubled := formula.GreaterEqualScalar(volumeRatio, 2.0)
	broken := formula.LessEqual(close, limitPrice)
	nearLimit := formula.GreaterEqual(high, formula.MulScalar(limitPrice, 0.98))
	pattern := formula.LogicAnd(formula.LogicAnd(doubled, broken), nearLimit)

	pn := formula.Barslast(pattern)
	pnWindow := formula.SeriesToWindow(pn)
	previousHigh := formula.RefWindow(close, pnWindow)
	countAbove := formula.CountWindow(formula.GreaterEqual(previousHigh, close), pnWindow)
	volumeHigh := formula.HhvWindow(volume, formula.SeriesToWindow(formula.AddScalar(pn, 1)))
	shrink := formula.Div(volume, volumeHigh)

	c21 := formula.LessEqualScalar(shrink, 0.50)
	c22 := formula.LogicOr(
		formula.GreaterEqualScalar(formula.Div(countAbove, pn), 0.50),
		formula.Equal(countAbove, pn),
	)
	c23 := formula.LogicAnd(formula.GreaterEqualScalar(pn, 4.0), formula.LessScalar(pn, 7.0))
	buy := formula.LogicAnd(formula.LogicAnd(c21, c22), c23)

	return HjkResult{B: buy}
}

// Ma1xResult 一线天输出列。
type Ma1xResult struct {
	Dn formula.Series
	Zs formula.Series
	Yl formula.Series
	Hc formula.Bools
	Tp formula.Bools
}

// Ma1x 一线天。
func Ma1x(bars []schema.Bar) Ma1xResult {
	close := column(bars, func(b schema.Bar) float64 { return b.Close })
	high := column(bars, func(b schema.Bar) float64 { return b.High })
	low := column(bars, func(b schema.Bar) float64 { return b.Low })
	volume := column(bars, func(b schema.Bar) float64 { return b.Volume })

	const cs = 0.191

	male := formula.Greater(close, formula.Ref(close, 1))
	volumeUp := formula.Greater(volume, formula.Ref(volume, 1))
	volumeAverage := formula.Ma(volume, 5)
	indexVolume := formula.GreaterEqual(volume, formula.MulScalar(volumeAverage, 1.0+cs/10.0))
	generalVolume := formula.GreaterEqual(volume, formula.MulScalar(volumeAverage, 1.0+cs))
	branch := formula.IfSelectBools(formula.GreaterEqualScalar(close, 100.0), indexVolume, generalVolume)
	breakout := formula.LogicAnd(formula.LogicAnd(branch, male), volumeUp)

	dn := formula.Barslast(breakout)
	stopLoss := formula.RefWindow(low, formula.SeriesToWindow(dn))

	h0 := formula.Hhv(high, 5)
	resistancePattern := formula.LogicAnd(formula.LogicAnd(male, formula.Equal(high, h0)), volumeUp)
	ylN := formula.Barslast(resistancePattern)
	resistanceHigh := formula.RefWindow(high, formula.SeriesToWindow(ylN))

	dnZero := formula.EqualScalar(dn, 0)
	supportPeriod := formula.IfSelect(dnZero, formula.AddScalar(formula.Ref(stopLoss, 1), 1), dn)
	supportLine := formula.IfSelect(dnZero, formula.Ref(stopLoss, 1), stopLoss)

	previousHigh := formula.Ref(high, 1)
	wash := formula.MulScalar(formula.Div(formula.Sub(previousHigh, low), previousHigh), 100.0)
	pullback := formula.LogicAnd(
		formula.LogicAnd(formula.GreaterEqualScalar(supportPeriod, 2.0), formula.GreaterEqualScalar(wash, 2.33)),
		formula.LogicAnd(formula.Less(low, supportLine), formula.GreaterEqual(close, supportLine)),
	)

	ylZero := formula.EqualScalar(ylN, 0)
	resistancePeriod := formula.IfSelect(ylZero, formula.AddScalar(formula.Ref(ylN, 1), 1), ylN)
	resistanceLine := formula.IfSelect(ylZero, formula.Ref(resistanceHigh, 1), resistanceHigh)

	touched := formula.LogicOr(
		formula.LogicOr(
			formula.Less(formula.Ref(close, 1), resistanceLine),
			formula.Less(formula.Ref(low, 1), resistanceLine),
		),
		formula.Less(low, resistanceLine),
	)
	recovered := formula.GreaterEqual(close, resistanceLine)
	penetrated := formula.LogicAnd(
		formula.LogicAnd(touched, recovered),
		formula.GreaterEqualScalar(resistancePeriod, 2.0),
	)

	return Ma1xResult{Dn: dn, Zs: stopLoss, Yl: resistanceHigh, Hc: pullback, Tp: penetrated}
}

// P2fbResult 趋势反包输出列。
type P2fbResult struct {
	Hgzx formula.Series
}

// P2fb 趋势反包。
func P2fb(bars []schema.Bar, limitUpRatio float64) P2fbResult {
	close := column(bars, func(b schema.Bar) float64 { return b.Close })
	_ = limitUpRatio

	const slopePeriod = 10

	a := formula.MulScalar(formula.Div(formula.Slope(close, slopePeriod), close), 100.0)
	b := formula.Div(formula.Forcast(close, slopePeriod), close)
	d := formula.MulScalar(a, -1.0)
	term := formula.Div(formula.Add(formula.Mul(a, close), formula.Mul(b, close)), close)
	turn := formula.LogicOr(formula.Cross(a, d), formula.Cross(term, d))
	hgzx := formula.MulScalar(formula.Filter(turn, 5), 4.0)

	return P2fbResult{Hgzx: hgzx}
}
