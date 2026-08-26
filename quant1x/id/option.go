package id

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
// 默认值为 1，表示每次发号后都执行一次文件同步。
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
