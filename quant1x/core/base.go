package core

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"sync"

	"gitee.com/quant1x/quant1x/quant1x/std"
	"gopkg.in/yaml.v3"
)

const (
	language              = "go"
	defaultBasePath       = "~/.q1x-" + language
	quant1xConfigFilename = "quant1x.yaml"
)

var (
	onceBasePath    sync.Once // 确保基础路径只初始化一次
	quant1xBasePath string    // 基础路径
)

// BaseConfig 基础配置结构体
type BaseConfig struct {
	Debug     bool           `yaml:"debug"`   // 是否开启调试模式
	BaseDir   string         `yaml:"basedir"` // 基础目录路径
	LogDir    string         `yaml:"logdir"`  // 日志目录路径
	ConfigMap map[string]any `yaml:"-"`       // 原始配置 map，用于扩展读取
	Filename  string         `yaml:"-"`       // 配置文件名
}

var (
	cacheOnce sync.Once  // yaml配置文件只读取一次
	cacheCfg  BaseConfig // 全局配置实例
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

// BlockFilename 返回板块缓存文件路径，默认文件名为 blocks.csv
func BlockFilename(ns ...string) string {
	name := "blocks"
	if len(ns) > 0 && ns[0] != "" {
		name = ns[0]
	}
	return filepath.Join(GetMetaPath(), name+".csv")
}

func parseYamlConfig(filename string, config *BaseConfig) error {
	// 默认值（即使配置文件不存在，也应返回可用配置）
	_ = ApplyDefaults(config)
	config.Filename = strings.TrimSpace(filename)
	config.ConfigMap = map[string]any{}

	// 若配置文件不存在：使用默认 BaseDir/LogDir，并保留空 map
	if !std.FileExist(filename) {
		config.BaseDir = GetBasePath()
		config.LogDir = filepath.Join(config.BaseDir, "logs")
		return nil
	}

	dataBytes, err := os.ReadFile(filename)
	if err != nil {
		return err
	}

	// 先保留一份通用 map，供后续其它模块扩展/读取（避免重复解析）
	var node map[string]any
	if err := yaml.Unmarshal(dataBytes, &node); err == nil && node != nil {
		config.ConfigMap = node
	}

	// 再解析到强类型配置
	err = yaml.Unmarshal(dataBytes, config)
	if err != nil {
		return err
	}

	config.BaseDir = strings.TrimSpace(config.BaseDir)
	// 如果配置文件中没有basedir，则使用默认的basedir
	if len(config.BaseDir) > 0 {
		// 展开用户目录
		basedir, _ := std.ExpandUser(config.BaseDir)
		config.BaseDir = basedir
	} else {
		config.BaseDir = GetBasePath()
	}
	// 设置日志目录
	if len(strings.TrimSpace(config.LogDir)) > 0 {
		logdir, _ := std.ExpandUser(strings.TrimSpace(config.LogDir))
		config.LogDir = logdir
	} else {
		config.LogDir = filepath.Join(config.BaseDir, "logs")
	}

	// 归一化后的值也写回 map，保证后续模块读取到的路径一致
	if config.ConfigMap != nil {
		config.ConfigMap["basedir"] = config.BaseDir
		config.ConfigMap["logdir"] = config.LogDir
		config.ConfigMap["debug"] = config.Debug
	}

	return nil
}

// lazyInitCacheConfig 懒加载缓存配置，从配置文件解析并初始化缓存配置
//
//	配置文件路径为 basePath/quant1xConfigFilename
//	如果解析失败会记录错误日志
func lazyInitCacheConfig() {
	cfgFilename := filepath.Join(GetBasePath(), quant1xConfigFilename)
	err := parseYamlConfig(cfgFilename, &cacheCfg)
	if err != nil {
		panic(fmt.Errorf("failed to parse config file: %w", err))
	}
}

func GetConfigfilePath() string {
	cacheOnce.Do(lazyInitCacheConfig)
	return cacheCfg.Filename
}

// GetLogsPath 获取日志目录路径
func GetLogsPath() string {
	cacheOnce.Do(lazyInitCacheConfig)
	return cacheCfg.LogDir
}

func GetDataPath() string {
	cacheOnce.Do(lazyInitCacheConfig)
	return cacheCfg.BaseDir
}

// DefaultCachePath 返回默认缓存路径，该路径在首次调用时通过懒加载初始化
func DefaultCachePath() string {
	cacheOnce.Do(lazyInitCacheConfig)
	return cacheCfg.BaseDir
}

// GetConfigMap 返回已解析的配置 map（用于其它模块扩展/读取，避免重复解析）。
// 返回的是浅拷贝，防止调用方误修改全局配置。
func GetConfigMap() map[string]any {
	cacheOnce.Do(lazyInitCacheConfig)
	return copyAnyMap(cacheCfg.ConfigMap)
}

// GetConfigMapRef 返回已解析的配置 map 的原始引用（可用于其它模块写入扩展配置）。
// 注意：map 不是并发安全的；若跨 goroutine 读写，请由调用方自行加锁或约定仅在初始化阶段写入。
func GetConfigMapRef() map[string]any {
	cacheOnce.Do(lazyInitCacheConfig)
	return cacheCfg.ConfigMap
}

func copyAnyMap(src map[string]any) map[string]any {
	if src == nil {
		return map[string]any{}
	}
	dst := make(map[string]any, len(src))
	for k, v := range src {
		dst[k] = v
	}
	return dst
}
