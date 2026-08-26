package id

import (
	"encoding/base64"
	"encoding/binary"
)

// ID 是按原始字节编码的 128 位可排序标识。
// 它可比较，也可以直接作为 map 键使用。
type ID [16]byte

// Bytes 返回原始字节切片。
func (id ID) Bytes() []byte {
	return id[:]
}

// String 返回不带填充的 base64url 字符串。
func (id ID) String() string {
	return base64.RawURLEncoding.EncodeToString(id[:])
}

// NodeID 提取节点标识。
func (id ID) NodeID() uint32 {
	return binary.BigEndian.Uint32(id[8:12])
}

// Seq 提取 32 位序列号。
func (id ID) Seq() uint32 {
	return binary.BigEndian.Uint32(id[12:16])
}

// HLC 提取原始 HLC 高位时间戳。
func (id ID) HLC() uint64 {
	return binary.BigEndian.Uint64(id[0:8])
}
