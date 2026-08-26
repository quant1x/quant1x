package id128

type Option func(*HLC)

// WithClock 注入自定义时钟，主要用于测试。
func WithClock(now func() int64) Option {
	return func(h *HLC) {
		if now != nil {
			h.now = now
		}
	}
}

// WithLogicalSeed 设置初始逻辑分量。
// 当调用方自行持久化或协调该值时，可以降低重启碰撞概率。
func WithLogicalSeed(seed uint16) Option {
	return func(h *HLC) {
		h.seed = seed
	}
}

// WithStateFile 指定状态文件路径。
// 启用后，每次发号都会落盘最新状态，重启后会从上次高水位继续递增。
func WithStateFile(path string) Option {
	return func(h *HLC) {
		if path != "" {
			h.store = newFileStateStore(path)
		}
	}
}

// WithStateSyncEvery 设置状态文件的同步频率。
// 默认值为 1000，表示每追加 1000 条记录执行一次文件同步。
// 大于 1 时可显著降低 fsync 成本，但进程异常退出时最近若干条记录可能尚未落盘。
func WithStateSyncEvery(every uint32) Option {
	return func(h *HLC) {
		if every == 0 {
			every = 1
		}
		h.syncEvery = every
		if fileStore, ok := h.store.(*fileStateStore); ok {
			fileStore.syncEvery = every
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
