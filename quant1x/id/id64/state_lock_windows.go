//go:build windows

package id64

import (
	"fmt"
	"os"
	"syscall"
	"unsafe"
)

// strictLockCacheSupported 表示该平台支持缓存锁文件句柄
// （Windows 的 LockFileEx 可以对同一句柄反复加锁/解锁）。
const strictLockCacheSupported = true

const lockfileExclusiveLock = 0x00000002

var (
	kernel32Proc     = syscall.NewLazyDLL("kernel32.dll")
	lockFileExProc   = kernel32Proc.NewProc("LockFileEx")
	unlockFileExProc = kernel32Proc.NewProc("UnlockFileEx")
)

// lockProcessFileHandle 对已打开的句柄获取跨进程独占锁。
// 返回的解锁函数只释放锁、不关闭文件，供严格模式缓存句柄复用。
func lockProcessFileHandle(file *os.File) (func() error, error) {
	var overlapped syscall.Overlapped
	r1, _, callErr := lockFileExProc.Call(
		file.Fd(),
		uintptr(lockfileExclusiveLock),
		0,
		1,
		0,
		uintptr(unsafe.Pointer(&overlapped)),
	)
	if r1 == 0 {
		return nil, fmt.Errorf("id64: 获取 Windows 文件锁失败: %w", callErr)
	}

	return func() error {
		r1, _, callErr := unlockFileExProc.Call(
			file.Fd(),
			0,
			1,
			0,
			uintptr(unsafe.Pointer(&overlapped)),
		)
		if r1 == 0 {
			return fmt.Errorf("id64: 释放 Windows 文件锁失败: %w", callErr)
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
