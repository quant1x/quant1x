package std

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"math"
	"strings"
	"time"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
	quant "github.com/quant1x/quant1x/quant1x/data"
)

// TradeState mirrors the C++ TradeState enum.
type TradeState uint8

const (
	TradeDelisting TradeState = iota
	TradeNormal
	TradeSuspend
	TradeIPO
)

// Level represents a price/volume level.
type Level struct {
	Price float64
	Vol   int64
}

// SpreadLevel mirrors the C++ SpreadLevel enum.
type SpreadLevel uint8

const (
	SpreadVeryLow SpreadLevel = iota
	SpreadLow
	SpreadMedium
	SpreadHigh
	SpreadVeryHigh
)

const (
	SPREAD_PCT_VERY_LOW = 0.05
	SPREAD_PCT_LOW      = 0.2
	SPREAD_PCT_MEDIUM   = 0.8
	SPREAD_PCT_HIGH     = 2.0
)

type StockInfo struct {
	Market uint8
	Code   string
}

// SecurityQuote mirrors a single quote entry in the response.
type SecurityQuote struct {
	State           TradeState
	Market          uint8
	Code            string
	Active1         uint16
	Price           float64
	LastClose       float64
	Open            float64
	High            float64
	Low             float64
	ServerTime      string
	ReversedBytes0  int64
	ReversedBytes1  int64
	Vol             int64
	CurVol          int64
	Amount          float64
	SVol            int64
	BVol            int64
	IndexOpenAmount int64
	StockOpenAmount int64
	OpenVolume      int64
	CloseVolume     int64
	IndexUp         int64
	IndexUpLimit    int64
	IndexDown       int64
	IndexDownLimit  int64

	Bid1    float64
	Ask1    float64
	BidVol1 int64
	AskVol1 int64

	Bid2    float64
	Ask2    float64
	BidVol2 int64
	AskVol2 int64

	Bid3    float64
	Ask3    float64
	BidVol3 int64
	AskVol3 int64

	Bid4    float64
	Ask4    float64
	BidVol4 int64
	AskVol4 int64

	Bid5    float64
	Ask5    float64
	BidVol5 int64
	AskVol5 int64

	ReversedBytes4 uint16
	ReversedBytes5 int64
	ReversedBytes6 int64
	ReversedBytes7 int64
	ReversedBytes8 int64

	Rate      float64
	Active2   uint16
	TimeStamp string
}

func (q *SecurityQuote) ImplicitSpread() float64 {
	if math.IsNaN(q.Price) || q.Price <= 0.0 {
		if q.Ask1 > 0.0 && q.Bid1 > 0.0 {
			return q.Ask1 - q.Bid1
		}
		return 0.0
	}
	if q.Ask1 > 0.0 && q.Bid1 > 0.0 {
		mid := (q.Ask1 + q.Bid1) / 2.0
		return 2.0 * math.Abs(q.Price-mid)
	}
	if q.Ask1 > 0.0 && q.Bid1 > 0.0 {
		return q.Ask1 - q.Bid1
	}
	return 0.0
}

func (q *SecurityQuote) ImplicitSpreadPct() float64 {
	if q.Ask1 > 0.0 && q.Bid1 > 0.0 {
		mid := (q.Ask1 + q.Bid1) / 2.0
		s := q.ImplicitSpread()
		if mid > 0.0 {
			return s / mid * 100.0
		}
	}
	if q.LastClose > 0.0 {
		s := q.ImplicitSpread()
		return s / q.LastClose * 100.0
	}
	return 0.0
}

func (q *SecurityQuote) ImplicitSpreadLevel() SpreadLevel {
	pct := q.ImplicitSpreadPct()
	if pct < SPREAD_PCT_VERY_LOW {
		return SpreadVeryLow
	}
	if pct < SPREAD_PCT_LOW {
		return SpreadLow
	}
	if pct < SPREAD_PCT_MEDIUM {
		return SpreadMedium
	}
	if pct < SPREAD_PCT_HIGH {
		return SpreadHigh
	}
	return SpreadVeryHigh
}

func getPrice(baseUnit float64, price int64, diff int64) float64 {
	return float64(price+diff) / baseUnit
}

// SecurityQuoteContext 对齐 C++/Rust/Python SecurityQuoteContext, 合并请求和响应.
type SecurityQuoteContext struct {
	tdxproto.FrameBase
	Padding    []byte
	StockList  []StockInfo
	RespCount  uint16
	QuoteList  []SecurityQuote
}

