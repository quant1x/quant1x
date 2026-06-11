package level1

import "fmt"

const hello2InfoOffset = 58

var hello2Payload = []byte{
	0xd5, 0xd0, 0xc9, 0xcc, 0xd6, 0xa4, 0xa8, 0xaf, 0x00, 0x00,
	0x00, 0x8f, 0xc2, 0x25, 0x40, 0x13, 0x00, 0x00, 0xd5, 0x00,
	0xc9, 0xcc, 0xbd, 0xf0, 0xd7, 0xea, 0x00, 0x00, 0x00, 0x02,
}

// Hello2Request 对应第二次握手请求。
type Hello2Request struct{}

// Bytes 序列化请求数据。
func (Hello2Request) Bytes() []byte {
	return buildRequest(StdCommandLogin2, packetTypeRequest, hello2Payload)
}

// Command 返回命令类型。
func (Hello2Request) Command() StdCommand { return StdCommandLogin2 }

// String 返回描述。
func (Hello2Request) String() string { return "Hello2Request" }

// Hello2Response 对应第二次握手响应。
type Hello2Response struct {
	ResponseBase
	Info string
}

// Deserialize 解析响应数据。
func (r *Hello2Response) Deserialize(body []byte) error {
	info, err := decodeHelloInfo(body, hello2InfoOffset)
	if err != nil {
		return err
	}
	r.Info = info
	return nil
}

// String 返回响应描述。
func (r *Hello2Response) String() string { return fmt.Sprintf("Hello2Response{Info:%q}", r.Info) }
