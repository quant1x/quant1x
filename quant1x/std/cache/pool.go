package cache

import "sync"

// Pool 是一个泛型化的 sync.Pool 封装，用于缓存和复用对象。
// 对象在释放回池时会被重置为零值。
type Pool[E any] struct {
	once sync.Once
	pool sync.Pool
	zero E // 零值
}

// init 初始化底层的 sync.Pool，确保 New 函数只被设置一次。
func (p *Pool[E]) init() {
	p.pool = sync.Pool{
		New: func() any {
			return new(E) // 新对象自动初始化为零值
		},
	}
}

// Acquire 从池中获取一个对象。如果池为空则创建新对象。
// 返回的对象会被重置为零值（通过 Release 时的重置保证）。
func (p *Pool[E]) Acquire() *E {
	p.once.Do(p.init)
	return p.pool.Get().(*E)
}

// Release 将对象放回池中。如果对象为 nil 则直接返回。
// 放回前会将对象重置为零值以保证下次获取时的状态。
func (p *Pool[E]) Release(obj *E) {
	if obj == nil {
		return
	}
	p.once.Do(p.init)

	*obj = p.zero // 重置对象为零值
	p.pool.Put(obj)
}
