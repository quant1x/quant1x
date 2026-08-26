//go:build !windows && !unix

package id128

import (
	"fmt"
	"os"
	"time"
)

func lockProcessFile(path string) (func() error, error) {
	deadline := time.Now().Add(10 * time.Second)
	for {
		file, err := os.OpenFile(path, os.O_CREATE|os.O_EXCL|os.O_RDWR, 0o644)
		if err == nil {
			return func() error {
				closeErr := file.Close()
				removeErr := os.Remove(path)
				if closeErr != nil {
					return fmt.Errorf("id: 关闭锁文件失败: %w", closeErr)
				}
				if removeErr != nil {
					return fmt.Errorf("id: 删除锁文件失败: %w", removeErr)
				}
				return nil
			}, nil
		}

		if !os.IsExist(err) {
			return nil, fmt.Errorf("id: 获取兼容锁失败: %w", err)
		}
		if time.Now().After(deadline) {
			return nil, fmt.Errorf("id: 获取兼容锁超时")
		}
		time.Sleep(10 * time.Millisecond)
	}
}
