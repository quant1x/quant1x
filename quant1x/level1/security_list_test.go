package level1

import (
	"sync"
	"testing"
)

func TestSecurityListReal(t *testing.T) {
	if testing.Short() {
		t.Skip("skipping real security list test in short mode")
	}

	poolOnce = sync.Once{}
	poolInstance = nil
	poolErr = nil

	dir := t.TempDir()
	t.Setenv("QUANT1X_HOME", dir)

	conn, release, err := GetStdConnection()
	if err != nil {
		t.Fatalf("GetStdConnection() returned error: %v", err)
	}
	defer release()

	markets := []struct {
		id   int
		name string
	}{
		{marketShangHai, "sh"},
		{marketShenZhen, "sz"},
		{marketBeiJing, "bj"},
	}

	for _, m := range markets {
		req := NewSecurityListRequest(m.id, 0, SecurityListPreRequestMax)
		resp := &SecurityListResponse{}
		if err := Process(conn, req, resp); err != nil {
			t.Fatalf("Process(%s) failed: %v", m.name, err)
		}
		if resp.Count == 0 {
			t.Fatalf("expected non-zero count for market %s", m.name)
		}
		if len(resp.List) != int(resp.Count) {
			t.Fatalf("list length mismatch for market %s: count=%d len=%d", m.name, resp.Count, len(resp.List))
		}
		first := resp.List[0]
		if first.Code == "" {
			t.Fatalf("empty code for market %s", m.name)
		}
		if first.Name == "" {
			t.Fatalf("empty name for market %s", m.name)
		}
	}
}
