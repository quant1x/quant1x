package std

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"strings"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/encoding"
)

const (
	SecurityListPerRequestMax = 1600 // 单次请求的最大记录数
)

type Security struct {
	Code         string  // 证券代码
	VolUnit      uint16  // 成交量单位
	Name         string  // 证券名称
	Reversed2    [4]byte // 保留字段2
	DecimalPoint uint8   // 小数点位置
	PreClose     float64 // 昨收价
	Reversed3    [4]byte // 保留字段3
}

// SecurityListContext 对齐 C++/Rust/Python SecurityListContext, 合并请求和响应.
type SecurityListContext struct {
	tdxproto.FrameBase
	Market uint16
	Start  uint32
	Count  uint32
	Unknown uint32
	RespCount uint16
	List    []Security
}

// NewSecurityListContext 构造证券列表请求, 对齐 C++/Rust.
func NewSecurityListContext(exchange data.Exchange, start, count int) *SecurityListContext {
	if count <= 0 || count > SecurityListPerRequestMax {
		count = SecurityListPerRequestMax
	}
	if start < 0 {
		start = 0
	}
	return &SecurityListContext{
		FrameBase: tdxproto.NewFrameBase(tdxproto.StdCommandSecurityList, tdxproto.FlagUncompressed, tdxproto.PacketTypeRequest),
		Market:    uint16(tdxproto.ExchangeToMarketId(exchange)),
		Start:     uint32(start),
		Count:     uint32(count),
		Unknown:   0,
	}
}

// SerializeRequestBody 序列化请求体, 对齐 C++/Rust/Python.
func (s *SecurityListContext) SerializeRequestBody() []byte {
	payload := &bytes.Buffer{}
	_ = binary.Write(payload, binary.LittleEndian, s.Market)
	_ = binary.Write(payload, binary.LittleEndian, s.Start)
	_ = binary.Write(payload, binary.LittleEndian, s.Count)
	_ = binary.Write(payload, binary.LittleEndian, s.Unknown)
	return payload.Bytes()
}

// DeserializeResponseBody 解析证券列表响应体, 对齐 C++/Rust/Python.
func (s *SecurityListContext) DeserializeResponseBody(body []byte) error {
	reader := bytes.NewReader(body)
	if err := binary.Read(reader, binary.LittleEndian, &s.RespCount); err != nil {
		return err
	}
	if cap(s.List) < int(s.RespCount) {
		s.List = make([]Security, 0, int(s.RespCount))
	} else {
		s.List = s.List[:0]
	}
	for i := 0; i < int(s.RespCount); i++ {
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
		entry.PreClose = tdxproto.IntegerToFloat64(tmp)

		if _, err := io.ReadFull(reader, entry.Reversed3[:]); err != nil {
			return err
		}

		s.List = append(s.List, entry)
	}
	return nil
}

func (s *SecurityListContext) String() string {
	return fmt.Sprintf("SecurityListContext{Market:%d,Start:%d,Count:%d,RespCount:%d}", s.Market, s.Start, s.Count, s.RespCount)
}
