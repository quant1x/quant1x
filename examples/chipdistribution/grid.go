package chipdistribution

import "math"

// 生成价格区间网格
func generatePriceGrid(low, high, step float64, digits int) []float64 {
	scale := math.Pow10(digits)
	lowInt := int(math.Round(low * scale))
	highInt := int(math.Round(high * scale))
	stepInt := int(math.Round(step * scale))

	// 检查参数有效性
	if lowInt > highInt || stepInt <= 0 {
		return nil
	}

	var grid []float64
	for i := lowInt; i <= highInt; i += stepInt {
		price := float64(i) / scale
		grid = append(grid, price)
	}

	return grid
}
