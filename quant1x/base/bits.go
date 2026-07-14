package base

const (
	MaxPower2 uint64 = 1 << (64 - 1)
)

// RoundUpToPowerOfTwo 返回大于等于 x 的最小 2 的幂次方（驼峰命名，供 Go 使用）.
// 特殊情况:
//   - x = 0 -> 返回 1（在 ring buffer 场景更合理）
//   - 如果 x 已经是 2 的幂，直接返回 x
//
// 实现：使用可扩展的位传播循环，支持任意无符号宽度。
//
//go:noinline
func RoundUpToPowerOfTwo(x uint64) uint64 {
	// 使用纯 Go 实现
	return nativeRoundUpToPowerOfTwo(x)
}

// nativeRoundUpToPowerOfTwo: 实际实现（可被其他 wrapper 调用）
//
//go:noinline
func nativeRoundUpToPowerOfTwo(x uint64) uint64 {
	if x == 0 {
		return 1
	}
	// 如果已经是 2 的幂，直接返回
	if (x & (x - 1)) == 0 {
		return x
	}
	v := x - 1
	const bits = 64
	for shift := 1; shift < bits; shift <<= 1 {
		v |= v >> shift
	}
	v++
	// 溢出防护：如果 v+1 回绕为 0，说明超出该类型能表示的最大 2 的幂，退化到 1 << (bits-1)
	if v == 0 {
		return uint64(1) << (bits - 1)
	}
	return v
}
