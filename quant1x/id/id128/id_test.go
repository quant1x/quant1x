package id128

import (
	"os"
	"path/filepath"
	"slices"
	"sync"
	"testing"
)

func TestHLC_RollbackMonotonic(t *testing.T) {
	fakeNow := int64(1000)
	hlc := NewHLC(
		WithClock(func() int64 { return fakeNow }),
		WithLogicalSeed(7),
	)

	prevHLC, prevSeq := hlc.Now()
	fakeNow = 500
	curHLC, curSeq := hlc.Now()

	if curHLC < prevHLC || (curHLC == prevHLC && curSeq <= prevSeq) {
		t.Fatalf("rollback violated monotonicity: prev=(%#x,%d) cur=(%#x,%d)", prevHLC, prevSeq, curHLC, curSeq)
	}
}

func TestHLC_UsesOptionsAtConstruction(t *testing.T) {
	const fakeNow = int64(4321)
	hlc := NewHLC(
		WithClock(func() int64 { return fakeNow }),
		WithLogicalSeed(9),
	)

	if got := hlc.Timestamp(); got != fakeNow {
		t.Fatalf("Timestamp()=%d want %d", got, fakeNow)
	}

	hlcValue, _ := hlc.Now()
	if got := uint16(hlcValue); got != 9 {
		t.Fatalf("logical seed=%d want %d", got, 9)
	}
}

func TestID_FieldDecoding(t *testing.T) {
	const (
		hlcValue = uint64(0x0102030405060708)
		nodeID   = uint32(0x11223344)
		seq      = uint32(0xaabbccdd)
	)

	raw := Uint128{
		Hi: hlcValue,
		Lo: (uint64(nodeID) << 32) | uint64(seq),
	}
	id := ID(raw.Bytes())

	if got := id.HLC(); got != hlcValue {
		t.Fatalf("HLC()=%#x want %#x", got, hlcValue)
	}
	if got := id.NodeID(); got != nodeID {
		t.Fatalf("NodeID()=%#x want %#x", got, nodeID)
	}
	if got := id.Seq(); got != seq {
		t.Fatalf("Seq()=%#x want %#x", got, seq)
	}
}

func TestHLC_PersistentStateAcrossRestart(t *testing.T) {
	stateFile := filepath.Join(t.TempDir(), "hlc.state")
	fakeNow := int64(1000)
	opts := []Option{
		WithClock(func() int64 { return fakeNow }),
		WithLogicalSeed(7),
		WithStateFile(stateFile),
	}

	firstHLC := NewHLC(opts...)
	first := NewGenerator(1, firstHLC).Next()
	// 快速路径为批量缓冲：优雅退出前 Close 刷盘，确保重启恢复最新水位
	if err := firstHLC.Close(); err != nil {
		t.Fatalf("Close() error = %v", err)
	}

	second := NewGenerator(1, NewHLC(opts...)).Next()

	if !first.Lt(second) {
		t.Fatalf("restart state did not advance: first=%#x second=%#x", first, second)
	}
}

func TestHLC_SharedStateFileAcrossInstances(t *testing.T) {
	stateFile := filepath.Join(t.TempDir(), "hlc.state")
	fakeNow := int64(1000)
	opts := []Option{
		WithClock(func() int64 { return fakeNow }),
		WithLogicalSeed(7),
		WithStateFile(stateFile),
		// 多写者活跃共享：必须显式开启严格模式（每次发号读盘取 max）
		WithStateStrict(),
	}

	left := NewGenerator(1, NewHLC(opts...))
	right := NewGenerator(1, NewHLC(opts...))

	first := left.Next()
	second := right.Next()

	if !first.Lt(second) {
		t.Fatalf("shared state file did not serialize progress: first=%#x second=%#x", first, second)
	}
}

func TestStateStore_LoadIgnoresCorruptedTail(t *testing.T) {
	stateFile := filepath.Join(t.TempDir(), "hlc.state")
	store := &fileStateStore{
		path:     stateFile,
		lockPath: stateFile + ".lock",
	}

	want := persistentState{Physical: 1234, Logical: 7, Seq: 99}
	if err := store.appendState(want); err != nil {
		t.Fatalf("appendState() error = %v", err)
	}

	file, err := os.OpenFile(stateFile, os.O_WRONLY|os.O_APPEND, 0o644)
	if err != nil {
		t.Fatalf("OpenFile() error = %v", err)
	}
	defer file.Close()

	if _, err := file.Write([]byte{0xde, 0xad, 0xbe, 0xef}); err != nil {
		t.Fatalf("Write() error = %v", err)
	}

	got, ok, err := store.Load()
	if err != nil {
		t.Fatalf("Load() error = %v", err)
	}
	if !ok {
		t.Fatal("Load() ok = false, want true")
	}
	if got != want {
		t.Fatalf("Load() = %#v, want %#v", got, want)
	}
}

