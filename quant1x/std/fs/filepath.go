package fs

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

const (
	DefaultDirMode  = 0755
	DefaultFileMode = 0644
)

// EnsureDirExists 确保指定路径的目录存在，如果不存在则递归创建
//
// 参数：
//   - path: 目录路径
//
// 返回值：
//   - error: 路径无效、创建失败或路径非目录时返回错误
func EnsureDirExists(path string) error {
	path = strings.TrimSpace(path)
	if path == "" {
		return fmt.Errorf("invalid directory path: '%s'", path)
	}
	path = filepath.Clean(path)

	fileInfo, err := os.Stat(path)
	if err != nil {
		if os.IsNotExist(err) {
			// 递归创建目录（包括父目录）
			if err := os.MkdirAll(path, DefaultDirMode); err != nil {
				return fmt.Errorf("failed to create directory: %w", err)
			}
			return nil
		}
		return err // 其他错误（如权限不足）
	}

	if !fileInfo.IsDir() {
		return fmt.Errorf("path '%s' exists but is not a directory", path)
	}
	return nil
}

// EnsureFileDirExists 确保文件所在的目录存在
//
// 参数：
//   - filePath: 文件路径（如 `/data/logs/file.txt`）
//
// 返回值：
//   - error: 目录创建失败或路径非目录时返回错误
func EnsureFileDirExists(filePath string) error {
	filePath = strings.TrimSpace(filePath)
	if filePath == "" {
		return fmt.Errorf("invalid file path: '%s'", filePath)
	}
	filePath = filepath.Clean(filePath)

	dir := filepath.Dir(filePath)
	return EnsureDirExists(dir)
}
