package cluster

import (
	"fmt"
	"math"

	"gitee.com/quant1x/data/level1/quotes"
	"gitee.com/quant1x/num"
	"gitee.com/quant1x/quant1x/learn/preprocessing"
)

// TradeBehavior 表示一个分笔窗口的交易行为特征
//
//	用于 KMeans 聚类分析，刻画市场参与者的交易模式
type TradeBehavior struct {
	MeanPrice       float64 // 窗口内成交价的算术平均值
	StdPrice        float64 // 成交价的标准差，反映价格波动强度
	VWAP            float64 // 成交量加权均价（Volume Weighted Average Price），反映实际成交成本。 公式: VWAP = Σ(Price × Volume) / Σ(Volume)
	PriceChange     float64 // 窗口内价格变化 = 最后一笔价格 - 第一笔价格，反映趋势方向
	VolumeImbalance float64 // 买卖不平衡度 = (主动买入量 - 主动卖出量) / 总成交量，>0 偏买，<0 偏卖
	TradeCount      float64 // 窗口内的交易笔数，反映交易活跃度
	TotalVolume     float64 // 窗口内所有成交的成交量总和（单位：手）
	AvgTradeSize    float64 // 平均每笔成交量 = TotalVolume / TradeCount，用于区分散户/主力
	BuyVolume       float64 // 主动买入成交量总和（价格上涨或平盘时成交），反映买方力量
	SellVolume      float64 // 主动卖出成交量总和（价格下跌或平盘时成交），反映卖方力量
}

// String 实现 fmt.Stringer 接口，用于格式化输出
func (tb TradeBehavior) String() string {
	buySellRatio := float64(tb.BuyVolume) / math.Max(1.0, float64(tb.SellVolume))
	direction := "均衡"
	if tb.VolumeImbalance > 0.1 {
		direction = "偏买入"
	} else if tb.VolumeImbalance < -0.1 {
		direction = "偏卖出"
	}

	return fmt.Sprintf(
		"📊 价格: %.3f±%.4f | 变化: %+.3f\n"+
			"📈 成交: 总%.0f | 买%.0f | 卖%.0f | 比%.2f | %s\n"+
			"🎯 交易: %d笔 | 均%.1f | VWAP: %.3f",
		tb.MeanPrice, tb.StdPrice, tb.PriceChange,
		tb.TotalVolume, tb.BuyVolume, tb.SellVolume, buySellRatio, direction,
		int(tb.TradeCount), tb.AvgTradeSize,
		tb.VWAP,
	)
}

// ToSlice 从结构体转为 []float64（适配 KMeans）
func (tb TradeBehavior) ToSlice() []float64 {
	return []float64{
		tb.MeanPrice,
		tb.StdPrice,
		tb.VWAP,
		tb.PriceChange,
		tb.VolumeImbalance,
		tb.TradeCount,
		tb.TotalVolume,
		tb.AvgTradeSize,
		tb.BuyVolume,
		tb.SellVolume,
	}
}

// SliceToTradeBehavior 从 []float64 转为结构体（用于分析）
func SliceToTradeBehavior(data []float64) TradeBehavior {
	if len(data) < 10 {
		return TradeBehavior{}
	}
	return TradeBehavior{
		MeanPrice:       data[0],
		StdPrice:        data[1],
		VWAP:            data[2],
		PriceChange:     data[3],
		VolumeImbalance: data[4],
		TradeCount:      data[5],
		TotalVolume:     data[6],
		AvgTradeSize:    data[7],
		BuyVolume:       data[8],
		SellVolume:      data[9],
	}
}

// TickDataExtractor 实现 DataExtractor 接口
type TickDataExtractor struct {
	WindowSize int

	// 覆盖率统计字段（仅在 Extract 后有效）
	TotalVolumeFromTicks int64   // 全天总成交量（股/手）
	EffectiveVolumeInUse int64   // 被窗口使用的成交量
	CoverageRate         float64 // 覆盖率
}

