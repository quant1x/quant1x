// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package formula

import (
	"math"
	"strconv"
	"strings"

	"github.com/quant1x/quant1x/quant1x/num"
)

//go:noinline
func mulOnly(a, b float64) float64 { return a * b }

func constantWindow(n int64, length int) []int64 { return num.ConstantWindow(n, length) }

// Hhv / Llv N 周期最高/最低值。
func Hhv(x Series, n int64) Series         { return num.RollingMax(x, constantWindow(n, len(x))) }
func Llv(x Series, n int64) Series         { return num.RollingMin(x, constantWindow(n, len(x))) }
func HhvWindow(x Series, w []int64) Series { return num.RollingMax(x, w) }
func LlvWindow(x Series, w []int64) Series { return num.RollingMin(x, w) }

// Hhvbars / Llvbars N 周期极值位置。
func Hhvbars(x Series, n int64) Series { return num.RollingHhvbars(x, constantWindow(n, len(x))) }
func Llvbars(x Series, n int64) Series { return num.RollingLlvbars(x, constantWindow(n, len(x))) }

// Ma N 日简单移动平均。
func Ma(x Series, n int64) Series         { return num.RollingMean(x, constantWindow(n, len(x))) }
func MaWindow(x Series, w []int64) Series { return num.RollingMean(x, w) }

// Ewm 指数加权均值。
func Ewm(x Series, alpha float64) Series { return num.EwmMean(x, alpha) }

// Ema 指数移动平均: alpha = 2/(N+1)。
func Ema(x Series, span int64) Series {
	period := 1.0
	if span > 0 {
		period = float64(span)
	}
	return num.EwmMean(x, 2.0/(period+1.0))
}

// Sma 中国式 SMA: alpha = M/N。
func Sma(x Series, span, weight int64) Series {
	period := 1.0
	if span != 0 {
		period = float64(span)
	}
	mass := 1.0
	if weight != 0 {
		mass = float64(weight)
	}
	return num.EwmMean(x, mass/period)
}

// Wma 加权移动平均。
func Wma(x Series, n int64) Series {
	return num.RollingApply(x, constantWindow(n, len(x)), func(b []float64) float64 {
		weighted := 0.0
		total := 0.0
		for i, v := range b {
			mass := float64(i + 1)
			weighted += mulOnly(mass, v)
			total += mass
		}
		return weighted / total
	})
}

// Dma 动态移动平均: 常量平滑因子。
func Dma(x Series, alpha float64) Series { return num.EwmMean(x, alpha) }

// DmaSeries 动态移动平均: 逐位置平滑因子, NaN 按 1 处理。
func DmaSeries(x Series, weights Series) Series {
	out := MakeSeries(len(x))
	if len(x) == 0 {
		return out
	}
	out[0] = x[0]
	for i := 1; i < len(x); i++ {
		alpha := 1.0
		if i < len(weights) && !math.IsNaN(weights[i]) {
			alpha = weights[i]
		}
		out[i] = mulOnly(alpha, x[i]) + mulOnly(1.0-alpha, out[i-1])
	}
	return out
}

// Std N 日总体标准差。
func Std(x Series, n int64) Series { return num.RollingStd(x, constantWindow(n, len(x))) }

// Avedev 平均绝对偏差。
func Avedev(x Series, n int64) Series         { return num.RollingAvedev(x, constantWindow(n, len(x))) }
func AvedevWindow(x Series, w []int64) Series { return num.RollingAvedev(x, w) }

// Slope 线性回归斜率。
func Slope(x Series, n int64) Series {
	return num.RollingApply(x, constantWindow(n, len(x)), num.Slope)
}

// Forcast 线性回归预测值。
func Forcast(x Series, n int64) Series {
	return num.RollingApply(x, constantWindow(n, len(x)), num.Forcast)
}
func ForcastWindow(x Series, w []int64) Series { return num.RollingApply(x, w, num.Forcast) }

