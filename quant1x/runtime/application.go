package runtime

import (
	"os"
	"path/filepath"
	"sync"
)

var (
	onceApp     sync.Once
	application = ""
)

func lazyLoadApplication() {
	path, _ := os.Executable()
	_, exec := filepath.Split(path)
	application = exec
}

// ApplicationName 获取执行文件名
func ApplicationName() string {
	onceApp.Do(lazyLoadApplication)
	return application
}
