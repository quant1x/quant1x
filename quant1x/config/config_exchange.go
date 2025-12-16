package config

import (
	"path/filepath"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/std"
)

// 缓存id, 由市场代码+证券代码组成
func CacheId(code string) string {
	_, marketCode, code_, _ := exchange.DetectMarket(code)
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

func Top10HoldersFilename(code, date string) string {
	idPath := CacheIdPath(code)
	quarter, _, _ := std.GetQuarterByDate(date, 0)
	return filepath.Join(GetHoldingPath(), quarter, idPath+".csv")
}
