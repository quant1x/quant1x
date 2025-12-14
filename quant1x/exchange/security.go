package exchange

import (
	"encoding/csv"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"gitee.com/quant1x/quant1x/quant1x/core"
	"gitee.com/quant1x/quant1x/quant1x/runtime"
)

// SecurityInfo 证券信息
type SecurityInfo struct {
	Code           string
	Name           string
	LotSize        int
	PricePrecision int
}

var (
	// persistent RollingOnce: reset daily at pre-market to mirror C++ global_security_once
	securityRollingOnce = runtime.CreateDaily(PreMarketHour, PreMarketMinute)
	securityMap         = map[string]*SecurityInfo{}
)

func GetSecurityFilename() string {
	return filepath.Join(core.GetMetaPath(), "securities.csv")
}

// initSecurities mirrors the C++ init_securities: refresh cache when missing/stale
// and then load the CSV into the in-memory map. It intentionally swallows
// errors during the update phase (parity with C++ which logs and continues).
func initSecurities() {
	fname := GetSecurityFilename()

	// Note: we intentionally do not attempt to actively refresh securities via
	// level1 here to avoid creating an import cycle (level1 imports exchange).
	// We only load the CSV into memory and rely on external processes to
	// refresh `securities.csv` when necessary.

	// load CSV into memory (ignore update errors)
	f, err := os.Open(fname)
	if err != nil {
		// leave empty map
		return
	}
	defer f.Close()
	r := csv.NewReader(f)
	// read header (ignore)
	if _, err := r.Read(); err != nil {
		return
	}
	for {
		rec, err := r.Read()
		if err != nil {
			break
		}
		if len(rec) < 4 {
			continue
		}
		code := strings.TrimSpace(rec[0])
		lot := 0
		if v, e := strconv.Atoi(strings.TrimSpace(rec[1])); e == nil {
			lot = v
		}
		prec := 0
		if v, e := strconv.Atoi(strings.TrimSpace(rec[2])); e == nil {
			prec = v
		}
		name := strings.TrimSpace(rec[3])
		securityMap[code] = &SecurityInfo{Code: code, Name: name, LotSize: lot, PricePrecision: prec}
	}
}

// GetSecurityInfo 获取证券信息（使用内存缓存并从 securities.csv 加载）
func GetSecurityInfo(code string) *SecurityInfo {
	// Ensure the in-memory cache is loaded; use persistent RollingOnce (daily reset)
	securityRollingOnce.Do(initSecurities)
	if code == "" {
		return nil
	}
	c := CorrectSecurityCode(code)
	if p, ok := securityMap[c]; ok {
		return p
	}
	return nil
}
