package fpgrowth

// Pattern 泛型结果
type Pattern[T any] struct {
	Items   []T
	Support float64
}

// FPGrowth 泛型实现
type FPGrowth[T comparable] struct {
	core *FPGrowthCore
}

func New[T comparable](minSupport float64) *FPGrowth[T] {
	return &FPGrowth[T]{
		core: NewCore(minSupport),
	}
}

func NewWithCount[T comparable](minSupportCount int) *FPGrowth[T] {
	return &FPGrowth[T]{
		core: NewCoreWithCount(minSupportCount),
	}
}

func (fp *FPGrowth[T]) Mine(transactions [][]T) []Pattern[T] {
	if len(transactions) == 0 {
		return nil
	}

	// 1. Map T -> int
	itemToID := make(map[T]int)
	idToItem := make(map[int]T)
	nextID := 1 // Start from 1, 0 is reserved for root

	coreTransactions := make([][]int, len(transactions))
	for i, tx := range transactions {
		coreTx := make([]int, len(tx))
		for j, item := range tx {
			if id, exists := itemToID[item]; exists {
				coreTx[j] = id
			} else {
				id = nextID
				nextID++
				itemToID[item] = id
				idToItem[id] = item
				coreTx[j] = id
			}
		}
		coreTransactions[i] = coreTx
	}

	// 2. Call core
	corePatterns := fp.core.Mine(coreTransactions)

	// 3. Map back
	patterns := make([]Pattern[T], len(corePatterns))
	for i, p := range corePatterns {
		items := make([]T, len(p.Items))
		for j, id := range p.Items {
			items[j] = idToItem[id]
		}
		patterns[i] = Pattern[T]{
			Items:   items,
			Support: float64(p.Support),
		}
	}

	return patterns
}
