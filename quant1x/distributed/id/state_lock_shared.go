package id

import (
	"os"
	"runtime"
	"sync/atomic"
	"time"
	"unsafe"
)

// 状态文件共享内存锁（mmap 锁字）：
//
//	slots 共 64B（0..63），锁字位于映射内偏移 stateLockOffset（对齐 8 字节）。
//	锁字布局 64bit：高 32 位为持锁进程 PID，低 32 位为加锁时刻（截断 Unix 秒）。
//	值为 0 表示未加锁。
//
// 算法要点：
//   - 加锁：CAS(0 -> mine)；失败则自旋退避重试。
//   - 解锁：CAS(mine -> 0)。若期间被抢占（他人判定本进程已死后接管），
//     CAS 失败即放弃，不会误清他人的锁。
//   - 崩溃恢复：文件锁改共享内存锁后，持锁进程异常退出不会再由内核代为释放，
//     因此等待方通过 processAlive 检测持锁者存活：已死立刻抢占；
//     另有 TTL 兜底（stamp 超过 lockTakeoverAfterSeconds 强制接管，
//     覆盖无法检测 PID 存活的平台与 PID 复用的极端情况）。
//
// 注意：该锁仅适用于单机共享同一状态文件的进程组（mmap 语义天然如此）。

const (
	lockTakeoverAfterSeconds = 30
	lockBackoffMaxSleepUs    = 1024
)

// stateLockOffset 锁字在共享映射内的偏移：紧跟 64B 双槽 checkpoint 区之后，
// 8 字节对齐；状态文件总大小 stateFileSize 为此后的定长布局。
const stateLockOffset = checkpointAreaSize

// encodeLockWord 把 pid 与截断秒级时间戳打包为 64bit 锁字。
func encodeLockWord(pid int, stamp uint32) uint64 {
	return uint64(uint32(pid))<<32 | uint64(stamp)
}

// decodeLockWord 拆解锁字。
func decodeLockWord(word uint64) (pid int32, stamp uint32) {
	return int32(uint32(word >> 32)), uint32(word)
}

func lockStampNow() uint32 { return uint32(time.Now().Unix()) }

// lockMapped 打开映射并对锁字加锁，返回解锁闭包。
// 仅在调用者保证的单线程上下文使用（HLC.mu / store 单实例约束）。
func (s *fileStateStore) lockMapped() (func(), error) {
	if err := s.openMapped(); err != nil {
		return nil, err
	}
	data := s.mapped.Bytes()
	word := (*uint64)(unsafe.Pointer(&data[stateLockOffset]))
	self := os.Getpid()
	mine := encodeLockWord(self, lockStampNow())

	retries := 0
	for {
		current := atomic.LoadUint64(word)
		if current == 0 {
			if atomic.CompareAndSwapUint64(word, 0, mine) {
				return func() { _ = atomic.CompareAndSwapUint64(word, mine, 0) }, nil
			}
			continue // 竞争失败，立即重试
		}

		pid, stamp := decodeLockWord(current)
		if !lockHolderStale(pid, stamp) {
			lockBackoff(&retries)
			continue
		}

		// 持有者已死亡或超时：抢占。
		// CAS 保证与"持有者正要释放 (mine->0)"并发时的安全：
		// 只有当锁字仍是被抢占者的值时才覆写为自己的锁。
		if atomic.CompareAndSwapUint64(word, current, mine) {
			return func() { _ = atomic.CompareAndSwapUint64(word, mine, 0) }, nil
		}
		retries = 0 // 锁字刚发生变化，重置退避快速参与下一轮竞争
	}
}

// lockHolderStale 判定当前持有者是否可被抢占。
// 同进程持有者（另一个 HLC 实例）始终存活，只能自旋等待。
func lockHolderStale(pid int32, stamp uint32) bool {
	if int(pid) == os.Getpid() {
		return false
	}
	if !processAlive(int(pid)) {
		return true
	}
	elapsed := uint32(time.Now().Unix()) - stamp // 截断秒差，模 2^32 安全
	return elapsed >= lockTakeoverAfterSeconds
}

// lockBackoff 自旋退避：先空转/Gosched，再按微秒级翻倍睡眠。
func lockBackoff(retries *int) {
	switch {
	case *retries < 4:
		for i := 0; i < 1<<uint(*retries); i++ {
			runtime.Gosched()
		}
	case *retries < 12:
		runtime.Gosched()
	default:
		sleepUs := 1 << uint(*retries-12)
		if sleepUs > lockBackoffMaxSleepUs {
			sleepUs = lockBackoffMaxSleepUs
		}
		time.Sleep(time.Duration(sleepUs) * time.Microsecond)
	}
	if *retries < 24 {
		(*retries)++
	}
}
