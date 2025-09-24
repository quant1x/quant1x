// Copyright (c) 2023 Austin Ouyang. All rights reserved.
// Licensed under the MIT License. See go-fpgrowth.LICENSE for details.

package fpgrowth

type Transaction struct {
	ID      int
	Items   []string        // slice of item names
	itemSet map[string]bool // 缓存项集合，用于快速查找
}

func (t *Transaction) Exists(item string) bool {
	if t.itemSet == nil {
		t.itemSet = make(map[string]bool, len(t.Items))
		for _, i := range t.Items {
			t.itemSet[i] = true
		}
	}
	return t.itemSet[item]
}
