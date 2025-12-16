package instruments

import (
	"encoding/csv"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/core"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/level1"
	"gitee.com/quant1x/quant1x/quant1x/runtime"
	"gitee.com/quant1x/quant1x/quant1x/std"
)

// SecurityInfo 证券信息
type SecurityInfo struct {
	Code           string
	Name           string
	LotSize        int
	PricePrecision int
}

var (
	// use fixed pre-market time here to avoid importing exchange (prevent cycles)
	securityRollingOnce = runtime.CreateDaily(9, 0)
	securityMap         = map[string]*SecurityInfo{}
)

func GetSecurityFilename() string {
	return filepath.Join(core.GetMetaPath(), "securities.csv")
}

func initSecurities() {
	fname := GetSecurityFilename()

	// Decide whether to refresh from Level1
	bUpdate := false
	info, err := os.Stat(fname)
	now := exchange.NowTimestamp()
	checkTP := now.PreMarketTime().ToTime()
	if err != nil {
		bUpdate = true
	} else {
		// If current time already reached today's pre-market and file is older than
		// today's pre-market, consider it outdated and update from Level1.
		if time.Now().After(checkTP) && info.ModTime().Before(checkTP) {
			bUpdate = true
		}
	}

	if bUpdate {
		conn, release, err := level1.GetStdConnection()
		if err == nil {
			defer release()
			markets := []struct {
				id     int
				prefix string
			}{
				{int(exchange.ExchangeIdShangHai), string(exchange.ExchangeSSE)},
				{int(exchange.ExchangeIdShenZhen), string(exchange.ExchangeSZSE)},
				{int(exchange.ExchangeIdBeiJing), string(exchange.ExchangeBJSE)},
			}
			var all []level1.Security
			for _, m := range markets {
				start := 0
				for {
					req := level1.NewSecurityListRequest(m.id, start, level1.SecurityListPreRequestMax)
					resp := &level1.SecurityListResponse{}
					if err := level1.Process(conn.Conn(), req, resp); err != nil {
						break
					}
					if len(resp.List) > 0 {
						// prefix codes and append
						for i := 0; i < int(resp.Count) && i < len(resp.List); i++ {
							v := resp.List[i]
							v.Code = m.prefix + v.Code
							all = append(all, v)
						}
					}
					if len(resp.List) < level1.SecurityListPreRequestMax {
						break
					}
					start += level1.SecurityListPreRequestMax
				}
			}
			if len(all) > 0 {
				// write csv
				f, err := os.OpenFile(fname, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, std.CACHE_FILE_PERMS)
				if err == nil {
					w := csv.NewWriter(f)
					_ = w.Write([]string{"Code", "VolUnit", "DecimalPoint", "Name", "PreClose"})
					for _, v := range all {
						_ = w.Write([]string{
							strings.TrimSpace(v.Code),
							strconv.Itoa(int(v.VolUnit)),
							strconv.Itoa(int(v.DecimalPoint)),
							strings.TrimSpace(v.Name),
							fmt.Sprintf("%f", v.PreClose),
						})
					}
					w.Flush()
					f.Close()
				}
			}
		}
	}

	// Load CSV into memory
	f, err := os.Open(fname)
	if err != nil {
		return
	}
	defer f.Close()
	r := csv.NewReader(f)
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

// GetSecurityInfo returns security info for a code (will normalize internally).
func GetSecurityInfo(code string) *SecurityInfo {
	securityRollingOnce.Do(initSecurities)
	if strings.TrimSpace(code) == "" {
		return nil
	}
	c := exchange.CorrectSecurityCode(code)
	if p, ok := securityMap[c]; ok {
		return p
	}
	return nil
}

// GetUpLimitRate returns the up-limit ratio for a security code.
func GetUpLimitRate(securityCode string) float64 {
	mid, _, symbol, _ := exchange.DetectMarket(securityCode)
	const (
		highLimit    = 0.20
		normalLimit  = 0.10
		beijingLimit = 0.30
	)
	if mid == exchange.ExchangeIdBeiJing {
		return beijingLimit
	}
	if strings.HasPrefix(symbol, "30") || strings.HasPrefix(symbol, "68") {
		return highLimit
	}
	return normalLimit
}

// CalcLimitUpPrice calculates the rounded limit-up price based on previous close.
// It will round the result using the security's `PricePrecision` when available,
// otherwise defaults to 2 decimal places.
func CalcLimitUpPrice(securityCode string, prevClose float64) float64 {
	rate := GetUpLimitRate(securityCode)
	price := prevClose * (1.0 + rate)
	prec := 2
	if si := GetSecurityInfo(securityCode); si != nil && si.PricePrecision > 0 {
		prec = si.PricePrecision
	}
	return std.Decimal(price, prec)
}
