package fpgrowth

import (
	"math"
	"sort"
)

// FPGrowthCore 核心实现, 处理 int 类型的频繁项集挖掘
type FPGrowthCore struct {
	minSupport        float64
	minSupportCount   int
	useCountThreshold bool
}

type ItemSet []int
type Support float64

type FrequentPattern struct {
	Items   ItemSet
	Support Support
}

// internalFrequentPattern 用于内部计算, 存储计数
type internalFrequentPattern struct {
	Items []int
	Count int
}

func NewCore(minSupport float64) *FPGrowthCore {
	return &FPGrowthCore{
		minSupport: minSupport,
	}
}

func NewCoreWithCount(minSupportCount int) *FPGrowthCore {
	return &FPGrowthCore{
		minSupportCount:   minSupportCount,
		useCountThreshold: true,
	}
}

func (c *FPGrowthCore) Mine(transactions [][]int) []FrequentPattern {
	if len(transactions) == 0 {
		return nil
	}

	// 1. Count item frequencies
	itemCounts := c.countItemFrequencies(transactions)
	totalTransactions := len(transactions)

	// 2. Calculate min support count
	minCount := c.minSupportCount
	if !c.useCountThreshold {
		minCount = int(math.Ceil(c.minSupport * float64(totalTransactions)))
	}

	// 3. Get frequent items sorted by frequency desc
	frequentItems := c.getFrequentItems(itemCounts, minCount)
	if len(frequentItems) == 0 {
		return nil
	}

	// 4. Build FP-Tree
	fpTree := newFPTree()
	headerTable := make([]*headerEntry, len(frequentItems))
	rankMap := make(map[int]int)

	for i, item := range frequentItems {
		headerTable[i] = &headerEntry{
			itemID:  item,
			support: itemCounts[item],
		}
		rankMap[item] = i
	}

	for _, tx := range transactions {
		var filteredTx []int
		for _, item := range tx {
			if itemCounts[item] >= minCount {
				filteredTx = append(filteredTx, item)
			}
		}

		if len(filteredTx) > 0 {
			c.sortTransactionByFrequency(filteredTx, rankMap)
			fpTree.insert(filteredTx, rankMap, headerTable, 1)
		}
	}

	// 5. Mine patterns
	internalPatterns := fpTree.minePatterns(headerTable, minCount)

	// 6. Convert to ratio
	patterns := make([]FrequentPattern, len(internalPatterns))
	for i, p := range internalPatterns {
		patterns[i] = FrequentPattern{
			Items:   p.Items,
			Support: Support(float64(p.Count) / float64(totalTransactions)),
		}
	}

	return patterns
}

func (c *FPGrowthCore) countItemFrequencies(transactions [][]int) map[int]int {
	counts := make(map[int]int)
	for _, tx := range transactions {
		for _, item := range tx {
			counts[item]++
		}
	}
	return counts
}

func (c *FPGrowthCore) getFrequentItems(itemCounts map[int]int, minCount int) []int {
	type itemPair struct {
		id    int
		count int
	}
	var items []itemPair
	for id, count := range itemCounts {
		if count >= minCount {
			items = append(items, itemPair{id, count})
		}
	}

	// Sort desc by count, then by id (to be deterministic and match C++ std::pair comparison)
	sort.Slice(items, func(i, j int) bool {
		if items[i].count == items[j].count {
			return items[i].id > items[j].id
		}
		return items[i].count > items[j].count
	})

	result := make([]int, len(items))
	for i, item := range items {
		result[i] = item.id
	}
	return result
}

func (c *FPGrowthCore) sortTransactionByFrequency(tx []int, rankMap map[int]int) {
	sort.Slice(tx, func(i, j int) bool {
		return rankMap[tx[i]] < rankMap[tx[j]]
	})
}

// --- Internal Structures ---

type fpNode struct {
	itemID   int
	count    int
	parent   *fpNode
	children map[int]*fpNode
	next     *fpNode
}

func newFPNode(item int, count int, parent *fpNode) *fpNode {
	return &fpNode{
		itemID:   item,
		count:    count,
		parent:   parent,
		children: make(map[int]*fpNode),
	}
}

type headerEntry struct {
	itemID  int
	support int
	head    *fpNode
}

type fpTree struct {
	root *fpNode
}

