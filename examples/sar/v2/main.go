package main

import (
	"fmt"

	"gonum.org/v1/gonum/mat"
)

type Point3D struct {
	X, Y, Z float64
}

type ParaboloidParams struct {
	A, B, C, D, E, F float64
	RSquared         float64
}

func FitParaboloid(points []Point3D) ParaboloidParams {
	n := len(points)

	// 数据标准化（新增部分）
	meanX, meanY := 0.0, 0.0
	for _, p := range points {
		meanX += p.X
		meanY += p.Y
	}
	meanX /= float64(n)
	meanY /= float64(n)

	// 构建设计矩阵时使用中心化坐标（改进数值稳定性）
	X := mat.NewDense(n, 6, nil)
	Y := mat.NewVecDense(n, nil)

	for i, p := range points {
		x, y := p.X-meanX, p.Y-meanY // 中心化处理
		X.SetRow(i, []float64{
			x * x,
			y * y,
			x * y,
			x,
			y,
			1,
		})
		Y.SetVec(i, p.Z)
	}

	// 使用QR分解求解（更稳定的数值方法）
	var qr mat.QR
	//if ok := qr.Factorize(X); !ok {
	//	panic("矩阵分解失败")
	//}
	qr.Factorize(X)

	var beta mat.VecDense
	if err := beta.SolveVec(X, Y); err != nil {
		panic("求解失败: " + err.Error())
	}

	// 计算预测值时考虑中心化影响（新增逆变换）
	var pred mat.VecDense
	pred.MulVec(X, &beta)

	// R²计算保持不变
	ssTotal, ssResidual := 0.0, 0.0
	meanZ := meanZ(points)
	for i := 0; i < n; i++ {
		z := Y.AtVec(i)
		ssTotal += (z - meanZ) * (z - meanZ)
		ssResidual += (z - pred.AtVec(i)) * (z - pred.AtVec(i))
	}
	r2 := 1 - ssResidual/ssTotal

	return ParaboloidParams{
		A:        beta.AtVec(0),
		B:        beta.AtVec(1),
		C:        beta.AtVec(2),
		D:        beta.AtVec(3) - 2*beta.AtVec(0)*meanX - beta.AtVec(2)*meanY, // 逆变换系数调整
		E:        beta.AtVec(4) - 2*beta.AtVec(1)*meanY - beta.AtVec(2)*meanX,
		F:        beta.AtVec(5) + beta.AtVec(0)*meanX*meanX + beta.AtVec(1)*meanY*meanY + beta.AtVec(2)*meanX*meanY - beta.AtVec(3)*meanX - beta.AtVec(4)*meanY,
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

// 新增函数：计算抛物面顶点
func FindVertex(params ParaboloidParams) (time, price float64, err error) {
	// 计算二阶条件行列式
	det := 4*params.A*params.B - params.C*params.C

	// 鞍点判断（新增）
	//if det <= 0 {
	//	return 0, 0, fmt.Errorf("抛物面为鞍形（无极大/极小点），行列式=%.4f", det)
	//}
	if det <= 0 {
		AnalyzeSaddle(params) // 鞍点情况分析
		return 0, 0, fmt.Errorf("无传统顶点")
	}

	// 构建线性方程组矩阵
	coefficients := mat.NewDense(2, 2, []float64{
		2 * params.A, params.C,
		params.C, 2 * params.B,
	})

	// 构建常数项向量
	constants := mat.NewVecDense(2, []float64{-params.D, -params.E})

	// 求解线性方程组
	var solution mat.VecDense
	if err := solution.SolveVec(coefficients, constants); err != nil {
		return 0, 0, fmt.Errorf("方程组求解失败: %v", err)
	}

	// 提取坐标
	x := solution.AtVec(0)
	y := solution.AtVec(1)

	// 计算对应价格
	z := params.A*x*x + params.B*y*y + params.C*x*y +
		params.D*x + params.E*y + params.F

	// 有效性验证（新增业务逻辑限制）
	if x < 0 { // 假设时间不能为负
		return 0, 0, fmt.Errorf("预测到不合理时间点: %.2f", x)
	}

	return x, z, nil
}

// 新增鞍点方向分析
func AnalyzeSaddle(params ParaboloidParams) {
	// 计算特征向量方向
	// 此处可添加特征值分解代码
	fmt.Println("检测到鞍形曲面，主要变化方向：")
	fmt.Printf("上升方向：沿(%.2f, %.2f)向量\n", params.C, 2*params.A)
	fmt.Printf("下降方向：沿(%.2f, %.2f)向量\n", 2*params.B, params.C)
}

func main() {
	// 示例数据保持不变
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

	// 预测时需要反向中心化（新增部分）
	meanX, meanY := 2.5, 13500.0 // 示例数据均值
	newX := 5.0 - meanX
	newY := 20000.0 - meanY
	predZ := params.A*newX*newX + params.B*newY*newY +
		params.C*newX*newY + params.D*newX + params.E*newY + params.F

	fmt.Printf("预测值 (t=5, vol=20000): %.2f\n", predZ)

	// 预测顶点（新增部分）
	if time, price, err := FindVertex(params); err == nil {
		fmt.Printf("\n顶点预测结果：\n  时间 = %.2f\n  价格 = %.2f\n", time, price)
	} else {
		fmt.Println("\n顶点预测失败:", err)
	}

	// 验证
	fmt.Println("\n验证计算：")
	expectedX := (2*params.B*params.E - params.C*params.D) / (params.C*params.C - 4*params.A*params.B)
	expectedY := (2*params.A*params.D - params.C*params.E) / (params.C*params.C - 4*params.A*params.B)
	fmt.Printf("解析解验证：x=%.4f, y=%.4f\n", expectedX, expectedY)
}
