package std

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"strings"

	"github.com/quant1x/quant1x/quant1x/data/exchange"
)

type KLineType uint8

const (
	KLine5Min KLineType = iota
	KLine15Min
	KLine30Min
	KLine1Hour
	KLineDaily
	KLineWeekly
	KLineMonthly
	KLineExHQ1Min
	KLine1Min
	KLineRIK
	KLine3Month
	KLineYearly
)

// SecurityBarsMax defines the maximum number of bars retrievable in one request.
const SecurityBarsMax = 800

// SecurityBarsParameter mirrors the packed request payload used by the C++ client.
type SecurityBarsParameter struct {
	Market   uint16  // Market identifier
	Code     [6]byte // Security code, padded to 6 bytes
	Category uint16  // K-line category/type
	I        uint16  // Fixed value, typically set to 1
	Start    uint16  // Starting index for the bars to retrieve
	Count    uint16  // Number of bars to retrieve
}

// SecurityBar represents a single K-line entry.
type SecurityBar struct {
	Open      float64 // 开盘价
	Close     float64 // 收盘价
	High      float64 // 最高价
	Low       float64 // 最低价
	Vol       float64 // 成交量
	Amount    float64 // 成交额
	Year      int     // 年
	Month     int     // 月
	Day       int     // 日
	Hour      int     // 时
	Minute    int     // 分
	DateTime  string  // 日期时间字符串
	UpCount   uint16  // 上涨家数（仅指数K线）
	DownCount uint16  // 下跌家数（仅指数K线）
}

// SecurityBarsRequest encodes a SECURITY_BARS command.
type SecurityBarsRequest struct {
	Param   SecurityBarsParameter
	Padding []byte
	IsIndex bool
}

// NewSecurityBarsRequest constructs a request aligned with the C++ structure.
func NewSecurityBarsRequest(sc exchange.InstrumentInfo, category KLineType, start, count uint16) SecurityBarsRequest {
	if count == 0 || count > SecurityBarsMax {
		count = SecurityBarsMax
	}

	var code [6]byte
	copy(code[:], sc.Ticker)

	param := SecurityBarsParameter{
		Market:   uint16(exchangeToMarketId(sc.Exchange)),
		Code:     code,
		Category: uint16(category),
		I:        1,
		Start:    start,
		Count:    count,
	}

	req := SecurityBarsRequest{
		Param:   param,
		Padding: make([]byte, 10),
	}
	req.IsIndex = sc.Type.IsIndex()
	return req
}

// Serialize serializes the request payload.
func (r SecurityBarsRequest) Serialize() []byte {
	payload := &bytes.Buffer{}
	_ = binary.Write(payload, binary.LittleEndian, r.Param.Market)
	payload.Write(r.Param.Code[:])
	_ = binary.Write(payload, binary.LittleEndian, r.Param.Category)
	_ = binary.Write(payload, binary.LittleEndian, r.Param.I)
	_ = binary.Write(payload, binary.LittleEndian, r.Param.Start)
	_ = binary.Write(payload, binary.LittleEndian, r.Param.Count)
	payload.Write(r.Padding)
	return buildRequest(StdCommandSecurityBars, packetTypeRequest, payload.Bytes())
}

// Command returns the associated StdCommand.
func (r SecurityBarsRequest) Command() StdCommand { return StdCommandSecurityBars }

// String provides a readable representation.
func (r SecurityBarsRequest) String() string {
	code := strings.TrimRight(string(r.Param.Code[:]), "\x00 ")
	return fmt.Sprintf("SecurityBarsRequest{Market:%d,Code:%s,Category:%d,Start:%d,Count:%d}", r.Param.Market, code, r.Param.Category, r.Param.Start, r.Param.Count)
}

// SecurityBarsResponse captures the parsed response body.
type SecurityBarsResponse struct {
	ResponseBase
	Count    uint16
	List     []SecurityBar
	isIndex  bool
	category uint16
}

// NewSecurityBarsResponse prepares a response helper with index/category flags.
func NewSecurityBarsResponse(isIndex bool, category uint16) *SecurityBarsResponse {
	return &SecurityBarsResponse{isIndex: isIndex, category: category}
}

