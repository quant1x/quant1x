package exchange

import (
	"regexp"
	"strings"
)

// MarketID mirrors the C++ exchange::MarketType enumeration.
type MarketID uint8

const (
	MarketIDShenZhen MarketID = 0  // 深圳证券交易所
	MarketIDShangHai MarketID = 1  // 上海证券交易所
	MarketIDBeiJing  MarketID = 2  // 北京证券交易所
	MarketIDHongKong MarketID = 21 // 香港交易所
	MarketIDUSA      MarketID = 22 // 美国交易所
)

// DetectMarket implements the same semantics as exchange::DetectMarket.
// It returns the inferred market id, market flag (sh/sz/...), and the pure 6-digit code.
func DetectMarket(symbol string) (MarketID, string, string) {
	trimmed := strings.TrimSpace(symbol)
	if trimmed == "" {
		return MarketIDShangHai, string(MarketSSE), ""
	}

	lowered := strings.ToLower(trimmed)
	marketCode, pureCode := splitMarketFlag(lowered)
	pureCode = sanitizeDigits(pureCode)

	if marketCode == "" {
		if len(pureCode) == 6 && digitsRegexp.MatchString(pureCode) {
			market := GetMarket(pureCode)
			marketCode = string(market)
		} else {
			marketCode = string(MarketSSE)
		}
	}

	switch marketCode {
	case string(MarketSZSE):
		return MarketIDShenZhen, marketCode, pureCode
	case string(MarketBJSE):
		return MarketIDBeiJing, marketCode, pureCode
	case "hk":
		return MarketIDHongKong, marketCode, pureCode
	case "us":
		return MarketIDUSA, marketCode, pureCode
	default:
		return MarketIDShangHai, string(MarketSSE), pureCode
	}
}

// AssertIndexByMarketAndCode mirrors the C++ helper for distinguishing index codes.
func AssertIndexByMarketAndCode(m MarketID, code string) bool {
	symbol := strings.TrimSpace(code)
	switch m {
	case MarketIDShangHai:
		return strings.HasPrefix(symbol, "000") || strings.HasPrefix(symbol, "880") || strings.HasPrefix(symbol, "881")
	case MarketIDShenZhen:
		return strings.HasPrefix(symbol, "399")
	case MarketIDBeiJing:
		return strings.HasPrefix(symbol, "899")
	default:
		return false
	}
}

var digitsRegexp = regexp.MustCompile(`^\d+$`)

func splitMarketFlag(symbol string) (string, string) {
	for _, flag := range []string{"sh", "sz", "bj", "hk", "us"} {
		if strings.HasPrefix(symbol, flag) {
			return flag, symbol[len(flag):]
		}
		if strings.HasSuffix(symbol, flag) {
			return flag, symbol[:len(symbol)-len(flag)]
		}
	}
	return "", symbol
}

func sanitizeDigits(s string) string {
	replaced := strings.ReplaceAll(s, ".", "")
	replaced = strings.ReplaceAll(replaced, "-", "")
	return replaced
}