// Last 从前 far 日到前 near 日是否一直满足条件。
func Last(cond Bools, far, near int64) Bools {
	out := make(Bools, len(cond))
	window := int(far + 1)
	for i := range cond {
		if i+1 < window {
			continue
		}
		good := true
		for offset := int(near); offset < window; offset++ {
			if !cond[i-offset] {
				good = false
				break
			}
		}
		out[i] = good
	}
	return out
}

// Count 最近 N 天满足条件的天数。
func Count(cond Bools, n int64) Series { return Sum(AsSeries(cond), constantWindow(n, len(cond))) }

// CountWindow 逐位置窗口计数。
func CountWindow(cond Bools, w []int64) Series { return Sum(AsSeries(cond), w) }

// Every 最近 N 天是否全为真。
func Every(cond Bools, n int64) Bools {
	total := Count(cond, n)
	out := make(Bools, len(total))
	for i, v := range total {
		out[i] = v == float64(n)
	}
	return out
}

// Exist 最近 N 天是否存在真值。
func Exist(cond Bools, n int64) Bools {
	total := Count(cond, n)
	out := make(Bools, len(total))
	for i, v := range total {
		out[i] = v > 0
	}
	return out
}

// Filter 信号过滤: 条件成立后 N 个周期置 0。
func Filter(cond Bools, n int64) Series {
	out := AsSeries(cond)
	for i := range out {
		if !IsTrue(out[i]) {
			continue
		}
		end := i + 1 + int(n)
		if end > len(out) {
			end = len(out)
		}
		for cursor := i + 1; cursor < end; cursor++ {
			out[cursor] = 0
		}
	}
	return out
}

// Barslast 上一次条件成立到当前的周期数。
func Barslast(cond Bools) Series {
	out := MakeSeries(len(cond))
	previous := int64(0)
	for i, flag := range cond {
		if flag {
			previous = 0
		} else {
			previous++
		}
		out[i] = float64(previous)
	}
	return out
}

// Barslastcount 连续满足条件的周期数。
func Barslastcount(cond Bools) Series {
	out := MakeSeries(len(cond))
	counter := int64(0)
	for i, flag := range cond {
		if flag {
			counter++
		} else {
			counter = 0
		}
		out[i] = float64(counter)
	}
	return out
}

// Barssincen N 周期内第一次条件成立到现在的周期数。
func Barssincen(cond Bools, n int64) Series {
	out := Filled(len(cond), 0)
	if n <= 0 {
		return out
	}
	size := int(n)
	for i := range cond {
		if i+1 < size {
			continue
		}
		begin := i + 1 - size
		position := 0
		for position < size && !cond[begin+position] {
			position++
		}
		if position == size {
			position = 0
		}
		if position != 0 || cond[begin] {
			out[i] = float64(size - 1 - position)
		}
	}
	return out
}

// Cross 向上金叉。
func Cross(fast, slow Series) Bools {
	previous := Less(Ref(fast, 1), Ref(slow, 1))
	current := GreaterEqual(fast, slow)
	return LogicAnd(previous, current)
}

// Longcross 维持 N 周期后上穿。
func Longcross(fast, slow Series, n int64) Bools {
	return LogicAnd(Last(Less(fast, slow), n, 1), Greater(fast, slow))
}

// Valuewhen 条件成立时取值, 否则沿用上一次。
func Valuewhen(cond Bools, values Series) Series {
	out := MakeSeries(len(cond))
	previous := math.NaN()
	seeded := false
	for i, flag := range cond {
		if flag && i < len(values) && !math.IsNaN(values[i]) {
			previous = values[i]
			seeded = true
		}
		if seeded {
			out[i] = previous
		}
	}
	return out
}

// Between 区间判断。
func Between(values Series, low, high float64) Bools {
	left := LogicAnd(GreaterScalar(values, low), LessScalar(values, high))
	right := LogicAnd(LessScalar(values, low), GreaterScalar(values, high))
	return LogicOr(left, right)
}

// Toprange 当前值距区间最高点的周期数。
func Toprange(x Series) Series {
	out := Filled(len(x), 0)
	for i := 1; i < len(x); i++ {
		cursor := 0
		for cursor < i && x[i-1-cursor] < x[i] {
			cursor++
		}
		if cursor != i {
			out[i] = float64(cursor)
		}
	}
	return out
}

