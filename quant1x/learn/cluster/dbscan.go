package cluster

import (
	"math"
)

type DBSCAN struct {
	Eps    float64
	MinPts int
}

func NewDBSCAN(eps float64, minPts int) *DBSCAN {
	if eps <= 0 {
		panic("eps must be positive")
	}
	if minPts <= 0 {
		panic("minPts must be positive")
	}
	return &DBSCAN{Eps: eps, MinPts: minPts}
}

func (d *DBSCAN) distance(a, b []float64) float64 {
	if len(a) != len(b) {
		panic("points have different dimensions")
	}
	sum := 0.0
	for i := range a {
		diff := a[i] - b[i]
		sum += diff * diff
	}
	return math.Sqrt(sum)
}

// regionQuery: 包含自身
func (d *DBSCAN) regionQuery(data [][]float64, p int) []int {
	pointP := data[p]
	epsSq := d.Eps * d.Eps
	var neighbors []int

	for i, pointI := range data {
		if len(pointI) != len(pointP) {
			continue
		}
		distSq := 0.0
		for j := range pointP {
			diff := pointP[j] - pointI[j]
			distSq += diff * diff
			if distSq >= epsSq {
				break
			}
		}
		if distSq < epsSq {
			neighbors = append(neighbors, i)
		}
	}
	return neighbors
}

func (d *DBSCAN) Fit(data [][]float64) []int {
	n := len(data)
	if n == 0 {
		return []int{}
	}

	labels := make([]int, n) // 0=unvisited, -1=noise, >=0=cluster
	visited := make([]bool, n)
	clusterID := 0

	for i := 0; i < n; i++ {
		if visited[i] {
			continue
		}
		visited[i] = true

		neighbors := d.regionQuery(data, i)

		// 核心点判断：邻域点数（含自己）>= MinPts
		if len(neighbors) < d.MinPts {
			labels[i] = -1
			continue
		}

		// 创建新簇
		labels[i] = clusterID

		// BFS 队列 + inQueue 标记
		queue := make([]int, 0, len(neighbors))
		inQueue := make([]bool, n)

		for _, idx := range neighbors {
			if !visited[idx] && !inQueue[idx] {
				queue = append(queue, idx)
				inQueue[idx] = true
			}
		}

		// BFS 扩展
		for len(queue) > 0 {
			q := queue[0]
			queue = queue[1:]

			if !visited[q] {
				visited[q] = true
				qNeighbors := d.regionQuery(data, q)
				if len(qNeighbors) >= d.MinPts {
					for _, nn := range qNeighbors {
						if !visited[nn] && !inQueue[nn] {
							queue = append(queue, nn)
							inQueue[nn] = true
						}
					}
				}
			}

			// 仅当未标记时才分配簇（防止噪声被覆盖）
			if labels[q] == 0 {
				labels[q] = clusterID
			}
		}

		clusterID++
	}

	return labels
}

// Predict 返回距离 newPoint 最近的非噪声点的簇标签。
//
//	注意：这不是原生 DBSCAN 的预测方式，而是一种常用的启发式扩展。
//	如果无有效邻居，返回 -1。
func (d *DBSCAN) Predict(data [][]float64, labels []int, newPoint []float64) int {
	if len(data) == 0 || len(labels) != len(data) {
		return -1
	}
	minDist := math.MaxFloat64
	nearestLabel := -1
	for i, point := range data {
		if labels[i] == -1 {
			continue // 忽略噪声
		}
		dist := d.distance(point, newPoint)
		if dist < minDist {
			minDist = dist
			nearestLabel = labels[i]
		}
	}
	return nearestLabel
}

// GetClusterStats 返回各簇点数
func (d *DBSCAN) GetClusterStats(labels []int) map[int]int {
	stats := make(map[int]int)
	for _, label := range labels {
		stats[label]++
	}
	return stats
}
