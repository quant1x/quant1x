package factors

import (
	"fmt"
	"log"
	"testing"

	"github.com/quant1x/quant1x/quant1x/data/meta"
)

func TestOptionFinanceBoard(t *testing.T) {
	data, err := OptionFinanceBoard("华夏上证50ETF期权", "2509")
	if err != nil {
		log.Printf("获取期权行情失败: %v", err)
	} else {
		for _, d := range data {
			fmt.Printf("%+v\n", d)
		}
	}
}

func TestOptions(t *testing.T) {
	// 示例1: 获取上证50ETF期权行情（到期月 06）
	data, err := OptionFinanceBoard("华夏上证50ETF期权", "2508")
	if err != nil {
		log.Printf("获取期权行情失败: %v", err)
	} else {
		for _, d := range data {
			fmt.Printf("%+v\n", d)
		}
	}

	// 示例3: 获取风险指标
	riskData, err := OptionRiskIndicatorSSE("20240626")
	if err != nil {
		log.Printf("获取风险指标失败: %v", err)
	} else {
		for _, r := range riskData {
			fmt.Printf("%+v\n", r)
		}
	}
}

func TestVIX(t *testing.T) {
	now := meta.NowTimestamp()
	latestTradingDay := meta.LastTradingDay(now)
	tradeDate := meta.PrevTradingDay(latestTradingDay)

	date := tradeDate.ToString("20060102") // 格式化为 YYYYMMDD
	fmt.Println("🚀 开始执行 300ETF 恐慌指数监控...")

	// 1. 获取风险数据 (真实接口)
	fmt.Printf("📡 正在获取 %s 风险数据...\n", date)
	riskData, err := OptionRiskIndicatorSSE(date)
	if err != nil {
		t.Fatalf("❌ 获取风险数据失败: %v", err)
	}
	fmt.Printf("✅ 成功获取 %d 条风险数据\n", len(riskData))
	if len(riskData) == 0 {
		t.Skipf("交易日 %s 暂无风险指标数据，跳过实时 VIX 计算", date)
	}

	// 2. 提取并合并数据 (会自动调用 OptionFinanceBoard 获取价格)
	mergedData, err := extractAndMergeData(riskData, date)
	if err != nil {
		t.Fatalf("❌ 数据合并失败: %v", err)
	}

	// 3. 计算恐慌指数
	fmt.Println("\n🔍 正在计算【真实VIX】（CBOE官方方法）...")
	vixValue, err := calculateRealVix(mergedData, date, RISK_FREE_RATE)
	if err != nil {
		log.Printf("❌ 真实VIX计算失败: %v", err)
		// 回退逻辑...
	} else {
		fmt.Printf("🎯 真实 A股300ETF恐慌指数（VIX）: %.2f\n", vixValue)
	}

	fmt.Println("\n🎉 全部完成！")
}
