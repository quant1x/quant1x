//go:build unix

package id64

import (
	"fmt"
	"os"
	"syscall"
)

// strictLockCacheSupported 表示该平台支持缓存锁文件句柄
// （Unix 的 flock 可以对同一句柄反复加锁/解锁）。
const strictLockCacheSupported = true

// lockProcessFileHandle 对已打开的句柄获取跨进程独占锁（flock）。
// 返回的解锁函数只释放锁、不关闭文件，供严格模式缓存句柄复用。
func lockProcessFileHandle(file *os.File) (func() error, error) {
	if err := syscall.Flock(int(file.Fd()), syscall.LOCK_EX); err != nil {
		return nil, fmt.Errorf("id64: 获取 Unix 文件锁失败: %w", err)
	}

	return func() error {
		if err := syscall.Flock(int(file.Fd()), syscall.LOCK_UN); err != nil {
			return fmt.Errorf("id64: 释放 Unix 文件锁失败: %w", err)
		}
		return nil
	}, nil
}

// lockProcessFile 打开锁文件并获取跨进程独占锁（一次性用法，如快速路径批量落盘）。
func lockProcessFile(path string) (func() error, error) {
	file, err := os.OpenFile(path, os.O_CREATE|os.O_RDWR, 0o644)
	if err != nil {
		return nil, fmt.Errorf("id64: 打开锁文件失败: %w", err)
	}

	unlock, err := lockProcessFileHandle(file)
	if err != nil {
		_ = file.Close()
		return nil, err
	}

	return func() error {
		unlockErr := unlock()
		closeErr := file.Close()
		if unlockErr != nil {
			return unlockErr
		}
		if closeErr != nil {
			return fmt.Errorf("id64: 关闭锁文件失败: %w", closeErr)
		}
		return nil
	}, nil
}
