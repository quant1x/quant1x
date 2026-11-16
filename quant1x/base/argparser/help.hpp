// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <string_view>

namespace argparser {

    class Parser;
    class UsageError;

    /// 版式: usage 行、分节帮助、错误块与 ANSI 着色。
    ///
    /// 版式固定, 不随内容猜测: 章节顺序是 `Usage` → `Commands` → `Arguments` → `Options` → 自定义分组
    /// → `epilog`; 左列超过半幅时说明文字换到下一行并对齐缩进。颜色只加在标题与选项字面量上,
    /// `[default: ...]` / `[env: ...]` / `[possible values: ...]` 一律保留原样, 便于用例按文本断言。
    ///
    /// 宽度按码点计 (与 Python 的 `len()` 同口径), 因此中文说明与英文说明的折行位置一致。
    class Style {
    public:
        /// 按开关构造; 关闭时所有方法原样返回文本。
        explicit Style(bool enabled = false) : enabled_(enabled) {}

        /// 是否着色。
        [[nodiscard]] bool enabled() const { return enabled_; }

        /// 章节标题与 `Usage:` / `error:` 前缀。
        [[nodiscard]] std::string heading(std::string_view text) const;
        /// 强调。
        [[nodiscard]] std::string bold(std::string_view text) const;
        /// 选项与命令字面量。
        [[nodiscard]] std::string literal(std::string_view text) const;
        /// `error:` 前缀。
        [[nodiscard]] std::string error(std::string_view text) const;
        /// `tip:` 前缀。
        [[nodiscard]] std::string warning(std::string_view text) const;

    private:
        [[nodiscard]] std::string wrap(std::string_view code, std::string_view text) const;

        bool enabled_ = false;
    };

    /// 单行用法文本 (`Usage:` 前缀在内)。
    std::string render_usage(const Parser &parser);

    /// 整段帮助文本, 段间空一行; `long_help` 为真时用长简介与长说明。
    std::string render_help(const Parser &parser, bool long_help = false);

    /// 错误块: 首行 `error:` + 缩进的细节 / 建议 + usage + 帮助提示; `color` 为假时走非着色版式。
    std::string render_error(const UsageError &error, bool color);

}  // namespace argparser