// NewSecurityQuoteContext 构造行情请求, 对齐 C++/Rust.
func NewSecurityQuoteContext(list []StockInfo) *SecurityQuoteContext {
	return &SecurityQuoteContext{
		FrameBase: tdxproto.NewFrameBase(tdxproto.StdCommandSecurityQuotesOld, tdxproto.FlagUncompressed, tdxproto.PacketTypeRequest),
		StockList: list,
		Padding:   []byte{0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	}
}

// SerializeRequestBody 序列化请求体, 对齐 C++/Rust/Python.
func (s *SecurityQuoteContext) SerializeRequestBody() []byte {
	payload := &bytes.Buffer{}
	if len(s.Padding) == 0 {
		s.Padding = []byte{0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
	}
	payload.Write(s.Padding)
	count := uint16(len(s.StockList))
	_ = binary.Write(payload, binary.LittleEndian, count)
	for _, v := range s.StockList {
		payload.WriteByte(v.Market)
		codeBytes := []byte(v.Code)
		if len(codeBytes) > 6 {
			codeBytes = codeBytes[:6]
		}
		if len(codeBytes) < 6 {
			pad := make([]byte, 6-len(codeBytes))
			codeBytes = append(codeBytes, pad...)
		}
		payload.Write(codeBytes)
	}
	return payload.Bytes()
}

// DeserializeResponseBody 解析行情响应体, 对齐 C++/Rust/Python.
func (s *SecurityQuoteContext) DeserializeResponseBody(data []byte) error {
	if len(data) < 4 {
		return fmt.Errorf("data too short")
	}
	pos := 2 // skip 2 bytes as in C++ implementation
	if pos+2 > len(data) {
		return fmt.Errorf("truncated data")
	}
	s.RespCount = binary.LittleEndian.Uint16(data[pos : pos+2])
	pos += 2
	s.QuoteList = make([]SecurityQuote, 0, s.RespCount)
	now := time.Now()
	timestamp := now.Format("20060102150405") + fmt.Sprintf("%03d", now.Nanosecond()/1e6)

	for i := 0; i < int(s.RespCount); i++ {
		var ele SecurityQuote
		if pos >= len(data) {
			return fmt.Errorf("unexpected EOF")
		}
		ele.Market = data[pos]
		pos++
		if pos+6 > len(data) {
			return fmt.Errorf("unexpected EOF for code")
		}
		code := string(data[pos : pos+6])
		code = strings.TrimRight(code, "\x00")
		ele.Code = code
		pos += 6

		baseUnit := tdxproto.DefaultBaseUnit(int(ele.Market), ele.Code)

		if pos+2 > len(data) {
			return fmt.Errorf("unexpected EOF for active1")
		}
		ele.Active1 = binary.LittleEndian.Uint16(data[pos : pos+2])
		pos += 2

		priceBase := tdxproto.VarintDecode(data, &pos)
		ele.Price = getPrice(baseUnit, priceBase, 0)

		tmp := tdxproto.VarintDecode(data, &pos)
		ele.LastClose = getPrice(baseUnit, priceBase, tmp)

		ele.Open = getPrice(baseUnit, priceBase, tdxproto.VarintDecode(data, &pos))
		ele.High = getPrice(baseUnit, priceBase, tdxproto.VarintDecode(data, &pos))
		ele.Low = getPrice(baseUnit, priceBase, tdxproto.VarintDecode(data, &pos))

		ele.ReversedBytes0 = tdxproto.VarintDecode(data, &pos)
		if ele.ReversedBytes0 > 0 {
			ele.ServerTime = tdxproto.FormatTimestampFromI64(ele.ReversedBytes0)
		} else {
			ele.ServerTime = "0"
		}

		ele.ReversedBytes1 = tdxproto.VarintDecode(data, &pos)

		vol := tdxproto.VarintDecode(data, &pos)
		ele.Vol = vol * 100

		ele.CurVol = tdxproto.VarintDecode(data, &pos)

		if pos+4 > len(data) {
			return fmt.Errorf("unexpected EOF amount")
		}
		rawAmount := binary.LittleEndian.Uint32(data[pos : pos+4])
		pos += 4
		ele.Amount = tdxproto.IntegerToFloat64(uint32(rawAmount))

		ele.SVol = tdxproto.VarintDecode(data, &pos)
		ele.BVol = tdxproto.VarintDecode(data, &pos)

		ele.IndexOpenAmount = tdxproto.VarintDecode(data, &pos) * 100
		ele.StockOpenAmount = tdxproto.VarintDecode(data, &pos) * 100
		ex := tdxproto.MarketIdToExchange(int(ele.Market))
		isIndexOrBlock := quant.AssertIndexByMarketAndCode(ex, ele.Code)

		var tmpOpenVolume float64
		if isIndexOrBlock {
			if ele.Open > 0 {
				tmpOpenVolume = math.Round(float64(ele.IndexOpenAmount) / ele.Open)
			}
		} else {
			if ele.Open > 0 {
				tmpOpenVolume = math.Round(float64(ele.StockOpenAmount) / ele.Open)
			}
		}
		if math.IsNaN(tmpOpenVolume) {
			tmpOpenVolume = 0.0
		}
		ele.OpenVolume = int64(tmpOpenVolume)

		var bidPrices [5]float64
		var askPrices [5]float64
		var bidVols [5]int64
		var askVols [5]int64
		for l := 0; l < 5; l++ {
			bidDiff := tdxproto.VarintDecode(data, &pos)
			askDiff := tdxproto.VarintDecode(data, &pos)
			bidVol := tdxproto.VarintDecode(data, &pos)
			askVol := tdxproto.VarintDecode(data, &pos)
			bidPrices[l] = getPrice(baseUnit, bidDiff, priceBase)
			askPrices[l] = getPrice(baseUnit, askDiff, priceBase)
			bidVols[l] = bidVol
			askVols[l] = askVol
		}
		ele.Bid1, ele.BidVol1 = bidPrices[0], bidVols[0]
		ele.Bid2, ele.BidVol2 = bidPrices[1], bidVols[1]
		ele.Bid3, ele.BidVol3 = bidPrices[2], bidVols[2]
		ele.Bid4, ele.BidVol4 = bidPrices[3], bidVols[3]
		ele.Bid5, ele.BidVol5 = bidPrices[4], bidVols[4]

		ele.Ask1, ele.AskVol1 = askPrices[0], askVols[0]
		ele.Ask2, ele.AskVol2 = askPrices[1], askVols[1]
		ele.Ask3, ele.AskVol3 = askPrices[2], askVols[2]
		ele.Ask4, ele.AskVol4 = askPrices[3], askVols[3]
		ele.Ask5, ele.AskVol5 = askPrices[4], askVols[4]

		ele.ReversedBytes4 = binary.LittleEndian.Uint16(data[pos : pos+2])
		pos += 2

		ele.ReversedBytes5 = tdxproto.VarintDecode(data, &pos)
		ele.ReversedBytes6 = tdxproto.VarintDecode(data, &pos)
		ele.ReversedBytes7 = tdxproto.VarintDecode(data, &pos)
		ele.ReversedBytes8 = tdxproto.VarintDecode(data, &pos)

		if pos+2 > len(data) {
			return fmt.Errorf("unexpected EOF for rate")
		}
		reversed9 := int16(binary.LittleEndian.Uint16(data[pos : pos+2]))
		pos += 2
		ele.Rate = float64(reversed9) / 100.0

		if pos+2 > len(data) {
			return fmt.Errorf("unexpected EOF for active2")
		}
		ele.Active2 = binary.LittleEndian.Uint16(data[pos : pos+2])
		pos += 2

		if ele.LastClose == 0.0 && ele.Open == 0.0 {
			ele.State = TradeDelisting
		} else {
			if ele.Open != 0.0 {
				ele.State = TradeNormal
			} else {
				ele.State = TradeSuspend
			}
		}

		if isIndexOrBlock {
			ele.IndexUp = ele.BidVol1
			ele.IndexDown = ele.AskVol1
			ele.IndexUpLimit = ele.BidVol2
			ele.IndexDownLimit = ele.AskVol2
		}

		ele.TimeStamp = timestamp
		s.QuoteList = append(s.QuoteList, ele)
	}

	return nil
}

func (s *SecurityQuoteContext) String() string {
	return fmt.Sprintf("SecurityQuoteContext{StockCount:%d, RespCount:%d}", len(s.StockList), s.RespCount)
}
