package id

import (
	"encoding/base64"
	"encoding/binary"
	"fmt"
)

const (
	EpochMs      int64 = 1767225600000
	payloadBits        = 22
	physicalBits       = 41
)

// ID is a 64-bit sortable distributed identifier.
type ID uint64

func (id ID) Bytes() [8]byte {
	var result [8]byte
	binary.BigEndian.PutUint64(result[:], uint64(id))
	return result
}

func (id ID) String() string {
	bytes := id.Bytes()
	return base64.RawURLEncoding.EncodeToString(bytes[:])
}

func (id ID) Physical() int64 { return int64(uint64(id) >> payloadBits) }

func (id ID) NodeID(workerBits uint8) uint32 {
	shift := payloadBits - workerBits
	return uint32(uint64(id)>>shift) & (uint32(1)<<workerBits - 1)
}

func (id ID) Seq(workerBits uint8) uint32 {
	shift := payloadBits - workerBits
	return uint32(id) & (uint32(1)<<shift - 1)
}

func FromBytes(bytes [8]byte) ID { return ID(binary.BigEndian.Uint64(bytes[:])) }

func checkEpoch(elapsed int64) int64 {
	if elapsed < 0 || elapsed >= 1<<physicalBits {
		panic(fmt.Sprintf("distributed/id: epoch elapsed out of range: %d", elapsed))
	}
	return elapsed
}
