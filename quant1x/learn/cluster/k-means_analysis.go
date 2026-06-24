package cluster

import (
	"fmt"
	"math"
	"sort"

	"github.com/quant1x/quant1x/quant1x/learn/preprocessing"
)

// getOriginalCentroids 反标准化
func getOriginalCentroids(scaler *preprocessing.StandardScaler, normalizedCentroids [][]float64) [][]float64 {
	original := make([][]float64, len(normalizedCentroids))
	for i, center := range normalizedCentroids {
		original[i] = make([]float64, len(center))
		for j, val := range center {
			original[i][j] = val*scaler.Std[j] + scaler.Mean[j]
		}
	}
	return original
}

// analyzeMarketBehavior 基于四个簇的整体行为, 输出市场级结论
func analyzeMarketBehavior(originalCentroids []TradeBehavior) {
	n := len(originalCentroids)
	if n != 4 {
		fmt.Println("⚠️  警告: 预期4个簇, 实际:", n)
		return
	}

	// 1. 提取特征向量, 用于推导阈值
	var priceChanges, netRatios, stdPrices, avgSizes []float64

	for _, tb := range originalCentroids {
		priceChanges = append(priceChanges, tb.PriceChange)
		if tb.TotalVolume > 0 {
			netRatio := math.Abs(tb.BuyVolume-tb.SellVolume) / tb.TotalVolume
			netRatios = append(netRatios, netRatio)
		}
		stdPrices = append(stdPrices, tb.StdPrice)
		avgSizes = append(avgSizes, tb.AvgTradeSize)
	}

	// 2. 从数据中推导阈值(避免魔法数字)
	priceChangeThreshold := getSignificantThreshold(priceChanges, 0.01)
	netRatioThreshold := getSignificantThreshold(netRatios, 0.1)
	stdPriceThreshold := median(stdPrices) * 1.5

	// 3. 按 AvgTradeSize 排序, 确定规模等级
	indices := make([]int, 4)
	for i := range indices {
		indices[i] = i
	}
	sort.Slice(indices, func(i, j int) bool {
		return originalCentroids[indices[i]].AvgTradeSize < originalCentroids[indices[j]].AvgTradeSize
	})

	// 4. 映射角色
	roles := make(map[string]TradeBehavior)
	labels := []string{"散户", "中户", "大单", "超大单"}
	for rank, id := range indices {
		roles[labels[rank]] = originalCentroids[id]
	}

	super, hasSuper := roles["超大单"]
	large, hasLarge := roles["大单"]
	small, hasSmall := roles["散户"] // ✅ 修正: 现在使用它

	// 5. 输出推导的阈值(可解释性)
	fmt.Println("📊 自动推导判断阈值:")
	fmt.Printf("   • 显著价格变化: > %.4f 元\n", priceChangeThreshold)
	fmt.Printf("   • 显著净流入比例: > %.2f%%\n", netRatioThreshold*100)
	fmt.Printf("   • 高波动标准: > %.4f\n", stdPriceThreshold)
	fmt.Println("")

	// 6. 市场行为分析结论
	fmt.Println("📈 市场行为整体分析结论: ")

	// 1. 低位吸筹 or 主力拉升
	var netSuperRatio float64 // ✅ 修复: 定义变量
	if hasSuper {
		netSuper := super.BuyVolume - super.SellVolume
		if super.TotalVolume > 0 {
			netSuperRatio = netSuper / super.TotalVolume
		}
		if super.PriceChange > priceChangeThreshold && netSuperRatio > netRatioThreshold {
			fmt.Printf("1. ✅ 主力拉升: 主力在 %.3f 元主动买入, 净流入 %+v 手, 推动价格上涨. \n",
				super.MeanPrice, int64(netSuper))
		} else if super.PriceChange < -priceChangeThreshold && netSuperRatio < -netRatioThreshold {
			fmt.Printf("1. ❌ 主力减仓: 主力在 %.3f 元集中卖出, 净流出 %+v 手, 参与下跌. \n",
				super.MeanPrice, int64(-netSuper))
		}
	}

	// 2. 高位出货
	var netLargeRatio float64 // ✅ 修复: 定义变量
	if hasLarge {
		netLarge := large.BuyVolume - large.SellVolume
		if large.TotalVolume > 0 {
			netLargeRatio = netLarge / large.TotalVolume
		}
		if large.PriceChange < -priceChangeThreshold && netLargeRatio < -netRatioThreshold {
			fmt.Printf("2. ✅ 高位派发: 大单在 %.3f 元集中卖出, 净流出 %+v 手, 主导价格下行. \n",
				large.MeanPrice, int64(-netLarge))
		}
	}

	// 3. 吸引跟风盘 or 下跌踩踏？
	if hasSuper && super.PriceChange > priceChangeThreshold && netSuperRatio > netRatioThreshold {
		// 主力主动拉升
		if hasLarge && large.PriceChange < -priceChangeThreshold && netLargeRatio < -netRatioThreshold {
			fmt.Println("3. ✅ 吸引跟风盘: 主力拉升吸引关注, 大单趁机派发, 形成辨识度. ")
		} else {
			fmt.Println("3. ✅ 主力拉升: 主动买入推动上涨, 但抛压未现, 尚未形成辨识度. ")
		}
	} else {
		// 计算整体价格趋势
		totalPriceChange := 0.0
		for _, tb := range originalCentroids {
			totalPriceChange += tb.PriceChange
		}
		avgPriceChange := totalPriceChange / 4

		if avgPriceChange < -priceChangeThreshold*2 && hasSmall {
			netSmall := small.BuyVolume - small.SellVolume
			if netSmall < 0 {
				fmt.Println("3. ❌ 非吸引跟风盘: 市场大幅下跌, 散户割肉踩踏, 主力未护盘. ")
			} else {
				fmt.Println("3. ⚠️ 市场恐慌: 价格大跌, 但散户仍在抄底, 风险极高. ")
			}
		} else {
			fmt.Println("3. ⚠️ 市场震荡: 无明确方向, 尚未形成辨识度. ")
		}
	}

	// 4. 洗盘, 震仓
	if hasLarge {
		if large.StdPrice > stdPriceThreshold && math.Abs(large.PriceChange) < priceChangeThreshold {
			fmt.Println("4. ✅ 洗盘震仓: 大单制造高波动但价格未持续下跌, 测试抛压, 清理浮筹. ")
		}
	}

	// 5. 对倒(异常行为)
	if hasSuper {
		netSuper := super.BuyVolume - super.SellVolume
		netRatio := 0.0
		if super.TotalVolume > 0 {
			netRatio = math.Abs(netSuper) / super.TotalVolume
		}
		if netRatio < 0.05 && super.StdPrice > stdPriceThreshold*2 {
			fmt.Println("5. ⚠️  对倒嫌疑: 超大单买卖均衡但波动剧烈, 需结合盘口数据确认. ")
		} else {
			fmt.Println("5. ❌ 未发现对倒行为: 主力方向明确, 非自成交. ")
		}
	}

	// 6. 总结
	fmt.Println("\n📌 综合判断: 需结合趋势, 主力行为, 散户反应综合评估市场阶段. ")
}

// median 计算中位数
func median(values []float64) float64 {
	sorted := make([]float64, len(values))
	copy(sorted, values)
	sort.Float64s(sorted)
	n := len(sorted)
	if n%2 == 1 {
		return sorted[n/2]
	}
	return (sorted[n/2-1] + sorted[n/2]) / 2
}

// getSignificantThreshold 推导显著变化阈值
func getSignificantThreshold(values []float64, fallback float64) float64 {
	if len(values) == 0 {
		return fallback
	}
	// 用 75% 分位数作为“显著”阈值
	sorted := make([]float64, len(values))
	copy(sorted, values)
	sort.Float64s(sorted)
	return sorted[len(sorted)*3/4]
}
