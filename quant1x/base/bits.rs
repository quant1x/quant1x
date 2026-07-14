// bits.rs
// 实现：round_up_to_power_of_two
// 说明：返回大于等于 x 的最小 2 的幂次方。与项目其他语言保持一致：
// - 当 x == 0 时返回 1
// - 如果 x 已经是 2 的幂则返回 x
// 实现：使用位传播循环，避免针对特定位宽写死常量。

/// 定义一个 Trait，为所有支持的无符号整数类型提供统一的行为契约
pub trait RoundUpPowerOfTwo: Sized {
    fn round_up_power_of_two(self) -> Self;
}

/// 宏：为指定的无符号整数类型生成高度优化的 Trait 实现
macro_rules! impl_round_up_power_of_two {
    ($type:ty, $bits:expr) => {
        impl RoundUpPowerOfTwo for $type {
            #[inline(always)]
            fn round_up_power_of_two(self) -> Self {
                if self == 0 {
                    return 1;
                }

                // 核心逻辑：先减 1，天然处理了“x 已经是 2 的幂”的情况
                let mut v = self - 1;
                
                // 使用 wrapping_shr 避免小位宽类型(如 u8, u16)在编译期的移位溢出检查报错
                // wrapping_shr 会将移位量对位宽取模，对于超出位宽的移位，等价于移位 0 (即原值)，
                // v |= v 在逻辑上不影响结果，且编译器会将其优化为零开销指令。
                v |= v.wrapping_shr(1);
                v |= v.wrapping_shr(2);
                v |= v.wrapping_shr(4);
                v |= v.wrapping_shr(8);
                v |= v.wrapping_shr(16);
                v |= v.wrapping_shr(32);
                v |= v.wrapping_shr(64);
                
                let mut res = v.wrapping_add(1);
                
                // 溢出防护：如果 v+1 发生无符号整数溢出，结果必然是 0。
                // 此时说明原值 > 2^(bits-1)，安全退化为该类型能表示的最大 2 的幂
                if res == 0 {
                    res = (1 as $type) << ($bits - 1);
                }
                
                res
            }
        }
    };
}

// 为常用类型生成专属的零开销实现
impl_round_up_power_of_two!(u8, 8);
impl_round_up_power_of_two!(u16, 16);
impl_round_up_power_of_two!(u32, 32);
impl_round_up_power_of_two!(u64, 64);
impl_round_up_power_of_two!(u128, 128);
impl_round_up_power_of_two!(usize, core::mem::size_of::<usize>() as u32 * 8);

/// 泛型包装函数：提供与 C++ 模板函数完全一致的调用体验
#[inline(always)]
pub fn round_up_to_power_of_two<T: RoundUpPowerOfTwo>(x: T) -> T {
    x.round_up_power_of_two()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_round_up_u64() {
        assert_eq!(round_up_to_power_of_two(0u64), 1);
        assert_eq!(round_up_to_power_of_two(1u64), 1);
        assert_eq!(round_up_to_power_of_two(2u64), 2);
        assert_eq!(round_up_to_power_of_two(3u64), 4);
        assert_eq!(round_up_to_power_of_two(5u64), 8);
        assert_eq!(round_up_to_power_of_two(1023u64), 1024);
        assert_eq!(round_up_to_power_of_two(1u64 << 63), 1u64 << 63);
        
        // 测试溢出退化逻辑 (输入大于 2^63)
        assert_eq!(round_up_to_power_of_two((1u64 << 63) + 1), 1u64 << 63);
        assert_eq!(round_up_to_power_of_two(u64::MAX), 1u64 << 63);
    }

    #[test]
    fn test_round_up_u8() {
        assert_eq!(round_up_to_power_of_two(0u8), 1);
        assert_eq!(round_up_to_power_of_two(3u8), 4);
        assert_eq!(round_up_to_power_of_two(5u8), 8);
        // u8 最大是 255，大于 128 的数都会退化到 128 (1 << 7)
        assert_eq!(round_up_to_power_of_two(129u8), 128);
        assert_eq!(round_up_to_power_of_two(u8::MAX), 128);
    }

    #[test]
    fn test_round_up_u128() {
        assert_eq!(round_up_to_power_of_two(0u128), 1u128);
        assert_eq!(round_up_to_power_of_two(1u128), 1u128);
        assert_eq!(round_up_to_power_of_two(2u128), 2u128);
        assert_eq!(round_up_to_power_of_two(3u128), 4u128);
        assert_eq!(round_up_to_power_of_two(5u128), 8u128);
        assert_eq!(round_up_to_power_of_two((1u128 << 127) - 1), 1u128 << 127);
        assert_eq!(round_up_to_power_of_two(1u128 << 127), 1u128 << 127);
        // 大于 2^127 的值应退化到 2^127
        assert_eq!(round_up_to_power_of_two((1u128 << 127) + 1), 1u128 << 127);
        assert_eq!(round_up_to_power_of_two(u128::MAX), 1u128 << 127);
    }
}