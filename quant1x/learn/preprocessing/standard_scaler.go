package preprocessing

import (
	"errors"
	"fmt"
	"math"
)

// 预定义错误(全局变量)
var (
	ErrEmptyData          = errors.New("preprocessing: cannot fit on empty data")
	ErrZeroFeatures       = errors.New("preprocessing: each sample must have at least one feature")
	ErrNotFitted          = errors.New("preprocessing: StandardScaler must be fitted before transform")
	ErrInconsistentDimFit = errors.New("preprocessing: inconsistent feature dimensions during fit")
	ErrInconsistentDim    = errors.New("preprocessing: feature dimension mismatch during transform")
)

// StandardScaler 标准化器, 将数据标准化为均值为0, 标准差为1
type StandardScaler struct {
	Mean   []float64 // 均值(导出用于外部查看)
	Std    []float64 // 标准差(导出用于外部查看)
	n      int       // 样本数量
	fitted bool      // 是否已完成拟合
}

// NewStandardScaler 创建新的标准化器
func NewStandardScaler() *StandardScaler {
	return &StandardScaler{}
}

// Fit 使用Welford在线算法拟合数据, 计算均值和标准差
//
//	支持多维数据(每行是一个样本, 每列是一个特征)
func (s *StandardScaler) Fit(data [][]float64) (*StandardScaler, error) {
	if len(data) == 0 {
		return nil, ErrEmptyData
	}
	if len(data[0]) == 0 {
		return nil, ErrZeroFeatures
	}

	nFeatures := len(data[0])
	s.Mean = make([]float64, nFeatures)
	m2 := make([]float64, nFeatures) // 二阶矩(用于计算方差)
	s.n = 0

	// Welford 在线算法
	for _, sample := range data {
		if len(sample) != nFeatures {
			return nil, fmt.Errorf("inconsistent feature dimension: expected %d, got %d", nFeatures, len(sample))
		}
		s.n++
		for j := 0; j < nFeatures; j++ {
			x := sample[j]
			delta := x - s.Mean[j]
			s.Mean[j] += delta / float64(s.n)
			delta2 := x - s.Mean[j]
			m2[j] += delta * delta2
		}
	}

	// 计算标准差
	s.Std = make([]float64, nFeatures)
	for j := 0; j < nFeatures; j++ {
		variance := m2[j] / float64(s.n)
		if variance < 0 {
			variance = 0 // 防止浮点误差导致负数
		}
		std := math.Sqrt(variance)
		if std == 0 {
			std = 1.0 // 防止除以0
		}
		s.Std[j] = std
	}

	s.fitted = true
	return s, nil
}

// Transform 对数据进行标准化: z = (x - mean) / std
func (s *StandardScaler) Transform(data [][]float64) ([][]float64, error) {
	if !s.fitted {
		return nil, ErrNotFitted
	}
	if len(data) == 0 {
		return [][]float64{}, nil // 空数据返回空结果
	}

	nFeatures := len(s.Mean)
	result := make([][]float64, len(data))

	for i, point := range data {
		if len(point) != nFeatures {
			return nil, fmt.Errorf("feature dimension mismatch at index %d: expected %d, got %d", i, nFeatures, len(point))
		}
		newPoint := make([]float64, nFeatures)
		for j := 0; j < nFeatures; j++ {
			newPoint[j] = (point[j] - s.Mean[j]) / s.Std[j]
		}
		result[i] = newPoint
	}
	return result, nil
}

// FitTransform 一步完成拟合并转换
func (s *StandardScaler) FitTransform(data [][]float64) ([][]float64, error) {
	if _, err := s.Fit(data); err != nil {
		return nil, err
	}
	return s.Transform(data)
}

// InverseTransform 将标准化后的数据还原
func (s *StandardScaler) InverseTransform(data [][]float64) ([][]float64, error) {
	if !s.fitted {
		return nil, ErrNotFitted
	}
	if len(data) == 0 {
		return [][]float64{}, nil
	}

	nFeatures := len(s.Mean)
	result := make([][]float64, len(data))

	for i, point := range data {
		if len(point) != nFeatures {
			return nil, fmt.Errorf("feature dimension mismatch in inverse transform at index %d", i)
		}
		newPoint := make([]float64, nFeatures)
		for j := 0; j < nFeatures; j++ {
			newPoint[j] = point[j]*s.Std[j] + s.Mean[j]
		}
		result[i] = newPoint
	}
	return result, nil
}
