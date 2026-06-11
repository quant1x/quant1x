package level1

import (
	"bytes"
	"compress/zlib"
	"encoding/binary"
	"errors"
	"fmt"
	stdio "io"
	"sync/atomic"

	qio "gitee.com/quant1x/quant1x/quant1x/io"
	"gitee.com/quant1x/quant1x/quant1x/log"
)

// StdCommand 标准命令类型
type StdCommand uint16

const (
	StdCommandHeartbeat              StdCommand = 0x0004 // 心跳
	StdCommandLogin1                 StdCommand = 0x000d // 登录1
	StdCommandLogin2                 StdCommand = 0x0fdb // 登录2
	StdCommandXdxrInfo               StdCommand = 0x000f // 除权除息信息
	StdCommandFinanceInfo            StdCommand = 0x0010 // 财务信息
	StdCommandPing                   StdCommand = 0x0015 // Ping
	StdCommandCompanyCategory        StdCommand = 0x02cf // 公司信息分类
	StdCommandCompanyContent         StdCommand = 0x02d0 //	公司信息内容
	StdCommandSecurityCount          StdCommand = 0x044e // 证券数量
	StdCommandSecurityList           StdCommand = 0x044d // 证券列表
	StdCommandOldSecurityList        StdCommand = 0x0450 // 旧版证券列表
	StdCommandIndexBars              StdCommand = 0x052d // 指数K线数据
	StdCommandSecurityBars           StdCommand = 0x052d // 证券K线数据
	StdCommandSecurityQuotesOld      StdCommand = 0x053e // 旧版证券行情数据
	StdCommandSecurityQuotesNew      StdCommand = 0x054c // 新版证券行情数据
	StdCommandMinuteTimeData         StdCommand = 0x051d // 分时数据
	StdCommandBlockMeta              StdCommand = 0x02c5 // 板块元数据
	StdCommandBlockData              StdCommand = 0x06b9 // 板块数据
	StdCommandTransactionData        StdCommand = 0x0fc5 // 逐笔数据
	StdCommandHistoryMinuteData      StdCommand = 0x0fb4 // 历史分时数据
	StdCommandHistoryTransactionData StdCommand = 0x0fb5 // 历史逐笔数据
)

const (
	packetTypeRequest   uint8 = 0x01
	packetTypeHeartbeat uint8 = 0x02
)

const (
	FlagZip          uint8 = 0x10                       // zip压缩标志位
	FlagUncompressed uint8 = 0x0C                       // 未压缩
	FlagZipped             = FlagZip | FlagUncompressed // zip压缩
)

var seqId uint32

func nextSequenceId() uint32 {
	return atomic.AddUint32(&seqId, 1)
}

func commandToString(cmd StdCommand) string {
	switch cmd {
	case StdCommandHeartbeat:
		return "L1:HEARTBEAT"
	case StdCommandLogin1:
		return "L1:LOGIN1"
	case StdCommandLogin2:
		return "L1:LOGIN2"
	case StdCommandXdxrInfo:
		return "L1:XDXR_INFO"
	case StdCommandFinanceInfo:
		return "L1:FINANCE_INFO"
	case StdCommandPing:
		return "L1:PING"
	case StdCommandCompanyCategory:
		return "L1:COMPANY_CATEGORY"
	case StdCommandCompanyContent:
		return "L1:COMPANY_CONTENT"
	case StdCommandSecurityCount:
		return "L1:SECURITY_COUNT"
	case StdCommandSecurityList:
		return "L1:SECURITY_LIST"
	case StdCommandSecurityBars:
		return "L1:SECURITY_BARS"
	case StdCommandSecurityQuotesOld:
		return "L1:SECURITY_QUOTES_OLD"
	case StdCommandSecurityQuotesNew:
		return "L1:SECURITY_QUOTES_NEW"
	case StdCommandMinuteTimeData:
		return "L1:MINUTE_TIME_DATA"
	case StdCommandBlockMeta:
		return "L1:BLOCK_META"
	case StdCommandBlockData:
		return "L1:BLOCK_DATA"
	case StdCommandTransactionData:
		return "L1:TRANSACTION_DATA"
	case StdCommandHistoryMinuteData:
		return "L1:HISTORY_MINUTE_DATA"
	case StdCommandHistoryTransactionData:
		return "L1:HISTORY_TRANSACTION_DATA"
	default:
		return "L1:UNKNOWN_CMD"
	}
}

// RequestHeader 请求-消息头
type RequestHeader struct {
	ZipFlag    uint8  // ZipFlag
	SeqID      uint32 // 请求编号
	PacketType uint8  // 包类型
	PkgLen1    uint16 // 消息体长度1
	PkgLen2    uint16 // 消息体长度2
	Method     uint16 // 命令字
}

