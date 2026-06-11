package level1

import (
	"fmt"
	"strings"
)

const heartbeatInfoLength = 10

// HeartbeatRequest mirrors C++ HeartbeatRequest.
type HeartbeatRequest struct{}

// Bytes serializes the heartbeat request payload.
func (HeartbeatRequest) Bytes() []byte {
	return buildRequest(StdCommandHeartbeat, packetTypeHeartbeat, nil)
}

// Command returns the heartbeat command identifier.
func (HeartbeatRequest) Command() StdCommand { return StdCommandHeartbeat }

// String returns a description of the request.
func (HeartbeatRequest) String() string { return "HeartbeatRequest" }

// HeartbeatResponse mirrors C++ HeartbeatResponse.
type HeartbeatResponse struct {
	ResponseBase
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
