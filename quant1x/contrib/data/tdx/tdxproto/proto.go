// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

// Package tdxproto contains the shared protocol types that both tdx and
// its level1/std sub-package depend on, breaking the import cycle.
//
// 对齐 Python/C++/Rust: BaseFrame + transact_message_sync
package tdxproto

import (
	"bytes"
	"compress/zlib"
	"encoding/binary"
	"errors"
	"fmt"
	stdio "io"
	"sync/atomic"

	qio "github.com/quant1x/quant1x/quant1x/io"
	logger "github.com/quant1x/quant1x/quant1x/log"
)

// ============================================================
// StdCommand — 标准命令字
// ============================================================

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
	StdCommandCompanyContent         StdCommand = 0x02d0 // 公司信息内容
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

// ============================================================
// 常量
// ============================================================

const (
	PacketTypeRequest   uint8 = 0x01
	PacketCtrlHeartbeat uint8 = 0x02
)

const (
	FlagZip          uint8 = 0x10                       // zip帧类型标志位
	FlagUncompressed uint8 = 0x0C                       // 未压缩
	FlagZipped             = FlagZip | FlagUncompressed // zip压缩
)

// 协议头长度常量, 对齐 C++/Rust
const (
	RequestHeaderLength  = 0x0c // 12 字节
	ResponseHeaderLength = 0x10 // 16 字节
)

// ============================================================
// sequence_id — 对齐 C++ get_sequence_id / Python msg_sequence_id
// ============================================================

var seqId uint32

func nextSequenceId() uint32 {
	return atomic.AddUint32(&seqId, 1)
}

// ============================================================
// CommandToString — 对齐 C++ command_to_string / Rust Command.desc
// ============================================================

