// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package scoring

import (
	"errors"
	"fmt"
)

// RangeProcessor 区间处理器：把落入 [Min, Max] 的原始值线性映射为 0-100 分
type RangeProcessor struct {
	Min, Max float64
}

// Process 计算得分：低于 Min 记 0 分，高于 Max 记 100 分，区间内线性映射
func (p *RangeProcessor) Process(data interface{}) (float64, error) {
	val, ok := data.(float64)
	if !ok {
		return 0, fmt.Errorf("invalid data type")
	}

	if p.Max < p.Min {
		return 0, errors.New("range max must not be less than min")
	}
	if p.Max == p.Min {
		if val < p.Min {
			return 0, nil
		}
		return 100, nil
	}
	if val < p.Min {
		return 0, nil
	}
	if val > p.Max {
		return 100, nil
	}
	return (val - p.Min) / (p.Max - p.Min) * 100, nil
}

// ThresholdProcessor 阈值处理器：以阈值为 100 分按比例折算
type ThresholdProcessor struct {
	Threshold float64
}

// Process 计算得分：达到阈值记 100 分，未达阈值按 val/Threshold 折算
func (p *ThresholdProcessor) Process(data interface{}) (float64, error) {
	val, ok := data.(float64)
	if !ok {
		return 0, fmt.Errorf("invalid data type")
	}

	if p.Threshold <= 0 {
		return 0, errors.New("threshold must be positive")
	}
	if val >= p.Threshold {
		return 100, nil
	}
	return val / p.Threshold * 100, nil
}
