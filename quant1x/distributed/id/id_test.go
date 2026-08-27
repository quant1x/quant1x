package id

import (
	"context"
	"errors"
	"path/filepath"
	"sync"
	"testing"
	"time"

	"github.com/quant1x/quant1x/quant1x/runtime"
)

var (
	benchmarkIDBytes  [8]byte
	benchmarkIDString string
	benchmarkSinkID   ID
)

func TestDefaultGenerator(t *testing.T) {
	hlc := NewHLC(WithClock(func() int64 { return EpochMs }))
	generator := NewGenerator(1, hlc)
	first := generator.Next()
	second := generator.Next()
	if second <= first {
		t.Fatalf("IDs are not increasing: %d -> %d", first, second)
	}
	if first.NodeID(generator.WorkerBits()) != 1 {
		t.Fatalf("NodeID() = %d, want 1", first.NodeID(generator.WorkerBits()))
	}
}

func TestHLC_RollbackMonotonic(t *testing.T) {
	now := int64(EpochMs)
	hlc := NewHLC(WithClock(func() int64 { return now }), WithSeqSeed(9))
	first, firstSeq := hlc.Now()
	now--
	second, secondSeq := hlc.Now()
	if second < first || (second == first && secondSeq <= firstSeq) {
		t.Fatalf("rollback broke monotonicity: (%d,%d) -> (%d,%d)", first, firstSeq, second, secondSeq)
	}
}

func TestNodeCountDerivation(t *testing.T) {
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
	for _, testCase := range cases {
		hlc := NewHLC(WithNodeCount(testCase.count))
		generator := NewGenerator(0, hlc)
		if generator.WorkerBits() != testCase.workerBits || hlc.SeqBits() != testCase.seqBits {
			t.Fatalf("count=%d: workerBits=%d seqBits=%d", testCase.count, generator.WorkerBits(), hlc.SeqBits())
		}
	}
}

func TestIDFieldsAndEncoding(t *testing.T) {
	hlc := NewHLC(WithClock(func() int64 { return EpochMs + 123 }))
	generator := NewGenerator(7, hlc)
	id := generator.Next()
	if id.Physical() != 123 || id.NodeID(generator.WorkerBits()) != 7 {
		t.Fatalf("decoded fields are incorrect: physical=%d node=%d", id.Physical(), id.NodeID(generator.WorkerBits()))
	}
	if FromBytes(id.Bytes()) != id || len(id.String()) != 11 {
		t.Fatalf("ID encoding round trip failed: %q", id.String())
	}
}

func TestConcurrentUnique(t *testing.T) {
	hlc := NewHLC(WithClock(func() int64 { return EpochMs }))
	generator := NewGenerator(1, hlc)
	const count = 10000
	ids := make(chan ID, count)
	var waitGroup sync.WaitGroup
	for worker := 0; worker < 8; worker++ {
		waitGroup.Add(1)
		go func() {
			defer waitGroup.Done()
			for index := 0; index < count/8; index++ {
				ids <- generator.Next()
			}
		}()
	}
	waitGroup.Wait()
	close(ids)
	seen := make(map[ID]struct{}, count)
	for id := range ids {
		if _, exists := seen[id]; exists {
			t.Fatalf("duplicate ID: %d", id)
		}
		seen[id] = struct{}{}
	}
}

func TestStateFileAcrossRestart(t *testing.T) {
	path := filepath.Join(t.TempDir(), "state.bin")
	now := int64(EpochMs)
	hlc1 := NewHLC(WithClock(func() int64 { return now }), WithStateFile(path), WithSeqSeed(9))
	first, firstSeq := hlc1.Now()
	if err := hlc1.Close(); err != nil {
		t.Fatal(err)
	}
	hlc2 := NewHLC(WithClock(func() int64 { return now }), WithStateFile(path), WithSeqSeed(9))
	defer hlc2.Close()
	second, secondSeq := hlc2.Now()
	if second < first || (second == first && secondSeq <= firstSeq) {
		t.Fatalf("restart broke monotonicity: (%d,%d) -> (%d,%d)", first, firstSeq, second, secondSeq)
	}
}

