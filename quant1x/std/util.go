package std

import (
	"fmt"
	"os"
	"path/filepath"
)

const (
	CACHE_DIR_PERMS  = 0755 // 目录权限
	CACHE_FILE_PERMS = 0644 // 文件权限
)

// MkDirs 创建指定路径的所有目录（包括任何必要的父目录），使用默认的目录权限
func MkDirs(path string, notExistToCreate ...bool) error {
	create := true
	if len(notExistToCreate) > 0 {
		create = notExistToCreate[0]
	}
	if create {
		return os.MkdirAll(path, CACHE_DIR_PERMS)
	}
	info, err := os.Stat(path)
	if err != nil {
		return err
	}
	if !info.IsDir() {
		return fmt.Errorf("path exists but is not a directory: %s", path)
	}
	return nil
}

// CheckFilepath
//
//	检查filename 文件路径, 如果不存在就创建
func CheckFilepath(path string, notExistToCreate ...bool) error {
	dir := filepath.Dir(path)
	return MkDirs(dir, notExistToCreate...)
}
