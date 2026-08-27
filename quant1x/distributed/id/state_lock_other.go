//go:build !windows && !unix

package id

// processAlive 是无法探测进程存活的平台上的保守实现：
// 一律视为存活，由锁字 TTL（lockTakeoverAfterSeconds）兜底回收。
func processAlive(_ int) bool { return true }
