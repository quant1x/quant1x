package level1

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"strings"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
)

const (
	TickTransactionPerRequestMax = 1800
)

// TickTransaction mirrors the C++ TickTransaction structure.
type TickTransaction struct {
	Time      string  // 成交时间 HH:MM
	Price     float64 // 成交价格
	Vol       int64   // 成交量(股)
	Num       int64   // 成交笔数
	Amount    float64 // 成交金额
	BuyOrSell int64   // 买卖方向
}

func (t TickTransaction) String() string {
	return fmt.Sprintf("time: %s price: %v vol: %d num: %d amount: %v buyOrSell: %d", t.Time, t.Price, t.Vol, t.Num, t.Amount, t.BuyOrSell)
}

// TransactionRequest builds a TRANSACTION_DATA request payload.
type TransactionRequest struct {
	Market uint16
	Code   [6]byte
	Start  uint16
	Count  uint16
}

func NewTransactionRequest(securityCode string, offset, size int) TransactionRequest {
	if size <= 0 || size > TickTransactionPerRequestMax {
		size = TickTransactionPerRequestMax
	}
	if offset < 0 {
		offset = 0
	}
	mid, _, symbol, _ := exchange.DetectMarket(securityCode)
	var code [6]byte
	copy(code[:], symbol)
	return TransactionRequest{
		Market: uint16(mid),
		Code:   code,
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
	Count  uint16
	List   []TickTransaction
	market int
	code   string
}

func NewTransactionResponse(market int, code string) *TransactionResponse {
	return &TransactionResponse{market: market, code: code}
}

func (r *TransactionResponse) Deserialize(body []byte) error {
	reader := bytes.NewReader(body)
	if err := binary.Read(reader, binary.LittleEndian, &r.Count); err != nil {
		return err
	}
	if cap(r.List) < int(r.Count) {
		r.List = make([]TickTransaction, 0, int(r.Count))
	} else {
		r.List = r.List[:0]
	}

	baseUnit := DefaultBaseUnit(r.market, r.code)
	isIndex := exchange.AssertIndexByMarketAndCode(exchange.ExchangeId(r.market), r.code)
	var lastPrice int64 = 0

	for i := 0; i < int(r.Count); i++ {
		var seconds uint16
		if err := binary.Read(reader, binary.LittleEndian, &seconds); err != nil {
			if err == io.EOF || err == io.ErrUnexpectedEOF {
				r.Count = uint16(len(r.List))
				return nil
			}
			return err
		}
		h := seconds / 60
		m := seconds % 60

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
		num, err := varintRead(reader)
		if err != nil {
			return err
		}
		buyOrSell, err := varintRead(reader)
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
		ele.BuyOrSell = buyOrSell

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

func (r *TransactionResponse) String() string {
	return fmt.Sprintf("TransactionResponse{Count:%d}", r.Count)
}
