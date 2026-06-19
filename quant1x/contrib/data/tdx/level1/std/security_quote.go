package std

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"math"
	"strings"
	"time"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx"
	helpers "github.com/quant1x/quant1x/quant1x/contrib/data/tdx"
	"github.com/quant1x/quant1x/quant1x/data/exchange"
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

// StockInfo holds minimal security identity info.
type StockInfo struct {
	Market uint8
	Code   string
}

// SecurityQuoteContext is a lightweight representation of the C++ request.
// Full serialization is implemented elsewhere; we keep a simple structure here.
type SecurityQuoteContext struct {
	Padding []byte
	List    []StockInfo
}

// Serialize builds the SECURITY_QUOTES_OLD request payload.
func (r SecurityQuoteContext) Serialize() []byte {
	payload := &bytes.Buffer{}
	// padding: 8 bytes as in C++ implementation
	if len(r.Padding) == 0 {
		r.Padding = []byte{0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
	}
	payload.Write(r.Padding)
	// count
	count := uint16(len(r.List))
	_ = binary.Write(payload, binary.LittleEndian, count)
	for _, v := range r.List {
		payload.WriteByte(v.Market)
		// write code as 6 bytes, padded with 0
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
	return tdx.BuildRequest(tdx.StdCommandSecurityQuotesOld, tdx.PacketTypeRequest, payload.Bytes())
}

// Command returns the associated StdCommand.
func (r SecurityQuoteContext) Command() tdx.StdCommand { return tdx.StdCommandSecurityQuotesOld }

// String provides a short description.
func (r SecurityQuoteContext) String() string {
	return fmt.Sprintf("SecurityQuoteContext{Count:%d}", len(r.List))
}

// SecurityQuoteContext maps the C++ structure to Go.
type SecurityQuoteContext struct {
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

// ImplicitSpread computes the effective spread (price units).
func (q *SecurityQuoteContext) ImplicitSpread() float64 {
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

// ImplicitSpreadPct returns the spread as percentage.
func (q *SecurityQuoteContext) ImplicitSpreadPct() float64 {
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

// ImplicitSpreadLevel maps percentage to an enum.
func (q *SecurityQuoteContext) ImplicitSpreadLevel() SpreadLevel {
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

// SecurityQuoteResponse mirrors the C++ response header + list.
type SecurityQuoteResponse struct {
	Count uint16
	List  []SecurityQuoteContext
}

// getPrice helper mirrors C++ getPrice(baseUnit, price, diff)
func getPrice(baseUnit float64, price int64, diff int64) float64 {
	return float64(price+diff) / baseUnit
}

// Deserialize parses the binary payload into the response structure.
func (r *SecurityQuoteResponse) Deserialize(data []byte) error {
	if len(data) < 4 {
		return fmt.Errorf("data too short")
	}
	pos := 2 // skip 2 bytes as in C++ implementation
	if pos+2 > len(data) {
		return fmt.Errorf("truncated data")
	}
	r.Count = binary.LittleEndian.Uint16(data[pos : pos+2])
	pos += 2
	r.List = make([]SecurityQuoteContext, 0, r.Count)
	now := time.Now()
	timestamp := now.Format("20060102150405") + fmt.Sprintf("%03d", now.Nanosecond()/1e6)

	for i := 0; i < int(r.Count); i++ {
		var ele SecurityQuoteContext
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

		baseUnit := helpers.DefaultBaseUnit(int(ele.Market), ele.Code)

		if pos+2 > len(data) {
			return fmt.Errorf("unexpected EOF for active1")
		}
		ele.Active1 = binary.LittleEndian.Uint16(data[pos : pos+2])
		pos += 2

		priceBase := helpers.VarintDecode(data, &pos)
		ele.Price = getPrice(baseUnit, priceBase, 0)

		tmp := helpers.VarintDecode(data, &pos)
		ele.LastClose = getPrice(baseUnit, priceBase, tmp)

		ele.Open = getPrice(baseUnit, priceBase, helpers.VarintDecode(data, &pos))
		ele.High = getPrice(baseUnit, priceBase, helpers.VarintDecode(data, &pos))
		ele.Low = getPrice(baseUnit, priceBase, helpers.VarintDecode(data, &pos))

		ele.ReversedBytes0 = helpers.VarintDecode(data, &pos)
		if ele.ReversedBytes0 > 0 {
			ele.ServerTime = helpers.FormatTimestampFromI64(ele.ReversedBytes0)
		} else {
			ele.ServerTime = "0"
		}

		ele.ReversedBytes1 = helpers.VarintDecode(data, &pos)

		vol := helpers.VarintDecode(data, &pos)
		ele.Vol = vol * 100

		ele.CurVol = helpers.VarintDecode(data, &pos)

		if pos+4 > len(data) {
			return fmt.Errorf("unexpected EOF amount")
		}
		rawAmount := binary.LittleEndian.Uint32(data[pos : pos+4])
		pos += 4
		ele.Amount = helpers.IntegerToFloat64(uint32(rawAmount))

		ele.SVol = helpers.VarintDecode(data, &pos)
		ele.BVol = helpers.VarintDecode(data, &pos)

		ele.IndexOpenAmount = helpers.VarintDecode(data, &pos) * 100
		ele.StockOpenAmount = helpers.VarintDecode(data, &pos) * 100
		ex := helpers.MarketIdToExchange(int(ele.Market))
		isIndexOrBlock := exchange.AssertIndexByMarketAndCode(ex, ele.Code)

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

		// bid/ask levels
		var bidPrices [5]float64
		var askPrices [5]float64
		var bidVols [5]int64
		var askVols [5]int64
		for l := 0; l < 5; l++ {
			bidDiff := helpers.VarintDecode(data, &pos)
			askDiff := helpers.VarintDecode(data, &pos)
			bidVol := helpers.VarintDecode(data, &pos)
			askVol := helpers.VarintDecode(data, &pos)
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

		ele.ReversedBytes5 = helpers.VarintDecode(data, &pos)
		ele.ReversedBytes6 = helpers.VarintDecode(data, &pos)
		ele.ReversedBytes7 = helpers.VarintDecode(data, &pos)
		ele.ReversedBytes8 = helpers.VarintDecode(data, &pos)

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

		// state logic
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

		// closing logic omitted (requires exchange time status)

		ele.TimeStamp = timestamp
		r.List = append(r.List, ele)
	}

	return nil
}