func (h RequestHeader) String() string {
	return fmt.Sprintf("{ZipFlag: %d, SeqID: %d, PacketType: %d, PkgLen1: %d, PkgLen2: %d, Method: %d}",
		h.ZipFlag, h.SeqID, h.PacketType, h.PkgLen1, h.PkgLen2, h.Method)
}

// ResponseHeader 响应-消息头
type ResponseHeader struct {
	I1        uint32 // reserved
	ZipFlag   uint8  // 压缩标志
	SeqID     uint32 // 序列号
	I2        uint8  // reserved
	Method    uint16 // 命令字
	ZipSize   uint16 // 压缩后大小
	UnZipSize uint16 // 解压后大小
}

func readResponseHeader(r stdio.Reader) (*ResponseHeader, error) {
	var buf [16]byte
	if _, err := stdio.ReadFull(r, buf[:]); err != nil {
		return nil, err
	}
	hdr := &ResponseHeader{}
	hdr.I1 = binary.LittleEndian.Uint32(buf[0:4])
	hdr.ZipFlag = buf[4]
	hdr.SeqID = binary.LittleEndian.Uint32(buf[5:9])
	hdr.I2 = buf[9]
	hdr.Method = binary.LittleEndian.Uint16(buf[10:12])
	hdr.ZipSize = binary.LittleEndian.Uint16(buf[12:14])
	hdr.UnZipSize = binary.LittleEndian.Uint16(buf[14:16])
	return hdr, nil
}

type ProtocolRequest interface {
	Serialize() []byte
	Command() StdCommand
	String() string
}

type ProtocolResponse interface {
	SetHeader(*ResponseHeader)
	Header() *ResponseHeader
	Deserialize([]byte) error
	String() string
}

type ResponseBase struct {
	header ResponseHeader
}

func (b *ResponseBase) Header() *ResponseHeader {
	return &b.header
}

func (b *ResponseBase) SetHeader(h *ResponseHeader) {
	if h == nil {
		b.header = ResponseHeader{}
		return
	}
	b.header = *h
}

// buildRequest 构建请求数据包
//
// 参数:
//
//	method: 标准命令类型
//	packetType: 数据包类型
//	payload: 请求负载数据
//
// 返回值:
//
//	构建完成的请求字节数组
func buildRequest(method StdCommand, packetType uint8, payload []byte) []byte {
	seqId := nextSequenceId()
	pkgLen := uint16(2)
	if payload != nil {
		pkgLen = uint16(2 + len(payload))
	}
	req := RequestHeader{
		ZipFlag:    FlagUncompressed,
		SeqID:      seqId,
		PacketType: packetType,
		PkgLen1:    pkgLen,
		PkgLen2:    pkgLen,
		Method:     uint16(method),
	}
	buf := &bytes.Buffer{}
	_ = binary.Write(buf, binary.LittleEndian, req)
	if payload != nil {
		buf.Write(payload)
	}
	return buf.Bytes()
}

// unzipZlib 解压zlib压缩的数据
//
// 参数:
//
//	data - 待解压的zlib压缩数据
//
// 返回值:
//
//	[]byte - 解压后的原始数据
//	error - 解压过程中遇到的错误
func unzipZlib(data []byte) ([]byte, error) {
	r, err := zlib.NewReader(bytes.NewReader(data))
	if err != nil {
		return nil, err
	}
	defer r.Close()
	out := &bytes.Buffer{}
	if _, err := stdio.Copy(out, r); err != nil {
		return nil, err
	}
	return out.Bytes(), nil
}

// Process 处理请求并获取响应
//
// 参数:
//
//	conn - 已建立的 TCP 连接
//	req - 待发送的请求对象
//	resp - 用于接收响应数据的响应对象
//
// 返回值:
//
//	error - 处理过程中遇到的错误
func Process[T ProtocolRequest, R ProtocolResponse](conn_ *qio.Connection, req T, resp R) error {
	conn := conn_.Conn()
	if conn == nil {
		return errors.New("nil connection")
	}

	cmd := commandToString(req.Command())
	payload := req.Serialize()
	log.Debugf("[%s] send request bytes: %d", cmd, len(payload))
	log.Debugf("[%s] request: %s", cmd, req.String())

	if _, err := conn.Write(payload); err != nil {
		return err
	}

	hdr, err := readResponseHeader(conn)
	if err != nil {
		return err
	}
	resp.SetHeader(hdr)
	log.Debugf("[%s] response header: %+v", cmd, *hdr)

	if hdr.ZipSize == 0 {
		return nil
	}

	body := make([]byte, hdr.ZipSize)
	if _, err := stdio.ReadFull(conn, body); err != nil {
		return err
	}

	if hdr.ZipSize != hdr.UnZipSize {
		body, err = unzipZlib(body)
		if err != nil {
			return err
		}
	}

	log.Debugf("[%s] response body length: %d", cmd, len(body))
	if err := resp.Deserialize(body); err != nil {
		return err
	}
	log.Debugf("[%s] response: %s", cmd, resp.String())
	return nil
}
