// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_BASE_GLOBMATCH_H
#define QUANT1X_BASE_GLOBMATCH_H

#include <string_view>
#include <vector>

namespace globmatch {

    /// 单条模式是否匹配整条路径 (锚定)。
    ///
    /// 整条路径从根起逐段比对: `**` 匹配零到多段, 其余段按 `*` / `?` / `[...]`
    /// 通配 (`*` 不跨段、大小写敏感、点开头的名字照常匹配)。模式以 `/**` 结尾时
    /// 要求目录下至少一个条目, 等价 `/**/*`。`path` 或 `pattern` 为空时为假。
    ///
    /// 路径与模式都可写成项目根绝对形式 (前导 `/`), 与相对形式等价: 匹配一律以项目根
    /// 为锚, `/third_party/a.h` 与 `third_party/a.h` 是同一个位置, 两侧可混写。
    ///
    /// @param path `/` 分隔的路径
    /// @param pattern `/` 分隔的模式
    /// @return 整条匹配为真
    bool matches(std::string_view path, std::string_view pattern);

    /// 任一条模式匹配整条路径即为真。
    bool matches_any(std::string_view path, const std::vector<std::string_view> &patterns);

}// namespace globmatch

#endif // QUANT1X_BASE_GLOBMATCH_H
