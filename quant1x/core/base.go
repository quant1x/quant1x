package core

import (
	"path/filepath"
	"sync"

	"gitee.com/quant1x/quant1x/quant1x/std"
)

const (
	language        = "go"
	defaultBasePath = "~/.q1x-" + language
)

var (
	onceBasePath    sync.Once // 确保基础路径只初始化一次
	quant1xBasePath string    // 基础路径
)

// lazyInitBasePath 初始化基础路径，如果扩展用户路径失败则使用默认路径
func lazyInitBasePath() {
	path, err := std.ExpandUser(defaultBasePath)
	if err != nil {
		quant1xBasePath = defaultBasePath
	} else {
		quant1xBasePath = path
	}
}

// GetBasePath 返回默认的基础路径，如果无法展开用户目录则返回默认路径
func GetBasePath() string {
	onceBasePath.Do(lazyInitBasePath)
	return quant1xBasePath
}

// GetMetaPath 返回元数据存储的基础路径
//
//	meta目录位于基础路径下的meta子目录中
func GetMetaPath() string {
	return filepath.Join(GetBasePath(), "meta")
}
