package level1

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"strings"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/std"
)

// TransactionRequest builds a TRANSACTION_DATA request payload.
type TransactionRequest struct {
	Market uint16
	Code   [6]byte
	Start  uint16
	Count  uint16
}

func NewTransactionRequest(instrument exchange.InstrumentInfo, offset, size int) TransactionRequest {
	if size <= 0 || size > TickTransactionPerRequestMax {
		size = TickTransactionPerRequestMax
	}
	if offset < 0 {
		offset = 0
	}

	return TransactionRequest{
		Market: uint16(exchangeToMarketId(instrument.Exchange)),
		Code:   [6]byte(std.String2Bytes(instrument.Ticker)),
		Start:  uint16(offset),
		Count:  uint16(size),
	}
}

func (r TransactionRequest) Serialize() []byte {
	payload := &bytes.Buffer{}
	_ = binary.Write(payload, binary.LittleEndian, r.Market)
	payload.Write(r.Code[:])
	_ = binary.Write(payload, binary.LittleEndian, r.Start)
	_ = binary.Write(payload, binary.LittleEndian, r.Count)
	return buildRequest(StdCommandTransactionData, packetTypeRequest, payload.Bytes())
}

func (TransactionRequest) Command() StdCommand { return StdCommandTransactionData }

func (r TransactionRequest) String() string {
	code := strings.TrimRight(string(r.Code[:]), "\x00 ")
	return fmt.Sprintf("TransactionRequest{Market:%d,Code:%s,Start:%d,Count:%d}", r.Market, code, r.Start, r.Count)
}

// TransactionResponse parses TRANSACTION_DATA responses.
type TransactionResponse struct {
	ResponseBase
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

	baseUnit := defaultBaseUnit(int(exchangeToMarketId(r.sc.Exchange)), r.sc.Ticker)
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
