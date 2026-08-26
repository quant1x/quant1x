//go:build windows

package id

import (
	"fmt"
	"os"
	"syscall"
	"unsafe"
)

const lockfileExclusiveLock = 0x00000002

var (
	kernel32Proc     = syscall.NewLazyDLL("kernel32.dll")
	lockFileExProc   = kernel32Proc.NewProc("LockFileEx")
	unlockFileExProc = kernel32Proc.NewProc("UnlockFileEx")
)

func lockProcessFile(path string) (func() error, error) {
	file, err := os.OpenFile(path, os.O_CREATE|os.O_RDWR, 0o644)
	if err != nil {
		return nil, fmt.Errorf("id: 打开锁文件失败: %w", err)
	}

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
		_ = file.Close()
		return nil, fmt.Errorf("id: 获取 Windows 文件锁失败: %w", callErr)
	}

	return func() error {
		r1, _, callErr := unlockFileExProc.Call(
			file.Fd(),
			0,
			1,
			0,
			uintptr(unsafe.Pointer(&overlapped)),
		)
		closeErr := file.Close()
		if r1 == 0 {
			return fmt.Errorf("id: 释放 Windows 文件锁失败: %w", callErr)
		}
		if closeErr != nil {
			return fmt.Errorf("id: 关闭锁文件失败: %w", closeErr)
		}
		return nil
	}, nil
}
