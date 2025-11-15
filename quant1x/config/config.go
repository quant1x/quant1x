package config

import (
	"fmt"
	"os"
	"os/user"
	"path/filepath"
	"strings"
	"sync"

	_ "unsafe"

	"gopkg.in/yaml.v3"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
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
	expanded := expandHome(path)
	if expanded == "" {
		expanded = path
	}
	_ = os.MkdirAll(expanded, 0o755)
	globalConfig.HomeDir = expanded
}

func lazyInit() {
	initPath(defaultQuant1xDataPath)
	globalConfig.Filename = expandHome(filepath.Join(globalConfig.HomeDir, "quant1x.yaml"))

	// try to read YAML and honor top-level basedir/debug and generic data
	data, err := os.ReadFile(globalConfig.Filename)
	if err == nil {
		var node map[string]interface{}
		if err := yaml.Unmarshal(data, &node); err == nil {
			if v, ok := node["basedir"].(string); ok && strings.TrimSpace(v) != "" {
				if expanded := expandHome(strings.TrimSpace(v)); expanded != "" {
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
	_ = os.MkdirAll(globalConfig.LogsDir, 0o755)

	// // register calendar filename resolver and a RollingOnce marker so exchange
	// // calendar updates are gated once-per-day (mirrors C++ behavior).
	// exchange.SetCalendarFilenameResolver(GetCalendarFilename)
	// marker := filepath.Join(GetMetaPath(), "rolling", "exchange-calendar")

	// _ = os.MkdirAll(filepath.Dir(marker), 0o755)
	// ro := runtimepkg.CreateDaily(9, 0)
	// exchange.SetCalendarRollingOnce(ro)
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

const suffixLength = 3

func subpath(code string) string {
	if len(code) <= suffixLength {
		return ""
	}
	return code[:len(code)-suffixLength]
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
	sub := subpath(code)
	p := filepath.Join(GetXdxrPath(), sub, code+".csv")
	return filepath.Clean(p)
}

func GetKlineFilename(code string, forward bool) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	sub := subpath(code)
	ext := "raw"
	if forward {
		ext = "csv"
	}
	p := filepath.Join(GetDayPath(), sub, code+"."+ext)
	return filepath.Clean(p)
}

func GetKlineFilenameEx(code, freq string) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	sub := subpath(code)
	p := filepath.Join(GetKlinePath(freq), sub, code+".csv")
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

// detail namespace
func CacheId(code string) string {
	_, marketCode, code_ := exchange.DetectMarket(code)
	return marketCode + code_
}

func CacheIdPath(code string) string {
	const N = 3
	cacheId := CacheId(code)
	if len(cacheId) <= N {
		return cacheId
	}
	prefix := cacheId[:len(cacheId)-N]
	return prefix + "/" + cacheId
}

func GetHoldingPath() string {
	return filepath.Join(DefaultCachePath(), "holding")
}

func getQuarterByDate(date string) string {
	// expect YYYY-MM-DD or YYYYMMDD
	clean := strings.ReplaceAll(date, "-", "")
	if len(clean) < 6 {
		return "unknown"
	}
	year := clean[:4]
	monthStr := clean[4:6]
	month := 1
	fmt.Sscanf(monthStr, "%02d", &month)
	q := ((month - 1) / 3) + 1
	return fmt.Sprintf("%sQ%d", year, q)
}

func Top10HoldersFilename(code, date string) string {
	idPath := CacheIdPath(code)
	quarter := getQuarterByDate(date)
	return filepath.Join(GetHoldingPath(), quarter, idPath+".csv")
}

func QuarterlyCachePath(date string) string {
	q := getQuarterByDate(date)
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

func expandHome(path string) string {
	if path == "~" {
		if u, err := user.Current(); err == nil {
			return u.HomeDir
		}
		return ""
	}
	if strings.HasPrefix(path, "~/") {
		if u, err := user.Current(); err == nil {
			return filepath.Join(u.HomeDir, path[2:])
		}
		return ""
	}
	return path
}
