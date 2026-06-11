package level1

import (
	"fmt"
	"math"
	"strings"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
)

const (
	tmHWidth = 1000000
	tmMWidth = 10000
)

// getDatetimeFromUint32 mirrors the C++ helper and returns year, month, day, hour, minute.
func getDatetimeFromUint32(category int, zipday uint32, tminutes uint16) (int, int, int, int, int) {
	year, month, day := 0, 0, 0
	hour, minute := 15, 0

	if category < 4 || category == 7 || category == 8 {
		year = int(zipday>>11) + 2004
		month = int(zipday%2048) / 100
		day = int(zipday%2048) % 100
		hour = int(tminutes / 60)
		minute = int(tminutes % 60)
	} else {
		year = int(zipday / 10000)
		month = int(zipday%10000) / 100
		day = int(zipday % 100)
	}

	return year, month, day, hour, minute
}

// varintEncode encodes a signed integer using the Level1 variable length scheme.
func varintEncode(value int64, buffer []byte, pos *int) int {
	sign := value < 0
	absValue := uint64(value)
	if sign {
		absValue = uint64(-value)
	}

	first := byte(absValue & 0x3F)
	absValue >>= 6
	if sign {
		first |= 0x40
	}
	if absValue != 0 {
		first |= 0x80
	}

	idx := *pos
	buffer[idx] = first
	idx++
	count := 1

	for absValue != 0 {
		b := byte(absValue & 0x7F)
		absValue >>= 7
		if absValue != 0 {
			b |= 0x80
		}
		buffer[idx] = b
		idx++
		count++
	}

	*pos = idx
	return count
}

// varintDecode decodes a value produced by varintEncode.
func varintDecode(b []byte, pos *int) int64 {
	idx := *pos
	if idx >= len(b) {
		panic("varint decode out of range")
	}

	current := b[idx]
	idx++
	sign := (current & 0x40) != 0
	data := int64(current & 0x3F)
	shift := 6

	for current&0x80 != 0 {
		if idx >= len(b) {
			panic("varint decode overflow")
		}
		current = b[idx]
		idx++
		data |= int64(current&0x7F) << shift
		shift += 7
	}

	*pos = idx
	if sign {
		return -data
	}
	return data
}

// formatTimestamp converts the packed timestamp used in snapshots into HH:mm:ss.SSS.
func formatTimestamp(stamp int64) string {
	h := stamp / tmHWidth
	tmp1 := stamp % tmHWidth
	m1 := tmp1 / tmMWidth
	tmp2 := tmp1 % tmMWidth

	if h > 100 {
		h /= 10
	}

	var m int64
	var st float64

	if m1 < 60 {
		m = m1
		tmp3 := tmp2 * 60
		st = float64(tmp3) / tmMWidth
	} else {
		h++
		tmp3 := tmp1
		m = tmp3 / tmHWidth
		tmp3 = (tmp3 % tmHWidth) * 60
		st = float64(tmp3) / tmHWidth
	}

	return fmt.Sprintf("%02d:%02d:%06.3f", h, m, st)
}

type intLike interface {
	~int | ~int8 | ~int16 | ~int32 | ~int64 | ~uint | ~uint8 | ~uint16 | ~uint32 | ~uint64
}

// integerToFloat64 reconstructs the floating point value encoded in a 32-bit integer.
func integerToFloat64[T intLike](integer T) float64 {
	uinteger := uint32(uint64(integer))

	logPoint := int((uinteger >> 24) & 0xFF)
	hleax := int((uinteger >> 16) & 0xFF)
	lheax := int((uinteger >> 8) & 0xFF)
	lleax := int(uinteger & 0xFF)

	dwEcx := logPoint*2 - 0x7F
	dwEdx := logPoint*2 - 0x86
	dwEsi := logPoint*2 - 0x8E
	dwEax := logPoint*2 - 0x96

	tmpEax := dwEcx
	if tmpEax < 0 {
		tmpEax = -tmpEax
	}
	dblXmm6 := math.Pow(2.0, float64(tmpEax))
	if dwEcx < 0 {
		dblXmm6 = 1.0 / dblXmm6
	}

	var dblXmm4 float64
	if hleax > 0x80 {
		dwtmpEax := dwEdx + 1
		tmpdblXmm3 := math.Pow(2.0, float64(dwtmpEax))
		dblXmm0 := math.Pow(2.0, float64(dwEdx)) * 128.0
		dblXmm0 += float64(hleax&0x7F) * tmpdblXmm3
		dblXmm4 = dblXmm0
	} else {
		if dwEdx >= 0 {
			dblXmm4 = math.Pow(2.0, float64(dwEdx)) * float64(hleax)
		} else {
			dblXmm4 = (1.0 / math.Pow(2.0, float64(-dwEdx))) * float64(hleax)
		}
	}

	dblXmm3 := math.Pow(2.0, float64(dwEsi)) * float64(lheax)
	dblXmm1 := math.Pow(2.0, float64(dwEax)) * float64(lleax)

	if hleax&0x80 != 0 {
		dblXmm3 *= 2.0
		dblXmm1 *= 2.0
	}

	return dblXmm6 + dblXmm4 + dblXmm3 + dblXmm1
}

// float64IsNaN reports whether f is NaN or Inf.
func float64IsNaN(f float64) bool {
	return math.IsNaN(f) || math.IsInf(f, 0)
}

type numeric interface {
	~uint16 | ~uint32 | ~float32
}

// numberToFloat64 converts supported numeric types to float64.
func numberToFloat64[T numeric](v T) float64 {
	return float64(v)
}

const (
	marketShenZhen = 0
	marketShangHai = 1
	marketBeiJing  = 2
)

// exchangeToMarketId 根据交易所枚举返回对应的市场ID
// 参数:
//
//	exchange: 交易所枚举值
//
// 返回值:
//
//	对应市场的整型ID，如果交易所不匹配则返回-1
func exchangeToMarketId(ex exchange.Exchange) int {
	switch ex {
	case exchange.ExchangeSZSE:
		return marketShenZhen
	case exchange.ExchangeSSE:
		return marketShangHai
	case exchange.ExchangeBSE:
		return marketBeiJing
	default:
		return -1
	}
}

func marketIdToExchange(marketID int) exchange.Exchange {
	switch marketID {
	case marketShenZhen:
		return exchange.ExchangeSZSE
	case marketShangHai:
		return exchange.ExchangeSSE
	case marketBeiJing:
		return exchange.ExchangeBSE
	default:
		return exchange.ExchangeUnknown
	}
}

// defaultBaseUnit returns the lot size heuristic used by Level1 helpers.
func defaultBaseUnit(marketID int, code string) float64 {
	if len(code) == 0 {
		return 100.0
	}

	if (marketID == marketShangHai && code[0] == '5') || (marketID == marketShenZhen && strings.HasPrefix(code, "159")) {
		return 1000.0
	}

	return 100.0
}

// instrumentsToString 将一组InstrumentInfo转换为逗号分隔的字符串，格式为"ticker:exchangeId,ticker:exchangeId..."
// 自动去除结果字符串开头的逗号
func instrumentsToString(codes []exchange.InstrumentInfo) string {
	var result strings.Builder
	for _, code := range codes {
		result.WriteString(fmt.Sprintf(",%s:%d", code.Ticker, exchangeToMarketId(code.Exchange)))
	}
	return result.String()[1:]
}
