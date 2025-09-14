package cluster

import (
	"fmt"
	"math"

	"gitee.com/quant1x/num"
)

// DataExtractor 提取可用于聚类的数值样本集
//
//	聚类数据提取接口
type DataExtractor interface {
	Extract(data any) [][]float64
}

func find_4plus1_clusters_basic(X_scaled [][]float64, max_iterations int) (best_params map[string]float64, best_labels []int) {
	best_score := -1.0
	best_labels = nil
	best_params = nil

	eps_values := num.Linspace(0.1, 2.0, 20)
	min_samples_values := []int{5, 10, 15, 20, 25, 30}
	iteration := 0

	for _, eps := range eps_values {
		for _, min_samples := range min_samples_values {
			iteration++
			if iteration > max_iterations {
				break
			}

			dbscan := &DBSCAN{
				Eps:    eps,
				MinPts: min_samples,
			}
			labels := dbscan.Fit(X_scaled)

			// 计算簇数和噪声数
			labelSet := make(map[int]bool)
			n_noise := 0
			for _, label := range labels {
				if label == -1 {
					n_noise++
				} else {
					labelSet[label] = true
				}
			}
			n_clusters := len(labelSet)
			noise_ratio := float64(n_noise) / float64(len(labels))

			// 评分标准：正好4个簇，噪声比例10-30%
			if n_clusters == 4 {
				cluster_quality := 1.0
				noise_quality := 1.0 - math.Abs(noise_ratio-0.2) // 20%噪声比例理想
				score := cluster_quality*0.7 + noise_quality*0.3

				if score > best_score {
					best_score = score
					best_params = map[string]float64{
						"eps":         eps,
						"min_samples": float64(min_samples),
					}
					best_labels = make([]int, len(labels))
					copy(best_labels, labels)
					fmt.Printf("找到4簇方案: eps=%.2f, min_samples=%d, 噪声比例=%.1f%%\n", eps, min_samples, noise_ratio*100)
				}
			}
		}
		if iteration > max_iterations {
			break
		}
	}

	if best_params == nil {
		fmt.Println("⚠️ 未找到4簇方案，使用最近似方案")
		// 备用方案：寻找最接近4簇的参数
		for _, eps := range eps_values {
			for _, min_samples := range min_samples_values {
				dbscan := &DBSCAN{
					Eps:    eps,
					MinPts: min_samples,
				}
				labels := dbscan.Fit(X_scaled)

				labelSet := make(map[int]bool)
				for _, label := range labels {
					if label != -1 {
						labelSet[label] = true
					}
				}
				n_clusters := len(labelSet)

				if math.Abs(float64(n_clusters)-4) <= 1 { // 3-5个簇
					best_params = map[string]float64{
						"eps":         eps,
						"min_samples": float64(min_samples),
					}
					best_labels = make([]int, len(labels))
					copy(best_labels, labels)
					return best_params, best_labels
				}
			}
		}
	}

	return best_params, best_labels
}

func find_4plus1_clusters(X_scaled [][]float64, max_iterations int) (best_params map[string]float64, best_labels []int) {
	best_score := -1.0
	best_labels = nil
	best_params = nil

	// 生成所有参数组合
	type Param struct {
		eps        float64
		minSamples int
	}

	var params []Param
	epsValues := num.Linspace(0.1, 2.0, 20)
	minSamplesValues := []int{5, 10, 15, 20, 25, 30, 35, 40, 45, 50}
	for _, eps := range epsValues {
		for _, minSamp := range minSamplesValues {
			params = append(params, Param{eps: eps, minSamples: minSamp})
		}
	}

	// 截断到最多 max_iterations 次
	if len(params) > max_iterations {
		params = params[:max_iterations]
	}

	// 主搜索：寻找正好 4 个簇
	for _, p := range params {
		dbscan := &DBSCAN{
			Eps:    p.eps,
			MinPts: p.minSamples,
		}
		labels := dbscan.Fit(X_scaled)

		n_clusters, n_noise := 0, 0
		labelSet := make(map[int]bool)
		for _, label := range labels {
			if label == -1 {
				n_noise++
			} else {
				labelSet[label] = true
			}
		}
		n_clusters = len(labelSet)
		noise_ratio := float64(n_noise) / float64(len(labels))

		if n_clusters == 4 {
			noise_quality := 1.0 - math.Abs(noise_ratio-0.2)
			score := 0.7 + noise_quality*0.3 // cluster_quality = 1.0

			if score > best_score {
				best_score = score
				best_params = map[string]float64{
					"eps":         p.eps,
					"min_samples": float64(p.minSamples),
				}
				best_labels = make([]int, len(labels))
				copy(best_labels, labels)
				fmt.Printf("找到4簇方案: eps=%.2f, min_samples=%d, 噪声比例=%.1f%%\n", p.eps, p.minSamples, noise_ratio*100)
			}
		}
	}

	// 备选：3~5 个簇
	if best_params == nil {
		fmt.Println("⚠️ 未找到4簇方案，使用最近似方案")
		for _, p := range params {
			dbscan := &DBSCAN{
				Eps:    p.eps,
				MinPts: p.minSamples,
			}
			labels := dbscan.Fit(X_scaled)

			labelSet := make(map[int]bool)
			for _, label := range labels {
				if label != -1 {
					labelSet[label] = true
				}
			}
			n_clusters := len(labelSet)

			if n_clusters >= 3 && n_clusters <= 5 {
				best_params = map[string]float64{
					"eps":         p.eps,
					"min_samples": float64(p.minSamples),
				}
				best_labels = make([]int, len(labels))
				copy(best_labels, labels)
				fmt.Printf("使用近似方案: eps=%.2f, min_samples=%d, 簇数=%d\n", p.eps, p.minSamples, n_clusters)
				return best_params, best_labels
			}
		}
	}

	return best_params, best_labels
}
