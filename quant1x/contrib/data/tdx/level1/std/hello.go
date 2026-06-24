package std

import (
	"errors"
	"fmt"
	"strings"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
	"github.com/quant1x/quant1x/quant1x/encoding"
)

const stdLoginInfoOffset = 68

// StdLoginContext 对齐 C++/Rust/Python StdLoginContext (STD_SYNCHRONIZE1), 合并请求和响应.
type StdLoginContext struct {
	tdxproto.FrameBase
	Info string
}

// NewStdLoginContext 构造第一次握手请求, 对齐 C++/Rust.
func NewStdLoginContext() *StdLoginContext {
	return &StdLoginContext{
		FrameBase: tdxproto.NewFrameBase(tdxproto.StdCommandLogin1, tdxproto.FlagUncompressed, tdxproto.PacketTypeRequest),
	}
}

// SerializeRequestBody 序列化请求体(1字节padding), 对齐 C++/Rust/Python.
func (s *StdLoginContext) SerializeRequestBody() []byte { return []byte{0x01} }

// DeserializeResponseBody 解析响应体, 对齐 C++/Rust/Python.
func (s *StdLoginContext) DeserializeResponseBody(body []byte) error {
	info, err := decodeHelloInfo(body, stdLoginInfoOffset)
	if err != nil {
		return err
	}
	s.Info = info
	return nil
}

func (s *StdLoginContext) String() string { return fmt.Sprintf("StdLoginContext{Info:%q}", s.Info) }

func decodeHelloInfo(body []byte, offset int) (string, error) {
	if len(body) <= offset {
		return "", fmt.Errorf("hello info body too short: %d <= %d", len(body), offset)
	}
	infoBytes := body[offset:]
	if len(infoBytes) == 0 {
		return "", errors.New("hello info empty payload")
	}
	text, err := gbkToUTF8(infoBytes)
	if err != nil {
		return "", err
	}
	text = strings.TrimSpace(text)
	if text == "" {
		return "", errors.New("hello info blank message")
	}
	return text, nil
}

func gbkToUTF8(data []byte) (string, error) {
	return encoding.GBKToUTF8(data)
}

const upgradeTipInfoOffset = 58

var upgradeTipPayload = []byte{
	0xd5, 0xd0, 0xc9, 0xcc, 0xd6, 0xa4, 0xa8, 0xaf, 0x00, 0x00,
	0x00, 0x8f, 0xc2, 0x25, 0x40, 0x13, 0x00, 0x00, 0xd5, 0x00,
	0xc9, 0xcc, 0xbd, 0xf6, 0xd7, 0xea, 0x00, 0x00, 0x00, 0x02,
}

// UpgradeTipContext 对齐 C++/Rust/Python UpgradeTipContext (STD_SYNCHRONIZE2), 合并请求和响应.
type UpgradeTipContext struct {
	tdxproto.FrameBase
	Info string
}

// NewUpgradeTipContext 构造第二次握手请求, 对齐 C++/Rust.
func NewUpgradeTipContext() *UpgradeTipContext {
	return &UpgradeTipContext{
		FrameBase: tdxproto.NewFrameBase(tdxproto.StdCommandLogin2, tdxproto.FlagUncompressed, tdxproto.PacketTypeRequest),
	}
}

// SerializeRequestBody 序列化请求体(30字节硬编码payload), 对齐 C++/Rust/Python.
func (u *UpgradeTipContext) SerializeRequestBody() []byte { return upgradeTipPayload }

// DeserializeResponseBody 解析响应体, 对齐 C++/Rust/Python.
func (u *UpgradeTipContext) DeserializeResponseBody(body []byte) error {
	info, err := decodeHelloInfo(body, upgradeTipInfoOffset)
	if err != nil {
		return err
	}
	u.Info = info
	return nil
}

func (u *UpgradeTipContext) String() string { return fmt.Sprintf("UpgradeTipContext{Info:%q}", u.Info) }
