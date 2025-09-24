// Copyright (c) 2023 Austin Ouyang. All rights reserved.
// Licensed under the MIT License. See go-fpgrowth.LICENSE for details.

package fpgrowth

import (
	"errors"
	"runtime"
	"sync"
)

var (
	ErrNilTransaction    = errors.New("nil transaction")
	ErrInvalidMinSupport = errors.New("invalid minimum support. must be from 0 to 1.")
	RootName             = "__ROOT__"
)

type FPGrowth struct {
	MinSupport float64

	frequentItems *frequentItems
	transactions  []*Transaction // list of all transactions
	tree          *node

	patternBases []*PatternBase // stores pattern bases for each frequent item from most frequent to least
}

// New 创建一个新的FPGrowth实例，minSupport参数必须在0到1之间
//
//	如果minSupport无效则返回错误，否则返回初始化好的FPGrowth结构体
func New(minSupport float64) (*FPGrowth, error) {
	if minSupport > 1 || minSupport < 0 {
		return nil, ErrInvalidMinSupport
	}

	return &FPGrowth{
		MinSupport:    minSupport,
		tree:          newNode(RootName),
		frequentItems: newFrequentItems(),
	}, nil
}

// Fit 训练FP-Growth模型，通过给定的交易数据构建频繁项集和条件模式基
// 参数:
//
//	t: 交易数据集合，每个交易是一个*Transaction指针
//
// 返回值:
//
//	error: 处理过程中遇到的错误，无错误时返回nil
//
// 注意:
//   - 使用并发方式计算条件模式基，并发数受CPU核心数限制
//   - 会修改接收者f的内部状态
func (f *FPGrowth) Fit(t []*Transaction) error {
	// 设置交易数量
	f.frequentItems.n = len(t)

	for _, tx := range t {
		if err := f.insert(tx); err != nil {
			return err
		}
	}
	f.buildTree()

	// 并发计算条件模式基
	numItems := len(f.frequentItems.itemCounts)
	f.patternBases = make([]*PatternBase, numItems)

	var wg sync.WaitGroup
	sem := make(chan struct{}, runtime.NumCPU()) // 限制并发数

	for i := numItems - 1; i >= 0; i-- {
		wg.Add(1)
		go func(idx int) {
			defer wg.Done()
			sem <- struct{}{}        // 获取信号量
			defer func() { <-sem }() // 释放信号量

			ic := f.frequentItems.itemCounts[idx]
			cpb := f.conditionalPatternBases(ic.name)
			subpb := intersectConditionalPatternBases(cpb)
			f.patternBases[idx] = &PatternBase{
				Item:           ic.name,
				SubPatternBase: subpb,
			}
		}(i)
	}
	wg.Wait()
	return nil
}

// PatternBases returns the list of pattern bases found during the FPGrowth mining process
func (f *FPGrowth) PatternBases() []*PatternBase {
	return f.patternBases
}

// insert 将交易添加到FPGrowth实例中，并更新频繁项集
//
//	参数t为要添加的交易，不能为nil
//	返回错误如果交易为nil，否则返回nil
func (f *FPGrowth) insert(t *Transaction) error {
	if t == nil {
		return ErrNilTransaction
	}

	f.transactions = append(f.transactions, t)
	for _, item := range t.Items {
		// only add items that are non empty strings
		if item != "" {
			f.frequentItems.add(item)
		}
	}
	return nil
}

// buildTree 构建FP-growth算法的FP树结构
//  1. 首先获取按支持度排序的频繁项集
//  2. 第二次遍历事务数据集构建FP树
//  3. 对每个事务中的频繁项，从根节点开始构建树路径
//  4. 更新头表(head table)以维护相同项的链表连接
//  5. 遇到已存在节点时增加计数，否则创建新节点
func (f *FPGrowth) buildTree() {
	// find frequent items in sorted order
	fi := f.frequentItems.getSorted(f.MinSupport)

	// second pass of transactions building th FP Tree
	for _, t := range f.transactions {
		currNode := f.tree
		for _, i := range fi {
			if !t.Exists(i) {
				continue
			}
			nextNode, ok := currNode.children[i]
			if !ok {
				nextNode = newNode(i)
				currNode.children[i] = nextNode
				nextNode.parent = currNode

				// update header table - link to the end of the list
				if f.frequentItems.cnt[i].head == nil {
					f.frequentItems.cnt[i].head = nextNode
				} else {
					// find the last node in the header list and link the new node
					lastNode := f.frequentItems.cnt[i].head
					for lastNode.next != nil {
						lastNode = lastNode.next
					}
					lastNode.next = nextNode
				}
			}
			nextNode.count += 1
			currNode = nextNode
		}
	}
}

// conditionalPatternBases 返回给定项的频繁项集的条件模式基
//
//	参数 item: 需要查找条件模式基的项
//	返回值: 包含条件模式基的二维切片，每个子切片表示一个条件模式基路径及其计数
func (f *FPGrowth) conditionalPatternBases(item string) [][]itemCount {
	fi, exists := f.frequentItems.cnt[item]
	if !exists {
		return nil
	}

	var res [][]itemCount
	fip := fi.head

	for {
		if fip == nil {
			break
		}
		cpb := findPrefixPath(fip)
		if len(cpb) > 1 {
			items := make([]itemCount, 0, len(cpb)-1)
			for _, pathItem := range cpb[:len(cpb)-1] {
				items = append(items, itemCount{pathItem, fip.count})
			}
			res = append(res, items)
		}
		fip = fip.next
	}
	return res
}

// findPrefixPath 返回从给定节点到根节点的路径字符串切片
//
//	如果节点为空或为根节点，则返回nil
//	使用迭代方式实现以避免递归栈溢出
func findPrefixPath(n *node) []string {
	if n == nil || n.item == RootName {
		return nil
	}

	// 迭代实现，避免递归栈溢出
	var path []string
	current := n
	for current != nil && current.item != RootName {
		path = append([]string{current.item}, path...) // 插入到开头
		current = current.parent
	}
	return path
}

// intersectConditionalPatternBases 计算多个条件模式基的交集，并累加匹配项的计数
//
//	参数 cpb 是多个条件模式基的切片，每个模式基是一个itemCount切片
//	返回交集结果，保持第一个模式基的顺序，每个匹配项的计数是所有模式基中该项目的计数之和
func intersectConditionalPatternBases(cpb [][]itemCount) []itemCount {
	if len(cpb) == 0 {
		return nil
	}

	// 使用map优化交集计算
	countMap := make(map[string]int)
	for _, pb := range cpb {
		pbMap := make(map[string]int)
		for _, item := range pb {
			pbMap[item.name] = item.count
		}

		if len(countMap) == 0 {
			// 第一个模式基，直接复制
			for k, v := range pbMap {
				countMap[k] = v
			}
			continue
		}

		// 移除不在当前模式基中的项，并累加计数
		for item := range countMap {
			if cnt, exists := pbMap[item]; exists {
				countMap[item] += cnt
			} else {
				delete(countMap, item)
			}
		}
	}

	// 转换为结果slice，使用第一个模式基的顺序
	if len(cpb) == 0 {
		return nil
	}
	res := make([]itemCount, 0, len(countMap))
	for _, item := range cpb[0] {
		if cnt, exists := countMap[item.name]; exists {
			res = append(res, itemCount{item.name, cnt})
		}
	}
	return res
}
