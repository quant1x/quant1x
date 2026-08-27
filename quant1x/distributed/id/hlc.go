package id

import (
	"sync"
	"time"
)

type HLC struct {
	mu        sync.Mutex
	physical  int64
	seq       uint32
	now       func() int64
	seed      uint16
	seqBits   uint8
	syncEvery uint32
	strict    bool
	store     stateStore
}

func NewHLC(opts ...Option) *HLC {
	hlc := &HLC{
		now:       func() int64 { return time.Now().UnixMilli() },
		seed:      randomUint16(),
		seqBits:   11,
		syncEvery: defaultSyncEveryValue(),
	}
	for _, option := range opts {
		if option != nil {
			option(hlc)
		}
	}
	if fileStore, ok := hlc.store.(*fileStateStore); ok {
		fileStore.syncEvery = hlc.syncEvery
		fileStore.strict = hlc.strict
	}
	if restored, ok, err := hlc.loadState(); err != nil {
		panic(err)
	} else if ok {
		hlc.physical, hlc.seq = restored.Physical, restored.Seq
	} else {
		hlc.physical = hlc.now()
		hlc.seq = uint32(hlc.seed) & hlc.seqMask()
	}
	return hlc
}

func (h *HLC) seqMask() uint32 { return uint32(1)<<h.seqBits - 1 }

func (h *HLC) Now() (int64, uint32) {
	h.mu.Lock()
	defer h.mu.Unlock()
	now := h.now()
	if h.store != nil {
		next, err := h.store.Next(persistentState{Physical: h.physical, Seq: h.seq}, now, h.seqBits)
		if err != nil {
			panic(err)
		}
		h.physical, h.seq = next.Physical, next.Seq
		return h.physical, h.seq
	}
	if now > h.physical {
		h.physical, h.seq = now, 0
	} else if h.seq >= h.seqMask() {
		h.physical++
		h.seq = 0
	} else {
		h.seq++
	}
	return h.physical, h.seq
}

func (h *HLC) SeqBits() uint8   { return h.seqBits }
func (h *HLC) Timestamp() int64 { h.mu.Lock(); defer h.mu.Unlock(); return h.physical }
func (h *HLC) Close() error {
	h.mu.Lock()
	defer h.mu.Unlock()
	if h.store == nil {
		return nil
	}
	// 即使 Flush 失败也继续释放缓存资源（mmap/文件句柄），
	// 避免 Windows 上句柄泄漏导致文件无法删除。
	var firstErr error
	if f, ok := h.store.(interface{ Flush() error }); ok {
		if err := f.Flush(); err != nil {
			firstErr = err
		}
	}
	if c, ok := h.store.(interface{ Close() error }); ok {
		if err := c.Close(); err != nil && firstErr == nil {
			firstErr = err
		}
	}
	return firstErr
}

func (h *HLC) loadState() (persistentState, bool, error) {
	if h.store == nil {
		return persistentState{}, false, nil
	}
	return h.store.Load()
}

func advancePersistentState(state persistentState, now int64, seqBits uint8) persistentState {
	if now > state.Physical {
		state.Physical = now
		state.Seq = 0
		return state
	}
	mask := uint32(1)<<seqBits - 1
	if state.Seq >= mask {
		state.Physical++
		state.Seq = 0
		return state
	}
	state.Seq++
	return state
}
