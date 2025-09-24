// Copyright (c) 2023 Austin Ouyang. All rights reserved.
// Licensed under the MIT License. See go-fpgrowth.LICENSE for details.

package main

import (
	"fmt"

	fpgrowth2 "gitee.com/quant1x/labs/qlab/learn/fpgrowth"
)

func main() {
	// 超市购物篮数据（对应Python示例）
	transactions := []*fpgrowth2.Transaction{
		{ID: 0, Items: []string{"牛奶", "面包", "黄油"}},
		{ID: 1, Items: []string{"牛奶", "面包"}},
		{ID: 2, Items: []string{"牛奶", "黄油"}},
		{ID: 3, Items: []string{"面包", "黄油"}},
		{ID: 4, Items: []string{"牛奶", "面包", "黄油", "鸡蛋"}},
		{ID: 5, Items: []string{"鸡蛋", "黄油"}},
		{ID: 6, Items: []string{"牛奶", "鸡蛋"}},
		{ID: 7, Items: []string{"牛奶", "面包", "鸡蛋"}},
		{ID: 8, Items: []string{"牛奶", "面包", "黄油", "鸡蛋", "果汁"}},
		{ID: 9, Items: []string{"果汁", "面包"}},
	}

	fmt.Println("FP Growth Go实现示例")
	fmt.Println("====================")
	fmt.Println()

	fmt.Println("原始交易数据:")
	for i, tx := range transactions {
		fmt.Printf("交易 %d: %v\n", i+1, tx.Items)
	}

	// 设置最小支持度 (30%)
	minSupport := 0.3
	fmt.Printf("\n使用 FP Growth 挖掘频繁项集 (min_support=%.1f):\n", minSupport)

	// 创建FP Growth实例
	fpg, err := fpgrowth2.New(minSupport)
	if err != nil {
		fmt.Printf("创建FPGrowth失败: %v\n", err)
		return
	}

	// 训练模型
	if err := fpg.Fit(transactions); err != nil {
		fmt.Printf("训练FPGrowth失败: %v\n", err)
		return
	}

	// 获取频繁项集
	frequentItemsets := fpg.GetFrequentItemsets()

	fmt.Println("频繁项集:")
	fmt.Printf("%-30s %s\n", "项集", "支持度")
	fmt.Println("------------------------------")
	for _, itemset := range frequentItemsets {
		support := float64(itemset.Support) / float64(len(transactions))
		fmt.Printf("%-30v %.3f (%d/%d)\n", itemset.Items, support, itemset.Support, len(transactions))
	}

	fmt.Println()
	fmt.Println("说明:")
	fmt.Println("- 当前实现提取了频繁单项集")
	fmt.Println("- 组合项集（如{牛奶, 面包}）需要完整的递归FP Growth实现")
	fmt.Println("- 这演示了Go版本FP Growth的基本工作流程")
}
