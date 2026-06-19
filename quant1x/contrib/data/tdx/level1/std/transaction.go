package std

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"strings"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx"
	"github.com/quant1x/quant1x/quant1x/data/exchange"
	"github.com/quant1x/quant1x/quant1x/std"
)

const (
	TickTransactionPerRequestMax = 1800
)

type TransactionDirection int64

const (
	TickTransactionDirectionBuy  = 0 // 买盘
	TickTransactionDirectionSell = 1 // 卖盘
	TickTransactionDirectionNone = 2 // 中性盘

	// 明确表示竞价时段

	TickTransactionAuctionClose = 8 // 收盘集合竞价
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

// TransactionContext builds a TRANSACTION_DATA request payload.
type TransactionContext struct {
	Market uint16
	Code   [6]byte
	Start  uint16
	Count  uint16
}

func NewTransactionRequest(instrument exchange.InstrumentInfo, offset, size int) TransactionContext {
	if size <= 0 || size > TickTransactionPerRequestMax {
		size = TickTransactionPerRequestMax
	}
	if offset < 0 {
		offset = 0
	}

	return TransactionContext{
		Market: uint16(ExchangeToMarketId(instrument.Exchange)),
		Code:   [6]byte(std.String2Bytes(instrument.Ticker)),
		Start:  uint16(offset),
		Count:  uint16(size),
	}
}

func (r TransactionContext) Serialize() []byte {
	payload := &bytes.Buffer{}
	_ = binary.Write(payload, binary.LittleEndian, r.Market)
	payload.Write(r.Code[:])
	_ = binary.Write(payload, binary.LittleEndian, r.Start)
	_ = binary.Write(payload, binary.LittleEndian, r.Count)
	return tdx.BuildRequest(tdx.StdCommandTransactionData, tdx.PacketTypeRequest, payload.Bytes())
}

func (TransactionContext) Command() tdx.StdCommand { return tdx.StdCommandTransactionData }

func (r TransactionContext) String() string {
	code := strings.TrimRight(string(r.Code[:]), "\x00 ")
	return fmt.Sprintf("TransactionContext{Market:%d,Code:%s,Start:%d,Count:%d}", r.Market, code, r.Start, r.Count)
}

// TransactionResponse parses TRANSACTION_DATA responses.
type TransactionResponse struct {
	tdx.ResponseBase
	Reply TransactionReply
	sc    exchange.InstrumentInfo
}

func NewTransactionResponse(code exchange.InstrumentInfo) *TransactionResponse {
	return &TransactionResponse{sc: code}
}

func (r *TransactionResponse) Deserialize(body []byte) error {
	reader := bytes.NewReader(body)
	if err := binary.Read(reader, binary.LittleEndian, &r.Reply.Count); err != nil {
		return err
	}
	if cap(r.Reply.List) < int(r.Reply.Count) {
		r.Reply.List = make([]TickTransaction, 0, int(r.Reply.Count))
	} else {
		r.Reply.List = r.Reply.List[:0]
	}

	baseUnit := DefaultBaseUnit(int(ExchangeToMarketId(r.sc.Exchange)), r.sc.Ticker)
	isIndex := r.sc.Type.IsIndex()
	var lastPrice int64 = 0

	for i := 0; i < int(r.Reply.Count); i++ {
		var seconds uint16
		if err := binary.Read(reader, binary.LittleEndian, &seconds); err != nil {
			if err == io.EOF || err == io.ErrUnexpectedEOF {
				r.Reply.Count = uint16(len(r.Reply.List))
				return nil
			}
			return err
		}
		h := seconds / 60
		m := seconds % 60

		rawPrice, err := varintRead(reader)
		if err != nil {
			if err == io.EOF {
				r.Reply.Count = uint16(len(r.Reply.List))
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
				r.Reply.List = append(r.Reply.List, ele)
				r.Reply.Count = uint16(len(r.Reply.List))
				return nil
			}
			return err
		}

		r.Reply.List = append(r.Reply.List, ele)
	}
	return nil
}

func (r *TransactionResponse) String() string {
	return fmt.Sprintf("TransactionResponse{Reply{Count:%d}}", r.Reply.Count)
}

// HistoryTransactionRequest mirrors the C++ HistoryTransactionRequest.
type HistoryTransactionRequest struct {
	Date   uint32
	Market uint16
	Code   [6]byte
	Start  uint16
	Count  uint16
}

func NewHistoryTransactionRequest(securityCode exchange.InstrumentInfo, date uint32, offset, size int) HistoryTransactionRequest {
	if size <= 0 || size > TickTransactionPerRequestMax {
		size = TickTransactionPerRequestMax
	}
	if offset < 0 {
		offset = 0
	}

	return HistoryTransactionRequest{
		Date:   date,
		Market: uint16(ExchangeToMarketId(securityCode.Exchange)),
		Code:   [6]byte(std.String2Bytes(securityCode.Ticker)),
		Start:  uint16(offset),
		Count:  uint16(size),
	}
}

func (r HistoryTransactionRequest) Serialize() []byte {
	payload := &bytes.Buffer{}
	_ = binary.Write(payload, binary.LittleEndian, r.Date)
	_ = binary.Write(payload, binary.LittleEndian, r.Market)
	payload.Write(r.Code[:])
	_ = binary.Write(payload, binary.LittleEndian, r.Start)
	_ = binary.Write(payload, binary.LittleEndian, r.Count)
	return tdx.BuildRequest(tdx.StdCommandHistoryTransactionData, tdx.PacketTypeRequest, payload.Bytes())
}

func (HistoryTransactionRequest) Command() tdx.StdCommand {
	return tdx.StdCommandHistoryTransactionData
}

func (r HistoryTransactionRequest) String() string {
	code := strings.TrimRight(string(r.Code[:]), "\x00 ")
	return fmt.Sprintf("HistoryTransactionRequest{Date:%d,Market:%d,Code:%s,Start:%d,Count:%d}", r.Date, r.Market, code, r.Start, r.Count)
}

// HistoryTransactionResponse parses HISTORY_TRANSACTION_DATA responses.
type HistoryTransactionResponse struct {
	tdx.ResponseBase
	Reply TransactionReply
	code  exchange.InstrumentInfo
}

func NewHistoryTransactionResponse(code exchange.InstrumentInfo) *HistoryTransactionResponse {
	return &HistoryTransactionResponse{code: code}
}

func (r *HistoryTransactionResponse) Deserialize(body []byte) error {
	reader := bytes.NewReader(body)
	if err := binary.Read(reader, binary.LittleEndian, &r.Reply.Count); err != nil {
		return err
	}
	if cap(r.Reply.List) < int(r.Reply.Count) {
		r.Reply.List = make([]TickTransaction, 0, int(r.Reply.Count))
	} else {
		r.Reply.List = r.Reply.List[:0]
	}

	baseUnit := DefaultBaseUnit(int(ExchangeToMarketId(r.code.Exchange)), r.code.Ticker)
	isIndex := r.code.Type.IsIndex()
	var lastPrice int64 = 0

	var date uint32
	if err := binary.Read(reader, binary.LittleEndian, &date); err != nil {
		return err
	} else {
		fmt.Printf("data: %d\n", date)
	}

	for i := 0; i < int(r.Reply.Count); i++ {
		var minutes uint16
		if err := binary.Read(reader, binary.LittleEndian, &minutes); err != nil {
			if err == io.EOF || err == io.ErrUnexpectedEOF {
				r.Reply.Count = uint16(len(r.Reply.List))
				return nil
			}
			return err
		}
		h := minutes / 60
		m := minutes % 60

		rawPrice, err := varintRead(reader)
		if err != nil {
			if err == io.EOF {
				r.Reply.Count = uint16(len(r.Reply.List))
				return nil
			}
			return err
		}

		vol, err := varintRead(reader)
		if err != nil {
			return err
		}

		// historical record has no 'num' field

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
				r.Reply.List = append(r.Reply.List, ele)
				r.Reply.Count = uint16(len(r.Reply.List))
				return nil
			}
			return err
		}

		r.Reply.List = append(r.Reply.List, ele)
	}
	return nil
}

func (r *HistoryTransactionResponse) String() string {
	return fmt.Sprintf("HistoryTransactionResponse{Reply{Count:%d}}", r.Reply.Count)
}
