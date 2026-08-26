// Package id 提供 128 位无符号整数和基于 HLC 的 ID 生成器。
package id128

import (
	"encoding/binary"
	"math/bits"
)

/*
|---------------------------------------------------------------|
|                   Uint128（真实 128 位）                     |
|---------------------------------------------------------------|
|  Hi (64bit) | Lo (64bit)                                      |
|---------------------------------------------------------------|
|  使用 BigEndian，便于网络/数据库/跨语言保持一致               |
|---------------------------------------------------------------|
*/
type Uint128 struct {
	Hi uint64
	Lo uint64
}

// =====================
// 构造函数
// =====================

// NewUint128 使用高低 64 位构造 Uint128。
func NewUint128(hi, lo uint64) Uint128 {
	return Uint128{Hi: hi, Lo: lo}
}

// From64 使用单个 64 位值构造 Uint128，并放入低位。
func From64(v uint64) Uint128 {
	return Uint128{Lo: v}
}

// FromBytes 从 BigEndian [16]byte 解码 Uint128。
func FromBytes(b [16]byte) Uint128 {
	return Uint128{
		Hi: binary.BigEndian.Uint64(b[0:8]),
		Lo: binary.BigEndian.Uint64(b[8:16]),
	}
}

// Bytes 将 Uint128 编码为 BigEndian [16]byte。
func (u Uint128) Bytes() [16]byte {
	var b [16]byte
	binary.BigEndian.PutUint64(b[0:8], u.Hi)
	binary.BigEndian.PutUint64(b[8:16], u.Lo)
	return b
}

// =====================
// 常量
// =====================

var (
	Uint128Zero = Uint128{}
	Uint128One  = Uint128{Lo: 1}
	Uint128Max  = Uint128{Hi: ^uint64(0), Lo: ^uint64(0)}
)

// =====================
// 比较
// =====================

// Compare 返回：
//
//	-1 当 u < v
//	 0 当 u == v
//	+1 当 u > v
func (u Uint128) Compare(v Uint128) int {
	if u.Hi < v.Hi {
		return -1
	}
	if u.Hi > v.Hi {
		return 1
	}
	if u.Lo < v.Lo {
		return -1
	}
	if u.Lo > v.Lo {
		return 1
	}
	return 0
}

func (u Uint128) Lt(v Uint128) bool { return u.Compare(v) < 0 }
func (u Uint128) Le(v Uint128) bool { return u.Compare(v) <= 0 }
func (u Uint128) Gt(v Uint128) bool { return u.Compare(v) > 0 }
func (u Uint128) Ge(v Uint128) bool { return u.Compare(v) >= 0 }
func (u Uint128) Eq(v Uint128) bool { return u == v }

// =====================
// 算术
// =====================

// Add 执行带进位的 128 位加法。
func (u Uint128) Add(v Uint128) Uint128 {
	lo, carry := bits.Add64(u.Lo, v.Lo, 0)
	hi, _ := bits.Add64(u.Hi, v.Hi, carry)
	return Uint128{Hi: hi, Lo: lo}
}

// Sub 执行带借位的 128 位减法。
func (u Uint128) Sub(v Uint128) Uint128 {
	lo, borrow := bits.Sub64(u.Lo, v.Lo, 0)
	hi, _ := bits.Sub64(u.Hi, v.Hi, borrow)
	return Uint128{Hi: hi, Lo: lo}
}

// Inc 返回 u + 1。
func (u Uint128) Inc() Uint128 {
	return u.Add(Uint128One)
}

// Dec 返回 u - 1。
func (u Uint128) Dec() Uint128 {
	return u.Sub(Uint128One)
}

// =====================
// 位运算
// =====================

// Lsh 将 u 左移 n 位。
func (u Uint128) Lsh(n uint) Uint128 {
	if n >= 128 {
		return Uint128Zero
	}
	if n >= 64 {
		return Uint128{
			Hi: u.Lo << (n - 64),
			Lo: 0,
		}
	}
	return Uint128{
		Hi: (u.Hi << n) | (u.Lo >> (64 - n)),
		Lo: u.Lo << n,
	}
}

// Rsh 将 u 右移 n 位。
func (u Uint128) Rsh(n uint) Uint128 {
	if n >= 128 {
		return Uint128Zero
	}
	if n >= 64 {
		return Uint128{
			Hi: 0,
			Lo: u.Hi >> (n - 64),
		}
	}
	return Uint128{
		Hi: u.Hi >> n,
		Lo: (u.Hi << (64 - n)) | (u.Lo >> n),
	}
}

// Or 执行按位或。
func (u Uint128) Or(v Uint128) Uint128 {
	return Uint128{
		Hi: u.Hi | v.Hi,
		Lo: u.Lo | v.Lo,
	}
}

// And 执行按位与。
func (u Uint128) And(v Uint128) Uint128 {
	return Uint128{
		Hi: u.Hi & v.Hi,
		Lo: u.Lo & v.Lo,
	}
}

// Xor 执行按位异或。
func (u Uint128) Xor(v Uint128) Uint128 {
	return Uint128{
		Hi: u.Hi ^ v.Hi,
		Lo: u.Lo ^ v.Lo,
	}
}

// Not 执行按位取反。
func (u Uint128) Not() Uint128 {
	return Uint128{
		Hi: ^u.Hi,
		Lo: ^u.Lo,
	}
}

// =====================
// 辅助方法
// =====================

// IsZero 判断 u 是否为 0。
func (u Uint128) IsZero() bool {
	return u == Uint128Zero
}

// High64 返回高 64 位。
func (u Uint128) High64() uint64 {
	return u.Hi
}

// Low64 返回低 64 位。
func (u Uint128) Low64() uint64 {
	return u.Lo
}