// Lowrange 当前值距区间最低点的周期数。
func Lowrange(x Series) Series {
	out := Filled(len(x), 0)
	for i := 1; i < len(x); i++ {
		cursor := 0
		for cursor < i && x[i-1-cursor] > x[i] {
			cursor++
		}
		if cursor != i {
			out[i] = float64(cursor)
		}
	}
	return out
}

// Rd 对序列保留指定位数。
func Rd(x Series, digits int) Series { return num.DecimalSeries(x, digits) }

// Ztprice 涨停价: 前收盘价乘(1+比例), 按分四舍五入。
func Ztprice(price Series, ratio float64) Series {
	if ratio <= 0 {
		return append(Series(nil), price...)
	}
	out := make(Series, len(price))
	for i, p := range price {
		out[i] = ztpriceOne(p, ratio)
	}
	return out
}

type decimalParts struct {
	digits   string
	exponent int
	negative bool
}

func parseShortest(value float64) decimalParts {
	negative := math.Signbit(value)
	text := strconv.FormatFloat(math.Abs(value), 'f', -1, 64)
	exponent := 0
	if pos := strings.IndexAny(text, "eE"); pos >= 0 {
		exp, _ := strconv.Atoi(text[pos+1:])
		exponent = exp
		text = text[:pos]
	}
	fraction := 0
	if dot := strings.IndexByte(text, '.'); dot >= 0 {
		fraction = len(text) - dot - 1
		text = text[:dot] + text[dot+1:]
	}
	digits := strings.TrimLeft(text, "0")
	if digits == "" {
		digits = "0"
	}
	return decimalParts{digits: digits, exponent: exponent - fraction, negative: negative && digits != "0"}
}

func multiplyDigits(left, right string) string {
	li := make([]int, len(left))
	ri := make([]int, len(right))
	for i := range left {
		li[i] = int(left[i] - '0')
	}
	for i := range right {
		ri[i] = int(right[i] - '0')
	}
	acc := make([]int, len(li)+len(ri))
	for i := len(li) - 1; i >= 0; i-- {
		for j := len(ri) - 1; j >= 0; j-- {
			acc[i+j+1] += li[i] * ri[j]
		}
	}
	for k := len(acc) - 1; k > 0; k-- {
		acc[k-1] += acc[k] / 10
		acc[k] %= 10
	}
	var b strings.Builder
	for _, d := range acc {
		b.WriteByte(byte('0' + d))
	}
	out := strings.TrimLeft(b.String(), "0")
	if out == "" {
		out = "0"
	}
	return out
}

func quantizeCents(digits string, exponent int, negative bool) float64 {
	target := exponent + 2
	scaled := []byte(digits)
	if target >= 0 {
		scaled = append(scaled, strings.Repeat("0", target)...)
	} else {
		drop := -target
		var dropped byte = '0'
		if drop <= len(scaled) {
			dropped = scaled[len(scaled)-drop]
		}
		if drop > len(scaled) {
			scaled = []byte("0")
		} else {
			scaled = scaled[:len(scaled)-drop]
			if dropped >= '5' {
				cursor := len(scaled) - 1
				for cursor >= 0 {
					if scaled[cursor] == '9' {
						scaled[cursor] = '0'
						cursor--
					} else {
						scaled[cursor]++
						break
					}
				}
				if cursor < 0 {
					scaled = append([]byte{'1'}, scaled...)
				}
			}
			if len(scaled) == 0 {
				scaled = []byte("0")
			}
		}
	}
	var text string
	if len(scaled) <= 2 {
		text = "0." + strings.Repeat("0", 2-len(scaled)) + string(scaled)
	} else {
		text = string(scaled[:len(scaled)-2]) + "." + string(scaled[len(scaled)-2:])
	}
	if negative {
		text = "-" + text
	}
	value, _ := strconv.ParseFloat(text, 64)
	return value
}

func ztpriceOne(price, ratio float64) float64 {
	if ratio <= 0 || math.IsNaN(price) {
		return price
	}
	base := parseShortest(price)
	factor := parseShortest(1.0 + ratio)
	negative := base.negative != factor.negative
	digits := multiplyDigits(base.digits, factor.digits)
	return quantizeCents(digits, base.exponent+factor.exponent, negative)
}

