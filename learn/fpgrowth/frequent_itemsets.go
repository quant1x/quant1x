// Copyright (c) 2023 Austin Ouyang. All rights reserved.
// Licensed under the MIT License. See go-fpgrowth.LICENSE for details.

package fpgrowth

import (
	"sort"
)

// FrequentItemset 表示频繁项集
type FrequentItemset struct {
	Items   []string
	Support int
}

// GetFrequentItemsets 从FPGrowth结果中提取频繁项集, 按支持度降序排序
//
//	返回包含频繁项集及其支持度的切片，包括单项集和多项集
//	单项集直接从频繁项中获取，多项集通过挖掘条件模式基生成
func (f *FPGrowth) GetFrequentItemsets() []FrequentItemset {
	var result []FrequentItemset

	// 获取频繁单项集（按支持度降序排序）
	frequentItems := f.getFrequentItems()
	numTransactions := len(f.transactions)
	minSupportCount := int(float64(numTransactions) * f.MinSupport)

	// 添加单项集
	for _, item := range frequentItems {
		result = append(result, FrequentItemset{
			Items:   []string{item.name},
			Support: item.count,
		})
	}

	// 对每个频繁项进行条件模式基挖掘
	for _, item := range frequentItems {
		f.mineConditionalPatternBase([]string{item.name}, item.name, minSupportCount, &result)
	}

	// 按支持度降序排序
	sort.Slice(result, func(i, j int) bool {
		return result[i].Support > result[j].Support
	})

	return result
}

// getFrequentItems 返回满足最小支持度的频繁项集，按支持度降序排序
func (f *FPGrowth) getFrequentItems() []itemCount {
	numTransactions := len(f.transactions)
	minSupportCount := int(float64(numTransactions) * f.MinSupport)

	var result []itemCount
	for itemName, fic := range f.frequentItems.cnt {
		if fic.count >= minSupportCount {
			result = append(result, itemCount{itemName, fic.count})
		}
	}

	// 按支持度降序排序
	sort.Slice(result, func(i, j int) bool {
		return result[i].count > result[j].count
	})

	return result
}

// mineConditionalPatternBase 递归挖掘条件模式基中的频繁项集
//
//	prefix: 当前前缀项集
//	conditionalItem: 条件项
//	minSupportCount: 最小支持度计数
//	result: 用于存储发现的频繁项集的指针
func (f *FPGrowth) mineConditionalPatternBase(prefix []string, conditionalItem string, minSupportCount int, result *[]FrequentItemset) {
	// 获取条件模式基
	conditionalPB := f.conditionalPatternBases(conditionalItem)
	if len(conditionalPB) == 0 {
		return
	}

	// 从条件模式基中提取频繁项
	frequentInCPB := f.extractFrequentFromConditionalPB(conditionalPB, minSupportCount)

	// 对每个在条件模式基中的频繁项，递归挖掘
	for _, item := range frequentInCPB {
		// 创建新的前缀
		newPrefix := make([]string, len(prefix)+1)
		copy(newPrefix, prefix)
		newPrefix[len(prefix)] = item.name

		// 添加这个组合到结果
		*result = append(*result, FrequentItemset{
			Items:   append([]string(nil), newPrefix...), // 复制slice
			Support: item.count,
		})

		// 递归挖掘
		f.mineConditionalPatternBase(newPrefix, item.name, minSupportCount, result)
	}
}

// extractFrequentFromConditionalPB 从条件模式基中提取频繁项集
// 参数：
//
//	conditionalPB - 条件模式基，包含项集及其计数
//	minSupportCount - 最小支持度阈值
//
// 返回值：
//
//	按支持度降序排列的频繁项集及其计数
func (f *FPGrowth) extractFrequentFromConditionalPB(conditionalPB [][]itemCount, minSupportCount int) []itemCount {
	if len(conditionalPB) == 0 {
		return nil
	}

	// 统计每个项在条件模式基中的支持度
	itemSupport := make(map[string]int)
	for _, pb := range conditionalPB {
		for _, item := range pb {
			itemSupport[item.name] += item.count
		}
	}

	// 提取频繁项
	var result []itemCount
	for itemName, support := range itemSupport {
		if support >= minSupportCount {
			result = append(result, itemCount{itemName, support})
		}
	}

	// 按支持度降序排序
	sort.Slice(result, func(i, j int) bool {
		return result[i].count > result[j].count
	})

	return result
}
