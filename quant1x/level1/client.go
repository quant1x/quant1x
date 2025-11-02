package level1

import (
	"bytes"
	"compress/zlib"
	"encoding/binary"
	"errors"
	"io"
	stdnet "net"
	"sync/atomic"
	"time"

	qnet "gitee.com/quant1x/quant1x/quant1x/net"
	"golang.org/x/text/encoding/simplifiedchinese"
	"golang.org/x/text/transform"
)

// ProtocolHandler implements the Level1 protocol handshake and heartbeat.
// It is created in package level1 and implements the net.NetworkHandler interface.
type ProtocolHandler struct {
	timeout       time.Duration
	checkInterval time.Duration
}

// NewProtocolHandler constructs a handler with specified timeout and check interval.
func NewProtocolHandler(timeout, interval time.Duration) qnet.NetworkHandler {
	if timeout <= 0 {
		timeout = 10 * time.Second
	}
	if interval <= 0 {
		interval = 5 * time.Second
	}
	return &ProtocolHandler{timeout: timeout, checkInterval: interval}
}

func (h *ProtocolHandler) Timeout() time.Duration       { return h.timeout }
func (h *ProtocolHandler) CheckInterval() time.Duration { return h.checkInterval }

var seqId uint32

func nextSeqID() uint32 {
	return atomic.AddUint32(&seqId, 1)
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

func buildRequest(method uint16, packetType uint8, payload []byte) []byte {
	seq := nextSeqID()
	pkgLen := uint16(2)
	if payload != nil {
		pkgLen = uint16(2 + len(payload))
	}
	buf := &bytes.Buffer{}
	buf.WriteByte(0x0C) // ZipFlag NotZipped
	binary.Write(buf, binary.LittleEndian, seq)
	buf.WriteByte(packetType)
	binary.Write(buf, binary.LittleEndian, pkgLen)
	binary.Write(buf, binary.LittleEndian, pkgLen)
	binary.Write(buf, binary.LittleEndian, method)
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

func gbkToUTF8(b []byte) (string, error) {
	reader := transform.NewReader(bytes.NewReader(b), simplifiedchinese.GBK.NewDecoder())
	res, err := io.ReadAll(reader)
	if err != nil {
		return "", err
	}
	return string(res), nil
}

func (h *ProtocolHandler) processRequest(conn *stdnet.TCPConn, req []byte) ([]byte, *ResponseHeader, error) {
	if conn == nil {
		return nil, nil, errors.New("nil conn")
	}
	_ = conn.SetDeadline(time.Now().Add(h.timeout))
	defer conn.SetDeadline(time.Time{})
	if _, err := conn.Write(req); err != nil {
		return nil, nil, err
	}
	hdr, err := readResponseHeader(conn)
	if err != nil {
		return nil, nil, err
	}
	if hdr.ZipSize == 0 {
		return nil, hdr, nil
	}
	body := make([]byte, hdr.ZipSize)
	if _, err := io.ReadFull(conn, body); err != nil {
		return nil, hdr, err
	}
	if hdr.ZipSize != hdr.UnZipSize {
		un, err := unzipZlib(body)
		if err != nil {
			return nil, hdr, err
		}
		return un, hdr, nil
	}
	return body, hdr, nil
}

func (h *ProtocolHandler) Handshake(conn *stdnet.TCPConn) (bool, error) {
	payload1 := []byte{0x01}
	req1 := buildRequest(0x000d, 0x01, payload1)
	body1, _, err := h.processRequest(conn, req1)
	if err != nil {
		return false, err
	}
	if len(body1) >= 68 {
		if _, err := gbkToUTF8(body1[68:]); err == nil {
			// optionally use
		}
	}
	padding2 := []byte{0xd5, 0xd0, 0xc9, 0xcc, 0xd6, 0xa4, 0xa8, 0xaf, 0x00, 0x00, 0x00, 0x8f, 0xc2, 0x25, 0x40, 0x13, 0x00, 0x00, 0xd5, 0x00, 0xc9, 0xcc, 0xbd, 0xf0, 0xd7, 0xea, 0x00, 0x00, 0x00, 0x02}
	req2 := buildRequest(0x0fdb, 0x01, padding2)
	body2, _, err := h.processRequest(conn, req2)
	if err != nil {
		return false, err
	}
	if len(body2) >= 58 {
		if _, err := gbkToUTF8(body2[58:]); err == nil {
			// optionally use
		}
	}
	return true, nil
}

func (h *ProtocolHandler) Keepalive(conn *stdnet.TCPConn) (bool, error) {
	req := buildRequest(0x0004, 0x02, nil)
	body, _, err := h.processRequest(conn, req)
	if err != nil {
		return false, err
	}
	if len(body) >= 10 {
		return true, nil
	}
	return true, nil
}
