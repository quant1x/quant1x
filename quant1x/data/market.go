// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// market — 市场/证券代码识别与纠正, 与 Python data/market.py 对齐

package data

import (
	"fmt"
	"strings"

	"github.com/quant1x/quant1x/quant1x/base"
	"github.com/quant1x/quant1x/quant1x/data/meta"
	"github.com/quant1x/quant1x/quant1x/data/meta/ticker_rules"
	"github.com/quant1x/quant1x/quant1x/data/schema"
)

// ============================================================
// Type aliases from meta package (convenience re-exports).
// ============================================================

type Exchange = meta.Exchange
type InstrumentInfo = meta.Instrument
type Timestamp = meta.Timestamp
type InstrumentType = meta.InstrumentType
type Transaction = schema.Transaction

// ============================================================
// Exchange constants from meta.
// ============================================================

const (
	SSE  = meta.SSE
	SZSE = meta.SZSE
	BSE  = meta.BSE
)

// ExchangeSSE is a deprecated alias kept for test compatibility.
const ExchangeSSE = meta.SSE

// ExchangeSZSE is a deprecated alias kept for test compatibility.
const ExchangeSZSE = meta.SZSE

// ============================================================
// Instrument type constants.
// ============================================================

const SecurityTypeUnknown = meta.InstrumentTypeUnknown

// ============================================================
// Timestamp functions (re-exported from meta).
// ============================================================

var (
	NowTimestamp           = meta.NowTimestamp
	PreMarketTimestamp     = meta.PreMarketTimestamp
	ParseTimestamp         = meta.ParseTimestamp
	LastTradingDay         = meta.LastTradingDay
	NewTimestampFromString = meta.NewTimestampFromString
	DateRange              = meta.DateRange
	ZeroTimestamp          = meta.ZeroTimestamp
	NewTimestampFromTime   = meta.NewTimestampFromTime
	CanInitialize          = meta.CanInitialize
)

// ============================================================
// String/Bytes helpers (delegated to std).
// ============================================================

// String2Bytes converts a string to a byte slice.
func String2Bytes(s string) []byte {
	return base.String2Bytes(s)
}

// Bytes2String converts a byte slice to a string.
func Bytes2String(b []byte) string {
	return base.Bytes2String(b)
}

// ============================================================
// BuildInstrument — constructs a full instrument code string.
// ============================================================

// BuildInstrument constructs a full instrument code string from exchange and ticker.
// For Chinese markets: "SH600000". For others: "AAPL.US".
func BuildInstrument(ex meta.Exchange, ticker string) string {
	if ex.Region() == meta.RegionCN {
		return fmt.Sprintf("%s%s", ex.Identifier(), ticker)
	}
	return fmt.Sprintf("%s.%s", ticker, ex.Identifier())
}

// ============================================================
// GetFirstMarketDate — earliest trading day for a given exchange.
// ============================================================

// GetFirstMarketDate returns the first market date timestamp for a given exchange.
func GetFirstMarketDate(ex meta.Exchange) Timestamp {
	switch ex {
	case meta.SSE, meta.SZSE:
		return meta.PreMarketTimestamp(1990, 12, 19)
	case meta.BSE:
		return meta.PreMarketTimestamp(2021, 11, 15)
	default:
		return meta.PreMarketTimestamp(1990, 12, 19)
	}
}

// ============================================================
// AssertIndexByMarketAndCode — checks if a code represents an index/block.
// ============================================================

// AssertIndexByMarketAndCode checks if a security code represents an index or block
// for the given exchange. Mirrors the Rust assert_index_by_market_and_code and
// the C++ assert_index_by_security_code.
//
// SSE codes starting with 000/880/881 are indices/blocks.
// SZSE codes starting with 399 are indices.
// BSE codes starting with 899 are indices.
func AssertIndexByMarketAndCode(ex meta.Exchange, code string) bool {
	if len(code) < 3 {
		return false
	}
	switch ex {
	case meta.SSE:
		return code[:3] == "000" || code[:3] == "880" || code[:3] == "881"
	case meta.SZSE:
		return code[:3] == "399"
	case meta.BSE:
		return code[:3] == "899"
	default:
		return false
	}
}

// ============================================================
// DetectInstrumentTypeByRule — 根据交易所与代码检测证券类型
// ============================================================

