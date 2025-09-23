package main

import (
	"fmt"
	"math"

	"gonum.org/v1/gonum/optimize"
)

// GARCH 模型结构体
type GARCH struct {
	P      int       // ARCH项的阶数
	Q      int       // GARCH项的阶数
	Alpha0 float64   // 常数项
	Alpha  []float64 // ARCH项系数
	Beta   []float64 // GARCH项系数
	MaxLag int       // p和q中的最大值
}

// Fit 使用最大似然估计拟合GARCH模型
func (g *GARCH) Fit(epsilon []float64) error {
	n := len(epsilon)
	if n == 0 {
		return fmt.Errorf("epsilon is empty")
	}

	g.MaxLag = max(g.P, g.Q)
	if n <= g.MaxLag {
		return fmt.Errorf("insufficient data points")
	}

	// 定义优化问题
	problem := optimize.Problem{
		Func: func(params []float64) float64 {
			if !g.validParams(params) {
				return math.MaxFloat64
			}

			// 提取参数
			g.Alpha0 = params[0]
			g.Alpha = params[1 : 1+g.P]
			g.Beta = params[1+g.P : 1+g.P+g.Q]

			// 计算条件方差和对数似然
			sigma2, ok := g.computeSigma2(epsilon)
			if !ok {
				return math.MaxFloat64
			}

			// 计算对数似然（仅使用MaxLag之后的数据）
			logLikelihood := 0.0
			for t := g.MaxLag; t < n; t++ {
				if sigma2[t] <= 0 {
					return math.MaxFloat64
				}
				logLikelihood += -0.5 * (math.Log(sigma2[t]) + (epsilon[t]*epsilon[t])/sigma2[t])
			}

			return -logLikelihood // 最小化负对数似然
		},
	}

	// 初始参数猜测
	initialParams := make([]float64, 1+g.P+g.Q)
	initialParams[0] = 0.1 // Alpha0
	for i := 1; i < len(initialParams); i++ {
		initialParams[i] = 0.05
	}

	// 使用Nelder-Mead算法进行优化
	result, err := optimize.Minimize(problem, initialParams, nil, &optimize.NelderMead{})
	if err != nil {
		return fmt.Errorf("optimization failed: %v", err)
	}

	// 保存优化后的参数
	g.Alpha0 = result.X[0]
	g.Alpha = make([]float64, g.P)
	copy(g.Alpha, result.X[1:1+g.P])
	g.Beta = make([]float64, g.Q)
	copy(g.Beta, result.X[1+g.P:1+g.P+g.Q])

	return nil
}

// 检查参数有效性
func (g *GARCH) validParams(params []float64) bool {
	if params[0] <= 0 {
		return false
	}

	sumAlphaBeta := 0.0
	for _, a := range params[1 : 1+g.P] {
		if a < 0 {
			return false
		}
		sumAlphaBeta += a
	}
	for _, b := range params[1+g.P : 1+g.P+g.Q] {
		if b < 0 {
			return false
		}
		sumAlphaBeta += b
	}

	if sumAlphaBeta >= 1.0 {
		return false
	}

	return true
}

// 计算条件方差
func (g *GARCH) computeSigma2(epsilon []float64) ([]float64, bool) {
	n := len(epsilon)
	sigma2 := make([]float64, n)

	// 初始方差（使用前MaxLag个数据的方差）
	varInit := variance(epsilon[:g.MaxLag])
	for t := 0; t < g.MaxLag; t++ {
		sigma2[t] = varInit
	}

	// 计算后续的条件方差
	for t := g.MaxLag; t < n; t++ {
		sigma2[t] = g.Alpha0

		// 添加ARCH项
		for i := 0; i < g.P; i++ {
			lag := i + 1
			sigma2[t] += g.Alpha[i] * epsilon[t-lag] * epsilon[t-lag]
		}

		// 添加GARCH项
		for j := 0; j < g.Q; j++ {
			lag := j + 1
			sigma2[t] += g.Beta[j] * sigma2[t-lag]
		}

		if sigma2[t] <= 0 {
			return nil, false
		}
	}

	return sigma2, true
}

// 辅助函数：计算方差
func variance(data []float64) float64 {
	sum := 0.0
	for _, x := range data {
		sum += x * x
	}
	return sum / float64(len(data))
}

func main() {
	// 示例：使用模拟数据拟合GARCH(1,1)模型
	epsilon := []float64{
		// 这里填入您的时间序列数据（如收益率残差）
		// 示例数据（假设已经去均值）
		0.1, -0.2, 0.05, -0.1, 0.3, -0.15, 0.02, -0.1, 0.2, -0.25,
		0.15, -0.05, 0.1, -0.2, 0.15, -0.1, 0.05, -0.15, 0.25, -0.1,
	}

	garch := &GARCH{P: 1, Q: 1}
	if err := garch.Fit(epsilon); err != nil {
		fmt.Println("Error fitting GARCH model:", err)
		return
	}

	fmt.Printf("Fitted GARCH(%d,%d) parameters:\n", garch.P, garch.Q)
	fmt.Printf("Alpha0: %.4f\n", garch.Alpha0)
	fmt.Printf("Alpha: %v\n", garch.Alpha)
	fmt.Printf("Beta: %v\n", garch.Beta)
}