func CommandToString(cmd StdCommand) string {
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

// ============================================================
// RequestHeader — 请求头 (12字节), 对齐 C++/Rust/Python
// ============================================================

// RequestHeader 请求-消息头
//
// 布局 (小端): frame_type(u8) + seq_id(u32) + packet_ctrl(u8) +
//
//	body_wire_len(u16) + body_raw_len(u16) + method(u16) = 12字节
type RequestHeader struct {
	FrameType   uint8  // frame_type
	SeqId       uint32 // 请求编号
	PacketCtrl  uint8  // 包类型
	BodyWireLen uint16 // 消息体长度(压缩)
	BodyRawLen  uint16 // 消息体长度(原始)
	Method      uint16 // 命令字
}

func (h RequestHeader) String() string {
	return fmt.Sprintf(
		"RequestHeader{frame_type:%d, seq_id:%d, packet_ctrl:%d, body_wire_len:%d, body_raw_len:%d, cmd:%s}",
		h.FrameType, h.SeqId, h.PacketCtrl, h.BodyWireLen, h.BodyRawLen,
		CommandToString(StdCommand(h.Method)),
	)
}

// ByteSize 固定12字节, 对齐 Rust RequestHeader::byte_size
func (h RequestHeader) ByteSize() int { return RequestHeaderLength }

// Serialize 序列化为小端字节数组, 对齐 Rust/Python RequestHeader::serialize
func (h RequestHeader) Serialize() []byte {
	buf := &bytes.Buffer{}
	_ = binary.Write(buf, binary.LittleEndian, h)
	return buf.Bytes()
}

// ============================================================
// ResponseHeader — 响应头 (16字节), 对齐 C++/Rust/Python
// ============================================================

// ResponseHeader 响应-消息头
//
// 布局 (小端): magic_number(u32) + frame_type(u8) + seq_id(u32) +
//
//	packet_ctrl(u8) + method(u16) + body_wire_len(u16) + body_raw_len(u16) = 16字节
type ResponseHeader struct {
	MagicNumber uint32 // reserved
	FrameType   uint8  // 帧类型标志
	SeqId       uint32 // 序列号
	PacketCtrl  uint8  // 包控制
	Method      uint16 // 命令字
	BodyWireLen uint16 // 压缩后大小
	BodyRawLen  uint16 // 解压后大小
}

func (h ResponseHeader) String() string {
	return fmt.Sprintf(
		"ResponseHeader{magic_number:%d, frame_type:%d, seq_id:%d, packet_ctrl:%d, cmd:%s, body_wire_len:%d, body_raw_len:%d}",
		h.MagicNumber, h.FrameType, h.SeqId, h.PacketCtrl,
		CommandToString(StdCommand(h.Method)),
		h.BodyWireLen, h.BodyRawLen,
	)
}

// ByteSize 固定16字节, 对齐 Rust ResponseHeader::byte_size
func (h ResponseHeader) ByteSize() int { return ResponseHeaderLength }

// ReadResponseHeader 从 io.Reader 读取 16 字节并反序列化响应头, 对齐 Python/C++
func ReadResponseHeader(r stdio.Reader) (*ResponseHeader, error) {
	var buf [ResponseHeaderLength]byte
	if _, err := stdio.ReadFull(r, buf[:]); err != nil {
		return nil, err
	}
	hdr := &ResponseHeader{}
	hdr.MagicNumber = binary.LittleEndian.Uint32(buf[0:4])
	hdr.FrameType = buf[4]
	hdr.SeqId = binary.LittleEndian.Uint32(buf[5:9])
	hdr.PacketCtrl = buf[9]
	hdr.Method = binary.LittleEndian.Uint16(buf[10:12])
	hdr.BodyWireLen = binary.LittleEndian.Uint16(buf[12:14])
	hdr.BodyRawLen = binary.LittleEndian.Uint16(buf[14:16])
	return hdr, nil
}

// ============================================================
// FrameBase — 消息基类, 对齐 Python BaseFrame / C++ BaseFrame / Rust BaseFrame trait
// ============================================================

// FrameBase 消息基类, 包含请求头和响应头.
// 嵌入此结构的类型只需实现 SerializeRequestBody 和 DeserializeResponseBody 即可满足 BaseFrame 接口.
type FrameBase struct {
	ReqHeader  RequestHeader
	RespHeader ResponseHeader
}

// NewFrameBase 构造一个 FrameBase, 自动分配 sequence_id, 对齐 Python/C++ 构造逻辑
func NewFrameBase(cmd StdCommand, frameType uint8, packetCtrl uint8) FrameBase {
	return FrameBase{
		ReqHeader: RequestHeader{
			FrameType:  frameType,
			SeqId:      nextSequenceId(),
			PacketCtrl: packetCtrl,
			Method:     uint16(cmd),
		},
	}
}

func (f *FrameBase) RequestHeader() *RequestHeader       { return &f.ReqHeader }
func (f *FrameBase) SetRequestHeader(h *RequestHeader)   { f.ReqHeader = *h }
func (f *FrameBase) ResponseHeader() *ResponseHeader      { return &f.RespHeader }
func (f *FrameBase) SetResponseHeader(h *ResponseHeader)  { f.RespHeader = *h }

// Command 从请求头获取命令字, 对齐 C++/Rust BaseFrame::command()
func (f *FrameBase) Command() StdCommand { return StdCommand(f.ReqHeader.Method) }

// ============================================================
// BaseFrame interface — 对齐 Python abc / C++ CRTP / Rust trait
// ============================================================

// BaseFrame 消息接口, 对齐 Python BaseFrame / C++ BaseFrame<Derived> / Rust BaseFrame trait.
//
// 嵌入 FrameBase 可自动满足 RequestHeader/SetRequestHeader/ResponseHeader/SetResponseHeader/Command.
// 子类型只需实现:
//   - SerializeRequestBody() []byte     — 对齐 Python serialize_request_body / C++ serialize_request_body_impl
//   - DeserializeResponseBody([]byte) error — 对齐 Python deserialize_response_body / Rust deserialize_response_body
//   - String() string                   — 可选, FrameBase 提供默认实现
type BaseFrame interface {
	RequestHeader() *RequestHeader
	SetRequestHeader(*RequestHeader)
	ResponseHeader() *ResponseHeader
	SetResponseHeader(*ResponseHeader)
	SerializeRequestBody() []byte
	DeserializeResponseBody([]byte) error
	Command() StdCommand
	String() string
}

// ============================================================
// SerializeRequest — 完整序列化请求(头+体), 对齐 C++ BaseFrame::serialize_request / Rust BaseFrame::serialize_request
// ============================================================

// SerializeRequest 序列化完整请求 = 消息头 + 消息体.
// 自动设置 body_wire_len 和 body_raw_len, 对齐 C++/Rust/Python BaseFrame::serialize_request.
func SerializeRequest(msg BaseFrame) []byte {
	body := msg.SerializeRequestBody()
	hdr := msg.RequestHeader()
	pkgLen := uint16(2)
	if body != nil {
		pkgLen = uint16(2 + len(body))
	}
	hdr.BodyWireLen = pkgLen
	hdr.BodyRawLen = pkgLen

	buf := hdr.Serialize()
	if body != nil {
		buf = append(buf, body...)
	}
	return buf
}

// ============================================================
// UnzipZlib — 对齐 C++ unzip / Rust unzip / Python zlib.decompress
// ============================================================

// UnzipZlib 解压zlib压缩的数据
func UnzipZlib(data []byte) ([]byte, error) {
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

// ============================================================
// TransactMessageSync — 对齐 Python transact_message_sync / C++ transact_message_sync / Rust transact_message_sync
// ============================================================

// TransactMessageSync 同步发送请求并接收响应.
// 发送请求 → 读取16字节响应头 → 按需读取并解压响应体 → 解析响应体.
// 对齐 Python/C++/Rust transact_message_sync 逻辑.
func TransactMessageSync(conn_ *qio.Connection, msg BaseFrame) error {
	conn := conn_.Conn()
	if conn == nil {
		return errors.New("nil connection")
	}

	cmd := CommandToString(msg.Command())

	// 1. 序列化并发送请求
	payload := SerializeRequest(msg)
	logger.Debugf("[%s] send request bytes: %d", cmd, len(payload))
	logger.Debugf("[%s] request: %s", cmd, msg.RequestHeader())

	if _, err := conn.Write(payload); err != nil {
		return err
	}

	// 2. 读取16字节响应头
	hdr, err := ReadResponseHeader(conn)
	if err != nil {
		return err
	}
	msg.SetResponseHeader(hdr)
	logger.Debugf("[%s] response header: %s", cmd, hdr)

	// 3. body_wire_len == 0 时直接返回
	if hdr.BodyWireLen == 0 {
		return nil
	}

	// 4. 读取响应体
	body := make([]byte, hdr.BodyWireLen)
	if _, err := stdio.ReadFull(conn, body); err != nil {
		return err
	}

	// 5. 按需 zlib 解压 (对齐 C++/Rust/Python)
	if hdr.BodyWireLen != hdr.BodyRawLen {
		body, err = UnzipZlib(body)
		if err != nil {
			return err
		}
	}

	logger.Debugf("[%s] response body length: %d", cmd, len(body))

	// 6. 反序列化响应体
	if err := msg.DeserializeResponseBody(body); err != nil {
		return err
	}

	logger.Debugf("[%s] response: %s", cmd, msg.String())
	return nil
}
