package std

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"strings"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
	"github.com/quant1x/quant1x/quant1x/data/meta"
)

type BarFreq uint8

const (
	Freq5Min BarFreq = iota
	Freq15Min
	Freq30Min
	Freq1Hour
	FreqDaily
	FreqWeekly
	FreqMonthly
	FreqExHQ1Min
	Freq1Min
	FreqRIK
	Freq3Month
	FreqYearly
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
	UpCount   uint16  // 上涨家数(仅指数K线)
	DownCount uint16  // 下跌家数(仅指数K线)
}

// SecurityBarsContext 对齐 C++/Rust/Python SecurityBarsContext, 合并请求和响应.
type SecurityBarsContext struct {
	tdxproto.FrameBase
	Param    SecurityBarsParameter
	Padding  []byte
	IsIndex  bool
	Category uint16 // 保存category用于响应解析
	Count    uint16
	List     []SecurityBar
}

// NewSecurityBarsContext 构造K线请求, 对齐 C++/Rust.
func NewSecurityBarsContext(inst meta.Instrument, category BarFreq, start, count uint16) *SecurityBarsContext {
	if count == 0 || count > SecurityBarsMax {
		count = SecurityBarsMax
	}

	var code [6]byte
	copy(code[:], inst.Ticker)

	param := SecurityBarsParameter{
		Market:   uint16(tdxproto.ExchangeToMarketId(inst.Exchange)),
		Code:     code,
		Category: uint16(category),
		I:        1,
		Start:    start,
		Count:    count,
	}

	return &SecurityBarsContext{
		Param:    param,
		Padding:  make([]byte, 10),
		IsIndex:  inst.Type.IsIndex(),
		Category: uint16(category),
	}
}

// SerializeRequestBody 序列化请求体, 对齐 C++/Rust/Python.
func (s *SecurityBarsContext) SerializeRequestBody() []byte {
	payload := &bytes.Buffer{}
	_ = binary.Write(payload, binary.LittleEndian, s.Param.Market)
	payload.Write(s.Param.Code[:])
	_ = binary.Write(payload, binary.LittleEndian, s.Param.Category)
	_ = binary.Write(payload, binary.LittleEndian, s.Param.I)
	_ = binary.Write(payload, binary.LittleEndian, s.Param.Start)
	_ = binary.Write(payload, binary.LittleEndian, s.Param.Count)
	payload.Write(s.Padding)
	return payload.Bytes()
}

// DeserializeResponseBody 解析K线响应体, 对齐 C++/Rust/Python.
func (s *SecurityBarsContext) DeserializeResponseBody(body []byte) error {
	reader := bytes.NewReader(body)
	if err := binary.Read(reader, binary.LittleEndian, &s.Count); err != nil {
		return err
	}

	if cap(s.List) < int(s.Count) {
		s.List = make([]SecurityBar, 0, int(s.Count))
	} else {
		s.List = s.List[:0]
	}

	var preDiffBase int64
	for i := 0; i < int(s.Count); i++ {
		bar, err := s.parseBar(reader, &preDiffBase)
		if err != nil {
			return err
		}
		s.List = append(s.List, bar)
	}
	return nil
}

func (s *SecurityBarsContext) parseBar(reader *bytes.Reader, preDiffBase *int64) (SecurityBar, error) {
	var bar SecurityBar
	var zipday32 uint32
	var tminutes uint16
	if s.Category < 4 || s.Category == 7 || s.Category == 8 {
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
	year, month, day, hour, minute := tdxproto.GetDatetimeFromUint32(int(s.Category), zipday32, tminutes)
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
	bar.Vol = tdxproto.IntegerToFloat64(volRaw)

	var amountRaw uint32
	if err := binary.Read(reader, binary.LittleEndian, &amountRaw); err != nil {
		return bar, err
	}
	bar.Amount = tdxproto.IntegerToFloat64(amountRaw)

	base := *preDiffBase + openDiff
	bar.Open = float64(base) / 1000.0
	bar.Close = float64(base+closeDiff) / 1000.0
	bar.High = float64(base+highDiff) / 1000.0
	bar.Low = float64(base+lowDiff) / 1000.0

	*preDiffBase = base + closeDiff

	if s.IsIndex {
		if err := binary.Read(reader, binary.LittleEndian, &bar.UpCount); err != nil {
			return bar, err
		}
		if err := binary.Read(reader, binary.LittleEndian, &bar.DownCount); err != nil {
			return bar, err
		}
	}

	return bar, nil
}

func (s *SecurityBarsContext) String() string {
	code := strings.TrimRight(string(s.Param.Code[:]), "\x00 ")
	return fmt.Sprintf("SecurityBarsContext{Market:%d,Code:%s,Category:%d,Start:%d,Count:%d,ListLen:%d}",
		s.Param.Market, code, s.Param.Category, s.Param.Start, s.Param.Count, len(s.List))
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
func klineTypeToString(t BarFreq) string {
	switch t {
	case Freq5Min:
		return "5MIN"
	case Freq15Min:
		return "15MIN"
	case Freq30Min:
		return "30MIN"
	case Freq1Hour:
		return "1HOUR"
	case FreqDaily:
		return "DAILY"
	case FreqWeekly:
		return "WEEKLY"
	case FreqMonthly:
		return "MONTHLY"
	case FreqExHQ1Min:
		return "EXHQ_1MIN"
	case Freq1Min:
		return "1MIN"
	case FreqRIK:
		return "RI_K"
	case Freq3Month:
		return "3MONTH"
	case FreqYearly:
		return "YEARLY"
	default:
		return "UNKNOWN_KLINE"
	}
}
