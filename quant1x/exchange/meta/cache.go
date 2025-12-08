package meta

import (
	"fmt"
	"sync"

	"gitee.com/quant1x/gox/logger"
	"gitee.com/quant1x/quant1x/quant1x/std"
)

const (
	defaultQuant1xDataPath = "~/.q1x-go" // 默认的数据路径
)

var (
	globalCacheOnce   sync.Once                // 懒加载锁
	globalCachePath   = defaultQuant1xDataPath // 数据根路径
	onceTemporaryPath = defaultQuant1xDataPath // 临时路径
)

func initPath(path string) {
	finalPath, err := std.ExpandUser(path)
	if err != nil {
		logger.Fatalf("%+v", err)
	}
	onceTemporaryPath = path
	globalCachePath = finalPath
}

// InitCachePath 公开给外部调用的初始化路径的函数
//
//	lazyInit和InitCachePath两者只能真正被调用一次
func InitCachePath(path string) {
	globalCacheOnce.Do(func() {
		onceTemporaryPath = path
		initPath(path)
	})
}

// 默认的初始化路径
func lazyInit() {
	initPath(onceTemporaryPath)
}

// DefaultCachePath 数据缓存的根路径
func DefaultCachePath() string {
	globalCacheOnce.Do(lazyInit)
	return globalCachePath
}

// GetMetaPath 元数据缓存路径
func GetMetaPath() string {
	return DefaultCachePath() + "/meta"
}

// GetBlockPath 板块路径
func GetBlockPath() string {
	return GetMetaPath()
}

// CalendarFilename 交易日历文件路径
func CalendarFilename() string {
	filename := GetMetaPath() + "/calendar"
	return filename
}

// BlockFilename 板块缓存路径
func BlockFilename(ns ...string) string {
	// 默认取板块列表
	name := "blocks"
	if len(ns) > 0 {
		name = ns[0]
	}
	filename := fmt.Sprintf("%s/%s.csv", GetMetaPath(), name)
	return filename
}
