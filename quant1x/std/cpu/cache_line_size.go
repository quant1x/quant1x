package cpu

const (
	// CacheLineSize 伪共享（False Sharing）防护
	// 多线程并发修改同一缓存行内的不同变量会导致性能下降，通过填充使变量独占缓存行
	// 设立设定一个默认值64, 以后扩展从汇编指令集获取
	CacheLineSize = 64
)
