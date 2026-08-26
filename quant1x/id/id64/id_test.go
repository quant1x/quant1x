package id64

import (
	"os"
	"path/filepath"
	"sync"
	"testing"
)

func TestHLC_RollbackMonotonic(t *testing.T) {
	now := int64(1000)
	hlc := NewHLC(WithClock(func() int64 { return now }), WithSeqSeed(9))
	p1, s1 := hlc.Now()

	now = 500 // 时钟回拨
	p2, s2 := hlc.Now()
	if p2 < p1 || (p2 == p1 && s2 <= s1) {
		t.Fatalf("回拨后未保持单调: (%d,%d) -> (%d,%d)", p1, s1, p2, s2)
	}
}

func TestHLC_UsesOptionsAtConstruction(t *testing.T) {
	now := int64(4321)
	hlc := NewHLC(WithClock(func() int64 { return now }), WithSeqSeed(9))
	if hlc.Timestamp() != 4321 {
		t.Fatalf("Timestamp() = %d, want 4321", hlc.Timestamp())
	}
	if hlc.seq != 9 {
		t.Fatalf("initial seq = %d, want 9", hlc.seq)
	}
}

func TestHLC_NodeCountDerivation(t *testing.T) {
	cases := []struct {
		count      uint32
		workerBits uint8
		seqBits    uint8
	}{
		{1024, 11, 11},
		{5000, 13, 9},
		{3, 2, 20},
		{131072, 18, 4},
	}
	for _, tc := range cases {
		hlc := NewHLC(WithNodeCount(tc.count))
		if hlc.SeqBits() != tc.seqBits {
			t.Fatalf("count=%d: SeqBits()=%d, want %d", tc.count, hlc.SeqBits(), tc.seqBits)
		}
		gen := NewGenerator(0, hlc)
		if gen.WorkerBits() != tc.workerBits {
			t.Fatalf("count=%d: WorkerBits()=%d, want %d", tc.count, gen.WorkerBits(), tc.workerBits)
		}
	}
}

func TestHLC_NodeCountTooLarge(t *testing.T) {
	defer func() {
		if r := recover(); r == nil {
			t.Fatal("节点数超限应 panic")
		}
	}()
	NewHLC(WithNodeCount(262144)) // seqBits = 3 < 4
}

func TestID_FieldDecoding(t *testing.T) {
	elapsed := int64(0x123456789A)
	workerBits := uint8(11)
	seqBits := uint8(11)
	nodeID := uint32(0x1F)
	seq := uint32(0x2A)

	id := ID(uint64(elapsed)<<payloadBits | uint64(nodeID)<<seqBits | uint64(seq))
	if got := id.Physical(); got != elapsed {
		t.Fatalf("Physical() = %d, want %d", got, elapsed)
	}
	if got := id.NodeID(workerBits); got != nodeID {
		t.Fatalf("NodeID() = %d, want %d", got, nodeID)
	}
	if got := id.Seq(workerBits); got != seq {
		t.Fatalf("Seq() = %d, want %d", got, seq)
	}
	if b := id.Bytes(); FromBytes(b) != id {
		t.Fatalf("Bytes/FromBytes 往返不一致: %d", id)
	}
}

func TestID_StringRoundTrip(t *testing.T) {
	hlc := NewHLC(WithClock(func() int64 { return 1767225600123 }))
	gen := NewGenerator(7, hlc)
	id := gen.Next()
	if id.String() == "" {
		t.Fatal("String() 为空")
	}
}

func TestHLC_PersistentStateAcrossRestart(t *testing.T) {
	now := int64(1000)
	path := filepath.Join(t.TempDir(), "state.bin")
	hlc1 := NewHLC(WithClock(func() int64 { return now }), WithStateFile(path), WithSeqSeed(9))
	p1, s1 := hlc1.Now()
	// 快速路径为批量缓冲：优雅退出前 Close 刷盘，确保重启恢复最新水位
	if err := hlc1.Close(); err != nil {
		t.Fatal(err)
	}

	hlc2 := NewHLC(WithClock(func() int64 { return now }), WithStateFile(path), WithSeqSeed(9))
	p2, s2 := hlc2.Now()
	if p2 < p1 || (p2 == p1 && s2 <= s1) {
		t.Fatalf("重启后未保持单调: (%d,%d) -> (%d,%d)", p1, s1, p2, s2)
	}
}

func TestHLC_SharedStateFile(t *testing.T) {
	now := int64(1000)
	path := filepath.Join(t.TempDir(), "state.bin")
	// 多写者活跃共享：必须显式开启严格模式（每次发号读盘取 max）
	hlc1 := NewHLC(WithClock(func() int64 { return now }), WithStateFile(path), WithSeqSeed(9), WithStateStrict())
	defer hlc1.Close()
	hlc2 := NewHLC(WithClock(func() int64 { return now }), WithStateFile(path), WithSeqSeed(9), WithStateStrict())
	defer hlc2.Close()

	prevP, prevS := hlc1.Now()
	for i := 0; i < 1000; i++ {
		var p int64
		var s uint32
		if i%2 == 0 {
			p, s = hlc2.Now()
		} else {
			p, s = hlc1.Now()
		}
		if p < prevP || (p == prevP && s <= prevS) {
			t.Fatalf("共享状态文件下未保持单调: (%d,%d) -> (%d,%d)", prevP, prevS, p, s)
		}
		prevP, prevS = p, s
	}
}

