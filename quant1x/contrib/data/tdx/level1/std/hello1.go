package std

import (
	"errors"
	"fmt"
	"strings"

	"github.com/quant1x/quant1x/quant1x/encoding"
)

const hello1InfoOffset = 68

// Hello1Request 对应第一次握手请求.
type Hello1Request struct{}

// Bytes 序列化请求数据.
func (Hello1Request) Bytes() []byte {
	payload := []byte{0x01}
	return buildRequest(StdCommandLogin1, packetTypeRequest, payload)
}

// Command 返回命令类型.
func (Hello1Request) Command() StdCommand { return StdCommandLogin1 }

// String 返回描述.
func (Hello1Request) String() string { return "Hello1Request" }

// Hello1Response 对应第一次握手响应.
type Hello1Response struct {
	ResponseBase
	Info string
}

// Deserialize 解析响应数据.
func (r *Hello1Response) Deserialize(body []byte) error {
	info, err := decodeHelloInfo(body, hello1InfoOffset)
	if err != nil {
		return err
	}
	r.Info = info
	return nil
}

// String 返回响应描述.
func (r *Hello1Response) String() string { return fmt.Sprintf("Hello1Response{Info:%q}", r.Info) }

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
