package cluster

import (
	"fmt"
	"math"
	"sort"
	"testing"

	"github.com/quant1x/num"
	_ "github.com/quant1x/quant1x/quant1x/contrib/data/tdx"
	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/data/market"
	"github.com/quant1x/quant1x/quant1x/learn/preprocessing"
)

func TestDBSCAN_Basic(t *testing.T) {
	data := [][]float64{
		{1.0, 1.0},
		{1.1, 1.1},
		{0.9, 0.9},
		{5.0, 5.0},
		{3.0, 3.0},
	}

	dbscan := NewDBSCAN(0.2, 2)
	labels := dbscan.Fit(data)

	fmt.Println("聚类结果: ")
	for i, p := range data {
		if labels[i] == -1 {
			fmt.Printf("点 %v -> 噪声\n", p)
		} else {
			fmt.Printf("点 %v -> 簇%d\n", p, labels[i])
		}
	}

	stats := dbscan.GetClusterStats(labels)
	fmt.Println("\n统计: ")
	for k, v := range stats {
		if k == -1 {
			fmt.Printf("噪声: %d\n", v)
		} else {
			fmt.Printf("簇%d: %d\n", k, v)
		}
	}

	// 预测
	test := [][]float64{{1.5, 2.0}, {5.5, 5.0}, {10, 10}}
	fmt.Println("\n预测: ")
	for _, p := range test {
		l := dbscan.Predict(data, labels, p)
		if l == -1 {
			fmt.Printf("点 %v -> 噪声\n", p)
		} else {
			fmt.Printf("点 %v -> 簇%d\n", p, l)
		}
	}
}

//func TestFind4Plus1_Basic(t *testing.T) {
//	data := [][]float64{
//		{0.1, 0.2, 100, 10},
//		{0.15, 0.25, 110, 12},
//		{0.09, 0.18, 95, 9},
//		{1.5, 1.6, 1500, 150},
//		{1.6, 1.7, 1600, 160},
//		{1.55, 1.65, 1550, 155},
//		{0.5, 0.6, 500, 50},
//		{0.45, 0.55, 480, 48},
//		// 添加一个新点, 帮助形成第 4 个簇
//		{0.8, 0.85, 800, 80},
//		{0.75, 0.8, 750, 75},
//	}
//
//	eps, minPts, labels, found := find_4plus1_clusters(data, 100)
//	if !found {
//		t.Fatal("未能找到 4 个簇的参数组合")
//		return
//	}
//
//	fmt.Printf("✅ 找到 4+1 模式！eps=%.2f, minPts=%d\n", eps, minPts)
//	fmt.Println("聚类结果:")
//	for i, label := range labels {
//		noise := ""
//		if label == -1 {
//			noise = " (噪声)"
//		}
//		fmt.Printf("点 %d: 簇 %d%s\n", i, label, noise)
//	}
//
//	// 统计
//	counts := make(map[int]int)
//	for _, l := range labels {
//		counts[l]++
//	}
//	fmt.Println("\n簇分布:")
//	for k, v := range counts {
//		name := "噪声"
//		if k > 0 {
//			name = fmt.Sprintf("资金模式_%d", k)
//		}
//		fmt.Printf("%s: %d 个点\n", name, v)
//	}
//}

