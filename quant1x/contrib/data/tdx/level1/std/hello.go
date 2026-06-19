package std

import (
	"errors"
	"fmt"
	"strings"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx"
	"github.com/quant1x/quant1x/quant1x/encoding"
)

const stdLoginInfoOffset = 68

// StdLoginContext 对应第一次握手请求.
type StdLoginContext struct{}

// Bytes 序列化请求数据.
func (StdLoginContext) Bytes() []byte {
	payload := []byte{0x01}
	return BuildRequest(StdCommandLogin1, PacketTypeRequest, payload)
}

// Command 返回命令类型.
func (StdLoginContext) Command() tdx.StdCommand { return tdx.StdCommandLogin1 }

// String 返回描述.
func (StdLoginContext) String() string { return "StdLoginContext" }

// StdLoginResponse 对应第一次握手响应.
type StdLoginResponse struct {
	tdx.ResponseBase
	Info string
}

// Deserialize 解析响应数据.
func (r *StdLoginResponse) Deserialize(body []byte) error {
	info, err := decodeHelloInfo(body, stdLoginInfoOffset)
	if err != nil {
		return err
	}
	r.Info = info
	return nil
}

// String 返回响应描述.
func (r *StdLoginResponse) String() string { return fmt.Sprintf("StdLoginResponse{Info:%q}", r.Info) }

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
	0xc9, 0xcc, 0xbd, 0xf0, 0xd7, 0xea, 0x00, 0x00, 0x00, 0x02,
}

// UpgradeTipContext 对应第二次握手请求.
type UpgradeTipContext struct{}

// Bytes 序列化请求数据.
func (UpgradeTipContext) Bytes() []byte {
	return BuildRequest(tdx.StdCommandLogin2, PacketTypeRequest, upgradeTipPayload)
}

// Command 返回命令类型.
func (UpgradeTipContext) Command() tdx.StdCommand { return tdx.StdCommandLogin2 }

// String 返回描述.
func (UpgradeTipContext) String() string { return "UpgradeTipContext" }

// UpgradeTipResponse 对应第二次握手响应.
type UpgradeTipResponse struct {
	ResponseBase
	Info string
}

// Deserialize 解析响应数据.
func (r *UpgradeTipResponse) Deserialize(body []byte) error {
	info, err := decodeHelloInfo(body, upgradeTipInfoOffset)
	if err != nil {
		return err
	}
	r.Info = info
	return nil
}

// String 返回响应描述.
func (r *UpgradeTipResponse) String() string {
	return fmt.Sprintf("UpgradeTipResponse{Info:%q}", r.Info)
}
