package bayesian

import (
	"fmt"
	"testing"
)

type Feature int // 自定义特征类型

const (
	Age Feature = iota
	Income
	CreditScore
)

func TestNewNaiveBayes(t *testing.T) {
	// 初始化分类器(使用字符串作为特征和类别)
	nb := NewNaiveBayes[string, string](1.0)

	// 正确的训练数据格式
	samples := []map[string]bool{
		{"viagra": true, "cash": true, "win": true},
		{"hello": true, "meeting": true, "free": true},
	}
	labels := []string{"spam", "ham"}

	nb.Train(samples, labels)

	// 测试样本
	testEmail := map[string]bool{
		"viagra": true,
		"free":   true,
	}

	fmt.Printf("Predicted class: %s\n", nb.Predict(testEmail)) // 应该输出: spam
}
