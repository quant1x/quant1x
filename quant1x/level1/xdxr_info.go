package level1

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"strings"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
)

// XdxrInfoRequest encodes the XDXR_INFO request payload.
type XdxrInfoRequest struct {
	Padding []byte
	Market  uint8
	Code    [6]byte
}

// NewXdxrInfoRequest constructs a request like the C++ XdxrInfoRequest.
func NewXdxrInfoRequest(securityCode string) XdxrInfoRequest {
	mid, _, symbol, _ := exchange.DetectMarket(securityCode)
	var code [6]byte
	copy(code[:], symbol)
	return XdxrInfoRequest{Padding: []byte{0x01, 0x00}, Market: uint8(mid), Code: code}
}

func (r XdxrInfoRequest) Bytes() []byte {
	payload := &bytes.Buffer{}
	payload.Write(r.Padding)
	payload.WriteByte(r.Market)
	payload.Write(r.Code[:])
	return buildRequest(StdCommandXdxrInfo, packetTypeRequest, payload.Bytes())
}

func (r XdxrInfoRequest) Command() StdCommand { return StdCommandXdxrInfo }

func (r XdxrInfoRequest) String() string {
	code := strings.TrimRight(string(r.Code[:]), "\x00 ")
	return fmt.Sprintf("XdxrInfoRequest{Market:%d,Code:%s}", r.Market, code)
}

// XdxrInfo represents a parsed XDXR event returned by the server.
type XdxrInfo struct {
	Date          string
	Category      uint8
	Name          string
	FenHong       float64
	PeiGuJia      float64
	SongZhuanGu   float64
	PeiGu         float64
	SuoGu         float64
	QianLiuTong   float64
	HouLiuTong    float64
	QianZongGuBen float64
	HouZongGuBen  float64
	FenShu        float64
	XingQuanJia   float64
}

// XdxrInfoResponse decodes the response body for XDXR_INFO
type XdxrInfoResponse struct {
	ResponseBase
	Count uint16
	List  []XdxrInfo
}

func NewXdxrInfoResponse() *XdxrInfoResponse { return &XdxrInfoResponse{} }

func (r *XdxrInfoResponse) Deserialize(body []byte) error {
	reader := bytes.NewReader(body)
	// skip 9 bytes as in C++ (Unknown header)
	if _, err := reader.Seek(9, io.SeekStart); err != nil {
		return err
	}
	if err := binary.Read(reader, binary.LittleEndian, &r.Count); err != nil {
		return err
	}
	r.List = make([]XdxrInfo, 0, int(r.Count))
	for i := 0; i < int(r.Count); i++ {
		var market uint8
		if err := binary.Read(reader, binary.LittleEndian, &market); err != nil {
			return err
		}
		codeBuf := make([]byte, 6)
		if _, err := io.ReadFull(reader, codeBuf); err != nil {
			return err
		}
		// unknown byte
		var _unk uint8
		if err := binary.Read(reader, binary.LittleEndian, &_unk); err != nil {
			return err
		}
		var dateRaw uint32
		if err := binary.Read(reader, binary.LittleEndian, &dateRaw); err != nil {
			return err
		}
		var category uint8
		if err := binary.Read(reader, binary.LittleEndian, &category); err != nil {
			return err
		}
		data := make([]byte, 16)
		if _, err := io.ReadFull(reader, data); err != nil {
			return err
		}

		y, m, d, _, _ := GetDatetimeFromUint32(9, dateRaw, 0)
		xi := XdxrInfo{Date: fmt.Sprintf("%04d-%02d-%02d", y, m, d), Category: category, Name: toStringXdxrCategory(int(category))}

		// parse data per category similar to C++ logic
		db := bytes.NewReader(data)
		switch category {
		case 1: // 除权除息
			var f32v float32
			_ = binary.Read(db, binary.LittleEndian, &f32v)
			xi.FenHong = float64(f32v)
			_ = binary.Read(db, binary.LittleEndian, &f32v)
			xi.PeiGuJia = float64(f32v)
			_ = binary.Read(db, binary.LittleEndian, &f32v)
			xi.SongZhuanGu = float64(f32v)
			_ = binary.Read(db, binary.LittleEndian, &f32v)
			xi.PeiGu = float64(f32v)
		case 11, 12:
			// skip 8 then suogu float32
			if _, err := db.Seek(8, io.SeekStart); err == nil {
				var f32v float32
				_ = binary.Read(db, binary.LittleEndian, &f32v)
				xi.SuoGu = float64(f32v)
			}
		case 13, 14:
			var f32v float32
			_ = binary.Read(db, binary.LittleEndian, &f32v)
			xi.XingQuanJia = float64(f32v)
			if _, err := db.Seek(8, io.SeekCurrent); err == nil {
				var f32v2 float32
				_ = binary.Read(db, binary.LittleEndian, &f32v2)
				xi.FenShu = float64(f32v2)
			}
		default:
			var v uint32
			_ = binary.Read(db, binary.LittleEndian, &v)
			xi.QianLiuTong = IntToFloat64(v)
			_ = binary.Read(db, binary.LittleEndian, &v)
			xi.QianZongGuBen = IntToFloat64(v)
			_ = binary.Read(db, binary.LittleEndian, &v)
			xi.HouLiuTong = IntToFloat64(v)
			_ = binary.Read(db, binary.LittleEndian, &v)
			xi.HouZongGuBen = IntToFloat64(v)
		}

		r.List = append(r.List, xi)
	}
	return nil
}

func toStringXdxrCategory(c int) string {
	switch c {
	case 1:
		return "除权除息"
	case 11:
		return "拆股/合股"
	case 12:
		return "缩股"
	case 13:
		return "送认购权证"
	case 14:
		return "送认沽权证"
	default:
		return fmt.Sprintf("Unknown(%d)", c)
	}
}

func (r *XdxrInfoResponse) String() string {
	return fmt.Sprintf("XdxrInfoResponse{Count:%d}", r.Count)
}
