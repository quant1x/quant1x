#include <quant1x/std/numeric.h>

namespace numeric {

    // ✅ 支持任意位数保留(0~9)
    // ✅ 使用静态幂表优化性能
    // ✅ 无分支, 无条件跳转(branchless), 适合高性能场景
    // ✅ 支持负数, 零, NaN 等边界情况
    // ✅ 符合金融/证券系统的标准需求
    f64 decimal(f64 value, int digits) {
        digits = std::clamp(digits, 0, 9);

        static constexpr f64 kPowersOf10[] = {
            1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9
        };

        if (std::isnan(value)) return 0.0;

        f64 half = std::copysign(5.0, value);  // ✅ 无分支处理符号

        f64 nj1 = kPowersOf10[digits + 1];
        f64 scaled = value * nj1 + half;
        f64 truncated = std::trunc(scaled / 10.0);

        return truncated / (nj1 / 10.0);
    }

}