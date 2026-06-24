package runtime

import (
	"errors"
	"runtime"
	"sync"
	"sync/atomic"
	"unsafe"
)

var (
	ErrQueueFull   = errors.New("queue is full")
	ErrInvalidSize = errors.New("size must be power of two")
	ErrClosed      = errors.New("ring buffer closed")
)

// Slot 表示环形缓冲区中的单个槽位
type Slot[T any] struct {
	data unsafe.Pointer // 数据存储
	flag uint32         // 状态标志 (0: empty, 1: writing, 2: readable)
}

// RingBuffer 表示 MPMC 环形缓冲区
type RingBuffer[T any] struct {
	slots       []Slot[T] // 使用槽位数组存储数据
	size        uint32
	mask        uint32
	producerPos uint32    // 全局生产者位置
	consumerPos uint32    // 全局消费者位置
	closed      uint32    // 关闭标记
	pool        sync.Pool // 对象池, 用于复用 T 的包装对象
}

// New 创建并返回一个新的 MPMC 环形缓冲区
func New[T any](size uint32) (*RingBuffer[T], error) {
	if size == 0 || (size&(size-1)) != 0 {
		return nil, ErrInvalidSize
	}

	rb := &RingBuffer[T]{
		slots: make([]Slot[T], size),
		size:  size,
		mask:  size - 1,
		pool: sync.Pool{
			New: func() any { return new(T) },
		},
	}

	for i := range rb.slots {
		atomic.StoreUint32(&rb.slots[i].flag, 0) // 初始化为empty状态
	}

	return rb, nil
}

// spinWait 自旋等待, 使用指数退避以减少忙等带来的开销
func spinWait(retries *int32) {
	r := atomic.AddInt32(retries, 1)
	switch {
	case r < 4:
		for i := 0; i < 1<<(r*2); i++ {
		}
	case r < 8:
		runtime.Gosched()
	default:
		runtime.Gosched()
	}
}

// Write 由生产者向环形缓冲区写入数据.
//
// 为确保写入的数据指针在堆上有效, 该实现将值装箱(在对象池中获取对象并写入),
// 避免依赖编译器的逃逸分析带来的不确定性.
func (rb *RingBuffer[T]) Write(value T) error {
	if atomic.LoadUint32(&rb.closed) == 1 {
		return errors.New("queue closed")
	}

	var currentProd, minCons uint32
	var retries int32 = 0 // ✅ 引入重试计数
	for {
		currentProd = atomic.LoadUint32(&rb.producerPos)
		minCons = atomic.LoadUint32(&rb.consumerPos)

		if currentProd-minCons >= rb.size {
			spinWait(&retries)
			continue
		}

		index := currentProd & rb.mask
		slot := &rb.slots[index]

		// 尝试获取写权限
		if atomic.LoadUint32(&slot.flag) != 0 {
			spinWait(&retries)
			continue
		}

		// CAS更新槽位状态为writing
		if !atomic.CompareAndSwapUint32(&slot.flag, 0, 1) {
			spinWait(&retries)
			continue
		}

		// 写入数据: 使用对象池复用, 避免频繁分配
		boxed := rb.pool.Get().(*T)
		*boxed = value
		atomic.StorePointer(&slot.data, unsafe.Pointer(boxed))
		atomic.StoreUint32(&slot.flag, 2)

		// 更新全局生产者位置
		if atomic.CompareAndSwapUint32(&rb.producerPos, currentProd, currentProd+1) {
			return nil
		}

		// 如果更新失败, 回滚槽位状态
		atomic.StoreUint32(&slot.flag, 0)
		spinWait(&retries)
	}
}

// Read 由消费者从环形缓冲区读取数据
func (rb *RingBuffer[T]) Read() (T, error) {
	var zero T
	var retries int32 = 0 // ✅ 引入重试计数
	for {
		currentCons := atomic.LoadUint32(&rb.consumerPos)
		currentProd := atomic.LoadUint32(&rb.producerPos)

		// 检查队列是否关闭且没有更多数据
		if atomic.LoadUint32(&rb.closed) == 1 && currentCons >= currentProd {
			return zero, ErrClosed
		}

		if currentCons >= currentProd {
			// 队列为空时让出 CPU 时间片
			spinWait(&retries)
			continue
		}

		index := currentCons & rb.mask
		slot := &rb.slots[index]

		// 检查槽位是否可读
		if atomic.LoadUint32(&slot.flag) != 2 {
			spinWait(&retries)
			continue
		}

		// CAS更新槽位状态为empty
		if !atomic.CompareAndSwapUint32(&slot.flag, 2, 0) {
			spinWait(&retries)
			continue
		}

		// 读取数据并更新全局消费者位置
		valPtr := atomic.LoadPointer(&slot.data)
		if valPtr == nil {
			atomic.StoreUint32(&slot.flag, 2) // 回滚槽位状态
			spinWait(&retries)
			continue
		}

		if atomic.CompareAndSwapUint32(&rb.consumerPos, currentCons, currentCons+1) {
			val := *(*T)(valPtr)
			rb.pool.Put((*T)(valPtr)) // 放回对象池复用
			return val, nil
		}

		// 如果更新失败, 回滚槽位状态
		atomic.StoreUint32(&slot.flag, 2)
		spinWait(&retries)
	}
}

// Len 返回缓冲区中当前元素的数量
func (rb *RingBuffer[T]) Len() int {
	prod := atomic.LoadUint32(&rb.producerPos)
	cons := atomic.LoadUint32(&rb.consumerPos)
	return int(prod - cons)
}

// Cap 返回缓冲区的容量
func (rb *RingBuffer[T]) Cap() int {
	return int(rb.size)
}

// IsEmpty 当缓冲区为空时返回 true
func (rb *RingBuffer[T]) IsEmpty() bool {
	return rb.Len() == 0
}

// IsFull 当缓冲区已满时返回 true
func (rb *RingBuffer[T]) IsFull() bool {
	return rb.Len() == int(rb.size)
}

// Close 关闭环形缓冲区(设置关闭标志), 写入方将被拒绝写入
func (rb *RingBuffer[T]) Close() {
	atomic.StoreUint32(&rb.closed, 1)
}

// WaitForClose 在关闭后阻塞直到所有数据被消费完成
func (rb *RingBuffer[T]) WaitForClose() {
	for !rb.IsEmpty() {
		runtime.Gosched()
	}
}