func TestQueueUsesVyukovRingBuffer(t *testing.T) {
	queue, err := NewQueue(3)
	if err != nil {
		t.Fatal(err)
	}
	if queue.Cap() != 4 {
		t.Fatalf("queue capacity = %d, want 4", queue.Cap())
	}
	value := ID(42)
	if err := queue.TryPush(value); err != nil {
		t.Fatal(err)
	}
	got, err := queue.TryPop()
	if err != nil {
		t.Fatal(err)
	}
	if got != value {
		t.Fatalf("TryPop() = %d, want %d", got, value)
	}
	queue.Close()
	if _, err := queue.TryPop(); err != runtime.ErrClosed {
		t.Fatalf("closed TryPop() error = %v, want %v", err, runtime.ErrClosed)
	}
}

// Serve 生产 + TryPop 消费：取出的 ID 必须全局严格递增且唯一。
func TestGeneratorServeFeedsQueue(t *testing.T) {
	hlc := NewHLC(WithClock(func() int64 { return EpochMs }))
	generator := NewGenerator(1, hlc)

	queue, err := NewQueue(1024)
	if err != nil {
		t.Fatal(err)
	}
	ctx, cancel := context.WithCancel(context.Background())
	serveDone := make(chan error, 1)
	go func() { serveDone <- generator.Serve(ctx, queue) }()

	const count = 4096
	var previous ID
	for index := 0; index < count; index++ {
		id, err := queue.TryPop()
		if errors.Is(err, runtime.ErrQueueEmpty) {
			index--
			continue
		}
		if err != nil {
			t.Fatalf("TryPop() at %d: %v", index, err)
		}
		if index > 0 && id <= previous {
			t.Fatalf("IDs are not increasing: %d -> %d", previous, id)
		}
		previous = id
	}

	cancel()
	if err := <-serveDone; !errors.Is(err, context.Canceled) {
		t.Fatalf("Serve() error = %v, want context.Canceled", err)
	}
}

// 关闭队列后 Serve 必须立即停止且不再发号。
func TestGeneratorServeStopsOnClosedQueue(t *testing.T) {
	hlc := NewHLC(WithClock(func() int64 { return EpochMs }))
	generator := NewGenerator(1, hlc)

	queue, err := NewQueue(4)
	if err != nil {
		t.Fatal(err)
	}
	queue.Close()

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	if err := generator.Serve(ctx, queue); err != nil {
		t.Fatalf("Serve() on closed queue = %v, want nil", err)
	}
}

// 取消后存量 ID 允许继续消费（graceful drain），耗尽后返回 ErrClosed。
func TestGeneratorServeDrainAfterCancel(t *testing.T) {
	hlc := NewHLC(WithClock(func() int64 { return EpochMs }))
	generator := NewGenerator(1, hlc)

	queue, err := NewQueue(8)
	if err != nil {
		t.Fatal(err)
	}
	ctx, cancel := context.WithCancel(context.Background())
	serveDone := make(chan error, 1)
	go func() { serveDone <- generator.Serve(ctx, queue) }()

	// 等待生产者至少填入一个 ID
	for queue.Len() == 0 {
		time.Sleep(time.Millisecond)
	}
	cancel()
	if err := <-serveDone; !errors.Is(err, context.Canceled) {
		t.Fatalf("Serve() error = %v, want context.Canceled", err)
	}

	queue.Close() // 进入只读排空
	last, err := queue.TryPop()
	for err == nil {
		if last <= 0 {
			t.Fatalf("invalid drained ID %d", last)
		}
		last, err = queue.TryPop()
	}
	if !errors.Is(err, runtime.ErrClosed) {
		t.Fatalf("final TryPop() error = %v, want %v", err, runtime.ErrClosed)
	}
}

func BenchmarkGeneratorNext(b *testing.B) {
	hlc := NewHLC(WithClock(func() int64 { return EpochMs }))
	generator := NewGenerator(1, hlc)
	b.ReportAllocs()
	b.ResetTimer()
	for index := 0; index < b.N; index++ {
		_ = generator.Next()
	}
}

func benchmarkGeneratorNext(b *testing.B, options ...Option) {
	hlc := NewHLC(options...)
	defer hlc.Close()
	generator := NewGenerator(1, hlc)
	b.ReportAllocs()
	b.ResetTimer()
	for index := 0; index < b.N; index++ {
		_ = generator.Next()
	}
}

func BenchmarkGeneratorNextDefault(b *testing.B) {
	benchmarkGeneratorNext(b)
}

func BenchmarkGeneratorNextWithClock(b *testing.B) {
	benchmarkGeneratorNext(b, WithClock(func() int64 { return EpochMs }))
}