func TestDBSCAN_TickData(t *testing.T) {
	// 1. 数据加载
	code := "sh000001"
	code = "sh600744"
	code = "sz000737"
	code = "sh600110"
	code = "sz001696"
	code = "sz000701"
	code = "sz002067"
	date := "2025-08-22"
	date = "2025-06-26"
	date = "2025-06-16"
	date = "2024-10-17"
	//date = "2025-06-04"
	date = "2025-06-30"
	//date = "2025-07-01"
	//date = "2025-07-02"
	date = "2025-09-12"
	//date = "2026-01-15"
	D := data.DataHandler()
	securityCode := data.CorrectSecurityCode(code)
	securityName := market.GetStockName(securityCode)
	fmt.Printf("%s(%s) - %s\n", securityName, securityCode, date)
	ticks, err := D.GetTradeDetails(securityCode, date)
	if len(ticks) == 0 || err != nil {
		t.Fatalf("❌ 无数据: %v", err)
	}
	fmt.Printf("✅ 获取 %d 条分笔数据\n", len(ticks))

	// 2. 转换为Points类型(严格匹配函数签名)
	X_scaled := make([][]float64, len(ticks))
	totalAmount := 0.0
	open_ := 0.0
	close_ := 0.0
	for i, tick := range ticks {
		if i == 0 {
			open_ = tick.Price
		} else if i == len(ticks)-1 {
			close_ = tick.Price
		}
		// 特征工程(与Python版本完全一致)
		num := tick.Num
		if num == 0 {
			num = 1 // 处理除零
		}
		vol := tick.Volume
		if vol == 0 {
			vol = 1
		}

		X_scaled[i] = []float64{
			math.Log1p(tick.Amount),          // amount_log
			math.Log1p(float64(tick.Volume)), // vol_log
			tick.Amount / float64(num),       // amount_per_trade
			tick.Amount / float64(vol),       // amount_per_vol
		}
		totalAmount += tick.Amount
	}
	//unit, divisor := qlab.GetAmountUnitAndDivisor(totalAmount)

	// 3. 标准化(复用已有函数)
	scaler := preprocessing.NewStandardScaler()
	X_scaled, err = scaler.FitTransform(X_scaled)
	if err != nil {
		t.Fatal(err)
	}

	// 4. 调用聚类函数(严格匹配签名)
	best_params, best_labels := find_4plus1_clusters(X_scaled, 50)
	if best_params == nil {
		t.Fatal("⚠️ 未找到4簇方案")
	}
	fmt.Printf("最佳参数: eps=%.2f, min_samples=%.2f\n", best_params["eps"], best_params["min_samples"])

	// 5. 分析结果(复用原始数据ticks和labels)
	fmt.Printf("涨跌幅: %+.2f\n", num.NetChangeRate(open_, close_))
	analyzeResults(ticks, best_labels)
}

type ClusterStat struct {
	Label       float64 // 0.0, 1.0, 2.0, 3.0, -1.0
	Name        string
	Count       int
	TotalAmount float64
	BuyAmount   float64
	SellAmount  float64
	MinAmount   float64
	MaxAmount   float64
	BuyCount    int
	SellCount   int
}

