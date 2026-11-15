// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package scoring

import (
	"math"
	"testing"
)

const epsilon = 1e-9

func assertScore(t *testing.T, name string, got, want float64) {
	t.Helper()
	if math.Abs(got-want) > epsilon {
		t.Errorf("%s: got %v, want %v", name, got, want)
	}
}

func TestScoreCalculatorMixedWeight(t *testing.T) {
	sc := NewScoreCalculator()

	if err := sc.AddComponent("用户体验", 80); err != nil {
		t.Fatalf("AddComponent(用户体验) 失败: %v", err)
	}
	if err := sc.AddComponent("性能", 90, 0.3); err != nil {
		t.Fatalf("AddComponent(性能) 失败: %v", err)
	}
	if err := sc.AddComponent("安全性", 70, 0.5); err != nil {
		t.Fatalf("AddComponent(安全性) 失败: %v", err)
	}
	if err := sc.AddComponent("兼容性", 85); err != nil {
		t.Fatalf("AddComponent(兼容性) 失败: %v", err)
	}

	// 自定义权重合计 0.8, 余量 0.2 在两个自动分配项间均分, 各 0.1
	score, err := sc.CalculateWeightedScore()
	if err != nil {
		t.Fatalf("CalculateWeightedScore 失败: %v", err)
	}
	assertScore(t, "加权总分", score, 78.5)

	weights := sc.WeightDistribution()
	assertScore(t, "用户体验权重", weights["用户体验"], 0.1)
	assertScore(t, "性能权重", weights["性能"], 0.3)
	assertScore(t, "安全性权重", weights["安全性"], 0.5)
	assertScore(t, "兼容性权重", weights["兼容性"], 0.1)
}

func TestScoreCalculatorAllAutoWeight(t *testing.T) {
	sc := NewScoreCalculator()
	for _, item := range []struct {
		name  string
		score float64
	}{{"a", 100}, {"b", 0}} {
		if err := sc.AddComponent(item.name, item.score); err != nil {
			t.Fatalf("AddComponent(%s) 失败: %v", item.name, err)
		}
	}

	score, err := sc.CalculateWeightedScore()
	if err != nil {
		t.Fatalf("CalculateWeightedScore 失败: %v", err)
	}
	assertScore(t, "均匀权重下总分", score, 50)

	weights := sc.WeightDistribution()
	assertScore(t, "a 权重", weights["a"], 0.5)
	assertScore(t, "b 权重", weights["b"], 0.5)
}

func TestScoreCalculatorEmpty(t *testing.T) {
	sc := NewScoreCalculator()
	score, err := sc.CalculateWeightedScore()
	if err != nil {
		t.Fatalf("空评分项不应报错: %v", err)
	}
	assertScore(t, "空评分项总分", score, 0)
	if len(sc.WeightDistribution()) != 0 {
		t.Errorf("空评分项的权重分布应为空, 实际 %d 项", len(sc.WeightDistribution()))
	}
}

func TestScoreCalculatorAddComponentError(t *testing.T) {
	cases := []struct {
		name   string
		score  float64
		weight []float64
	}{
		{"负分", -1, nil},
		{"超上限", 101, nil},
		{"负权重", 60, []float64{-0.1}},
		{"权重参数过多", 60, []float64{0.1, 0.2}},
	}

	for _, c := range cases {
		sc := NewScoreCalculator()
		if err := sc.AddComponent(c.name, c.score, c.weight...); err == nil {
			t.Errorf("%s: 期望返回错误, 实际为 nil", c.name)
		}
	}
}

func TestScoreCalculatorCustomWeightExceedsOne(t *testing.T) {
	sc := NewScoreCalculator()
	if err := sc.AddComponent("a", 80, 0.6); err != nil {
		t.Fatalf("AddComponent(a) 失败: %v", err)
	}
	if err := sc.AddComponent("b", 90, 0.6); err != nil {
		t.Fatalf("AddComponent(b) 失败: %v", err)
	}

	if _, err := sc.CalculateWeightedScore(); err == nil {
		t.Error("自定义权重合计超过 1 应返回错误")
	}
}

func TestScoreCalculatorWeightDistributionIsCopy(t *testing.T) {
	sc := NewScoreCalculator()
	if err := sc.AddComponent("a", 100, 1); err != nil {
		t.Fatalf("AddComponent(a) 失败: %v", err)
	}
	if _, err := sc.CalculateWeightedScore(); err != nil {
		t.Fatalf("CalculateWeightedScore 失败: %v", err)
	}

	weights := sc.WeightDistribution()
	weights["a"] = 0

	assertScore(t, "外部改动不应污染内部权重", sc.WeightDistribution()["a"], 1)
}

func TestRangeProcessor(t *testing.T) {
	p := &RangeProcessor{Min: 0, Max: 10}
	cases := []float64{-5, 0, 2.5, 10, 15}
	wants := []float64{0, 0, 25, 100, 100}

	for i, val := range cases {
		got, err := p.Process(val)
		if err != nil {
			t.Fatalf("Process(%v) 失败: %v", val, err)
		}
		assertScore(t, "区间得分", got, wants[i])
	}
}

func TestRangeProcessorDegenerateRange(t *testing.T) {
	p := &RangeProcessor{Min: 5, Max: 5}
	cases := []float64{4, 5, 6}
	wants := []float64{0, 100, 100}

	for i, val := range cases {
		got, err := p.Process(val)
		if err != nil {
			t.Fatalf("Process(%v) 失败: %v", val, err)
		}
		assertScore(t, "零宽区间得分", got, wants[i])
	}
}

func TestRangeProcessorError(t *testing.T) {
	if _, err := (&RangeProcessor{Min: 10, Max: 0}).Process(5.0); err == nil {
		t.Error("Max 小于 Min 应返回错误")
	}
	if _, err := (&RangeProcessor{Min: 0, Max: 10}).Process("5"); err == nil {
		t.Error("非 float64 入参应返回错误")
	}
	if _, err := (&RangeProcessor{Min: 0, Max: 10}).Process(5); err == nil {
		t.Error("int 入参应返回错误")
	}
}

func TestThresholdProcessor(t *testing.T) {
	p := &ThresholdProcessor{Threshold: 50}
	cases := []float64{0, 25, 50, 80}
	wants := []float64{0, 50, 100, 100}

	for i, val := range cases {
		got, err := p.Process(val)
		if err != nil {
			t.Fatalf("Process(%v) 失败: %v", val, err)
		}
		assertScore(t, "阈值得分", got, wants[i])
	}
}

func TestThresholdProcessorError(t *testing.T) {
	if _, err := (&ThresholdProcessor{Threshold: 0}).Process(10.0); err == nil {
		t.Error("阈值为 0 应返回错误")
	}
	if _, err := (&ThresholdProcessor{Threshold: -1}).Process(10.0); err == nil {
		t.Error("阈值为负应返回错误")
	}
	if _, err := (&ThresholdProcessor{Threshold: 50}).Process(10); err == nil {
		t.Error("非 float64 入参应返回错误")
	}
}
