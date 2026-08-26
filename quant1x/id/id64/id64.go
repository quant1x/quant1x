package id64

import (
	"encoding/base64"
	"encoding/binary"
	"fmt"
)

// EpochMs 是 ID 时间戳的起点（2026-01-01T00:00:00Z，毫秒）。
// 41 位毫秒时间戳可覆盖约 69.7 年（至 2095 年）。
const EpochMs int64 = 1767225600000

const (
	// physicalBits 是时间戳占用的位数（毫秒）。
	physicalBits = 41
	// 布局：1 位符号(恒 0) + physicalBits + workerBits + seqBits = 64，
	// 因此 workerBits + seqBits = 64 - 1 - 41 = 22。
	payloadBits = 22
)

// ID 是 64 位可排序标识（uint64 位模式）。
//
// 位布局（动态位宽）：
//
//	| 1bit 符号(恒 0) | Physical(41bit, epoch 相对毫秒) | NodeID(workerBits) | Seq(seqBits) |
//
// workerBits / seqBits 由节点总数推导（见 WithNodeCount），
// 因此 ID 的 NodeID / Seq 解析需要传入对应的 workerBits。
type ID uint64

// Bytes 返回 BigEndian 的 8 字节表示。
func (id ID) Bytes() [8]byte {
	var b [8]byte
	binary.BigEndian.PutUint64(b[:], uint64(id))
	return b
}

// String 返回 base64url 无填充字符串（8 字节 → 11 字符）。
func (id ID) String() string {
	b := id.Bytes()
	return base64.RawURLEncoding.EncodeToString(b[:])
}

// Physical 返回 epoch 相对毫秒（ID 高 41 位）。
func (id ID) Physical() int64 {
	return int64(uint64(id) >> payloadBits)
}

// NodeID 返回节点标识，workerBits 必须与生成器配置一致。
func (id ID) NodeID(workerBits uint8) uint32 {
	shift := payloadBits - workerBits
	mask := uint32(1)<<workerBits - 1
	return uint32(uint64(id)>>shift) & mask
}

// Seq 返回序列号，workerBits 必须与生成器配置一致。
func (id ID) Seq(workerBits uint8) uint32 {
	shift := payloadBits - workerBits
	return uint32(uint64(id)) & (uint32(1)<<shift - 1)
}

// FromBytes 从 BigEndian 的 8 字节解码 ID。
func FromBytes(b [8]byte) ID {
	return ID(binary.BigEndian.Uint64(b[:]))
}

// checkEpoch 校验 epoch 相对毫秒在 41 位容量内，超限即 panic（防御性）。
func checkEpoch(elapsed int64) int64 {
	if elapsed < 0 {
		panic(fmt.Sprintf("id64: 时钟早于 epoch，elapsed=%d", elapsed))
	}
	if elapsed >= 1<<physicalBits {
		panic(fmt.Sprintf("id64: 时钟超出 41 位容量，elapsed=%d", elapsed))
	}
	return elapsed
}
