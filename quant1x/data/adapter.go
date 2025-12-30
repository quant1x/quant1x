package data

import (
	"errors"
	"slices"
	"sync"
)

type Kind = uint64

const (
	PluginAll          Kind = 0x0000000000000000 // 所有类型的适配器
	PluginMaskBaseData Kind = 0x1000000000000000 // 基础数据
	PluginMaskFeature  Kind = 0x2000000000000000 // 特征数据
	PluginMaskStrategy Kind = 0x3000000000000000 // 策略
)

const (
	// DefaultDataProvider 默认数据提供者
	DefaultDataProvider = "quant1x"
)

// Schema 缓存的概要信息
type Schema interface {
	// Kind 数据类型
	Kind() Kind
	// Owner 提供者
	Owner() string
	// Key 数据关键词, key与cache落地强关联
	Key() string
	// Name 特性名称
	Name() string
	// Usage 控制台参数提示信息, 数据描述(data description)
	Usage() string
}

// DataAdapter 数据插件
type DataAdapter interface {
	// Schema 继承基础特性接口
	Schema
	// Print 控制台输出指定日期的数据
	Print(code string, date ...string)
}

// Handover 缓存切换接口
type Handover interface {
	// ChangingOverDate 缓存数据转换日期
	//	数据集等基础数据不需要切换日期
	ChangingOverDate(date string)
}

type Depend interface {
	DependOn() []Kind
}

var (
	ErrAlreadyExists = errors.New("the plugin already exists")
)

var (
	pluginMutex    sync.Mutex
	mapDataPlugins = map[Kind]DataAdapter{}
	//setupStatus map[string]bool
)

// Register 注册插件
func Register(plugin DataAdapter) error {
	pluginMutex.Lock()
	defer pluginMutex.Unlock()
	_, ok := mapDataPlugins[plugin.Kind()]
	if ok {
		return ErrAlreadyExists
	}
	mapDataPlugins[plugin.Kind()] = plugin
	return nil
}

// GetDataAdapter 根据指定的Kind类型获取对应的数据适配器
//
//	参数 kind 是数据适配器的类型
//	返回值 adapter 是匹配的数据适配器，如果找不到对应类型的适配器则返回nil
func GetDataAdapter(kind Kind) DataAdapter {
	pluginMutex.Lock()
	defer pluginMutex.Unlock()
	adapter, ok := mapDataPlugins[kind]
	if ok {
		return adapter
	}
	return nil
}

// Plugins 根据给定的插件类型掩码返回匹配的数据适配器列表
//
//	参数 mask 是可选的插件类型掩码，支持 PluginMaskBaseData 或 PluginMaskFeature 类型
//	返回值 list 是按插件类型排序后的数据适配器切片
//
// 注意：当前实现存在内存申请优化空间
func Plugins(mask ...Kind) (list []DataAdapter) {
	pluginMutex.Lock()
	defer pluginMutex.Unlock()
	pluginType := Kind(0)
	if len(mask) > 0 {
		if mask[0] == PluginMaskBaseData || mask[0] == PluginMaskFeature {
			pluginType = mask[0]
		}
	}
	// TODO: 这个地方的内存申请方面需要优化
	var kinds []Kind
	for kind, plugin := range mapDataPlugins {
		if pluginType == 0 || kind&pluginType == pluginType {
			kinds = append(kinds, kind)
		}
		_ = plugin
	}
	slices.Sort(kinds)
	for _, kind := range kinds {
		plugin, ok := mapDataPlugins[kind]
		if ok {
			list = append(list, plugin)
		}
	}
	return
}

// PluginsWithName 根据插件类型和关键词列表筛选匹配的数据适配器插件
//
//	pluginType: 插件类型筛选条件
//	keywords: 关键词列表，用于匹配插件Key
//	返回: 匹配的数据适配器插件列表，按插件类型排序
func PluginsWithName(pluginType Kind, keywords ...string) (list []DataAdapter) {
	pluginMutex.Lock()
	defer pluginMutex.Unlock()
	if len(keywords) == 0 {
		return
	}
	var kinds []Kind
	for kind, plugin := range mapDataPlugins {
		if kind&pluginType == pluginType && slices.Contains(keywords, plugin.Key()) {
			kinds = append(kinds, kind)
		}
	}
	if len(kinds) == 0 {
		return
	}
	slices.Sort(kinds)
	for _, kind := range kinds {
		plugin, ok := mapDataPlugins[kind]
		if ok {
			list = append(list, plugin)
		}
	}
	return
}
