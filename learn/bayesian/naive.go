package bayesian

import (
	"math"
	"sync"
)

// NaiveBayes 泛型朴素贝叶斯分类器
type NaiveBayes[K comparable, C comparable] struct {
	mu            float64         // 平滑系数
	classCounts   map[C]int       // 类别计数
	featureCounts map[C]map[K]int // 特征计数
	featureTotals map[C]int       // 各类别特征总数
	vocabulary    map[K]struct{}  // 特征词汇表
	totalSamples  int             // 总样本数
	lock          sync.RWMutex    // 并发安全锁
}

// NewNaiveBayes 创建新的分类器
func NewNaiveBayes[K comparable, C comparable](smoothing float64) *NaiveBayes[K, C] {
	return &NaiveBayes[K, C]{
		mu:            smoothing,
		classCounts:   make(map[C]int),
		featureCounts: make(map[C]map[K]int),
		featureTotals: make(map[C]int),
		vocabulary:    make(map[K]struct{}),
	}
}

// Train 训练模型（修复特征计数逻辑）
func (nb *NaiveBayes[K, C]) Train(samples []map[K]bool, labels []C) {
	nb.lock.Lock()
	defer nb.lock.Unlock()

	for i, features := range samples {
		label := labels[i]
		nb.classCounts[label]++
		nb.totalSamples++

		if _, exists := nb.featureCounts[label]; !exists {
			nb.featureCounts[label] = make(map[K]int)
		}

		for feature, present := range features {
			if present {
				nb.featureCounts[label][feature]++
				nb.featureTotals[label]++
				nb.vocabulary[feature] = struct{}{}
			}
		}
	}
}

// Predict 预测类别（修复概率计算）
func (nb *NaiveBayes[K, C]) Predict(features map[K]bool) C {
	nb.lock.RLock()
	defer nb.lock.RUnlock()

	maxProb := math.Inf(-1)
	var bestClass C

	vocabSize := len(nb.vocabulary)

	for class, total := range nb.classCounts {
		// 先验概率
		classPrior := math.Log(float64(total) / float64(nb.totalSamples))

		// 似然计算
		var likelihood float64
		for feature, present := range features {
			if !present {
				continue
			}

			count := nb.featureCounts[class][feature]
			totalFeatures := nb.featureTotals[class]

			prob := (float64(count) + nb.mu) /
				(float64(totalFeatures) + nb.mu*float64(vocabSize))
			likelihood += math.Log(prob)
		}

		if classPrior+likelihood > maxProb {
			maxProb = classPrior + likelihood
			bestClass = class
		}
	}

	return bestClass
}