// ---------------------------------------------------------------------------
// 2 级指标
// ---------------------------------------------------------------------------

// Macd 返回 (DIF, DEA, MACD)。
func Macd(close Series, fast, slow, signal int64) (Series, Series, Series) {
	dif := Sub(Ema(close, fast), Ema(close, slow))
	dea := Ema(dif, signal)
	histogram := MulScalar(Sub(dif, dea), 2.0)
	return Rd(dif, 3), Rd(dea, 3), Rd(histogram, 3)
}

// Kdj 返回 (K, D, J)。
func Kdj(close, high, low Series, period, m1, m2 int64) (Series, Series, Series) {
	rsv := MulScalar(Div(Sub(close, Llv(low, period)), Sub(Hhv(high, period), Llv(low, period))), 100.0)
	k := Ema(rsv, m1*2-1)
	d := Ema(k, m2*2-1)
	j := Sub(MulScalar(k, 3.0), MulScalar(d, 2.0))
	return k, d, j
}

// Rsi 相对强弱。
func Rsi(close Series, period int64) Series {
	delta := Sub(close, Ref(close, 1))
	up := Sma(MaximumScalar(delta, 0), period, 1)
	down := Sma(Abs(delta), period, 1)
	return Rd(MulScalar(Div(up, down), 100.0), 3)
}

// Wr 返回 (WR, WR1)。
func Wr(close, high, low Series, period, shortPeriod int64) (Series, Series) {
	one := func(span int64) Series {
		return MulScalar(Div(Sub(Hhv(high, span), close), Sub(Hhv(high, span), Llv(low, span))), 100.0)
	}
	return Rd(one(period), 3), Rd(one(shortPeriod), 3)
}

// Bias 返回 (BIAS1, BIAS2, BIAS3)。
func Bias(close Series, l1, l2, l3 int64) (Series, Series, Series) {
	one := func(span int64) Series {
		average := Ma(close, span)
		return MulScalar(Div(Sub(close, average), average), 100.0)
	}
	return Rd(one(l1), 3), Rd(one(l2), 3), Rd(one(l3), 3)
}

// Boll 返回 (UPPER, MID, LOWER)。
func Boll(close Series, period int64, width float64) (Series, Series, Series) {
	mid := Ma(close, period)
	band := MulScalar(Std(close, period), width)
	return Rd(Add(mid, band), 3), Rd(mid, 3), Rd(Sub(mid, band), 3)
}

// Psy 返回 (PSY, PSYMA)。
func Psy(close Series, period, averagePeriod int64) (Series, Series) {
	rising := Greater(close, Ref(close, 1))
	value := MulScalar(DivScalar(Count(rising, period), float64(period)), 100.0)
	return Rd(value, 3), Rd(Ma(value, averagePeriod), 3)
}

// Cci 顺势指标。
func Cci(close, high, low Series, period int64) Series {
	typical := DivScalar(Add(Add(high, low), close), 3.0)
	return Div(Sub(typical, Ma(typical, period)), MulScalar(Avedev(typical, period), 0.015))
}

// Atr 平均真实波幅。
func Atr(close, high, low Series, period int64) Series {
	previous := Ref(close, 1)
	tr := Maximum(Maximum(Sub(high, low), Abs(Sub(previous, high))), Abs(Sub(previous, low)))
	return Ma(tr, period)
}

// Bbi 多空指标。
func Bbi(close Series, m1, m2, m3, m4 int64) Series {
	return DivScalar(Add(Add(Ma(close, m1), Ma(close, m2)), Add(Ma(close, m3), Ma(close, m4))), 4.0)
}

