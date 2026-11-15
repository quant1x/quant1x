// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

// Package scoring 加权评分：按维度累加得分，再按权重合成总分。
package scoring

import (
	"errors"
)

// ScoreComponent 评分项
type ScoreComponent struct {
	Name         string  // 维度名称
	CustomWeight float64 // 自定义权重，仅当 IsAutoWeight 为 false 时有效
	Score        float64 // 维度得分，取值 0-100
	IsAutoWeight bool    // 是否自动分配权重
}

// ScoreCalculator 评分系统
type ScoreCalculator struct {
	components      []ScoreComponent
	computedWeights map[string]float64 // 最近一次计算得到的实际权重
}

// NewScoreCalculator 创建评分系统实例
func NewScoreCalculator() *ScoreCalculator {
	return &ScoreCalculator{
		components:      make([]ScoreComponent, 0),
		computedWeights: make(map[string]float64),
	}
}

// AddComponent 添加评分项，customWeight 省略时该维度参与自动分配权重
func (s *ScoreCalculator) AddComponent(name string, score float64, customWeight ...float64) error {
	if score < 0 || score > 100 {
		return errors.New("score must be between 0 and 100")
	}

	c := ScoreComponent{
		Name:         name,
		Score:        score,
		IsAutoWeight: true,
	}

	if len(customWeight) > 0 {
		if len(customWeight) > 1 {
			return errors.New("too many weight parameters")
		}
		if customWeight[0] < 0 {
			return errors.New("weight cannot be negative")
		}
		c.CustomWeight = customWeight[0]
		c.IsAutoWeight = false
	}

	s.components = append(s.components, c)
	return nil
}

// CalculateWeightedScore 计算加权总分：自定义权重先扣除，余量在自动分配项间均分
func (s *ScoreCalculator) CalculateWeightedScore() (float64, error) {
	s.computedWeights = make(map[string]float64)

	totalCustomWeight := 0.0
	autoCount := 0
	for _, c := range s.components {
		if !c.IsAutoWeight {
			totalCustomWeight += c.CustomWeight
		} else {
			autoCount++
		}
	}

	if totalCustomWeight > 1 {
		return 0, errors.New("total custom weight exceeds 1")
	}
	remainingWeight := 1 - totalCustomWeight
	if remainingWeight < 0 {
		return 0, errors.New("invalid weight distribution")
	}

	autoWeight := 0.0
	if autoCount > 0 {
		autoWeight = remainingWeight / float64(autoCount)
	}

	totalScore := 0.0
	for _, c := range s.components {
		actualWeight := autoWeight
		if !c.IsAutoWeight {
			actualWeight = c.CustomWeight
		}

		s.computedWeights[c.Name] = actualWeight
		totalScore += actualWeight * c.Score
	}

	return totalScore, nil
}

// WeightDistribution 获取最近一次计算的实际权重分布（返回副本，改动不影响内部状态）
func (s *ScoreCalculator) WeightDistribution() map[string]float64 {
	out := make(map[string]float64, len(s.computedWeights))
	for name, weight := range s.computedWeights {
		out[name] = weight
	}
	return out
}
