package id64

import "math/bits"

// Option 是 HLC 的可选配置项。
type Option func(*HLC)

// WithClock 覆盖默认时钟（返回绝对毫秒），测试用。
func WithClock(now func() int64) Option {
	return func(h *HLC) {
		if now != nil {
			h.now = now
		}
	}
}

// WithSeqSeed 设置序列号启动种子（默认随机）。
// 种子用于无状态文件时随机化初始 seq，降低重启碰撞概率。
func WithSeqSeed(seed uint16) Option {
	return func(h *HLC) {
		h.seed = seed
	}
}

// WithStateFile 启用状态文件持久化，跨进程/重启恢复高水位。
func WithStateFile(path string) Option {
	return func(h *HLC) {
		h.store = newFileStateStore(path)
	}
}

// WithStateSyncEvery 设置状态文件落盘间隔（每 N 次生成落盘一次）。
func WithStateSyncEvery(every uint32) Option {
	return func(h *HLC) {
		if every > 0 {
			h.syncEvery = every
		}
	}
}

// WithStateStrict 启用严格模式：每次发号前从磁盘读取最新状态并取 max。
//
// 默认关闭（快速路径）：构造时从状态文件恢复一次高水位，运行期只追加不读盘，
// 热路径仅一次写入。适用于单写者，以及多进程顺序接管（failover）场景——
// 新进程构造时读到前任写者的最新水位，保证跨重启不重复。
//
// 当多个进程（或同进程多个 HLC 实例）活跃共享同一状态文件、且都期望严格唯一时，
// 必须开启严格模式：它以每次发号增加一次磁盘读为代价，保证各写者水位同步。
func WithStateStrict() Option {
	return func(h *HLC) {
		h.strict = true
		if fileStore, ok := h.store.(*fileStateStore); ok {
			fileStore.strict = true
		}
	}
}

// WithNodeCount 设置预期的节点总数，据此动态推导节点位宽与序列号位宽：
//
//	workerBits = bits.Len(nodeCount)
//	seqBits    = 64 - 1 - 41 - workerBits
//
// 当 seqBits < 4（节点数 > 2^18）时 panic。
func WithNodeCount(count uint32) Option {
	return func(h *HLC) {
		if count < 1 {
			count = 1
		}
		workerBits := bits.Len(uint(count))
		h.seqBits = payloadBits - uint8(workerBits)
		if h.seqBits < 4 {
			panic("id64: 节点数过多，无法为序列号保留足够的位宽")
		}
	}
}

// WithSeqBits 直接设置序列号位宽（底层选项，通常用 WithNodeCount 代替）。
func WithSeqBits(seqBits uint8) Option {
	return func(h *HLC) {
		if seqBits < 4 || seqBits > payloadBits-1 {
			panic("id64: seqBits 超出有效范围 [4, 21]")
		}
		h.seqBits = seqBits
	}
}

// seqBitsFromNodeCount 与 WithNodeCount 的推导公式一致（用于默认值）。
func seqBitsFromNodeCount(count uint32) uint8 {
	if count < 1 {
		count = 1
	}
	return payloadBits - uint8(bits.Len(uint(count)))
}