func analyzeResults(ticks []data.Transaction, labels []int) {
	// 1. 方向预处理(严格匹配Python)
	dirNums, buyCount, sellCount := preprocessDirection(ticks)
	fmt.Printf("\n买卖方向分布:\nB    %d\nS    %d\n", buyCount, sellCount)

	// 2. 初始化簇统计(保持Python的簇标签)
	clusters := make(map[float64]*ClusterStat)

	// 初始化噪声簇(必须保留-1标签)
	clusters[-1] = &ClusterStat{
		Label:     -1,
		Name:      "市场噪声",
		MinAmount: math.MaxFloat64,
	}

	// 3. 统计交易数据(保持原始标签)
	for i, label := range labels {
		floatLabel := float64(label)
		if _, exists := clusters[floatLabel]; !exists {
			clusters[floatLabel] = &ClusterStat{
				Label:     floatLabel,
				MinAmount: math.MaxFloat64,
			}
		}

		stat := clusters[floatLabel]
		amount := ticks[i].Amount
		stat.Count++
		stat.TotalAmount += amount

		// 买卖方向统计
		if dirNums[i] == 1 {
			stat.BuyAmount += amount
			stat.BuyCount++
		} else {
			stat.SellAmount += amount
			stat.SellCount++
		}

		if amount < stat.MinAmount {
			stat.MinAmount = amount
		}
		if amount > stat.MaxAmount {
			stat.MaxAmount = amount
		}
	}

	// 4. 按平均金额排序(不改变原始标签)
	var sortedClusters []*ClusterStat
	for _, c := range clusters {
		if c.Label != -1 {
			sortedClusters = append(sortedClusters, c)
		}
	}
	sort.Slice(sortedClusters, func(i, j int) bool {
		return sortedClusters[i].TotalAmount/float64(sortedClusters[i].Count) <
			sortedClusters[j].TotalAmount/float64(sortedClusters[j].Count)
	})

	// 5. 命名规则(严格对应Python顺序)
	classNames := []string{"散户资金", "中单资金", "大单资金", "超大单资金"}
	for i := 0; i < len(sortedClusters) && i < len(classNames); i++ {
		sortedClusters[i].Name = classNames[i]
	}

	// 6. 打印簇信息(完全匹配Python格式)
	fmt.Println("\n===== 自动分类命名 =====")
	fmt.Println("分类结果:")
	for _, c := range sortedClusters {
		mean := c.TotalAmount / float64(c.Count)
		fmt.Printf("簇 %.1f: %s (样本数: %d, 平均金额: %.2f万)\n", c.Label, c.Name, c.Count, mean/1e4)
	}
	noise := clusters[-1]
	fmt.Printf("簇 -1: %s (样本数: %d, 平均金额: %.2f万)\n", noise.Name, noise.Count, noise.TotalAmount/float64(noise.Count)/1e4)

	// 7. 资金流向分析(三部分完整输出)
	printMoneyFlowAnalysis(clusters, sortedClusters, ticks)

	// 1. 计算各规模资金贡献
	fmt.Println("\n🎯 各规模资金对市场净流入的贡献:")
	totalNetAmount := 0.0
	for _, c := range clusters {
		totalNetAmount += c.BuyAmount - c.SellAmount
	}

	fmt.Println("\n🎯 各规模资金对市场净流入的贡献:")
	// 按资金规模从大到小排序输出
	var contributionOrder []*ClusterStat
	for _, c := range sortedClusters {
		contributionOrder = append(contributionOrder, c)
	}
	// 添加噪声簇
	contributionOrder = append(contributionOrder, clusters[-1])

	// 按平均金额从大到小排序
	sort.Slice(contributionOrder, func(i, j int) bool {
		meanI := contributionOrder[i].TotalAmount / float64(contributionOrder[i].Count)
		meanJ := contributionOrder[j].TotalAmount / float64(contributionOrder[j].Count)
		return meanI > meanJ
	})

	for _, c := range contributionOrder {
		net := c.BuyAmount - c.SellAmount
		if math.Abs(net) < 1000000 { // 忽略小于1万的净流入
			continue
		}
		contribution := 0.0
		if totalNetAmount != 0 {
			contribution = net / totalNetAmount * 100
		}
		fmt.Printf("%s: 贡献%+.2f万 (%+.1f%%)\n",
			c.Name,
			net/1e4,
			contribution)
	}

	// 2. 生成重点关注信号
	// 2. 生成重点关注信号
	fmt.Println("\n🔍 需要重点关注的信号:")
	signals := make([]string, 0)

	// 计算总市场金额
	totalMarketAmount := 0.0
	for _, tick := range ticks {
		totalMarketAmount += tick.Amount
	}

	for _, c := range contributionOrder {
		net := c.BuyAmount - c.SellAmount
		marketShare := c.TotalAmount / totalMarketAmount * 100
		netImpact := net / totalNetAmount * 100
		_ = marketShare

		// 判断信号类型和强度
		signalType := "积极"
		if net < 0 {
			signalType = "谨慎"
		}

		signalStrength := "一般"
		if math.Abs(netImpact) > 0.5 {
			signalStrength = "强烈"
		}

		// 只显示有显著影响的信号
		if math.Abs(netImpact) >= 0.1 {
			signals = append(signals, fmt.Sprintf(
				"%s: %s%s信号, 净流入%+.2f万, 影响%+.3f%%",
				c.Name,
				signalStrength,
				signalType,
				net/1e4,
				netImpact,
			))
		}
	}

	if len(signals) > 0 {
		for _, s := range signals {
			fmt.Printf("⚠️  %s\n", s)
		}
	} else {
		fmt.Println("📊 市场资金流向相对平稳, 无显著异常信号")
	}
}

