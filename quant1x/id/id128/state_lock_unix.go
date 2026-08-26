//go:build unix

package id128

import (
	"fmt"
	"os"
	"syscall"
)

func lockProcessFile(path string) (func() error, error) {
	file, err := os.OpenFile(path, os.O_CREATE|os.O_RDWR, 0o644)
	if err != nil {
		return nil, fmt.Errorf("id: 打开锁文件失败: %w", err)
	}

	if err := syscall.Flock(int(file.Fd()), syscall.LOCK_EX); err != nil {
		_ = file.Close()
		return nil, fmt.Errorf("id: 获取 Unix 文件锁失败: %w", err)
	}

	return func() error {
		unlockErr := syscall.Flock(int(file.Fd()), syscall.LOCK_UN)
		closeErr := file.Close()
		if unlockErr != nil {
			return fmt.Errorf("id: 释放 Unix 文件锁失败: %w", unlockErr)
		}
		if closeErr != nil {
			return fmt.Errorf("id: 关闭锁文件失败: %w", closeErr)
		}
		return nil
	}, nil
}