// Dmi 返回 (PDI, MDI, ADX, ADXR)。
func Dmi(close, high, low Series, m1, m2 int64) (Series, Series, Series, Series) {
	previous := Ref(close, 1)
	tr := Sum(Maximum(Maximum(Sub(high, low), Abs(Sub(high, previous))), Abs(Sub(low, previous))), constantWindow(m1, len(close)))
	highDelta := Sub(high, Ref(high, 1))
	lowDelta := Sub(Ref(low, 1), low)
	up := LogicAnd(GreaterScalar(highDelta, 0), Greater(highDelta, lowDelta))
	down := LogicAnd(GreaterScalar(lowDelta, 0), Greater(lowDelta, highDelta))
	dmp := Sum(IfSelectFalse(up, highDelta, 0), constantWindow(m1, len(close)))
	dmm := Sum(IfSelectFalse(down, lowDelta, 0), constantWindow(m1, len(close)))
	pdi := Div(MulScalar(dmp, 100.0), tr)
	mdi := Div(MulScalar(dmm, 100.0), tr)
	adx := Ma(MulScalar(Div(Abs(Sub(mdi, pdi)), Add(pdi, mdi)), 100.0), m2)
	adxr := DivScalar(Add(adx, Ref(adx, m2)), 2.0)
	return pdi, mdi, adx, adxr
}

// Taq 返回 (UP, MID, DOWN)。
func Taq(high, low Series, period int64) (Series, Series, Series) {
	up := Hhv(high, period)
	down := Llv(low, period)
	mid := DivScalar(Add(up, down), 2.0)
	return up, mid, down
}

// Ktn 返回 (UPPER, MID, LOWER)。
func Ktn(close, high, low Series, period, atrPeriod int64) (Series, Series, Series) {
	mid := Ema(DivScalar(Add(Add(high, low), close), 3.0), period)
	band := MulScalar(Atr(close, high, low, atrPeriod), 2.0)
	return Add(mid, band), mid, Sub(mid, band)
}

// Trix 返回 (TRIX, TRMA)。
func Trix(close Series, m1, m2 int64) (Series, Series) {
	smoothed := Ema(Ema(Ema(close, m1), m1), m1)
	value := MulScalar(Div(Sub(smoothed, Ref(smoothed, 1)), Ref(smoothed, 1)), 100.0)
	return value, Ma(value, m2)
}

// Vr 成交量比率。
func Vr(close, volume Series, period int64) Series {
	rising := Greater(close, Ref(close, 1))
	falling := LessEqual(close, Ref(close, 1))
	return MulScalar(Div(
		Sum(IfSelectFalse(rising, volume, 0), constantWindow(period, len(close))),
		Sum(IfSelectFalse(falling, volume, 0), constantWindow(period, len(close))),
	), 100.0)
}

// Emv 返回 (EMV, MAEMV)。
func Emv(high, low, volume Series, period, averagePeriod int64) (Series, Series) {
	volumeRatio := Div(Ma(volume, period), volume)
	mid := MulScalar(Div(Sub(Add(high, low), Ref(Add(high, low), 1)), Add(high, low)), 100.0)
	value := Ma(Div(Mul(Mul(mid, volumeRatio), Sub(high, low)), Ma(Sub(high, low), period)), period)
	return value, Ma(value, averagePeriod)
}

// Dpo 返回 (DPO, MADPO)。
func Dpo(close Series, m1, m2, m3 int64) (Series, Series) {
	value := Sub(close, Ref(Ma(close, m1), m2))
	return value, Ma(value, m3)
}

// Brar 返回 (AR, BR)。
func Brar(open, close, high, low Series, period int64) (Series, Series) {
	ar := MulScalar(Div(
		Sum(Sub(high, open), constantWindow(period, len(close))),
		Sum(Sub(open, low), constantWindow(period, len(close))),
	), 100.0)
	br := MulScalar(Div(
		Sum(MaximumScalar(Sub(high, Ref(close, 1)), 0), constantWindow(period, len(close))),
		Sum(MaximumScalar(Sub(Ref(close, 1), low), 0), constantWindow(period, len(close))),
	), 100.0)
	return ar, br
}

// Dfma 返回 (DIF, DIFMA)。
func Dfma(close Series, n1, n2, m int64) (Series, Series) {
	value := Sub(Ma(close, n1), Ma(close, n2))
	return value, Ma(value, m)
}

