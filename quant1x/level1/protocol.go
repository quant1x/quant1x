package level1

import (
	"bytes"
	"compress/zlib"
	"encoding/binary"
	"errors"
	"io"
	"net"
	"sync/atomic"

	"gitee.com/quant1x/quant1x/quant1x/log"
)

type StdCommand uint16

const (
	StdCommandHeartbeat              StdCommand = 0x0004
	StdCommandLogin1                 StdCommand = 0x000d
	StdCommandLogin2                 StdCommand = 0x0fdb
	StdCommandXdxrInfo               StdCommand = 0x000f
	StdCommandFinanceInfo            StdCommand = 0x0010
	StdCommandPing                   StdCommand = 0x0015
	StdCommandCompanyCategory        StdCommand = 0x02cf
	StdCommandCompanyContent         StdCommand = 0x02d0
	StdCommandSecurityCount          StdCommand = 0x044e
	StdCommandSecurityList           StdCommand = 0x044d
	StdCommandOldSecurityList        StdCommand = 0x0450
	StdCommandIndexBars              StdCommand = 0x052d
	StdCommandSecurityBars           StdCommand = 0x052d
	StdCommandSecurityQuotesOld      StdCommand = 0x053e
	StdCommandSecurityQuotesNew      StdCommand = 0x054c
	StdCommandMinuteTimeData         StdCommand = 0x051d
	StdCommandBlockMeta              StdCommand = 0x02c5
	StdCommandBlockData              StdCommand = 0x06b9
	StdCommandTransactionData        StdCommand = 0x0fc5
	StdCommandHistoryMinuteData      StdCommand = 0x0fb4
	StdCommandHistoryTransactionData StdCommand = 0x0fb5
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

func nextSeqID() uint32 {
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

type ResponseHeader struct {
	I1        uint32
	ZipFlag   uint8
	SeqID     uint32
	I2        uint8
	Method    uint16
	ZipSize   uint16
	UnZipSize uint16
}

func readResponseHeader(r io.Reader) (*ResponseHeader, error) {
	var buf [16]byte
	if _, err := io.ReadFull(r, buf[:]); err != nil {
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
	Bytes() []byte
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

func Process[T ProtocolRequest, R ProtocolResponse](conn *net.TCPConn, req T, resp R) error {
	if conn == nil {
		return errors.New("nil connection")
	}

	cmd := commandToString(req.Command())
	payload := req.Bytes()
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
	if _, err := io.ReadFull(conn, body); err != nil {
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

func buildRequest(method StdCommand, packetType uint8, payload []byte) []byte {
	seq := nextSeqID()
	pkgLen := uint16(2)
	if payload != nil {
		pkgLen = uint16(2 + len(payload))
	}
	buf := &bytes.Buffer{}
	buf.WriteByte(FlagUncompressed)
	_ = binary.Write(buf, binary.LittleEndian, seq)
	buf.WriteByte(packetType)
	_ = binary.Write(buf, binary.LittleEndian, pkgLen)
	_ = binary.Write(buf, binary.LittleEndian, pkgLen)
	_ = binary.Write(buf, binary.LittleEndian, uint16(method))
	if payload != nil {
		buf.Write(payload)
	}
	return buf.Bytes()
}

func unzipZlib(data []byte) ([]byte, error) {
	r, err := zlib.NewReader(bytes.NewReader(data))
	if err != nil {
		return nil, err
	}
	defer r.Close()
	out := &bytes.Buffer{}
	if _, err := io.Copy(out, r); err != nil {
		return nil, err
	}
	return out.Bytes(), nil
}
