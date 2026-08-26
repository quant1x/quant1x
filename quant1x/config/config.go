package config

import (
	"path/filepath"
	"strings"

	"github.com/quant1x/quant1x/quant1x/base"
	"github.com/quant1x/quant1x/quant1x/core"
)

// price cage defaults from C++ config/price_cage.h
const (
	ValidDeclarationPriceRange  = 0.02
	MinimumPriceFluctuationUnit = 0.10
	FixedSlippageForSell        = 0.01
)

// package-level helpers: use core defaults

// Trader loading and defaults moved to `trader_parameter.go`, `strategy_parameter.go`, and `rule_parameter.go`.

func ConfigFilename() string {
	return core.GetConfigfilePath()
}

func IsDebug() bool {
	m := core.GetConfigMap()
	if v, ok := m["debug"].(bool); ok {
		return v
	}
	return false
}

func GetMetaPath() string {
	return core.GetMetaPath()
}

func GetLogsPath() string {
	return core.GetLogsPath()
}

func GetCalendarFilename() string {
	return filepath.Join(core.GetMetaPath(), "calendar")
}

func GetSecurityFilename() string {
	return filepath.Join(core.GetMetaPath(), "securities.csv")
}

func GetSectorFilename(date string) string {
	filename := "blocks." + date
	p := filepath.Join(core.GetMetaPath(), filename)
	return filepath.Clean(p)
}

func GetHistoricalTradeFilename(code, cacheDate string) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	year := cacheDate[:4]
	date := strings.ReplaceAll(cacheDate, "-", "")
	p := filepath.Join(core.DefaultCachePath(), "trans", year, date, code+".csv")
	return filepath.Clean(p)
}

func GetChipDistributionFilename(code, cacheDate string) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	year := cacheDate[:4]
	date := strings.ReplaceAll(cacheDate, "-", "")
	p := filepath.Join(core.DefaultCachePath(), "trans", year, date, code+".cd")
	return filepath.Clean(p)
}

func GetBlockPath() string {
	return core.GetMetaPath()
}

func GetXdxrPath() string {
	return filepath.Join(core.DefaultCachePath(), "xdxr")
}

func GetDayPath() string {
	return filepath.Join(core.DefaultCachePath(), "day")
}

func GetBarPath(freq string) string {
	return filepath.Join(core.DefaultCachePath(), freq)
}

func GetMinutePath() string {
	return filepath.Join(core.DefaultCachePath(), "minutes")
}

// subpath 与 C++ config/cache.cpp::subpath 对齐, 截取代码前缀作为缓存子目录.
// 例如 "sh600000" -> "sh600"
func subpath(code string) string {
	length := len(code)
	const suffixLength = 3
	if length <= suffixLength {
		return ""
	}
	return code[:length-suffixLength]
}

// GetXdxrFilename 根据证券代码生成对应的除权除息数据文件路径
//
//	参数 code: 8位证券代码
//	返回: 完整的文件路径字符串
func GetXdxrFilename(code string) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	p := filepath.Join(GetXdxrPath(), subpath(code), code+".csv")
	return filepath.Clean(p)
}

// GetBarFilename 生成K线缓存文件路径, 与 C++ config/cache.cpp::get_bar_filename 对齐.
//
//	参数 code: 8位证券代码
//	参数 forward: true 为复权K线(csv), false 为原始K线(raw)
func GetBarFilename(code string, forward bool) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	ext := "raw"
	if forward {
		ext = "csv"
	}
	p := filepath.Join(GetDayPath(), subpath(code), code+"."+ext)
	return filepath.Clean(p)
}

// GetBarFilenameEx 生成指定频率K线缓存文件路径, 与 C++ config/cache.cpp::get_bar_filename_ex 对齐.
func GetBarFilenameEx(code, freq string) string {
	if len(code) != 8 {
		panic("invalid security code length")
	}
	p := filepath.Join(GetBarPath(freq), subpath(code), code+".csv")
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
	return filepath.Join(core.DefaultCachePath(), "holding")
}

func QuarterlyCachePath(date string) string {
	q, _, _ := base.GetQuarterByDate(date, 0)
	return filepath.Join(core.DefaultCachePath(), "infoq", q)
}

func QuarterlyFilename(date, keyword string) string {
	return filepath.Join(QuarterlyCachePath(date), keyword+".csv")
}

func ReportsFilename(date string) string {
	return QuarterlyFilename(date, "reports")
}

func DefaultQmtCachePath() string {
	return filepath.Join(core.DefaultCachePath(), "qmt")
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