func TestGenerator_Concurrent(t *testing.T) {
	hlc := NewHLC()
	gen := NewGenerator(1, hlc)

	const N = 200000
	ids := make([]Uint128, N)

	var wg sync.WaitGroup
	for i := 0; i < N; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			ids[i] = gen.Next()
		}(i)
	}
	wg.Wait()

	seen := make(map[Uint128]struct{}, N)
	for i, id := range ids {
		if _, ok := seen[id]; ok {
			t.Fatalf("duplicate id at %d: %#x", i, id)
		}
		seen[id] = struct{}{}
	}

	slices.SortFunc(ids, func(a, b Uint128) int {
		return a.Compare(b)
	})

	for i := 1; i < N; i++ {
		if !ids[i-1].Lt(ids[i]) {
			t.Fatalf("concurrent violation at %d\nprev=%#x\ncur =%#x",
				i, ids[i-1], ids[i])
		}
	}
}

func BenchmarkGeneratorNext(b *testing.B) {
	gen := NewGenerator(1, NewHLC())

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = gen.Next()
	}
}

func BenchmarkGeneratorNextWithStateFile(b *testing.B) {
	stateFile := filepath.Join(b.TempDir(), "hlc.state")
	gen := NewGenerator(1, NewHLC(WithStateFile(stateFile)))

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = gen.Next()
	}
}

func BenchmarkGeneratorNextWithStateFileSyncEvery256(b *testing.B) {
	stateFile := filepath.Join(b.TempDir(), "hlc.state")
	gen := NewGenerator(1, NewHLC(
		WithStateFile(stateFile),
		WithStateSyncEvery(256),
	))

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = gen.Next()
	}
}

// 严格模式：每次发号读盘取 max（多写者活跃共享唯一性）
func BenchmarkGeneratorNextWithStateFileStrict(b *testing.B) {
	stateFile := filepath.Join(b.TempDir(), "hlc.state")
	gen := NewGenerator(1, NewHLC(
		WithStateFile(stateFile),
		WithStateStrict(),
	))

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = gen.Next()
	}
}

// 快速路径批量缓冲语义：未攒满不落盘、攒满自动落盘、Close 刷盘后可重启恢复。
func TestHLC_BatchBufferFlushSemantics(t *testing.T) {
	const fakeNow = int64(1000)
	stateFile := filepath.Join(t.TempDir(), "hlc.state")
	hlc := NewHLC(
		WithClock(func() int64 { return fakeNow }),
		WithStateFile(stateFile),
		WithStateSyncEvery(10),
	)

	// 未攒满：状态文件不应落盘
	for i := 0; i < 5; i++ {
		hlc.Now()
	}
	if _, err := os.Stat(stateFile); !os.IsNotExist(err) {
		t.Fatalf("state file should not exist before batch flush, stat err = %v", err)
	}

	// 攒满 syncEvery：自动落盘一次
	for i := 0; i < 5; i++ {
		hlc.Now()
	}
	data, err := os.ReadFile(stateFile)
	if err != nil {
		t.Fatalf("state file should exist after batch flush: %v", err)
	}
	if len(data) < 10*persistentStateRecordSize {
		t.Fatalf("state file should contain >= 10 records, got %d bytes", len(data))
	}

	// Close 刷盘剩余缓冲后，重启恢复最新水位
	if err := hlc.Close(); err != nil {
		t.Fatalf("Close() error = %v", err)
	}
	prev := NewGenerator(1, hlc).Next()
	if err := hlc.Close(); err != nil {
		t.Fatalf("Close() (idempotent) error = %v", err)
	}
	next := NewGenerator(1, NewHLC(
		WithClock(func() int64 { return fakeNow }),
		WithStateFile(stateFile),
		WithStateSyncEvery(10),
	)).Next()
	if !prev.Lt(next) {
		t.Fatalf("restart state did not advance: prev=%#x next=%#x", prev, next)
	}
}
