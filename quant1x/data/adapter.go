package data

import (
	"errors"
	"path/filepath"
	"sort"
	"strings"
	"sync"

	"gitee.com/quant1x/quant1x/quant1x/core"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
)

// Kind 表示插件类型标识
type Kind = uint64

const (
	PluginMaskBaseData Kind = 0x1000000000000000 // 基础数据
	PluginMaskFeature  Kind = 0x2000000000000000 // 特征数据
	PluginMaskStrategy Kind = 0x3000000000000000 // 策略
)

// DefaultDataProvider 默认的数据提供者
const DefaultDataProvider = "quant1x"

// Schema 缓存的概要信息接口
type Schema interface {
	Kind() Kind
	Owner() string
	Key() string
	Name() string
	Usage() string
}

// DataAdapter 数据适配器接口
type DataAdapter interface {
	Schema
	// Print 控制台打印，dates 可选
	Print(code exchange.SecurityCode, dates ...exchange.Timestamp)
	// Update 更新数据
	Update(code exchange.SecurityCode, date exchange.Timestamp)
}

// FeatureAdapter 特征数据适配器接口
type FeatureAdapter interface {
	DataAdapter
	// Filename 返回对应的聚合文件路径
	Filename(timestamp exchange.Timestamp) string
	Init(timestamp exchange.Timestamp)
	Clone() FeatureAdapter
	Headers() []string
	Values() []string
}

// FeatureFilename 提供与 C++ FeatureAdapter::Filename 相同的默认实现
const cache1dPrefix = "flash"

// FeatureFilename 根据给定的FeatureAdapter和时间戳生成对应的缓存文件名
//
// 参数:
//
//	f: FeatureAdapter实例，提供缓存键值
//	timestamp: 交易所时间戳，用于确定日期路径
//
// 返回:
//
//	生成的完整缓存文件路径，路径已清理
func FeatureFilename(f FeatureAdapter, timestamp exchange.Timestamp) string {
	key := f.Key()
	pos := strings.IndexByte(key, '/')
	var cachePath, actualKey string
	if pos >= 0 {
		cachePath = key[:pos]
		actualKey = key[pos+1:]
	} else {
		actualKey = key
		cachePath = cache1dPrefix
	}

	date := timestamp.OnlyDate()
	year := ""
	if len(date) >= 4 {
		year = date[:4]
	}

	fullPath := filepath.Join(core.DefaultCachePath(), cachePath, year, actualKey+"."+date)
	return filepath.Clean(fullPath)
}

// 注册与插件管理
var (
	pluginMutex      sync.Mutex
	pluginMap        = make(map[Kind]DataAdapter)
	ErrAlreadyExists = errors.New("the plugin already exists")
)

// GetDataAdapter 按 Kind 获取已注册的适配器（或 nil）
func GetDataAdapter(kind Kind) DataAdapter {
	pluginMutex.Lock()
	defer pluginMutex.Unlock()
	if p, ok := pluginMap[kind]; ok {
		return p
	}
	return nil
}

// Register 注册一个 DataAdapter，若已存在返回 ErrAlreadyExists
func Register(plugin DataAdapter) error {
	pluginMutex.Lock()
	defer pluginMutex.Unlock()
	k := plugin.Kind()
	if _, ok := pluginMap[k]; ok {
		return ErrAlreadyExists
	}
	pluginMap[k] = plugin
	return nil
}

// PluginsWithName 根据类型掩码和关键字列表返回匹配的适配器
func PluginsWithName(pluginType Kind, keywords ...string) []DataAdapter {
	pluginMutex.Lock()
	defer pluginMutex.Unlock()

	if len(keywords) == 0 {
		return nil
	}

	kwSet := make(map[string]struct{}, len(keywords))
	for _, k := range keywords {
		kwSet[k] = struct{}{}
	}

	type pair struct {
		k Kind
		p DataAdapter
	}

	var candidates []pair
	for _, plugin := range pluginMap {
		if (plugin.Kind() & pluginType) == pluginType {
			if _, ok := kwSet[plugin.Key()]; ok {
				candidates = append(candidates, pair{plugin.Kind(), plugin})
			}
		}
	}

	if len(candidates) == 0 {
		return nil
	}

	sort.Slice(candidates, func(i, j int) bool { return candidates[i].k < candidates[j].k })

	out := make([]DataAdapter, 0, len(candidates))
	for _, c := range candidates {
		out = append(out, c.p)
	}
	return out
}

// Plugins 返回按 kind 排序的适配器列表。mask 为 0 返回全部。
func Plugins(mask Kind) []DataAdapter {
	pluginMutex.Lock()
	defer pluginMutex.Unlock()

	var list []DataAdapter
	for _, p := range pluginMap {
		if mask == 0 || ((p.Kind() & mask) == mask) {
			list = append(list, p)
		}
	}

	sort.Slice(list, func(i, j int) bool { return list[i].Kind() < list[j].Kind() })
	return list
}

const (
	LayoutTradeDate     = "2006-01-02"              // 交易日格式(仅日期)
	LayoutDateTimeMilli = "2006-01-02 15:04:05.000" // 日期时间格式(含毫秒)
)

// DataSummary 数据概要
type DataSummary struct {
	kind  Kind   // 类型
	key   string // 关键字
	name  string // 名称
	owner string // 拥有者
	usage string // 用法
}

func Summary(kind Kind, key, name, owner string, usage ...string) DataSummary {
	var description string
	if len(usage) > 0 {
		description = usage[0]
	}
	return DataSummary{
		kind:  kind,
		key:   key,
		name:  name,
		owner: owner,
		usage: description,
	}
}

func (d DataSummary) Kind() Kind {
	return d.kind
}

func (d DataSummary) Key() string {
	return d.key
}

func (d DataSummary) Name() string {
	return d.name
}

func (d DataSummary) Owner() string {
	return d.owner
}

func (d DataSummary) Usage() string {
	return d.usage
}

// SampleStatus 样本状态信息
type SampleStatus struct {
	CreateTime string `name:"创建时间" csv:"create_time"` // 创建时间
	UpdateTime string `name:"更新时间" csv:"update_time"` // 最后更新时间
	State      uint64 `name:"状态码" csv:"state"`        // 状态码
}
