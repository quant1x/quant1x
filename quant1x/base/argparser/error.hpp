// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace argparser {

    /// 用法错误的类别, 对应 clap 的 `ErrorKind`。
    ///
    /// 调用方按类别分支, 不必匹配文案; 文案改了不影响判据。
    enum class ErrorKind {
        OTHER,
        UNKNOWN_ARGUMENT,
        AMBIGUOUS_ARGUMENT,
        MISSING_VALUE,
        INVALID_VALUE,
        MISSING_REQUIRED,
        CONFLICT,
    };

    /// 类别名, 与 Python 的 `ErrorKind.value` 同形。
    std::string_view error_kind_name(ErrorKind kind);

    /// 命令行输入不合法。
    ///
    /// `kind` 是类别判据; `reason` 接在首行冒号之后; `lines` 逐条缩进列在首行之下;
    /// `tips` 是 `tip:` 开头的建议行。
    class UsageError {
    public:
        UsageError() = default;
        explicit UsageError(std::string message) : message_(std::move(message)) {}

        /// 首行正文。
        [[nodiscard]] const std::string &message() const { return message_; }
        /// 首行冒号之后的原因。
        [[nodiscard]] const std::string &reason() const { return reason_; }
        /// 缩进的明细行。
        [[nodiscard]] const std::vector<std::string> &lines() const { return lines_; }
        /// `tip:` 开头的建议行。
        [[nodiscard]] const std::vector<std::string> &tips() const { return tips_; }
        /// usage 行。
        [[nodiscard]] const std::string &usage() const { return usage_; }
        /// 帮助提示行。
        [[nodiscard]] const std::string &hint() const { return hint_; }
        /// 类别判据。
        [[nodiscard]] ErrorKind kind() const { return kind_; }
        /// 用法错误的退出码。
        [[nodiscard]] static int exit_code() { return 2; }

        /// 非着色版式: 首行 `error:` + 缩进的细节 / 建议 + usage + 帮助提示。
        [[nodiscard]] std::string render() const;

        void set_kind(ErrorKind kind) { kind_ = kind; }
        void set_reason(std::string reason) { reason_ = std::move(reason); }
        void set_usage(std::string usage) { usage_ = std::move(usage); }
        void set_hint(std::string hint) { hint_ = std::move(hint); }
        void add_line(std::string line) { lines_.push_back(std::move(line)); }
        void add_tip(std::string tip) { tips_.push_back(std::move(tip)); }
        void add_lines(std::vector<std::string> lines) {
            for (std::string &line : lines) {
                lines_.push_back(std::move(line));
            }
        }

    private:
        std::string              message_;
        ErrorKind                kind_ = ErrorKind::OTHER;
        std::string              reason_;
        std::vector<std::string> lines_;
        std::vector<std::string> tips_;
        std::string              usage_;
        std::string              hint_;
    };

    /// `-h` / `--help`: 请求帮助; `long_form` 为真表示走长帮助 (`--help`)。
    struct HelpRequested {
        bool        long_form = false;
        std::string prog;
    };

    /// `-V` / `--version`: 请求打印版本。
    struct VersionRequested {
        std::string text;
    };

    /// 定义期错误, 对应 Python 侧直接抛出的 `ValueError`, 不属于用法错误。
    struct DefinitionError {
        std::string message;
    };

    /// 解析失败或控制流请求。
    using ParseError = std::variant<UsageError, HelpRequested, VersionRequested, DefinitionError>;

    /// 取错误对应的进程退出码: 用法错误 2, 帮助与版本 0, 定义期错误 1。
    int exit_code(const ParseError &error);

    /// 错误的渲染文本; 用法错误走非着色版式。
    std::string render(const ParseError &error);

    /// Jaro 相似度, 匹配窗口为 `max(长度) / 2 - 1`。
    double jaro(std::string_view left, std::string_view right);

    /// Jaro-Winkler 相似度: 对相同前缀 (最多 4 字符) 加权。
    double jaro_winkler(std::string_view left, std::string_view right);

    /// 相似度阈值, 与 clap 的 `did_you_mean` 一致 (Jaro-Winkler > 0.8)。
    inline constexpr double MATCH_THRESHOLD = 0.8;

    /// 在候选里挑最像 `word` 的一个; 相似度不到阈值时为空 (宁缺勿滥)。
    std::optional<std::string> best_match(std::string_view word, const std::vector<std::string> &candidates);

    /// 同 `best_match`, 阈值由调用方给。
    std::optional<std::string>
    best_match_at(std::string_view word, const std::vector<std::string> &candidates, double threshold);

}  // namespace argparser
