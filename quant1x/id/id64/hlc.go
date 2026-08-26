package id64

import (
	"sync"
	"time"
)

// HLC 管理物理时间与序列号的单调推进。
//
// 内部维护 (physical, seq) 二元组：
//   - physical：绝对毫秒时间戳（epoch 相对值在组装 ID 时换算）
//   - seq：序列号，达到 seqBits 容量时进位 physical+1（时钟回拨时保持单调）
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

// NewHLC 构造 HLC，默认节点总数 1024（workerBits=11, seqBits=11）。
func NewHLC(opts ...Option) *HLC {
	h := &HLC{
		now:       func() int64 { return time.Now().UnixMilli() },
		seed:      randomUint16(),
		seqBits:   payloadBits - seqBitsFromNodeCount(1024),
		syncEvery: defaultSyncEveryValue(),
	}
	for _, opt := range opts {
		if opt != nil {
			opt(h)
		}
	}
	if fileStore, ok := h.store.(*fileStateStore); ok {
		fileStore.syncEvery = h.syncEvery
		fileStore.strict = h.strict
	}

	if restored, ok, err := h.loadState(); err != nil {
		panic(err)
	} else if ok {
		h.physical = restored.Physical
		h.seq = restored.Seq
	} else {
		h.physical = h.now()
		h.seq = uint32(h.seed) & h.seqMask()
	}
	return h
}

// seqMask 返回 seqBits 位全 1 掩码。
func (h *HLC) seqMask() uint32 {
	return uint32(1)<<h.seqBits - 1
}

// Now 返回严格单调递增的 (physical 绝对毫秒, seq)。
func (h *HLC) Now() (int64, uint32) {
	h.mu.Lock()
	defer h.mu.Unlock()

	now := h.now()
	current := persistentState{Physical: h.physical, Seq: h.seq}
	var next persistentState
	if h.store != nil {
		var err error
		next, err = h.store.Next(current, now, h.seqBits)
		if err != nil {
			panic(err)
		}
	} else {
		next = advancePersistentState(current, now, h.seqBits)
	}
	h.physical = next.Physical
	h.seq = next.Seq
	return h.physical, h.seq
}

// SeqBits 返回当前序列号位宽。
func (h *HLC) SeqBits() uint8 {
	return h.seqBits
}

// Close 把快速路径批量缓冲中尚未落盘的状态记录写入磁盘并同步，
// 同时释放严格模式缓存的文件句柄。
// 启用状态文件后，进程异常退出最多丢失最近 syncEvery-1 条进度
// （这些 ID 在重启后可能重复）；优雅退出前调用本方法可零丢失。
// 未启用状态文件时为空操作。可多次调用，幂等。
func (h *HLC) Close() error {
	h.mu.Lock()
	defer h.mu.Unlock()

	if h.store == nil {
		return nil
	}
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

// Timestamp 返回当前物理时间（绝对毫秒）。
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

// advancePersistentState 在共享状态上推进 (physical, seq)：
//   - 物理时间前进：重置 seq 为 0
//   - 否则 seq+1；seq 达容量时进位 physical+1 并重置 seq（保持单调，不等待墙钟追平）
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
