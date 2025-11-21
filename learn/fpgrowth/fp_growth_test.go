package fpgrowth

import (
	"fmt"
	"testing"
)

func TestFPGrowthInt(t *testing.T) {
	transactions := [][]int{
		{1, 2, 5},
		{2, 4},
		{2, 3},
		{1, 2, 4},
		{1, 3},
		{2, 3},
		{1, 3},
		{1, 2, 3, 5},
		{1, 2, 3},
	}

	fp := New[int](0.3)
	patterns := fp.Mine(transactions)

	fmt.Printf("Found %d patterns\n", len(patterns))
	for _, p := range patterns {
		fmt.Printf("Pattern: %v, Support: %.2f\n", p.Items, p.Support)
	}
}

func TestFPGrowthString(t *testing.T) {
	transactions := [][]string{
		{"牛奶", "面包", "黄油"},
		{"牛奶", "面包"},
		{"牛奶", "黄油"},
		{"面包", "黄油"},
		{"牛奶", "面包", "黄油", "鸡蛋"},
		{"鸡蛋", "黄油"},
		{"牛奶", "鸡蛋"},
		{"牛奶", "面包", "鸡蛋"},
		{"牛奶", "面包", "黄油", "鸡蛋", "果汁"},
		{"果汁", "面包"},
	}

	fp := New[string](0.3)
	patterns := fp.Mine(transactions)

	fmt.Printf("Found %d patterns\n", len(patterns))
	for _, p := range patterns {
		fmt.Printf("Pattern: %v, Support: %.2f\n", p.Items, p.Support)
	}
}
