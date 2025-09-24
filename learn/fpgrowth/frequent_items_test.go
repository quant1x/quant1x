package fpgrowth

import "testing"

func TestFrequentItemsGetSorted(t *testing.T) {
	transactions := testTransactions()

	fpg, err := New(0.6) // expect items with support >= 3 (0.6 * 5 = 3)
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

	fi := fpg.frequentItems.getSorted(fpg.MinSupport)
	expected := []string{"f", "c", "p", "m", "b", "a"}
	if len(fi) != len(expected) {
		t.Errorf("expected %d items, but got %d", len(expected), len(fi))
		return
	}

	for i, item := range expected {
		if fi[i] != item {
			t.Errorf("expected item %s at index %d, but got %s", item, i, fi[i])
			return
		}
	}
}
