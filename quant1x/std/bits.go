package std

const (
	MaxPower2 uint64 = 1 << (64 - 1)
)

// HighestOneBit 返回大于等于 x 的最小 2 的幂次方.
// 特殊情况：
//
//	x = 0 → 返回 1<<63(可根据需求调整)
//	x 是 2 的幂 → 直接返回 x
//
//go:noinline
func HighestOneBit(x uint64) uint64 {
	return nativeHighestOneBit(x)
}

//go:noinline
func nativeHighestOneBit(x uint64) uint64 {
	origin := x
	// 判断 x 是否为 2 的幂
	isPower2 := ((origin & (origin - 1)) == 0) && (origin != 0)
	if isPower2 {
		return origin // 快速路径: x 是 2 的幂
	}
	// 位展开：将最高位后的所有位置 1
	// HD, Figure 3-1
	x |= x >> 1
	x |= x >> 2
	x |= x >> 4
	x |= x >> 8
	x |= x >> 16
	x |= x >> 32
	//x |= (x >> 64)
	//x = x - (x >> 1)
	// 提取最高位
	x = x & ^(x >> 1)
	// 若结果小于原值，左移一位(确保结果 ≥ 原值)
	if x < origin {
		x <<= 1
	}
	// 处理 x = 0 的特殊情况(输入为 0 时)
	if x == 0 {
		x = MaxPower2
	}
	return x
}

//go:noinline
//go:noescape
func highestOneBit(x uint64) uint64
