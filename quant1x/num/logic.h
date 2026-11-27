// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_LOGIC_H
#define QUANT1X_NUM_LOGIC_H 1

/// 逻辑与比较 —— 非零即真, NaN 比较恒为假。

#include <quant1x/num/dtype.h>
#include <quant1x/num/elementwise.h>

#include <cstddef>

namespace quant1x::num {

/// 大于
inline Bools gt(const Series &left, const Series &right) {
    Bools out(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        out[index] = left[index] > right_at(right, index);
    }
    return out;
}

/// 大于等于
inline Bools gte(const Series &left, const Series &right) {
    Bools out(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        out[index] = left[index] >= right_at(right, index);
    }
    return out;
}

/// 小于
inline Bools lt(const Series &left, const Series &right) {
    Bools out(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        out[index] = left[index] < right_at(right, index);
    }
    return out;
}

/// 小于等于
inline Bools lte(const Series &left, const Series &right) {
    Bools out(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        out[index] = left[index] <= right_at(right, index);
    }
    return out;
}

/// 相等
inline Bools equal(const Series &left, const Series &right) {
    Bools out(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        out[index] = left[index] == right_at(right, index);
    }
    return out;
}

/// 不等
inline Bools not_equal(const Series &left, const Series &right) {
    Bools out(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        out[index] = left[index] != right_at(right, index);
    }
    return out;
}

/// 数值转布尔: 非零且非 NaN 为真
inline Bools to_bools(const Series &values) {
    Bools out(values.size());
    for (std::size_t index = 0; index < values.size(); ++index) {
        out[index] = is_true(values[index]);
    }
    return out;
}

/// 逐元素逻辑与, 不足部分视为假
inline Bools logic_and(const Bools &left, const Bools &right) {
    const std::size_t length = left.size() > right.size() ? left.size() : right.size();
    Bools out(length);
    for (std::size_t index = 0; index < length; ++index) {
        const bool lhs = index < left.size() && left[index];
        const bool rhs = index < right.size() && right[index];
        out[index] = lhs && rhs;
    }
    return out;
}

/// 逐元素逻辑或, 不足部分视为假
inline Bools logic_or(const Bools &left, const Bools &right) {
    const std::size_t length = left.size() > right.size() ? left.size() : right.size();
    Bools out(length);
    for (std::size_t index = 0; index < length; ++index) {
        const bool lhs = index < left.size() && left[index];
        const bool rhs = index < right.size() && right[index];
        out[index] = lhs || rhs;
    }
    return out;
}

/// 逐元素逻辑非
inline Bools logic_not(const Bools &values) {
    Bools out(values.size());
    for (std::size_t index = 0; index < values.size(); ++index) {
        out[index] = !values[index];
    }
    return out;
}

/// 是否全真
inline bool all_of(const Bools &values) {
    for (const bool value : values) {
        if (!value) {
            return false;
        }
    }
    return true;
}

/// 是否存在真值
inline bool any_of(const Bools &values) {
    for (const bool value : values) {
        if (value) {
            return true;
        }
    }
    return false;
}

/// 统计真值个数
inline std::size_t count_true(const Bools &values) {
    std::size_t count = 0;
    for (const bool value : values) {
        if (value) {
            ++count;
        }
    }
    return count;
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_LOGIC_H
