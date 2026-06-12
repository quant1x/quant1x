package config

import (
	"path/filepath"

	"github.com/quant1x/quant1x/quant1x/data/exchange"
	"github.com/quant1x/quant1x/quant1x/std"
)

// 缓存id, 由市场代码+证券代码组成
func CacheId(symbol string) string {
	instrument := exchange.DetectSymbol(symbol)
	return instrument.Symbol()

}

func CacheIdPath(symbol string) string {
	const N = 3
	cacheId := CacheId(symbol)
	if len(cacheId) <= N {
		return cacheId
	}
	prefix := cacheId[:len(cacheId)-N]
	return prefix + "/" + cacheId
}

func Top10HoldersFilename(symbol, date string) string {
	idPath := CacheIdPath(symbol)
	quarter, _, _ := std.GetQuarterByDate(date, 0)
	return filepath.Join(GetHoldingPath(), quarter, idPath+".csv")
}