func TestHLC_CorruptedTailTruncation(t *testing.T) {
	now := int64(1000)
	path := filepath.Join(t.TempDir(), "state.bin")
	hlc := NewHLC(WithClock(func() int64 { return now }), WithStateFile(path))
	hlc.Now()
	// 快速路径为批量缓冲：先 Close 落盘，保证文件存在且含有效记录
	if err := hlc.Close(); err != nil {
		t.Fatal(err)
	}

	f, err := os.OpenFile(path, os.O_APPEND|os.O_WRONLY, 0o644)
	if err != nil {
		t.Fatal(err)
	}
	// 18 字节、CRC 不匹配的坏记录，触发截断
	if _, err := f.Write([]byte("ABCDEFGHIJKLMNOPQR")); err != nil {
		t.Fatal(err)
	}
	f.Close()

	hlc2 := NewHLC(WithClock(func() int64 { return now }), WithStateFile(path))
	hlc2.Now() // 坏损尾部应被截断并继续工作（不 panic）
}

func TestGenerator_NodeIDOutOfRange(t *testing.T) {
	hlc := NewHLC(WithNodeCount(3)) // workerBits=2，nodeID 上限 3
	defer func() {
		if r := recover(); r == nil {
			t.Fatal("nodeID 越界应 panic")
		}
	}()
	NewGenerator(4, hlc)
}

func TestGenerator_Monotonic(t *testing.T) {
	hlc := NewHLC()
	gen := NewGenerator(1, hlc)
	prev := gen.Next()
	for i := 0; i < 1000; i++ {
		next := gen.Next()
		if next <= prev {
			t.Fatalf("ID 未保持单调: %d -> %d", prev, next)
		}
		prev = next
	}
}

func TestGenerator_ConcurrentUnique(t *testing.T) {
	hlc := NewHLC()
	gen := NewGenerator(1, hlc)
	const n = 10000
	ids := make(chan ID, n)
	var wg sync.WaitGroup
	for i := 0; i < 8; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for j := 0; j < n/8; j++ {
				ids <- gen.Next()
			}
		}()
	}
	wg.Wait()
	close(ids)

	seen := make(map[ID]bool, n)
	for id := range ids {
		if seen[id] {
			t.Fatalf("重复 ID: %d", id)
		}
		seen[id] = true
	}
}

func BenchmarkNext(b *testing.B) {
	hlc := NewHLC()
	gen := NewGenerator(1, hlc)
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_ = gen.Next()
	}
}

func BenchmarkNextStateFile(b *testing.B) {
	hlc := NewHLC(WithStateFile(filepath.Join(b.TempDir(), "state.bin")))
	gen := NewGenerator(1, hlc)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = gen.Next()
	}
}

// 严格模式：每次发号读盘取 max（多写者活跃共享唯一性）
func BenchmarkNextStateFileStrict(b *testing.B) {
	hlc := NewHLC(WithStateFile(filepath.Join(b.TempDir(), "state.bin")), WithStateStrict())
	defer hlc.Close()
	gen := NewGenerator(1, hlc)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = gen.Next()
	}
}

// 快速路径批量缓冲语义：未攒满不落盘、攒满自动落盘、Close 刷盘后可重启恢复。
func TestHLC_BatchBufferFlushSemantics(t *testing.T) {
	now := int64(1000)
	path := filepath.Join(t.TempDir(), "state.bin")
	hlc := NewHLC(
		WithClock(func() int64 { return now }),
		WithStateFile(path),
		WithStateSyncEvery(10),
	)

	// 未攒满：状态文件不应落盘
	for i := 0; i < 5; i++ {
		hlc.Now()
	}
	if _, err := os.Stat(path); !os.IsNotExist(err) {
		t.Fatalf("state file should not exist before batch flush, stat err = %v", err)
	}

	// 攒满 syncEvery：自动落盘一次
	for i := 0; i < 5; i++ {
		hlc.Now()
	}
	data, err := os.ReadFile(path)
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
	prevP, prevS := hlc.Now()
	if err := hlc.Close(); err != nil {
		t.Fatalf("Close() (idempotent) error = %v", err)
	}
	hlc2 := NewHLC(
		WithClock(func() int64 { return now }),
		WithStateFile(path),
		WithStateSyncEvery(10),
	)
	p2, s2 := hlc2.Now()
	if p2 < prevP || (p2 == prevP && s2 <= prevS) {
		t.Fatalf("restart state did not advance: prev=(%d,%d) next=(%d,%d)", prevP, prevS, p2, s2)
	}
}
