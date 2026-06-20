// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// market — 市场/证券代码识别与纠正, 与 Python data/market.py 对齐

package data

import (
	"fmt"
	"strings"

	"github.com/quant1x/quant1x/quant1x/data/meta"
	"github.com/quant1x/quant1x/quant1x/std"
)

// ============================================================
// Type aliases from meta package (convenience re-exports).
// ============================================================

type Exchange = meta.Exchange
type InstrumentInfo = meta.Instrument
type Timestamp = meta.Timestamp
type InstrumentType = meta.InstrumentType

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
	NowTimestamp       = meta.NowTimestamp
	PreMarketTimestamp = meta.PreMarketTimestamp
	ParseTimestamp     = meta.ParseTimestamp
	LastTradingDay     = meta.LastTradingDay
	NewTimestampFromString = meta.NewTimestampFromString
	DateRange          = meta.DateRange
)

// ============================================================
// String/Bytes helpers (delegated to std).
// ============================================================

// String2Bytes converts a string to a byte slice.
func String2Bytes(s string) []byte {
	return std.String2Bytes(s)
}

// Bytes2String converts a byte slice to a string.
func Bytes2String(b []byte) string {
	return std.Bytes2String(b)
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
	for _, c := range s {
		if (c < 'a' || c > 'z') && (c < 'A' || c > 'Z') {
			return false
		}
	}
	return true
}
