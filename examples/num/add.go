package num

import (
	"runtime"
	"sync"
)

// Add 两个float64数组相加
func Add(a, b []float64) []float64 {
	return v2Add(a, b)
}

func AddNaive(a, b []float64) []float64 {
	return v1Add(a, b)
}

func v1Add(a, b []float64) []float64 {
	aLen := len(a)
	bLen := len(b)
	// 默认左对齐
	n := min(aLen, bLen)
	length := max(aLen, bLen)
	s := make([]float64, length)
	for i := 0; i < n; i++ {
		s[i] = a[i] + b[i]
	}
	return s
}

func v2Add(a, b []float64) []float64 {
	padding := float64(0)
	aLen, bLen := len(a), len(b)
	minLen := min(aLen, bLen)
	maxLen := max(aLen, bLen)

	// 初始化结果切片为全padding
	s := make([]float64, maxLen)
	for i := range s {
		s[i] = padding
	}

	// 并行计算有效部分
	maxWorkers := runtime.GOMAXPROCS(0)
	chunkSize := (minLen + maxWorkers - 1) / maxWorkers

	var wg sync.WaitGroup
	for workerID := 0; workerID < maxWorkers; workerID++ {
		wg.Add(1)
		go func(workerID int) {
			defer wg.Done()
			start := workerID * chunkSize
			end := start + chunkSize
			if end > minLen {
				end = minLen
			}
			if start >= end {
				return
			}

			for i := start; i < end; i++ {
				s[i] = a[i] + b[i]
			}
		}(workerID)
	}
	wg.Wait()

	return s
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}

func max(a, b int) int {
	if a > b {
		return a
	}
	return b
}

// 使用SSE2优化的数组加法
func v3Add(a, b []float64) []float64 {
	n := len(a)
	if len(b) < n {
		n = len(b)
	}
	if n == 0 {
		return nil
	}

	// 结果数组
	c := make([]float64, n)

	// 调用汇编实现
	sse2Add(&a[0], &b[0], &c[0], n)
	return c
}

// 循环展开 (Loop unrolling)
func v4Add(a, b []float64) []float64 {
	aLen := len(a)
	bLen := len(b)
	// 默认左对齐
	n := min(aLen, bLen)
	length := max(aLen, bLen)
	// 结果数组
	c := make([]float64, length)
	for i := 0; i < n; i += 4 {
		c[i] = a[i] + b[i]
		c[i+1] = a[i+1] + b[i+1]
		c[i+2] = a[i+2] + b[i+2]
		c[i+3] = a[i+3] + b[i+3]
	}
	return c
}

// 自适应CPU SIMD指令集
func v5Add(a, b []float64) []float64 {
	n := len(a)
	if len(b) < n {
		n = len(b)
	}
	if n == 0 {
		return nil
	}

	// 结果数组
	c := make([]float64, n)

	// 调用汇编实现
	addSIMD(a, b, c)
	return c
}
