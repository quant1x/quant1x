package config

import (
	"path/filepath"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
)

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

func Top10HoldersFilename(code, date string) string {
	idPath := CacheIdPath(code)
	quarter := getQuarterByDate(date)
	return filepath.Join(GetHoldingPath(), quarter, idPath+".csv")
}
