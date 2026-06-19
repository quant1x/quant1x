package std

import (
	"fmt"
	"strings"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx"
)

const heartbeatInfoLength = 10

// HeartbeatContext mirrors C++ HeartbeatContext.
type HeartbeatContext struct{}

// Bytes serializes the heartbeat request payload.
func (HeartbeatContext) Bytes() []byte {
	return tdx.BuildRequest(tdx.StdCommandHeartbeat, tdx.PacketCtrlHeartbeat, nil)
}

// Command returns the heartbeat command identifier.
func (HeartbeatContext) Command() tdx.StdCommand { return tdx.StdCommandHeartbeat }

// String returns a description of the request.
func (HeartbeatContext) String() string { return "HeartbeatContext" }

// HeartbeatResponse mirrors C++ HeartbeatResponse.
type HeartbeatResponse struct {
	tdx.ResponseBase
	Info string
}

// Deserialize parses heartbeat response payload.
func (r *HeartbeatResponse) Deserialize(body []byte) error {
	if len(body) < heartbeatInfoLength {
		return fmt.Errorf("heartbeat response too short: %d", len(body))
	}
	raw := string(body[:heartbeatInfoLength])
	r.Info = strings.TrimRight(raw, "\x00 ")
	return nil
}

// String returns a description of the response.
func (r *HeartbeatResponse) String() string {
	return fmt.Sprintf("HeartbeatResponse{Info:%q}", r.Info)
}
