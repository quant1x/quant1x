// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace argparser {

    class Parser;

    /// 支持的 shell, 按登记顺序。
    inline constexpr std::array<std::string_view, 3> SHELLS{"bash", "zsh", "fish"};

    /// 生成 `shell` 的补全脚本; `prog` 缺省取 `parser.prog()`。
    ///
    /// 候选词只看「已敲定的子命令链」: 选项名、子命令名、位置参数的 `choices`; 不区分「正在写取值」的
    /// 语境, 也不补文件路径。`shell` 不是 `bash` / `zsh` / `fish` 之一时抛 `std::invalid_argument`。
    std::string complete(const Parser &parser, std::string_view shell, std::optional<std::string> prog = std::nullopt);

}  // namespace argparser
