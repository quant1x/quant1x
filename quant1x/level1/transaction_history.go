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

// HistoryTransactionRequest mirrors the C++ HistoryTransactionRequest.
type HistoryTransactionRequest struct {
	Date   uint32
	Market uint16
	Code   [6]byte
	Start  uint16
	Count  uint16
}

func NewHistoryTransactionRequest(securityCode exchange.SecurityCode, date uint32, offset, size int) HistoryTransactionRequest {
	if size <= 0 || size > TickTransactionPerRequestMax {
		size = TickTransactionPerRequestMax
	}
	if offset < 0 {
		offset = 0
	}

	return HistoryTransactionRequest{
		Date:   date,
		Market: uint16(securityCode.Exchange),
		Code:   [6]byte(std.String2Bytes(securityCode.Symbol)),
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
	return buildRequest(StdCommandHistoryTransactionData, packetTypeRequest, payload.Bytes())
}

func (HistoryTransactionRequest) Command() StdCommand { return StdCommandHistoryTransactionData }

func (r HistoryTransactionRequest) String() string {
	code := strings.TrimRight(string(r.Code[:]), "\x00 ")
	return fmt.Sprintf("HistoryTransactionRequest{Date:%d,Market:%d,Code:%s,Start:%d,Count:%d}", r.Date, r.Market, code, r.Start, r.Count)
}

// HistoryTransactionResponse parses HISTORY_TRANSACTION_DATA responses.
type HistoryTransactionResponse struct {
	ResponseBase
	Count uint16
	List  []TickTransaction
	code  exchange.SecurityCode
}

func NewHistoryTransactionResponse(code exchange.SecurityCode) *HistoryTransactionResponse {
	return &HistoryTransactionResponse{code: code}
}

func (r *HistoryTransactionResponse) Deserialize(body []byte) error {
	reader := bytes.NewReader(body)
	if err := binary.Read(reader, binary.LittleEndian, &r.Count); err != nil {
		return err
	}
	if cap(r.List) < int(r.Count) {
		r.List = make([]TickTransaction, 0, int(r.Count))
	} else {
		r.List = r.List[:0]
	}

	baseUnit := DefaultBaseUnit(int(r.code.Exchange), r.code.Symbol)
	isIndex := r.code.Type == exchange.SecurityIndex
	var lastPrice int64 = 0

	// skip 4 bytes as in C++ implementation
	if _, err := reader.Seek(4, io.SeekCurrent); err != nil {
		return err
	}

	for i := 0; i < int(r.Count); i++ {
		var minutes uint16
		if err := binary.Read(reader, binary.LittleEndian, &minutes); err != nil {
			if err == io.EOF || err == io.ErrUnexpectedEOF {
				r.Count = uint16(len(r.List))
				return nil
			}
			return err
		}
		h := minutes / 60
		m := minutes % 60

		rawPrice, err := varintRead(reader)
		if err != nil {
			if err == io.EOF {
				r.Count = uint16(len(r.List))
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
				r.List = append(r.List, ele)
				r.Count = uint16(len(r.List))
				return nil
			}
			return err
		}

		r.List = append(r.List, ele)
	}
	return nil
}

func (r *HistoryTransactionResponse) String() string {
	return fmt.Sprintf("HistoryTransactionResponse{Count:%d}", r.Count)
}