// DetectInstrumentTypeByRule 使用对应交易所的规则检测证券类型, 与 Python data/market.py 对齐.
// 若交易所无匹配规则, 返回 InstrumentTypeUnknown.
func DetectInstrumentTypeByRule(exchange meta.Exchange, code string) meta.InstrumentType {
	var rules []ticker_rules.CodeRule
	switch exchange {
	case meta.SSE:
		rules = ticker_rules.SseRules()
	case meta.SZSE:
		rules = ticker_rules.SzseRules()
	case meta.BSE:
		rules = ticker_rules.BseRules()
	case meta.HKEX:
		rules = ticker_rules.HkexRules()
	case meta.USA:
		rules = ticker_rules.UsaRules()
	default:
		return meta.InstrumentTypeUnknown
	}
	return ticker_rules.MatchRule(code, rules).InstrumentType
}

// ============================================================
// DetectSymbol — 检测并解析证券代码的市场及类型
// ============================================================

// DetectSymbol 检测并解析证券代码的市场类型及证券类型, 与 Python data/market.py::detect_symbol 对齐.
func DetectSymbol(inputStr string) meta.Instrument {
	s := strings.ToLower(strings.TrimSpace(inputStr))
	if s == "" {
		return meta.Instrument{Exchange: meta.UNKNOWN, Type: meta.InstrumentTypeUnknown}
	}
	pureCode := s

	var ticker string
	var exchange = meta.UNKNOWN
	var typ = meta.InstrumentTypeUnknown

	// 1. 判断前缀: sh600000
	if len(pureCode) >= 2 {
		if ex, err := meta.ParseExchange(pureCode[:2]); err == nil && ex != meta.UNKNOWN {
			ticker = pureCode[2:]
			exchange = ex
		}
	}
	// 2. 判断后缀: 600000.sh or AAPL.us
	if exchange == meta.UNKNOWN && len(pureCode) >= 3 && pureCode[len(pureCode)-3] == '.' {
		if ex, err := meta.ParseExchange(pureCode[len(pureCode)-2:]); err == nil && ex != meta.UNKNOWN {
			ticker = pureCode[:len(pureCode)-3]
			exchange = ex
		}
	}

	if exchange == meta.UNKNOWN {
		switch len(pureCode) {
		case 4:
			if isAllAlpha(pureCode) {
				return meta.Instrument{Exchange: meta.USA, Type: meta.InstrumentTypeStock, Ticker: pureCode}
			}
		case 5:
			if isAllDigit(pureCode) {
				return meta.Instrument{Exchange: meta.HKEX, Type: meta.InstrumentTypeStock, Ticker: pureCode}
			}
		case 6:
			// 1. 全局规则优先匹配
			cr := ticker_rules.MatchRule(pureCode, ticker_rules.GlobalRules())
			if cr.Exchange != meta.UNKNOWN {
				return meta.Instrument{Exchange: cr.Exchange, Type: cr.InstrumentType, Ticker: pureCode}
			}
			// 2.1 0, 159和3开头, 优先匹配深交所
			if strings.HasPrefix(pureCode, "0") || strings.HasPrefix(pureCode, "159") || strings.HasPrefix(pureCode, "3") {
				cr = ticker_rules.MatchRule(pureCode, ticker_rules.SzseRules())
				if cr.Exchange != meta.UNKNOWN {
					return meta.Instrument{Exchange: cr.Exchange, Type: cr.InstrumentType, Ticker: pureCode}
				}
			}
			// 2.2 6和5开头, 优先匹配上交所
			if strings.HasPrefix(pureCode, "6") || strings.HasPrefix(pureCode, "5") {
				cr = ticker_rules.MatchRule(pureCode, ticker_rules.SseRules())
				if cr.Exchange != meta.UNKNOWN {
					return meta.Instrument{Exchange: cr.Exchange, Type: cr.InstrumentType, Ticker: pureCode}
				}
			}
			// 2.3 匹配上交所
			cr = ticker_rules.MatchRule(pureCode, ticker_rules.SseRules())
			if cr.Exchange != meta.UNKNOWN {
				return meta.Instrument{Exchange: cr.Exchange, Type: cr.InstrumentType, Ticker: pureCode}
			}
			// 2.4 匹配深交所
			cr = ticker_rules.MatchRule(pureCode, ticker_rules.SzseRules())
			if cr.Exchange != meta.UNKNOWN {
				return meta.Instrument{Exchange: cr.Exchange, Type: cr.InstrumentType, Ticker: pureCode}
			}
			// 2.5 匹配北交所
			cr = ticker_rules.MatchRule(pureCode, ticker_rules.BseRules())
			if cr.Exchange != meta.UNKNOWN {
				return meta.Instrument{Exchange: cr.Exchange, Type: cr.InstrumentType, Ticker: pureCode}
			}
		default:
			exchange = meta.UNKNOWN
			typ = meta.InstrumentTypeUnknown
		}
	}

	// 3. 如果exchange是UNKNOWN, 则返回未知规则
	if exchange == meta.UNKNOWN {
		return meta.Instrument{Exchange: meta.UNKNOWN, Type: meta.InstrumentTypeUnknown}
	}

	if typ == meta.InstrumentTypeUnknown {
		var rules []ticker_rules.CodeRule
		switch exchange {
		case meta.SSE:
			rules = ticker_rules.SseRules()
		case meta.SZSE:
			rules = ticker_rules.SzseRules()
		case meta.BSE:
			rules = ticker_rules.BseRules()
		case meta.HKEX:
			rules = ticker_rules.HkexRules()
		case meta.USA:
			rules = ticker_rules.UsaRules()
		default:
			return meta.Instrument{Exchange: meta.UNKNOWN, Type: meta.InstrumentTypeUnknown}
		}
		cr := ticker_rules.MatchRule(ticker, rules)
		if cr.InstrumentType != meta.InstrumentTypeUnknown {
			return meta.Instrument{Exchange: cr.Exchange, Type: cr.InstrumentType, Ticker: ticker}
		}
		return meta.Instrument{Exchange: meta.UNKNOWN, Type: meta.InstrumentTypeUnknown}
	}
	return meta.Instrument{Exchange: exchange, Type: typ, Ticker: ticker}
}

