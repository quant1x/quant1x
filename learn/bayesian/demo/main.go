package main

import (
	"fmt"
	"math"
	"strings"
)

type NaiveBayes struct {
	classCounts    map[string]int            // 类别计数
	featureCounts  map[string]map[string]int // 特征计数 per class
	classFeatures  map[string]int            // 每个类别的特征总数
	totalDocuments int                       // 总文档数
	vocabulary     map[string]bool           // 词汇表
}

func NewNaiveBayes() *NaiveBayes {
	return &NaiveBayes{
		classCounts:   make(map[string]int),
		featureCounts: make(map[string]map[string]int),
		classFeatures: make(map[string]int),
		vocabulary:    make(map[string]bool),
	}
}

// 训练模型
func (nb *NaiveBayes) Train(documents []string, labels []string) {
	for i, doc := range documents {
		label := labels[i]
		nb.classCounts[label]++
		nb.totalDocuments++

		features := nb.extractFeatures(doc)

		if _, exists := nb.featureCounts[label]; !exists {
			nb.featureCounts[label] = make(map[string]int)
		}

		for _, feature := range features {
			nb.featureCounts[label][feature]++
			nb.classFeatures[label]++
			nb.vocabulary[feature] = true
		}
	}
}

// 预测类别
func (nb *NaiveBayes) Predict(document string) string {
	features := nb.extractFeatures(document)
	var bestClass string
	maxScore := math.Inf(-1)

	for class := range nb.classCounts {
		score := nb.calculateClassScore(class, features)
		if score > maxScore {
			maxScore = score
			bestClass = class
		}
	}

	return bestClass
}

// 计算类别得分（使用对数防止下溢）
func (nb *NaiveBayes) calculateClassScore(class string, features []string) float64 {
	// 先验概率 P(class)
	classPrior := math.Log(float64(nb.classCounts[class]) / float64(nb.totalDocuments))

	// 似然 P(features|class)
	var likelihood float64
	vocabSize := len(nb.vocabulary)

	for _, feature := range features {
		count := nb.featureCounts[class][feature] + 1 // 拉普拉斯平滑
		total := nb.classFeatures[class] + vocabSize
		likelihood += math.Log(float64(count) / float64(total))
	}

	return classPrior + likelihood
}

// 特征提取（简单分词处理）
func (nb *NaiveBayes) extractFeatures(document string) []string {
	// 转换为小写并分割单词
	words := strings.Fields(strings.ToLower(document))

	// 简单清洗数据，移除标点
	for i, word := range words {
		words[i] = strings.Trim(word, ",.!?;:\"'")
	}

	return words
}

func main() {
	// 示例训练数据
	documents := []string{
		"good amazing great awesome",
		"bad terrible awful horrible",
		"nice cool sweet positive",
		"ugly nasty negative",
	}

	labels := []string{"pos", "neg", "pos", "neg"}

	// 初始化并训练模型
	nb := NewNaiveBayes()
	nb.Train(documents, labels)

	// 测试预测
	testDocs := []string{
		"awesome positive experience",
		"terrible awful situation",
		"mixed feelings here",
	}

	for _, doc := range testDocs {
		fmt.Printf("Document: '%s' => Predicted class: %s\n", doc, nb.Predict(doc))
	}
}