// Extract 提取数据并返回 [][]float64（用于 KMeans），同时计算覆盖率
func (e *TickDataExtractor) Extract(data any) [][]float64 {
	ticks, ok := data.([]quotes.TickTransaction)
	if !ok || len(ticks) == 0 {
		return nil
	}

	size := e.WindowSize
	if size <= 0 {
		size = 20
	}

	var result [][]float64
	e.TotalVolumeFromTicks = 0
	e.EffectiveVolumeInUse = 0

	// 1. 先统计全天总成交量
	for _, tick := range ticks {
		vol := int64(tick.Vol)
		e.TotalVolumeFromTicks += vol
	}

	// 2. 滑动窗口切片 + 特征提取
	//for i := 0; i <= len(ticks)-size; i += size {
	//	window := ticks[i : i+size]
	//	behavior := e.extractAsStruct(window)
	//	result = append(result, behavior.ToSlice())
	//
	//	// 累加该窗口的成交量（用于覆盖率）
	//	for _, t := range window {
	//		e.EffectiveVolumeInUse += int64(t.Vol)
	//	}
	//}
	start := 0
	count := len(ticks)
	isEof := false
	for {
		end := start + size
		if end >= count {
			end = count
			isEof = true
		}
		window := ticks[start:end]
		behavior := e.extractAsStruct(window)
		result = append(result, behavior.ToSlice())

		// 累加该窗口的成交量（用于覆盖率）
		for _, t := range window {
			e.EffectiveVolumeInUse += int64(t.Vol)
		}
		if isEof {
			break
		}
		start += size
	}

	// 3. 计算覆盖率
	if e.TotalVolumeFromTicks > 0 {
		e.CoverageRate = float64(e.EffectiveVolumeInUse) / float64(e.TotalVolumeFromTicks)
	} else {
		e.CoverageRate = 0
	}

	return result
}

// extractAsStruct 核心特征提取（结构化）
func (e *TickDataExtractor) extractAsStruct(window []quotes.TickTransaction) TradeBehavior {
	n := len(window)
	if n == 0 {
		return TradeBehavior{}
	}

	var (
		totalVol    int64
		buyVol      int64
		sellVol     int64
		prices      []float64
		sumPriceVol float64
	)

	startPrice := window[0].Price
	endPrice := window[len(window)-1].Price

	for i := 0; i < n; i++ {
		priceChange := 0.00
		if i > 0 {
			priceChange = window[i].Price - window[i-1].Price
		}
		volume := int64(window[i].Vol)
		if priceChange > 0 {
			buyVol += volume
		} else if priceChange < 0 {
			sellVol += volume
		} else {
			half := volume / 2
			buyVol += half
			sellVol += volume - half
		}

		prices = append(prices, window[i].Price)
		sumPriceVol += window[i].Amount
		totalVol += volume
	}

	meanPrice := num.Sum(prices) / float64(len(prices))
	variance := 0.0
	for _, p := range prices {
		variance += (p - meanPrice) * (p - meanPrice)
	}
	stdPrice := math.Sqrt(variance / float64(len(prices)))

	vwap := sumPriceVol / float64(totalVol+1)
	priceChange := endPrice - startPrice
	volumeImbalance := float64(buyVol-sellVol) / float64(totalVol+1)
	avgTradeSize := float64(totalVol) / float64(n)

	return TradeBehavior{
		MeanPrice:       meanPrice,
		StdPrice:        stdPrice,
		VWAP:            vwap,
		PriceChange:     priceChange,
		VolumeImbalance: volumeImbalance,
		TradeCount:      float64(n),
		TotalVolume:     float64(totalVol),
		AvgTradeSize:    avgTradeSize,
		BuyVolume:       float64(buyVol),
		SellVolume:      float64(sellVol),
	}
}

// GetOriginalCentroids 反标准化并转为结构体（关键！）
func (e *TickDataExtractor) GetOriginalCentroids(scaler *preprocessing.StandardScaler, normalizedCentroids [][]float64) []TradeBehavior {
	original := make([]TradeBehavior, len(normalizedCentroids))
	for i, center := range normalizedCentroids {
		// 反标准化
		for j, val := range center {
			center[j] = val*scaler.Std[j] + scaler.Mean[j]
		}
		// 转为结构体
		original[i] = SliceToTradeBehavior(center)
	}
	return original
}

// GetCoverage 返回覆盖率信息
func (e *TickDataExtractor) GetCoverage() (total, effective int64, rate float64) {
	return e.TotalVolumeFromTicks, e.EffectiveVolumeInUse, e.CoverageRate
}

// GetCoverageReport 返回可读的覆盖率报告
func (e *TickDataExtractor) GetCoverageReport() string {
	return fmt.Sprintf("📊 数据覆盖率: %.2f%% (%d / %d 手)",
		e.CoverageRate*100,
		e.EffectiveVolumeInUse,
		e.TotalVolumeFromTicks,
	)
}