// ============================================================
// CorrectSecurityCode — corrects the format of a security code.
// ============================================================

// CorrectSecurityCode corrects the format of a security code string.
// Supports prefix (sh600000), suffix (600000.sh, AAPL.us), and raw (600000) formats.
// Returns the corrected code or the original on failure.
func CorrectSecurityCode(code string) string {
	code = strings.TrimSpace(code)
	if code == "" {
		return code
	}

	// Already has correct form (lowercase prefix + digits, e.g., "sh600000")
	lower := strings.ToLower(code)
	if len(lower) >= 8 {
		prefix := lower[:2]
		if (prefix == "sh" || prefix == "sz" || prefix == "bj") && isAllDigits(lower[2:]) {
			return lower
		}
	}

	// Suffix form: e.g., "600000.sh" → "sh600000"
	if len(lower) > 3 && lower[len(lower)-3] == '.' {
		suffix := lower[len(lower)-2:]
		ticker := lower[:len(lower)-3]
		switch suffix {
		case "sh":
			return "sh" + ticker
		case "sz":
			return "sz" + ticker
		case "bj":
			return "bj" + ticker
		default:
			return lower // non-CN markets keep suffix form
		}
	}

	// Raw 6-digit code: infer exchange
	if len(lower) == 6 && isAllDigits(lower) {
		switch {
		case lower[0] == '6' || lower[0] == '5' || lower[0] == '9' || strings.HasPrefix(lower, "88"):
			return "sh" + lower
		default:
			return "sz" + lower
		}
	}

	// Raw 4-digit code (BSE)
	if len(lower) == 4 && isAllDigits(lower) {
		if lower[0] == '8' || lower[0] == '4' {
			return "bj" + lower
		}
	}

	// US-style: 4 uppercase letters → keep as-is or add .us
	if len(lower) <= 5 && isAllAlpha(lower) {
		return lower
	}

	return lower
}

func isAllDigits(s string) bool {
	for _, c := range s {
		if c < '0' || c > '9' {
			return false
		}
	}
	return true
}

func isAllAlpha(s string) bool {
	if s == "" {
		return false
	}
	for _, c := range s {
		if (c < 'a' || c > 'z') && (c < 'A' || c > 'Z') {
			return false
		}
	}
	return true
}

func isAllDigit(s string) bool {
	if s == "" {
		return false
	}
	for _, c := range s {
		if c < '0' || c > '9' {
			return false
		}
	}
	return true
}
