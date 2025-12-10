package config

import (
	"os"
	"path/filepath"
	"strings"
	"sync"

	_ "unsafe"

	"gopkg.in/yaml.v3"

	"gitee.com/quant1x/quant1x/quant1x/std"
)

// Go translation of config::BaseConfig and helpers from config.cpp

const defaultQuant1xDataPath = "~/.q1x-go"

// price cage defaults from C++ config/price_cage.h
const (
	ValidDeclarationPriceRange  = 0.02
	MinimumPriceFluctuationUnit = 0.10
	FixedSlippageForSell        = 0.01
)

type BaseConfig struct {
	HomeDir        string
	Filename       string
	CacheDir       string
	LogsDir        string
	RunningInDebug bool
	Data           map[string]any
}

var globalConfig BaseConfig
var globalCacheOnce sync.Once

func initPath(path string) {
	expanded, _ := std.ExpandUser(path)
	if expanded == "" {
		expanded = path
	}
	_ = os.MkdirAll(expanded, 0o755)
	globalConfig.HomeDir = expanded
}

func lazyInit() {
	initPath(defaultQuant1xDataPath)
	globalConfig.Filename, _ = std.ExpandUser(filepath.Join(globalConfig.HomeDir, "quant1x.yaml"))

	// try to read YAML and honor top-level basedir/debug and generic data
	data, err := os.ReadFile(globalConfig.Filename)
	if err == nil {
		var node map[string]interface{}
		if err := yaml.Unmarshal(data, &node); err == nil {
			if v, ok := node["basedir"].(string); ok && strings.TrimSpace(v) != "" {
				if expanded, _ := std.ExpandUser(strings.TrimSpace(v)); expanded != "" {
					globalConfig.CacheDir = expanded
				}
			}
			if d, ok := node["debug"].(bool); ok {
				globalConfig.RunningInDebug = d
			}
			globalConfig.Data = node
		} else {
			// fallback to home
			globalConfig.CacheDir = globalConfig.HomeDir
			_ = err
		}
	} else {
		globalConfig.CacheDir = globalConfig.HomeDir
	}

	globalConfig.LogsDir = filepath.Join(globalConfig.CacheDir, "logs")
	_ = std.MkDirs(globalConfig.LogsDir)
}

// Trader loading and defaults moved to `trader_parameter.go`, `strategy_parameter.go`, and `rule_parameter.go`.

func ConfigFilename() string {
	globalCacheOnce.Do(lazyInit)
	return globalConfig.Filename
}

func IsDebug() bool {
	globalCacheOnce.Do(lazyInit)
	return globalConfig.RunningInDebug
}

func DefaultHomePath() string {
	globalCacheOnce.Do(lazyInit)
	return globalConfig.HomeDir
}

func DefaultCachePath() string {
	globalCacheOnce.Do(lazyInit)
	return globalConfig.CacheDir
}

func GetMetaPath() string {
	return filepath.Join(DefaultHomePath(), "meta")
}

func GetLogsPath() string {
	return filepath.Join(DefaultCachePath(), "logs")
}

func GetCalendarFilename() string {
	return filepath.Join(GetMetaPath(), "calendar")
}

func GetSecurityFilename() string {
	return filepath.Join(GetMetaPath(), "securities.csv")
}

func GetSectorFilename(date string) string {
	filename := "blocks." + date
	p := filepath.Join(GetMetaPath(), filename)
	return filepath.Clean(p)
}

func GetHistoricalTradeFilename(code, cacheDate string) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	year := cacheDate[:4]
	date := strings.ReplaceAll(cacheDate, "-", "")
	p := filepath.Join(DefaultCachePath(), "trans", year, date, code+".csv")
	return filepath.Clean(p)
}

func GetChipDistributionFilename(code, cacheDate string) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	year := cacheDate[:4]
	date := strings.ReplaceAll(cacheDate, "-", "")
	p := filepath.Join(DefaultCachePath(), "trans", year, date, code+".cd")
	return filepath.Clean(p)
}

func GetBlockPath() string {
	return GetMetaPath()
}

func GetXdxrPath() string {
	return filepath.Join(DefaultCachePath(), "xdxr")
}

func GetDayPath() string {
	return filepath.Join(DefaultCachePath(), "day")
}

func GetKlinePath(freq string) string {
	return filepath.Join(DefaultCachePath(), freq)
}

func GetMinutePath() string {
	return filepath.Join(DefaultCachePath(), "minutes")
}

// GetXdxrFilename 根据证券代码生成对应的除权除息数据文件路径
//
//	参数 code: 8位证券代码
//	返回: 完整的文件路径字符串
//	如果证券代码长度不为8位，会触发panic
func GetXdxrFilename(code string) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	p := filepath.Join(GetXdxrPath(), CacheIdPath(code)+".csv")
	return filepath.Clean(p)
}

func GetKlineFilename(code string, forward bool) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	ext := "raw"
	if forward {
		ext = "csv"
	}
	p := filepath.Join(GetDayPath(), CacheIdPath(code)+"."+ext)
	return filepath.Clean(p)
}

func GetKlineFilenameEx(code, freq string) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	p := filepath.Join(GetKlinePath(freq), CacheIdPath(code)+".csv")
	return filepath.Clean(p)
}

func GetMinuteFilename(code, cacheDate string) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	if len(strings.ReplaceAll(cacheDate, "-", "")) < 8 {
		panic("invalid date format for cache_date")
	}
	year := cacheDate[:4]
	date := strings.ReplaceAll(cacheDate, "-", "")
	p := filepath.Join(GetMinutePath(), year, date, code+".csv")
	return filepath.Clean(p)
}

func GetHoldingPath() string {
	return filepath.Join(DefaultCachePath(), "holding")
}

func QuarterlyCachePath(date string) string {
	q, _, _ := std.GetQuarterByDate(date, 0)
	return filepath.Join(DefaultCachePath(), "infoq", q)
}

func QuarterlyFilename(date, keyword string) string {
	return filepath.Join(QuarterlyCachePath(date), keyword+".csv")
}

func ReportsFilename(date string) string {
	return QuarterlyFilename(date, "reports")
}

func DefaultQmtCachePath() string {
	return filepath.Join(DefaultCachePath(), "qmt")
}

func GetQmtCachePath() string {
	qmtOrderPath := DefaultQmtCachePath()
	trader := TraderConfig()
	if trader != nil {
		orderPath := trader.OrderPath
		if orderPath != "" {
			// keep logic similar to C++: if not empty and check_filepath fails, use it
			// we don't have util::check_filepath here; assume provided path is usable
			qmtOrderPath = orderPath
		}
	}
	return qmtOrderPath
}
