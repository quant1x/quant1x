package level1

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"strings"

	"gitee.com/quant1x/quant1x/quant1x/encoding"
)

const (
	SecurityListPerRequestMax = 1600 // 单次请求的最大记录数
)

type SecurityListRequest struct {
	Market  uint16
	Start   uint32
	Count   uint32
	Unknown uint32
}

func NewSecurityListRequest(market int, start, count int) SecurityListRequest {
	if count <= 0 || count > SecurityListPerRequestMax {
		count = SecurityListPerRequestMax
	}
	if start < 0 {
		start = 0
	}
	return SecurityListRequest{
		Market:  uint16(market),
		Start:   uint32(start),
		Count:   uint32(count),
		Unknown: 0,
	}
}

func (r SecurityListRequest) Serialize() []byte {
	payload := &bytes.Buffer{}
	_ = binary.Write(payload, binary.LittleEndian, r.Market)
	_ = binary.Write(payload, binary.LittleEndian, r.Start)
	_ = binary.Write(payload, binary.LittleEndian, r.Count)
	_ = binary.Write(payload, binary.LittleEndian, r.Unknown)
	return buildRequest(StdCommandSecurityList, packetTypeRequest, payload.Bytes())
}

func (SecurityListRequest) Command() StdCommand { return StdCommandSecurityList }

func (r SecurityListRequest) String() string {
	return fmt.Sprintf("SecurityListRequest{Market:%d,Start:%d,Count:%d}", r.Market, r.Start, r.Count)
}

type Security struct {
	Code         string  // 证券代码
	VolUnit      uint16  // 成交量单位
	Name         string  // 证券名称
	Reversed2    [4]byte // 保留字段2
	DecimalPoint uint8   // 小数点位置
	PreClose     float64 // 昨收价
	Reversed3    [4]byte // 保留字段3
}

type SecurityListResponse struct {
	ResponseBase
	Count uint16
	List  []Security
}

func (r *SecurityListResponse) Deserialize(body []byte) error {
	reader := bytes.NewReader(body)
	if err := binary.Read(reader, binary.LittleEndian, &r.Count); err != nil {
		return err
	}
	if cap(r.List) < int(r.Count) {
		r.List = make([]Security, 0, int(r.Count))
	} else {
		r.List = r.List[:0]
	}
	for i := 0; i < int(r.Count); i++ {
		var entry Security
		var codeBytes [6]byte
		if _, err := io.ReadFull(reader, codeBytes[:]); err != nil {
			if errors.Is(err, io.EOF) {
				return fmt.Errorf("unexpected EOF reading code for index %d", i)
			}
			return err
		}
		entry.Code = strings.TrimSpace(string(bytes.TrimRight(codeBytes[:], "\x00 ")))

		if err := binary.Read(reader, binary.LittleEndian, &entry.VolUnit); err != nil {
			return err
		}

		nameRaw := make([]byte, 16)
		if _, err := io.ReadFull(reader, nameRaw); err != nil {
			return err
		}
		nameStr, err := encoding.GBKToUTF8(bytes.TrimRight(nameRaw, "\x00 "))
		if err != nil {
			return fmt.Errorf("gbk decode name: %w", err)
		}
		entry.Name = strings.TrimSpace(nameStr)

		if _, err := io.ReadFull(reader, entry.Reversed2[:]); err != nil {
			return err
		}

		if err := binary.Read(reader, binary.LittleEndian, &entry.DecimalPoint); err != nil {
			return err
		}

		var tmp uint32
		if err := binary.Read(reader, binary.LittleEndian, &tmp); err != nil {
			return err
		}
		entry.PreClose = IntToFloat64(tmp)

		if _, err := io.ReadFull(reader, entry.Reversed3[:]); err != nil {
			return err
		}

		r.List = append(r.List, entry)
	}
	return nil
}

func (r *SecurityListResponse) String() string {
	return fmt.Sprintf("SecurityListResponse{count:%d}", r.Count)
}
