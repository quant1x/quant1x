package id128

import (
	"sync"
	"time"
)

type HLC struct {
	mu        sync.Mutex
	physical  int64
	logical   uint16
	seq       uint32
	now       func() int64
	seed      uint16
	syncEvery uint32
	store     stateStore
}

func NewHLC(opts ...Option) *HLC {
	h := &HLC{
		now:       func() int64 { return time.Now().UnixMilli() },
		seed:      randomUint16(),
		syncEvery: 1,
	}

	for _, opt := range opts {
		if opt != nil {
			opt(h)
		}
	}

	if fileStore, ok := h.store.(*fileStateStore); ok {
		fileStore.syncEvery = h.syncEvery
	}

	if restored, ok, err := h.loadState(); err != nil {
		panic(err)
	} else if ok {
		h.physical = restored.Physical
		h.logical = restored.Logical
		h.seq = restored.Seq
	} else {
		h.physical = h.now()
		h.logical = h.seed
	}
	return h
}

// Now 返回严格单调的 (hlc, seq) 二元组。
// 整个更新过程只使用一把锁和一次时钟采样。
//
// TODO: 当 store != nil 时，调用链为 h.mu → store.Next() → lockProcessFile()。
// 锁获取顺序恒为 (内存锁 → 文件锁)。未来若有新路径先取文件锁再取 h.mu，会导致死锁。
// stateStore 实现者需确保内部不再回调到 HLC 的带锁方法。
func (h *HLC) Now() (uint64, uint32) {
	h.mu.Lock()
	defer h.mu.Unlock()

	now := h.now()
	current := persistentState{
		Physical: h.physical,
		Logical:  h.logical,
		Seq:      h.seq,
	}

	if h.store != nil {
		next, err := h.store.Next(current, now, h.seed)
		if err != nil {
			panic(err)
		}
		h.physical = next.Physical
		h.logical = next.Logical
		h.seq = next.Seq
	} else {
		next := advancePersistentState(current, now, h.seed)
		h.physical = next.Physical
		h.logical = next.Logical
		h.seq = next.Seq
	}

	hlc := uint64(h.physical)<<16 | uint64(h.logical)
	return hlc, h.seq
}

// Timestamp 返回当前保存的物理毫秒值。
//
// TODO: 当 logical 溢出导致 physical 自增后，此方法返回的是自增后的值而非系统时间。
// 调用方需理解该语义，或考虑同时返回 h.physical 是否经过自增的标志位。
func (h *HLC) Timestamp() int64 {
	h.mu.Lock()
	defer h.mu.Unlock()

	return h.physical
}

func (h *HLC) loadState() (persistentState, bool, error) {
	if h.store == nil {
		return persistentState{}, false, nil
	}

	return h.store.Load()
}

func advancePersistentState(state persistentState, now int64, seed uint16) persistentState {
	if now > state.Physical {
		state.Physical = now
		state.Logical = seed
		state.Seq = 0
		return state
	}

	state.Seq++
	if state.Seq == 0 {
		state.Logical++
		if state.Logical == 0 {
			state.Physical++
			state.Logical = seed
		}
	}

	return state
}