func newFPTree() *fpTree {
	return &fpTree{
		root: newFPNode(0, 0, nil), // Root has item 0
	}
}

func (t *fpTree) insert(tx []int, rankMap map[int]int, headerTable []*headerEntry, count int) {
	current := t.root
	for _, item := range tx {
		if child, exists := current.children[item]; exists {
			child.count += count
			current = child
		} else {
			newNode := newFPNode(item, count, current)
			current.children[item] = newNode

			// Update header table
			rank := rankMap[item]
			entry := headerTable[rank]
			newNode.next = entry.head
			entry.head = newNode

			current = newNode
		}
	}
}

func (t *fpTree) minePatterns(headerTable []*headerEntry, minCount int) []internalFrequentPattern {
	var patterns []internalFrequentPattern

	// Iterate from bottom (lowest support) to top
	for i := len(headerTable) - 1; i >= 0; i-- {
		entry := headerTable[i]

		// 1. Conditional pattern base
		conditionalPatterns := t.mineConditionalPatterns(headerTable, entry.itemID, minCount)

		// 2. Single item pattern
		patterns = append(patterns, internalFrequentPattern{
			Items: []int{entry.itemID},
			Count: entry.support,
		})

		// 3. Merge
		for _, p := range conditionalPatterns {
			// Create new slice to avoid modifying shared underlying array
			newItems := make([]int, len(p.Items)+1)
			copy(newItems, p.Items)
			newItems[len(p.Items)] = entry.itemID
			patterns = append(patterns, internalFrequentPattern{
				Items: newItems,
				Count: p.Count,
			})
		}
	}
	return patterns
}

func (t *fpTree) mineConditionalPatterns(headerTable []*headerEntry, suffixItem int, minCount int) []internalFrequentPattern {
	// Find suffix entry
	var suffixEntry *headerEntry
	for _, entry := range headerTable {
		if entry.itemID == suffixItem {
			suffixEntry = entry
			break
		}
	}
	if suffixEntry == nil || suffixEntry.head == nil {
		return nil
	}

	// 1st scan: count frequencies in conditional paths
	conditionalCounts := make(map[int]int)
	current := suffixEntry.head
	for current != nil {
		pathCount := current.count
		node := current.parent
		for node != nil && node.itemID != 0 { // Assuming 0 is root
			conditionalCounts[node.itemID] += pathCount
			node = node.parent
		}
		current = current.next
	}

	// Filter
	type itemPair struct {
		id    int
		count int
	}
	var conditionalItems []itemPair
	for id, count := range conditionalCounts {
		if count >= minCount {
			conditionalItems = append(conditionalItems, itemPair{id, count})
		}
	}
	if len(conditionalItems) == 0 {
		return nil
	}

	// Sort desc
	sort.Slice(conditionalItems, func(i, j int) bool {
		if conditionalItems[i].count == conditionalItems[j].count {
			return conditionalItems[i].id > conditionalItems[j].id
		}
		return conditionalItems[i].count > conditionalItems[j].count
	})

	// Order map
	orderMap := make(map[int]int)
	// conditionalItemOrder := make([]int, len(conditionalItems)) // Unused
	for i, item := range conditionalItems {
		// conditionalItemOrder[i] = item.id
		orderMap[item.id] = i
	}

	// Build conditional tree
	conditionalTree := newFPTree()
	conditionalHeaderTable := make([]*headerEntry, len(conditionalItems))
	for i, item := range conditionalItems {
		conditionalHeaderTable[i] = &headerEntry{
			itemID:  item.id,
			support: item.count,
		}
	}

	// 2nd scan: insert paths
	current = suffixEntry.head
	for current != nil {
		pathCount := current.count
		var filteredPattern []int
		node := current.parent
		for node != nil && node.itemID != 0 {
			if conditionalCounts[node.itemID] >= minCount {
				filteredPattern = append(filteredPattern, node.itemID)
			}
			node = node.parent
		}

		if len(filteredPattern) > 0 {
			// Sort by frequency in conditional tree
			sort.Slice(filteredPattern, func(i, j int) bool {
				return orderMap[filteredPattern[i]] < orderMap[filteredPattern[j]]
			})
			conditionalTree.insert(filteredPattern, orderMap, conditionalHeaderTable, pathCount)
		}
		current = current.next
	}

	return conditionalTree.minePatterns(conditionalHeaderTable, minCount)
}
