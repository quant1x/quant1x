package id

import (
	"github.com/quant1x/quant1x/quant1x/runtime"
)

// Queue is a bounded MPMC queue for distributed IDs.
// It uses the Go Vyukov ringbuffer implementation.
type Queue struct {
	queue *runtime.RingBuffer[ID]
}

// NewQueue creates a queue. Capacity is rounded up to a power of two.
func NewQueue(capacity uint32) (*Queue, error) {
	queue, err := runtime.New[ID](capacity)
	if err != nil {
		return nil, err
	}
	return &Queue{queue: queue}, nil
}

// TryPush adds an ID without blocking.
func (q *Queue) TryPush(value ID) error { return q.queue.TryPush(value) }

// TryPop removes an ID without blocking.
func (q *Queue) TryPop() (ID, error) { return q.queue.TryPop() }

// Push adds an ID and waits while the queue is full.
func (q *Queue) Push(value ID) error { return q.queue.Push(value) }

// Pop removes an ID and waits while the queue is empty.
func (q *Queue) Pop() (ID, error) { return q.queue.Pop() }

// Len returns the approximate number of queued IDs.
func (q *Queue) Len() int { return q.queue.Len() }

// Cap returns the queue capacity.
func (q *Queue) Cap() int { return q.queue.Cap() }

// Close prevents further pushes after the queue is closed.
func (q *Queue) Close() { q.queue.Close() }

// IsClosed reports whether the queue is closed.
func (q *Queue) IsClosed() bool { return q.queue.IsClosed() }

// WaitForClose waits until all queued IDs have been consumed.
func (q *Queue) WaitForClose() { q.queue.WaitForClose() }
