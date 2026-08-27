package runtime

import (
	"errors"
	"math/bits"
	"runtime"
	"sync/atomic"
	"time"
)

var (
	ErrQueueFull   = errors.New("queue is full")
	ErrQueueEmpty  = errors.New("queue is empty")
	ErrInvalidSize = errors.New("size must be positive")
	ErrClosed      = errors.New("ring buffer closed")
)

// alignedCounter keeps producer and consumer positions on separate cache lines.
type alignedCounter struct {
	value uint64
	_pad  [56]byte
}

// RingBuffer is a bounded lock-free MPMC queue based on Vyukov's algorithm.
type RingBuffer[T any] struct {
	slots       []T
	sequences   []alignedCounter
	mask        uint64
	producerPos alignedCounter
	consumerPos alignedCounter
	closed      uint32
}

// New creates a queue. Non-zero capacities are rounded up to a power of two,
// matching the C++ implementation.
func New[T any](size uint32) (*RingBuffer[T], error) {
	if size == 0 {
		return nil, ErrInvalidSize
	}
	capacity := uint64(size)
	if capacity&(capacity-1) != 0 {
		capacity = 1 << bits.Len32(size)
	}
	rb := &RingBuffer[T]{
		slots:     make([]T, int(capacity)),
		sequences: make([]alignedCounter, int(capacity)),
		mask:      capacity - 1,
	}
	for i := range rb.sequences {
		atomic.StoreUint64(&rb.sequences[i].value, uint64(i))
	}
	return rb, nil
}

func queueBackoff(retries *uint32) {
	switch {
	case *retries < 4:
		// 早期竞争时保持当前线程运行，近似 CPU pause，避免调度器介入。
		for i := uint32(0); i < 1<<*retries; i++ {
		}
	case *retries < 8:
		runtime.Gosched()
	default:
		delay := *retries - 8
		if delay > 4 {
			delay = 4
		}
		time.Sleep(time.Microsecond << delay)
	}
	if *retries < 16 {
		(*retries)++
	}
}

func queuePause(retries *uint32) {
	spins := uint32(1) << min(*retries, 4)
	for i := uint32(0); i < spins; i++ {
	}
	if *retries < 16 {
		(*retries)++
	}
}

// TryWrite performs a non-blocking enqueue, matching C++ try_push.
func (rb *RingBuffer[T]) TryWrite(value T) error {
	if atomic.LoadUint32(&rb.closed) != 0 {
		return ErrClosed
	}
	var retries uint32
	for {
		pos := atomic.LoadUint64(&rb.producerPos.value)
		index := pos & rb.mask
		seq := atomic.LoadUint64(&rb.sequences[index].value)
		dif := int64(seq - pos)
		if dif == 0 {
			if atomic.CompareAndSwapUint64(&rb.producerPos.value, pos, pos+1) {
				rb.slots[index] = value
				atomic.StoreUint64(&rb.sequences[index].value, pos+1)
				return nil
			}
			queuePause(&retries)
			continue
		}
		if dif < 0 {
			return ErrQueueFull
		}
		queuePause(&retries)
	}
}

// TryPush is the C++-style name for TryWrite.
func (rb *RingBuffer[T]) TryPush(value T) error { return rb.TryWrite(value) }

// TryRead performs a non-blocking dequeue, matching C++ try_pop.
func (rb *RingBuffer[T]) TryRead() (T, error) {
	var zero T
	var retries uint32
	for {
		pos := atomic.LoadUint64(&rb.consumerPos.value)
		index := pos & rb.mask
		seq := atomic.LoadUint64(&rb.sequences[index].value)
		dif := int64(seq - (pos + 1))
		if dif == 0 {
			if atomic.CompareAndSwapUint64(&rb.consumerPos.value, pos, pos+1) {
				value := rb.slots[index]
				atomic.StoreUint64(&rb.sequences[index].value, pos+rb.mask+1)
				return value, nil
			}
			queuePause(&retries)
			continue
		}
		if dif < 0 {
			if atomic.LoadUint32(&rb.closed) != 0 {
				return zero, ErrClosed
			}
			return zero, ErrQueueEmpty
		}
		queuePause(&retries)
	}
}

// TryPop is the C++-style name for TryRead.
func (rb *RingBuffer[T]) TryPop() (T, error) { return rb.TryRead() }

// Write is a blocking compatibility wrapper around TryWrite.
func (rb *RingBuffer[T]) Write(value T) error {
	var retries uint32
	for {
		if err := rb.TryWrite(value); err != ErrQueueFull {
			return err
		}
		queueBackoff(&retries)
	}
}

// Push is the blocking compatibility name for Write.
func (rb *RingBuffer[T]) Push(value T) error { return rb.Write(value) }

// Read is a blocking compatibility wrapper around TryRead.
func (rb *RingBuffer[T]) Read() (T, error) {
	var retries uint32
	for {
		value, err := rb.TryRead()
		if err != ErrQueueEmpty {
			return value, err
		}
		queueBackoff(&retries)
	}
}

// Pop is the blocking compatibility name for Read.
func (rb *RingBuffer[T]) Pop() (T, error) { return rb.Read() }

// Len 返回缓冲区中当前元素的数量
func (rb *RingBuffer[T]) Len() int {
	prod := atomic.LoadUint64(&rb.producerPos.value)
	cons := atomic.LoadUint64(&rb.consumerPos.value)
	return int(prod - cons)
}

// Cap 返回缓冲区的容量
func (rb *RingBuffer[T]) Cap() int {
	return len(rb.slots)
}

// IsEmpty 当缓冲区为空时返回 true
func (rb *RingBuffer[T]) IsEmpty() bool {
	return rb.Len() == 0
}

// IsFull 当缓冲区已满时返回 true
func (rb *RingBuffer[T]) IsFull() bool {
	return rb.Len() == rb.Cap()
}

// Close 关闭环形缓冲区(设置关闭标志), 写入方将被拒绝写入
func (rb *RingBuffer[T]) Close() {
	atomic.StoreUint32(&rb.closed, 1)
}

func (rb *RingBuffer[T]) IsClosed() bool {
	return atomic.LoadUint32(&rb.closed) != 0
}

// WaitForClose 在关闭后阻塞直到所有数据被消费完成
func (rb *RingBuffer[T]) WaitForClose() {
	var retries uint32
	for !rb.IsEmpty() {
		queueBackoff(&retries)
	}
}
