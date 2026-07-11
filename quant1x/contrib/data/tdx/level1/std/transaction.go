package std

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"strings"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/base"
)

const (
	TickTransactionPerRequestMax = 1800
)

type TransactionDirection int64

const (
	TickTransactionDirectionBuy  = 0 // 买盘
	TickTransactionDirectionSell = 1 // 卖盘
	TickTransactionDirectionNone = 2 // 中性盘
	TickTransactionAuctionClose  = 8 // 收盘集合竞价
)

// TickTransaction mirrors the C++ TickTransaction structure.
type TickTransaction struct {
	Time      string  // 成交时间 HH:MM
	Price     float64 // 成交价格
	Vol       int64   // 成交量(股)
	Num       int64   // 成交笔数
	Amount    float64 // 成交金额
	Direction int64   // 买卖方向
}

func (t TickTransaction) String() string {
	return fmt.Sprintf("time: %s price: %v vol: %d num: %d amount: %v direction: %d", t.Time, t.Price, t.Vol, t.Num, t.Amount, t.Direction)
}

type TransactionReply struct {
	Count uint16
	List  []TickTransaction
}

// TransactionContext 对齐 C++/Rust/Python TransactionContext, 合并请求和响应.
type TransactionContext struct {
	tdxproto.FrameBase
	Market uint16
	Code   [6]byte
	Start  uint16
	Count  uint16
	sc     data.InstrumentInfo
	Reply  TransactionReply
}

// NewTransactionContext 构造逐笔成交请求, 对齐 C++/Rust.
func NewTransactionContext(instrument data.InstrumentInfo, offset, size int) *TransactionContext {
	if size <= 0 || size > TickTransactionPerRequestMax {
		size = TickTransactionPerRequestMax
	}
	if offset < 0 {
		offset = 0
	}

	return &TransactionContext{
		FrameBase: tdxproto.NewFrameBase(tdxproto.StdCommandTransactionData, tdxproto.FlagUncompressed, tdxproto.PacketTypeRequest),
		Market:    uint16(tdxproto.ExchangeToMarketId(instrument.Exchange)),
		Code:      [6]byte(std.String2Bytes(instrument.Ticker)),
		Start:     uint16(offset),
		Count:     uint16(size),
		sc:        instrument,
	}
}

// SerializeRequestBody 序列化请求体, 对齐 C++/Rust/Python.
func (t *TransactionContext) SerializeRequestBody() []byte {
	payload := &bytes.Buffer{}
	_ = binary.Write(payload, binary.LittleEndian, t.Market)
	payload.Write(t.Code[:])
	_ = binary.Write(payload, binary.LittleEndian, t.Start)
	_ = binary.Write(payload, binary.LittleEndian, t.Count)
	return payload.Bytes()
}

// DeserializeResponseBody 解析逐笔成交响应体, 对齐 C++/Rust/Python.
func (t *TransactionContext) DeserializeResponseBody(body []byte) error {
	reader := bytes.NewReader(body)
	if err := binary.Read(reader, binary.LittleEndian, &t.Reply.Count); err != nil {
		return err
	}
	if cap(t.Reply.List) < int(t.Reply.Count) {
		t.Reply.List = make([]TickTransaction, 0, int(t.Reply.Count))
	} else {
		t.Reply.List = t.Reply.List[:0]
	}

	baseUnit := tdxproto.DefaultBaseUnit(int(tdxproto.ExchangeToMarketId(t.sc.Exchange)), t.sc.Ticker)
	isIndex := t.sc.Type.IsIndex()
	var lastPrice int64 = 0

	for i := 0; i < int(t.Reply.Count); i++ {
		var seconds uint16
		if err := binary.Read(reader, binary.LittleEndian, &seconds); err != nil {
			if err == io.EOF || err == io.ErrUnexpectedEOF {
				t.Reply.Count = uint16(len(t.Reply.List))
				return nil
			}
			return err
		}
		h := seconds / 60
		m := seconds % 60

		rawPrice, err := varintRead(reader)
		if err != nil {
			if err == io.EOF {
				t.Reply.Count = uint16(len(t.Reply.List))
				return nil
			}
			return err
		}
		vol, err := varintRead(reader)
		if err != nil {
			return err
		}
		num, err := varintRead(reader)
		if err != nil {
			return err
		}
		direction, err := varintRead(reader)
		if err != nil {
			return err
		}

		lastPrice += rawPrice
		price := float64(lastPrice) / baseUnit

		var ele TickTransaction
		ele.Time = fmt.Sprintf("%02d:%02d", h, m)
		ele.Price = price
		ele.Vol = vol
		ele.Num = num
		ele.Direction = direction

		if isIndex {
			amount := ele.Vol * 100
			ele.Amount = float64(amount)
			if ele.Price != 0 {
				ele.Vol = int64(ele.Amount / ele.Price)
			} else {
				ele.Vol = 0
			}
		} else {
			ele.Vol *= 100
			ele.Amount = float64(ele.Vol) * ele.Price
		}

		// skip reserved varint
		if _, err := varintRead(reader); err != nil {
			if err == io.EOF {
				t.Reply.List = append(t.Reply.List, ele)
				t.Reply.Count = uint16(len(t.Reply.List))
				return nil
			}
			return err
		}

		t.Reply.List = append(t.Reply.List, ele)
	}
	return nil
}

