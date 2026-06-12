package market

import (
	"path/filepath"
	"sort"
	"strings"

	"github.com/quant1x/quant1x/quant1x/core"
	"github.com/quant1x/quant1x/quant1x/encoding"
	"github.com/quant1x/quant1x/quant1x/exchange"
	"github.com/quant1x/quant1x/quant1x/level1"
	"github.com/quant1x/quant1x/quant1x/log"
	"github.com/quant1x/quant1x/quant1x/runtime"
	"github.com/quant1x/quant1x/quant1x/std"
)

var (
	// use fixed pre-market time here to avoid importing exchange (prevent cycles)
	instrumentsRollingOnce = runtime.RollingOnceDaily(exchange.GetInitTime(exchange.ExchangeSSE))
	instrumentsMap         = map[string]exchange.InstrumentInfo{}
)

func GetSecurityFilename() string {
	return filepath.Join(core.GetMetaPath(), "securities.csv")
}

func updateSecurities(fname string) {
	conn, release, err := level1.GetStdConnection()
	if err == nil {
		defer release()
		markets := []exchange.Exchange{
			exchange.ExchangeSSE,
			exchange.ExchangeSZSE,
			exchange.ExchangeBSE,
		}
		var all []exchange.InstrumentInfo
		for _, market := range markets {
			var codes []exchange.InstrumentInfo
			start := 0
			for {
				req := level1.NewSecurityListRequest(market, start, level1.SecurityListPerRequestMax)
				resp := &level1.SecurityListResponse{}
				if err := level1.Process(conn, req, resp); err != nil {
					break
				}
				if len(resp.List) > 0 {
					// prefix codes and append
					for i := 0; i < int(resp.Count) && i < len(resp.List); i++ {
						v := resp.List[i]
						sc := exchange.DetectWithExchange(market, v.Code)
						si := exchange.InstrumentInfo{
							Exchange:       market,
							Type:           sc.Type,
							Ticker:         v.Code,
							Name:           v.Name,
							LotSize:        int(v.VolUnit),
							PricePrecision: int(v.DecimalPoint),
						}
						codes = append(codes, si)
					}
				}
				if len(resp.List) < level1.SecurityListPerRequestMax {
					break
				}
				start += level1.SecurityListPerRequestMax
			}
			sort.Slice(codes, func(i, j int) bool {
				return codes[i].Ticker < codes[j].Ticker
			})
			all = append(all, codes...)
		}
		encoding.SlicesToCsv(fname, all, true)
	}
}

func initSecurities() {
	fname := GetSecurityFilename()

	// Decide whether to refresh from Level1
	bUpdate := exchange.ShouldInitializeFile(fname)

	if bUpdate {
		updateSecurities(fname)
	}

	// Load CSV into memory
	var all []exchange.InstrumentInfo
	err := encoding.CsvToSlices(fname, &all)
	if err != nil {
		log.Errorf("failed to load securities from CSV: %v", err)
		return
	}
	clear(instrumentsMap)
	for _, info := range all {
		code := info.Symbol()
		instrumentsMap[code] = info
	}
}

// GetSecurityInfo returns security info for a code (will normalize internally).
func GetSecurityInfo(instrument string) *exchange.InstrumentInfo {
	instrumentsRollingOnce.Do(initSecurities)
	if strings.TrimSpace(instrument) == "" {
		return nil
	}
	instrument = strings.TrimSpace(instrument)
	instrument = strings.ToLower(instrument)
	if p, ok := instrumentsMap[instrument]; ok {
		return &p
	}
	return nil
}

// GetStockName 获取证券名称
func GetStockName(instrument string) string {
	security := GetSecurityInfo(instrument)
	if security != nil {
		return security.Name
	}
	return "Unknown"
}

const (
	instrumentPriceHighLimit   = 0.20
	incrementPriceNormalLimit  = 0.10
	incrementPriceBeijingLimit = 0.30
)

func getInstrumentUpLimitRate(security *exchange.InstrumentInfo) float64 {
	if security == nil {
		return incrementPriceNormalLimit
	}
	if security.Exchange == exchange.ExchangeBSE {
		return incrementPriceBeijingLimit
	}
	if strings.HasPrefix(security.Ticker, "30") || strings.HasPrefix(security.Ticker, "68") {
		return instrumentPriceHighLimit
	}
	return incrementPriceNormalLimit
}

// GetUpLimitRate 根据证券代码返回对应的涨停幅度
//
//	instrument: 证券代码
//
//	返回值: 对应的涨停幅度(小数形式)
//
// 规则:
//   - 如果证券不存在，返回默认涨停幅度(incrementPriceNormalLimit)
//   - 如果是北京交易所(BSE)证券，返回北京交易所涨停幅度(incrementPriceBeijingLimit)
//   - 如果证券代码以"30"或"68"开头，返回创业板/科创板涨停幅度(instrumentPriceHighLimit)
//   - 其他情况返回默认涨停幅度(incrementPriceNormalLimit)
func GetUpLimitRate(instrument string) float64 {
	security := GetSecurityInfo(instrument)
	return getInstrumentUpLimitRate(security)
}

// CalcLimitUpPrice calculates the rounded limit-up price based on previous close.
// It will round the result using the security's `PricePrecision` when available,
// otherwise defaults to 2 decimal places.
func CalcLimitUpPrice(instrument string, prevClose float64) float64 {
	security := GetSecurityInfo(instrument)
	rate := getInstrumentUpLimitRate(security)
	price := prevClose * (1.0 + rate)
	prec := 2
	if security != nil && security.PricePrecision > 0 {
		prec = security.PricePrecision
	}
	return std.Decimal(price, prec)
}
