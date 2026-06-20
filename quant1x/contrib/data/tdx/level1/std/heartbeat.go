package std

import (
	"fmt"
	"strings"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
)

const HeartbeatInfoLength = 10

// HeartbeatContext 对齐 C++/Rust/Python HeartbeatContext, 合并请求和响应.
type HeartbeatContext struct {
	tdxproto.FrameBase
	Info string
}

// NewHeartbeatContext 构造心跳消息, packet_ctrl=0x02 对齐 C++/Rust.
func NewHeartbeatContext() *HeartbeatContext {
	return &HeartbeatContext{
		FrameBase: tdxproto.NewFrameBase(tdxproto.StdCommandHeartbeat, tdxproto.FlagUncompressed, tdxproto.PacketCtrlHeartbeat),
	}
}

// SerializeRequestBody 心跳无请求体, 对齐 C++/Rust HeartbeatContext::serialize_request_body.
func (h *HeartbeatContext) SerializeRequestBody() []byte { return nil }

// DeserializeResponseBody 解析心跳响应体, 对齐 C++/Rust/Python.
func (h *HeartbeatContext) DeserializeResponseBody(body []byte) error {
	if len(body) < HeartbeatInfoLength {
		return fmt.Errorf("heartbeat response too short: %d", len(body))
	}
	raw := string(body[:HeartbeatInfoLength])
	h.Info = strings.TrimRight(raw, "\x00 ")
	return nil
}

func (h *HeartbeatContext) String() string { return fmt.Sprintf("HeartbeatContext{Info:%q}", h.Info) }