func preprocessDirection(ticks []data.Transaction) (dirNums []int, buyCount, sellCount int) {
	dirNums = make([]int, len(ticks))

	for i := 0; i < len(ticks); i++ {
		switch ticks[i].Direction {
		case 0: // 买入
			dirNums[i] = 1
			buyCount++
		case 1: // 卖出
			dirNums[i] = -1
			sellCount++
		default: // 中性交易处理(严格匹配Python逻辑)
			if i == 0 {
				// 第一条中性交易: 按位置奇偶决定
				if i%2 == 0 {
					dirNums[i] = 1
					buyCount++
				} else {
					dirNums[i] = -1
					sellCount++
				}
			} else {
				// 非首条中性交易: 通过价格变化判断
				prevPrice := ticks[i-1].Price
				currentPrice := ticks[i].Price

				if currentPrice > prevPrice {
					dirNums[i] = 1
					buyCount++
				} else if currentPrice < prevPrice {
					dirNums[i] = -1
					sellCount++
				} else {
					// 价格相同: 按位置奇偶决定
					if i%2 == 0 {
						dirNums[i] = 1
						buyCount++
					} else {
						dirNums[i] = -1
						sellCount++
					}
				}
			}
		}
	}

	return dirNums, buyCount, sellCount
}

func printMoneyFlowAnalysis(
	clusters map[float64]*ClusterStat,
	sortedClusters []*ClusterStat,
	ticks []data.Transaction,
) {
	// 第一部分: 买卖金额表格
	fmt.Println("\n===== 各资金规模买卖方向分析 =====")
	fmt.Printf("%20s %14s %14s %14s %14s\n",
		"", "buy_amount", "sell_amount", "net_amount", "buy_count_ratio")

	// 计算总市场金额
	totalMarketAmount := 0.0
	for _, tick := range ticks {
		totalMarketAmount += tick.Amount
	}

	// 按散户→中单→大单→超大单→噪声顺序打印
	printOrder := []float64{}
	for _, c := range sortedClusters {
		printOrder = append(printOrder, c.Label)
	}
	printOrder = append(printOrder, -1) // 噪声放在最后

	for _, label := range printOrder {
		c := clusters[label]
		buyRatio := 0.0
		if c.BuyCount+c.SellCount > 0 {
			buyRatio = float64(c.BuyCount) / float64(c.BuyCount+c.SellCount) * 100
		}
		fmt.Printf("%20s %14.2f %14.2f %14.2f %14.2f\n",
			c.Name,
			c.BuyAmount,
			c.SellAmount,
			c.BuyAmount-c.SellAmount,
			buyRatio)
	}

	// 第二部分: 详细资金流向分析
	fmt.Println("\n💰 资金流向分析:")
	fmt.Println("============================================================")
	for _, label := range printOrder {
		c := clusters[label]
		netAmount := c.BuyAmount - c.SellAmount
		marketShare := c.TotalAmount / totalMarketAmount * 100
		netImpact := netAmount / totalMarketAmount * 100

		buyRatio := 0.0
		if c.BuyCount+c.SellCount > 0 {
			buyRatio = float64(c.BuyCount) / float64(c.BuyCount+c.SellCount) * 100
		}

		fmt.Printf("%s:\n", c.Name)
		fmt.Printf("  → 笔数: 买入%.1f%% vs 卖出%.1f%% (净%+.1f%%)\n",
			buyRatio, 100-buyRatio, buyRatio-(100-buyRatio))
		fmt.Printf("  → 金额: 买入%.2f万 vs 卖出%.2f万 (净%+.2f万)\n",
			c.BuyAmount/1e4, c.SellAmount/1e4, netAmount/1e4)
		fmt.Printf("  → 影响: 占市场%.2f%%, 净流入贡献%+.3f%%\n",
			marketShare, netImpact)
		fmt.Println("----------------------------------------")
	}

	// 第三部分: 总体市场统计
	totalBuy, totalSell := 0.0, 0.0
	for _, c := range clusters {
		totalBuy += c.BuyAmount
		totalSell += c.SellAmount
	}
	fmt.Println("\n📊 总体市场资金流向:")
	fmt.Printf("总买入金额: %.2f万\n", totalBuy/1e4)
	fmt.Printf("总卖出金额: %.2f万\n", totalSell/1e4)
	fmt.Printf("净流入金额: %+.2f万\n", (totalBuy-totalSell)/1e4)
	fmt.Printf("市场总成交: %.2f万\n", totalMarketAmount/1e4)
}
