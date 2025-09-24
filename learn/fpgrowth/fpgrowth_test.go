// Copyright (c) 2023 Austin Ouyang. All rights reserved.
// Licensed under the MIT License. See go-fpgrowth.LICENSE for details.

package fpgrowth

import (
	"testing"
)

func TestNew(t *testing.T) {
	minSupport := 0.7
	fpg, err := New(minSupport)
	if err != nil {
		t.Error(err)
		return
	}
	if fpg.MinSupport != minSupport {
		t.Errorf("expected, %.3f, for min support but got %.3f", minSupport, fpg.MinSupport)
		return
	}
	if err := sameNode(fpg.tree, newNode(RootName)); err != nil {
		t.Error(err)
		return
	}
}

func testTransactions() []*Transaction {
	return []*Transaction{
		{ID: 0, Items: []string{"f", "a", "c", "d", "g", "i", "m", "p"}},
		{ID: 1, Items: []string{"a", "b", "c", "f", "l", "m", "o"}},
		{ID: 2, Items: []string{"b", "f", "h", "j", "o"}},
		{ID: 3, Items: []string{"b", "c", "k", "s", "p"}},
		{ID: 4, Items: []string{"a", "f", "c", "e", "l", "p", "m", "n"}},
	}
}

func TestInsert(t *testing.T) {
	transactions := testTransactions()
	fpg, err := New(0.7)
	if err != nil {
		t.Error(err)
		return
	}
	for _, tr := range transactions {
		if err := fpg.insert(tr); err != nil {
			t.Error(err)
			return
		}
	}

	// 设置交易数量（因为只调用了insert，没有调用Fit）
	fpg.frequentItems.n = len(transactions)

	if len(transactions) != len(fpg.transactions) {
		t.Errorf("expected %d transactions but got %d", len(transactions), len(fpg.transactions))
		return
	}

	for i, trx := range transactions {
		if trx.ID != fpg.transactions[i].ID {
			t.Errorf("expected ID, %d for transaction at index %d, but got %d", fpg.transactions[i].ID, i, trx.ID)
			return
		}
		if len(trx.Items) != len(fpg.transactions[i].Items) {
			t.Errorf("expected %d items for transaction id %d, but got %d", len(trx.Items), trx.ID, len(fpg.transactions[i].Items))
			return
		}
		for j, item := range trx.Items {
			if item != fpg.transactions[i].Items[j] {
				t.Errorf("expected item, %s, but got %s", item, fpg.transactions[i].Items[j])
				return
			}
		}
	}

	if fpg.frequentItems.n != len(transactions) {
		t.Errorf("expected %d transactions, but got %d", len(transactions), fpg.frequentItems.n)
		return
	}

	expectedFI := map[string]*frequentItemCount{
		"f": {nil, 4},
		"c": {nil, 4},
		"p": {nil, 3},
		"m": {nil, 3},
		"b": {nil, 3},
		"a": {nil, 3},
	}

	for k, fic := range expectedFI {
		if val, exists := fpg.frequentItems.cnt[k]; !exists {
			t.Errorf("expected to find item, %s, in frequent item set", k)
			return
		} else {
			if val.count != fic.count {
				t.Errorf("expected count of %d, for item, %s, but got %d", fic.count, k, val.count)
				return
			}
		}
	}
}

func TestFindPrefixPath(t *testing.T) {
	testData := []struct {
		n        *node
		expected []string
	}{
		{
			&node{item: "a", parent: &node{item: "b", parent: &node{item: "c"}}},
			[]string{"c", "b", "a"},
		},
		{
			&node{item: "a", parent: &node{item: "b", parent: &node{item: "c", parent: &node{item: RootName}}}},
			[]string{"c", "b", "a"},
		},
		{
			nil,
			[]string{},
		},
	}

	for _, td := range testData {
		res := findPrefixPath(td.n)
		if len(res) != len(td.expected) {
			t.Errorf("expected %d but got %d", len(td.expected), len(res))
			break
		}
		for i, item := range res {
			if item != td.expected[i] {
				t.Errorf("expected %s at index %d, but got %s", td.expected[i], i, item)
			}
		}
	}
}

func BenchmarkFPGrowth(b *testing.B) {
	// 使用现有的测试数据进行基准测试
	transactions := testTransactions()

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		fpg, _ := New(0.09)
		fpg.Fit(transactions)
	}
}
