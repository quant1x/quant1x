package fpgrowth

import (
	"math"
	"sort"
)

type frequentItems struct {
	n          int                           // number of items stored
	cnt        map[string]*frequentItemCount // tracks most frequent items of all transactions
	itemCounts itemCounts                    // item names sorted by most frequent
}

// newFrequentItems 创建并返回一个新的frequentItems实例，初始化内部计数器映射
func newFrequentItems() *frequentItems {
	return &frequentItems{
		cnt: make(map[string]*frequentItemCount),
	}
}

// reset 重置 frequentItems 的内部状态，清空计数器和存储空间
func (f *frequentItems) reset() {
	f.n = 0
	f.cnt = make(map[string]*frequentItemCount)
	f.itemCounts = f.itemCounts[:0]
}

// add 增加指定项的计数，如果项已存在则计数加1，否则初始化计数为1
func (f *frequentItems) add(item string) {
	if fic, ok := f.cnt[item]; ok {
		fic.count += 1
	} else {
		f.cnt[item] = &frequentItemCount{nil, 1}
	}
}

// get 返回指定项的计数，如果项不存在则返回0
func (f *frequentItems) get(item string) int {
	fic, ok := f.cnt[item]
	if ok {
		return fic.count
	}
	return 0
}

// getSorted 根据最小支持度筛选并返回排序后的频繁项
//
//	minSupport: 最小支持度阈值(0-1之间)，小于0视为0，大于1视为1
//	返回值: 按出现次数降序排列的项名称列表，次数相同时按名称降序排列
func (f *frequentItems) getSorted(minSupport float64) []string {
	if minSupport < 0 {
		minSupport = 0
	} else if minSupport > 1 {
		minSupport = 1
	}
	minCnt := int(math.Ceil(minSupport * float64(f.n)))

	f.itemCounts = f.itemCounts[:0]
	for itemName, fic := range f.cnt {
		if fic.count >= minCnt {
			f.itemCounts = append(f.itemCounts, itemCount{itemName, fic.count})
		}
	}
	sort.Slice(f.itemCounts, func(i, j int) bool {
		if f.itemCounts[i].count > f.itemCounts[j].count {
			return true
		}
		if f.itemCounts[i].count < f.itemCounts[j].count {
			return false
		}
		return f.itemCounts[i].name > f.itemCounts[j].name
	})

	items := make([]string, 0, len(f.itemCounts))
	for _, ic := range f.itemCounts {
		items = append(items, ic.name)
	}

	return items
}

type itemCounts []itemCount

type itemCount struct {
	name  string
	count int
}

type frequentItemCount struct {
	head  *node // points to first item in the FPTree and serves as the Header Table
	count int
}
