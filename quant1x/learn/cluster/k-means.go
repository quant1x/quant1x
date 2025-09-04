package cluster

import (
	"math"
	"math/rand"
	"time"
)

const (
	// kmeansSeed 用于固定 KMeans 的随机源，确保结果可复现
	// 设为 -1 表示使用时间随机；>=0 表示固定种子
	kmeansSeed = 89 // 可复现模式
	// kmeansSeed = -1 // 随机模式
)

// KMeans K-means聚类器
type KMeans struct {
	K             int         // 聚类数量
	MaxIterations int         // 最大迭代次数
	Tolerance     float64     // 收敛阈值
	Centroids     [][]float64 // 聚类中心
	Labels        []int       // 每个点的标签
	Inertia       float64     // 最终 inertia（SSE）, 数据点到其所属聚类中心的距离平方和
	nFeatures     int         // 特征维度
	rng           *rand.Rand  // 使用私有 rng
}

// NewKMeans 创建K-means实例
func NewKMeans(k int, maxIterations int, tolerance float64) *KMeans {
	if k <= 0 {
		panic("k must be positive")
	}
	if maxIterations <= 0 {
		panic("maxIterations must be positive")
	}
	if tolerance <= 0 {
		panic("tolerance must be positive")
	}
	var rng *rand.Rand
	if kmeansSeed >= 0 {
		rng = rand.New(rand.NewSource(kmeansSeed))
	} else {
		rng = rand.New(rand.NewSource(time.Now().UnixNano()))
	}
	return &KMeans{
		K:             k,
		MaxIterations: maxIterations,
		Tolerance:     tolerance,
		rng:           rng,
	}
}

// distanceSquared 计算两点之间的欧氏距离平方（避免开方）
func (km *KMeans) distanceSquared(a, b []float64) float64 {
	if len(a) != len(b) {
		panic("points have different dimensions")
	}
	sum := 0.0
	for i := range a {
		diff := a[i] - b[i]
		sum += diff * diff
	}
	return sum
}

// distance 计算欧氏距离
func (km *KMeans) distance(a, b []float64) float64 {
	return math.Sqrt(km.distanceSquared(a, b))
}

// initializeCentroids 初始化聚类中心（K-means++）
func (km *KMeans) initializeCentroids(data [][]float64) [][]float64 {
	n := len(data)
	if n < km.K {
		panic("number of data points must be >= K")
	}

	centroids := make([][]float64, km.K)
	firstIdx := km.rng.Intn(n)
	centroids[0] = make([]float64, len(data[firstIdx]))
	copy(centroids[0], data[firstIdx])

	for i := 1; i < km.K; i++ {
		distances := make([]float64, n)
		totalDistance := 0.0

		for j, point := range data {
			minDist := math.MaxFloat64
			for k := 0; k < i; k++ {
				dist := km.distanceSquared(point, centroids[k])
				if dist < minDist {
					minDist = dist
				}
			}
			distances[j] = minDist
			totalDistance += minDist
		}

		r := km.rng.Float64() * totalDistance
		cumulative := 0.0
		for j, dist := range distances {
			cumulative += dist
			if cumulative >= r {
				centroids[i] = make([]float64, len(data[j]))
				copy(centroids[i], data[j])
				break
			}
		}
	}

	return centroids
}

// assignPoints 分配点到最近的聚类中心
func (km *KMeans) assignPoints(data [][]float64, centroids [][]float64) ([]int, float64) {
	labels := make([]int, len(data))
	inertia := 0.0

	for i, point := range data {
		minDist := math.MaxFloat64
		bestCluster := -1
		for j, centroid := range centroids {
			dist := km.distanceSquared(point, centroid)
			if dist < minDist {
				minDist = dist
				bestCluster = j
			}
		}
		labels[i] = bestCluster
		inertia += minDist
	}

	return labels, inertia
}

// updateCentroids 更新聚类中心
func (km *KMeans) updateCentroids(data [][]float64, labels []int, oldCentroids [][]float64) [][]float64 {
	counts := make([]int, km.K)
	sums := make([][]float64, km.K)
	for i := range sums {
		sums[i] = make([]float64, km.nFeatures)
	}

	for i, point := range data {
		cluster := labels[i]
		counts[cluster]++
		for j := range point {
			sums[cluster][j] += point[j]
		}
	}

	newCentroids := make([][]float64, km.K)
	for i := range newCentroids {
		newCentroids[i] = make([]float64, km.nFeatures)
		if counts[i] > 0 {
			for j := range newCentroids[i] {
				newCentroids[i][j] = sums[i][j] / float64(counts[i])
			}
		} else {
			// 空簇：随机选择一个点作为新中心
			randIdx := km.rng.Intn(len(data)) // ✅ 用 km.rng
			copy(newCentroids[i], data[randIdx])
		}
	}

	return newCentroids
}

