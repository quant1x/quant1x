package main

import (
	"fmt"

	"gonum.org/v1/gonum/mat"
)

// 三维数据点
type Point3D struct {
	X, Y, Z float64 // X=时间/自变量1, Y=成交量/自变量2, Z=价格/因变量
}

// 抛物面模型参数
type ParaboloidParams struct {
	A, B, C, D, E, F float64
	RSquared         float64
}

func FitParaboloid(points []Point3D) ParaboloidParams {
	n := len(points)

	// 构建设计矩阵 X 和观测值向量 Y
	X := mat.NewDense(n, 6, nil)
	Y := mat.NewVecDense(n, nil)

	for i, p := range points {
		x, y, z := p.X, p.Y, p.Z
		X.SetRow(i, []float64{x * x, y * y, x * y, x, y, 1})
		Y.SetVec(i, z)
	}

	// 求解 (XᵀX)⁻¹XᵀY
	var XT mat.Dense
	XT.CloneFrom(X.T())

	var XTX mat.Dense
	XTX.Mul(&XT, X)

	var XTXInv mat.Dense
	if err := XTXInv.Inverse(&XTX); err != nil {
		panic("矩阵不可逆，需检查数据共线性")
	}

	// 错误修正部分
	var xty mat.VecDense
	xty.MulVec(&XT, Y) // 正确计算 XᵀY

	var beta mat.VecDense
	beta.MulVec(&XTXInv, &xty) // 现在参数类型匹配
	//var beta mat.VecDense
	//beta.MulVec(&XTXInv, XT)

	beta.MulVec(&beta, Y)

	// 计算R²
	var pred mat.VecDense
	pred.MulVec(X, &beta)

	ssTotal, ssResidual := 0.0, 0.0
	meanZ := meanZ(points)
	for i := 0; i < n; i++ {
		z := Y.At(i, 0)
		ssTotal += (z - meanZ) * (z - meanZ)
		ssResidual += (z - pred.At(i, 0)) * (z - pred.At(i, 0))
	}
	r2 := 1 - ssResidual/ssTotal

	return ParaboloidParams{
		A:        beta.At(0, 0),
		B:        beta.At(1, 0),
		C:        beta.At(2, 0),
		D:        beta.At(3, 0),
		E:        beta.At(4, 0),
		F:        beta.At(5, 0),
		RSquared: r2,
	}
}

func meanZ(points []Point3D) float64 {
	sum := 0.0
	for _, p := range points {
		sum += p.Z
	}
	return sum / float64(len(points))
}

func main() {
	// 示例数据（时间，成交量，价格）
	data := []Point3D{
		{0, 10000, 50.0},
		{1, 12000, 51.5},
		{2, 15000, 53.8},
		{3, 9000, 49.2},
		{4, 18000, 55.3},
		{5, 17000, 52.3},
	}

	params := FitParaboloid(data)

	fmt.Printf("拟合方程: z = %.4fx² + %.4fy² + %.4fxy + %.4fx + %.4fy + %.4f\n",
		params.A, params.B, params.C, params.D, params.E, params.F)
	fmt.Printf("拟合优度 R² = %.4f\n", params.RSquared)

	// 预测新数据点
	newPoint := Point3D{X: 5, Y: 20000}
	predZ := params.A*newPoint.X*newPoint.X + params.B*newPoint.Y*newPoint.Y +
		params.C*newPoint.X*newPoint.Y + params.D*newPoint.X + params.E*newPoint.Y + params.F
	fmt.Printf("预测值 (t=5, vol=20000): %.2f\n", predZ)
}