// Deserialize decodes the Level1 response body.
func (r *SecurityBarsResponse) Deserialize(body []byte) error {
	reader := bytes.NewReader(body)
	if err := binary.Read(reader, binary.LittleEndian, &r.Count); err != nil {
		return err
	}

	if cap(r.List) < int(r.Count) {
		r.List = make([]SecurityBar, 0, int(r.Count))
	} else {
		r.List = r.List[:0]
	}

	var preDiffBase int64
	for i := 0; i < int(r.Count); i++ {
		bar, err := r.parseBar(reader, &preDiffBase)
		if err != nil {
			return err
		}
		r.List = append(r.List, bar)
	}
	return nil
}

func (r *SecurityBarsResponse) parseBar(reader *bytes.Reader, preDiffBase *int64) (SecurityBar, error) {
	var bar SecurityBar
	var zipday32 uint32
	var tminutes uint16
	if r.category < 4 || r.category == 7 || r.category == 8 {
		var zipday16 uint16
		if err := binary.Read(reader, binary.LittleEndian, &zipday16); err != nil {
			return bar, err
		}
		zipday32 = uint32(zipday16)
		if err := binary.Read(reader, binary.LittleEndian, &tminutes); err != nil {
			return bar, err
		}
	} else {
		if err := binary.Read(reader, binary.LittleEndian, &zipday32); err != nil {
			return bar, err
		}
	}
	year, month, day, hour, minute := getDatetimeFromUint32(int(r.category), zipday32, tminutes)
	bar.Year, bar.Month, bar.Day, bar.Hour, bar.Minute = year, month, day, hour, minute
	bar.DateTime = fmt.Sprintf("%04d-%02d-%02d %02d:%02d:00", year, month, day, hour, minute)

	openDiff, err := varintRead(reader)
	if err != nil {
		return bar, err
	}
	closeDiff, err := varintRead(reader)
	if err != nil {
		return bar, err
	}
	highDiff, err := varintRead(reader)
	if err != nil {
		return bar, err
	}
	lowDiff, err := varintRead(reader)
	if err != nil {
		return bar, err
	}

	var volRaw uint32
	if err := binary.Read(reader, binary.LittleEndian, &volRaw); err != nil {
		return bar, err
	}
	bar.Vol = integerToFloat64(volRaw)

	var amountRaw uint32
	if err := binary.Read(reader, binary.LittleEndian, &amountRaw); err != nil {
		return bar, err
	}
	bar.Amount = integerToFloat64(amountRaw)

	base := *preDiffBase + openDiff
	bar.Open = float64(base) / 1000.0
	bar.Close = float64(base+closeDiff) / 1000.0
	bar.High = float64(base+highDiff) / 1000.0
	bar.Low = float64(base+lowDiff) / 1000.0

	*preDiffBase = base + closeDiff

	if r.isIndex {
		if err := binary.Read(reader, binary.LittleEndian, &bar.UpCount); err != nil {
			return bar, err
		}
		if err := binary.Read(reader, binary.LittleEndian, &bar.DownCount); err != nil {
			return bar, err
		}
	}

	return bar, nil
}

// String summarises the response.
func (r *SecurityBarsResponse) String() string {
	return fmt.Sprintf("SecurityBarsResponse{Count:%d}", r.Count)
}

func varintRead(reader *bytes.Reader) (int64, error) {
	first, err := reader.ReadByte()
	if err != nil {
		return 0, err
	}
	sign := (first & 0x40) != 0
	data := int64(first & 0x3F)
	shift := 6

	current := first
	for current&0x80 != 0 {
		current, err = reader.ReadByte()
		if err != nil {
			return 0, err
		}
		data |= int64(current&0x7F) << shift
		shift += 7
	}

	if sign {
		return -data, nil
	}
	return data, nil
}

// klineTypeToString assists debugging and mirrors the C++ helper.
func klineTypeToString(t KLineType) string {
	switch t {
	case KLine5Min:
		return "5MIN"
	case KLine15Min:
		return "15MIN"
	case KLine30Min:
		return "30MIN"
	case KLine1Hour:
		return "1HOUR"
	case KLineDaily:
		return "DAILY"
	case KLineWeekly:
		return "WEEKLY"
	case KLineMonthly:
		return "MONTHLY"
	case KLineExHQ1Min:
		return "EXHQ_1MIN"
	case KLine1Min:
		return "1MIN"
	case KLineRIK:
		return "RI_K"
	case KLine3Month:
		return "3MONTH"
	case KLineYearly:
		return "YEARLY"
	default:
		return "UNKNOWN_KLINE"
	}
}