// Mtm 返回 (MTM, MTMMA)。
func Mtm(close Series, period, averagePeriod int64) (Series, Series) {
	value := Sub(close, Ref(close, period))
	return value, Ma(value, averagePeriod)
}

// Mass 返回 (MASS, MA_MASS)。
func Mass(high, low Series, n1, n2, m int64) (Series, Series) {
	rangeSeries := Sub(high, low)
	value := Sum(Div(Ma(rangeSeries, n1), Ma(Ma(rangeSeries, n1), n1)), constantWindow(n2, len(high)))
	return value, Ma(value, m)
}

// Roc 返回 (ROC, MAROC)。
func Roc(close Series, period, averagePeriod int64) (Series, Series) {
	lagged := Ref(close, period)
	value := MulScalar(Div(Sub(close, lagged), lagged), 100.0)
	return value, Ma(value, averagePeriod)
}

// Expma 返回 (EMA(N1), EMA(N2))。
func Expma(close Series, n1, n2 int64) (Series, Series) { return Ema(close, n1), Ema(close, n2) }

// Obv 能量潮。
func Obv(close, volume Series) Series {
	rising := Greater(close, Ref(close, 1))
	falling := Less(close, Ref(close, 1))
	signed := IfSelect(rising, volume, IfSelectFalse(falling, MulScalar(volume, -1), 0))
	return DivScalar(Cumsum(signed), 10000.0)
}

// Mfi 资金流量指标。
func Mfi(close, high, low, volume Series, period int64) Series {
	typical := DivScalar(Add(Add(high, low), close), 3.0)
	up := Greater(typical, Ref(typical, 1))
	down := Less(typical, Ref(typical, 1))
	ratio := Div(
		Sum(IfSelectFalse(up, Mul(typical, volume), 0), constantWindow(period, len(close))),
		Sum(IfSelectFalse(down, Mul(typical, volume), 0), constantWindow(period, len(close))),
	)
	return ScalarSub(100.0, ScalarDiv(100.0, AddScalar(ratio, 1.0)))
}

// Asi 返回 (ASI, ASIT)。
func Asi(open, close, high, low Series, m1, m2 int64) (Series, Series) {
	previousClose := Ref(close, 1)
	aa := Abs(Sub(high, previousClose))
	bb := Abs(Sub(low, previousClose))
	cc := Abs(Sub(high, Ref(low, 1)))
	dd := Abs(Sub(previousClose, Ref(open, 1)))
	firstBranch := LogicAnd(Greater(aa, bb), Greater(aa, cc))
	secondBranch := LogicAnd(Greater(bb, cc), Greater(bb, aa))
	branchOne := Add(Add(aa, DivScalar(bb, 2.0)), DivScalar(dd, 4.0))
	branchTwo := Add(Add(bb, DivScalar(aa, 2.0)), DivScalar(dd, 4.0))
	branchThree := Add(cc, DivScalar(dd, 4.0))
	r := IfSelect(firstBranch, branchOne, IfSelect(secondBranch, branchTwo, branchThree))
	x := Add(Add(Sub(close, previousClose), DivScalar(Sub(close, open), 2.0)), Sub(previousClose, Ref(open, 1)))
	si := Div(MulScalar(Div(x, r), 16.0), Maximum(aa, bb))
	value := Sum(si, constantWindow(m1, len(close)))
	return value, Ma(value, m2)
}

// Xsii 返回 (TD1, TD2, TD3, TD4)。
func Xsii(close, high, low Series, n, m int64) (Series, Series, Series, Series) {
	typical := DivScalar(Add(Add(MulScalar(close, 2.0), high), low), 4.0)
	aa := Ma(typical, 5)
	td1 := MulScalar(aa, float64(n)/100.0)
	td2 := MulScalar(aa, float64(200-n)/100.0)
	closeMa := Ma(close, 20)
	cc := Div(Abs(Sub(typical, closeMa)), closeMa)
	dd := DmaSeries(close, cc)
	td3 := MulScalar(dd, 1.0+float64(m)/100.0)
	td4 := MulScalar(dd, 1.0-float64(m)/100.0)
	return td1, td2, td3, td4
}
