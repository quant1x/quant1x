//go:build !windows && !unix

package id64

import (
	"fmt"
	"os"
	"sync"
	"time"
)

// strictLockCacheSupported 表示该平台是否支持缓存锁文件句柄。
// 兼容平台（无 flock / LockFileEx）依赖"独占锁文件"实现跨进程锁，
// 锁文件一旦存在即视为持锁，无法安全复用句柄，故不支持缓存。
const strictLockCacheSupported = false

// compatLockMu 兼容平台 lockProcessFileHandle 的进程内互斥锁。
var compatLockMu sync.Mutex

// lockProcessFileHandle 兼容平台退化为进程内互斥锁（解锁不关闭文件）。
// 跨进程互斥由 lockProcessFile 的独占锁文件近似保证；这类平台通常不用于多进程部署。
func lockProcessFileHandle(_ *os.File) (func() error, error) {
	compatLockMu.Lock()
	return func() error {
		compatLockMu.Unlock()
		return nil
	}, nil
}

// lockProcessFile 打开锁文件并获取跨进程独占锁（一次性用法，如快速路径批量落盘）。
func lockProcessFile(path string) (func() error, error) {
	deadline := time.Now().Add(10 * time.Second)
	for {
		file, err := os.OpenFile(path, os.O_CREATE|os.O_EXCL|os.O_RDWR, 0o644)
		if err == nil {
			return func() error {
				closeErr := file.Close()
				removeErr := os.Remove(path)
				if closeErr != nil {
					return fmt.Errorf("id64: 关闭锁文件失败: %w", closeErr)
				}
				if removeErr != nil {
					return fmt.Errorf("id64: 删除锁文件失败: %w", removeErr)
				}
				return nil
			}, nil
		}

		if !os.IsExist(err) {
			return nil, fmt.Errorf("id64: 获取兼容锁失败: %w", err)
		}
		if time.Now().After(deadline) {
			return nil, fmt.Errorf("id64: 获取兼容锁超时")
		}
		time.Sleep(10 * time.Millisecond)
	}
}
