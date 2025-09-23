package num

import (
	"golang.org/x/sys/cpu"
)

var hasAVX512 = cpu.X86.HasAVX512F && cpu.X86.HasAVX512DQ
var hasAVX2 = cpu.X86.HasAVX2
var hasSSE2 = cpu.X86.HasSSE2

func init() {
	if hasAVX2 {
		addSIMD = addAVX2
	} else if hasSSE2 {
		addSIMD = addSSE2
	} else {
		addSIMD = Add0 // 回退到纯 Go
	}
}

// 声明汇编函数（必须与汇编文件中的符号名完全一致）
//
//go:noescape
func addSSE2(a, b, result []float64)

//go:noescape
func addAVX2(a, b, result []float64)

// 定义动态派发的函数变量
var addSIMD func(a, b, result []float64)

func AddOptimized(a, b []float64) []float64 {
	if len(a) != len(b) {
		panic("slice lengths mismatch")
	}
	n := len(a)
	result := make([]float64, n)

	// 向量化部分
	vecSize := 8 // AVX512 的向量宽度（根据实际指令集动态调整）
	if hasAVX512 {
		vecSize = 8
	} else if hasAVX2 {
		vecSize = 4
	} else if hasSSE2 {
		vecSize = 2
	}

	vecEnd := n - n%vecSize
	if vecEnd > 0 {
		addSIMD(a[:vecEnd], b[:vecEnd], result[:vecEnd])
	}

	// 处理剩余元素
	for i := vecEnd; i < n; i++ {
		result[i] = a[i] + b[i]
	}
	return result
}

func Add0(a, b, c []float64) {
	if len(a) != len(b) {
		panic("slice lengths mismatch")
	}
	for i := 0; i < len(a); i++ {
		c[i] = a[i] + b[i]
	}
}