// hasConverged 检查是否收敛
func (km *KMeans) hasConverged(oldCentroids, newCentroids [][]float64, oldInertia, newInertia float64) bool {
	maxMove := 0.0
	for i := range oldCentroids {
		move := km.distance(oldCentroids[i], newCentroids[i])
		if move > maxMove {
			maxMove = move
		}
	}
	inertiaChange := math.Abs(oldInertia-newInertia) / oldInertia
	return maxMove < km.Tolerance && inertiaChange < km.Tolerance
}

// Fit 执行K-means聚类
func (km *KMeans) Fit(data [][]float64) *KMeans {
	if len(data) == 0 {
		panic("empty data")
	}
	if km.K > len(data) {
		panic("K cannot be greater than number of data points")
	}

	km.nFeatures = len(data[0])
	// 检查维度一致性
	for _, point := range data {
		if len(point) != km.nFeatures {
			panic("inconsistent feature dimensions")
		}
	}

	km.Centroids = km.initializeCentroids(data)
	var labels []int
	var inertia float64
	oldInertia := math.MaxFloat64
	var oldCentroids [][]float64 // ✅ 在循环外声明

	for iteration := 0; iteration < km.MaxIterations; iteration++ {
		labels, inertia = km.assignPoints(data, km.Centroids)

		// ✅ 第一次迭代不检查收敛
		if iteration > 0 {
			if km.hasConverged(oldCentroids, km.Centroids, oldInertia, inertia) {
				break
			}
		}

		// 保存旧状态
		oldCentroids = make([][]float64, len(km.Centroids))
		for i := range km.Centroids {
			oldCentroids[i] = make([]float64, len(km.Centroids[i]))
			copy(oldCentroids[i], km.Centroids[i])
		}
		oldInertia = inertia

		km.Centroids = km.updateCentroids(data, labels, oldCentroids)
	}

	km.Labels = labels
	km.Inertia = inertia
	return km
}

// Predict 预测新数据点的簇标签
func (km *KMeans) Predict(newPoints [][]float64) []int {
	if km.Centroids == nil {
		panic("KMeans not fitted")
	}
	if len(newPoints) == 0 {
		return []int{}
	}

	labels := make([]int, len(newPoints))
	for i, point := range newPoints {
		if len(point) != km.nFeatures {
			panic("point dimension mismatch")
		}
		minDist := math.MaxFloat64
		bestCluster := -1
		for j, centroid := range km.Centroids {
			dist := km.distanceSquared(point, centroid)
			if dist < minDist {
				minDist = dist
				bestCluster = j
			}
		}
		labels[i] = bestCluster
	}
	return labels
}

// GetClusterSizes 获取每个簇的大小
func (km *KMeans) GetClusterSizes() []int {
	if km.Labels == nil {
		return nil
	}
	sizes := make([]int, km.K)
	for _, label := range km.Labels {
		sizes[label]++
	}
	return sizes
}

// GetClusterPoints 获取每个簇的点
func (km *KMeans) GetClusterPoints(data [][]float64) [][][]float64 {
	if km.Labels == nil || len(data) != len(km.Labels) {
		return nil
	}
	clusters := make([][][]float64, km.K)
	for i, point := range data {
		cluster := km.Labels[i]
		clusters[cluster] = append(clusters[cluster], point)
	}
	return clusters
}

// SilhouetteScore 计算轮廓系数
func (km *KMeans) SilhouetteScore(data [][]float64) float64 {
	if km.Labels == nil {
		panic("KMeans not fitted")
	}
	n := len(data)
	if n == 0 {
		return 0
	}

	totalScore := 0.0
	for i := range data {
		a := 0.0
		countA := 0
		clusterDistances := make([]float64, km.K)
		clusterCounts := make([]int, km.K)

		for j := range data {
			if i == j {
				continue
			}
			distance := km.distance(data[i], data[j])
			if km.Labels[i] == km.Labels[j] {
				a += distance
				countA++
			}
			clusterDistances[km.Labels[j]] += distance
			clusterCounts[km.Labels[j]]++
		}

		if countA > 0 {
			a /= float64(countA)
		}

		b := math.MaxFloat64
		for cluster := 0; cluster < km.K; cluster++ {
			if cluster == km.Labels[i] || clusterCounts[cluster] == 0 {
				continue
			}
			avgDist := clusterDistances[cluster] / float64(clusterCounts[cluster])
			if avgDist < b {
				b = avgDist
			}
		}

		if b == math.MaxFloat64 {
			b = a // 所有点在同一簇
		}

		maxVal := math.Max(a, b)
		if maxVal > 0 {
			totalScore += (b - a) / maxVal
		}
	}

	return totalScore / float64(n)
}