func BenchmarkGeneratorNextWithSeqSeed(b *testing.B) {
	benchmarkGeneratorNext(b, WithSeqSeed(9))
}

func BenchmarkGeneratorNextWithNodeCount(b *testing.B) {
	benchmarkGeneratorNext(b, WithNodeCount(5000))
}

func BenchmarkGeneratorNextWithSeqBits(b *testing.B) {
	benchmarkGeneratorNext(b, WithSeqBits(9))
}

func BenchmarkGeneratorNextStateFile(b *testing.B) {
	benchmarkGeneratorNext(b, WithStateFile(filepath.Join(b.TempDir(), "state.bin")))
}

func BenchmarkGeneratorNextStateFileSyncEvery(b *testing.B) {
	benchmarkGeneratorNext(b,
		WithStateFile(filepath.Join(b.TempDir(), "state.bin")),
		WithStateSyncEvery(256),
	)
}

func BenchmarkGeneratorNextStateFileStrict(b *testing.B) {
	benchmarkGeneratorNext(b,
		WithStateFile(filepath.Join(b.TempDir(), "state.bin")),
		WithStateStrict(),
	)
}

func BenchmarkHLCNow(b *testing.B) {
	hlc := NewHLC(WithClock(func() int64 { return EpochMs }))
	b.ReportAllocs()
	b.ResetTimer()
	for index := 0; index < b.N; index++ {
		_, _ = hlc.Now()
	}
}

func BenchmarkGeneratorNextParallel(b *testing.B) {
	hlc := NewHLC(WithClock(func() int64 { return EpochMs }))
	generator := NewGenerator(1, hlc)
	b.ReportAllocs()
	b.RunParallel(func(parallel *testing.PB) {
		for parallel.Next() {
			_ = generator.Next()
		}
	})
}

func BenchmarkIDBytes(b *testing.B) {
	value := ID(0x1234567890)
	b.ReportAllocs()
	b.ResetTimer()
	for index := 0; index < b.N; index++ {
		benchmarkIDBytes = value.Bytes()
	}
}

func BenchmarkIDString(b *testing.B) {
	value := ID(0x1234567890)
	b.ReportAllocs()
	b.ResetTimer()
	for index := 0; index < b.N; index++ {
		benchmarkIDString = value.String()
	}
}

func BenchmarkQueuePushPop(b *testing.B) {
	queue, err := NewQueue(1024)
	if err != nil {
		b.Fatal(err)
	}
	value := ID(42)
	b.ReportAllocs()
	b.ResetTimer()
	for index := 0; index < b.N; index++ {
		if err := queue.TryPush(value); err != nil {
			b.Fatal(err)
		}
		if _, err := queue.TryPop(); err != nil {
			b.Fatal(err)
		}
	}
}

func BenchmarkQueueTryPushPop(b *testing.B) {
	queue, err := NewQueue(1024)
	if err != nil {
		b.Fatal(err)
	}
	value := ID(42)
	b.ReportAllocs()
	b.ResetTimer()
	for index := 0; index < b.N; index++ {
		if err := queue.TryPush(value); err != nil {
			b.Fatal(err)
		}
		if _, err := queue.TryPop(); err != nil {
			b.Fatal(err)
		}
	}
}

func BenchmarkQueueMixedParallel(b *testing.B) {
	queue, err := NewQueue(65536)
	if err != nil {
		b.Fatal(err)
	}
	b.ReportAllocs()
	b.RunParallel(func(parallel *testing.PB) {
		value := ID(42)
		for parallel.Next() {
			if queue.TryPush(value) == nil {
				_, _ = queue.TryPop()
			}
		}
	})
}

// Serve 生产 + 单消费者 TryPop 取号（含真实生产者竞争）。
func BenchmarkGeneratorServeTryPop(b *testing.B) {
	hlc := NewHLC(WithClock(func() int64 { return EpochMs }))
	generator := NewGenerator(1, hlc)
	queue, err := NewQueue(65536)
	if err != nil {
		b.Fatal(err)
	}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	go func() { _ = generator.Serve(ctx, queue) }()

	var sink ID
	b.ReportAllocs()
	b.ResetTimer()
	for index := 0; index < b.N; index++ {
		for {
			value, err := queue.TryPop()
			if err == nil {
				sink = value
				break
			}
			if errors.Is(err, runtime.ErrQueueEmpty) {
				continue // 生产者尚未跟上，忙等
			}
			b.Fatal(err)
		}
	}
	benchmarkSinkID = sink
}