func (t *TransactionContext) String() string {
	code := strings.TrimRight(string(t.Code[:]), "\x00 ")
	return fmt.Sprintf("TransactionContext{Market:%d,Code:%s,Start:%d,Count:%d,ReplyCount:%d}", t.Market, code, t.Start, t.Count, t.Reply.Count)
}

// HistoryTransactionContext 对齐 C++/Rust/Python HistoryTransactionContext, 合并请求和响应.
type HistoryTransactionContext struct {
	tdxproto.FrameBase
	Date   uint32
	Market uint16
	Code   [6]byte
	Start  uint16
	Count  uint16
	sc     data.InstrumentInfo
	Reply  TransactionReply
}

// NewHistoryTransactionContext 构造历史逐笔成交请求, 对齐 C++/Rust.
func NewHistoryTransactionContext(instrument data.InstrumentInfo, date uint32, offset, size int) *HistoryTransactionContext {
	if size <= 0 || size > TickTransactionPerRequestMax {
		size = TickTransactionPerRequestMax
	}
	if offset < 0 {
		offset = 0
	}

	return &HistoryTransactionContext{
		FrameBase: tdxproto.NewFrameBase(tdxproto.StdCommandHistoryTransactionData, tdxproto.FlagUncompressed, tdxproto.PacketTypeRequest),
		Date:      date,
		Market:    uint16(tdxproto.ExchangeToMarketId(instrument.Exchange)),
		Code:      [6]byte(std.String2Bytes(instrument.Ticker)),
		Start:     uint16(offset),
		Count:     uint16(size),
		sc:        instrument,
	}
}

// SerializeRequestBody 序列化请求体, 对齐 C++/Rust/Python.
func (h *HistoryTransactionContext) SerializeRequestBody() []byte {
	payload := &bytes.Buffer{}
	_ = binary.Write(payload, binary.LittleEndian, h.Date)
	_ = binary.Write(payload, binary.LittleEndian, h.Market)
	payload.Write(h.Code[:])
	_ = binary.Write(payload, binary.LittleEndian, h.Start)
	_ = binary.Write(payload, binary.LittleEndian, h.Count)
	return payload.Bytes()
}

// DeserializeResponseBody 解析历史逐笔成交响应体, 对齐 C++/Rust/Python.
func (h *HistoryTransactionContext) DeserializeResponseBody(body []byte) error {
	reader := bytes.NewReader(body)
	if err := binary.Read(reader, binary.LittleEndian, &h.Reply.Count); err != nil {
		return err
	}
	if cap(h.Reply.List) < int(h.Reply.Count) {
		h.Reply.List = make([]TickTransaction, 0, int(h.Reply.Count))
	} else {
		h.Reply.List = h.Reply.List[:0]
	}

	baseUnit := tdxproto.DefaultBaseUnit(tdxproto.ExchangeToMarketId(h.sc.Exchange), h.sc.Ticker)
	isIndex := h.sc.Type.IsIndex()
	var lastPrice int64 = 0

	var date uint32
	if err := binary.Read(reader, binary.LittleEndian, &date); err != nil {
		return err
	} else {
		fmt.Printf("data: %d\n", date)
	}

	for i := 0; i < int(h.Reply.Count); i++ {
		var minutes uint16
		if err := binary.Read(reader, binary.LittleEndian, &minutes); err != nil {
			if err == io.EOF || err == io.ErrUnexpectedEOF {
				h.Reply.Count = uint16(len(h.Reply.List))
				return nil
			}
			return err
		}
		hh := minutes / 60
		mm := minutes % 60

		rawPrice, err := varintRead(reader)
		if err != nil {
			if err == io.EOF {
				h.Reply.Count = uint16(len(h.Reply.List))
				return nil
			}
			return err
		}

		vol, err := varintRead(reader)
		if err != nil {
			return err
		}

		direction, err := varintRead(reader)
		if err != nil {
			return err
		}

		lastPrice += rawPrice
		price := float64(lastPrice) / baseUnit

		var ele TickTransaction
		ele.Time = fmt.Sprintf("%02d:%02d", hh, mm)
		ele.Price = price
		ele.Vol = vol
		ele.Direction = direction

		if isIndex {
			amount := ele.Vol * 100
			ele.Amount = float64(amount)
			if ele.Price != 0 {
				ele.Vol = int64(ele.Amount / ele.Price)
			} else {
				ele.Vol = 0
			}
		} else {
			ele.Vol *= 100
			ele.Amount = float64(ele.Vol) * ele.Price
		}

		// skip reserved varint
		if _, err := varintRead(reader); err != nil {
			if err == io.EOF {
				h.Reply.List = append(h.Reply.List, ele)
				h.Reply.Count = uint16(len(h.Reply.List))
				return nil
			}
			return err
		}

		h.Reply.List = append(h.Reply.List, ele)
	}
	return nil
}

func (h *HistoryTransactionContext) String() string {
	code := strings.TrimRight(string(h.Code[:]), "\x00 ")
	return fmt.Sprintf("HistoryTransactionContext{Date:%d,Market:%d,Code:%s,Start:%d,Count:%d,ReplyCount:%d}", h.Date, h.Market, code, h.Start, h.Count, h.Reply.Count)
}

func Reverse(list []TickTransaction) []TickTransaction {
	if len(list) == 0 {
		return list
	}
	result := make([]TickTransaction, len(list))
	for i, v := range list {
		result[len(list)-1-i] = v
	}
	return result
}
