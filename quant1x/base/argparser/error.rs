// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 解析失败与控制流异常, 以及"你是不是想输入"的相似项建议。
//!
//! 三类失败的分工:
//!
//! - [`UsageError`]: 命令行输入不合法 (未知参数、缺值、取值非法、校验未通过), 进程退出码 2;
//! - [`ParseError::Help`] / [`ParseError::Version`]: 请求打印帮助 / 版本, 不算错误, 退出码 0。
//!
//! 色版由帮助渲染层负责, 这里的 `Display` 是**非着色**文本, 也是用例的断言对象。

use std::fmt;

/// 用法错误的类别, 对应 clap 的 `ErrorKind`。
///
/// 调用方按类别分支, 不必匹配文案; 文案改了不影响判据。
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ErrorKind {
    /// 认不出的参数。
    UnknownArgument,
    /// 长选项前缀命中多个。
    AmbiguousArgument,
    /// 该给取值却没给。
    MissingValue,
    /// 取值转换失败或不在可选值内。
    InvalidValue,
    /// 缺必填项。
    MissingRequired,
    /// 冲突或互斥组不满足。
    Conflict,
    /// 其余用法问题。
    Other,
}

/// 命令行输入不合法。
///
/// `kind` 是类别判据; `reason` 接在首行冒号之后 (如类型转换失败的原因); `lines` 逐条缩进
/// 列在首行之下 (如可选值清单、缺失的必填项); `tips` 是 `tip:` 开头的建议行。
#[derive(Debug, Clone, PartialEq)]
pub struct UsageError {
    /// 首行正文。
    pub message: String,
    /// 类别判据。
    pub kind: ErrorKind,
    /// 首行冒号之后的原因。
    pub reason: Option<String>,
    /// 首行之下的缩进明细。
    pub lines: Vec<String>,
    /// `tip:` 开头的建议。
    pub tips: Vec<String>,
    /// usage 行正文。
    pub usage: Option<String>,
    /// 末尾的帮助提示。
    pub hint: Option<String>,
}

impl UsageError {
    /// 只给首行正文的用法错误; 类别为 [`ErrorKind::Other`], 末尾带帮助提示。
    pub fn new(message: impl Into<String>) -> Self {
        Self {
            message: message.into(),
            kind: ErrorKind::Other,
            reason: None,
            lines: Vec::new(),
            tips: Vec::new(),
            usage: None,
            hint: Some("For more information, try '--help'.".to_string()),
        }
    }

    /// 改类别。
    pub fn with_kind(mut self, kind: ErrorKind) -> Self {
        self.kind = kind;
        self
    }

    /// 补首行冒号之后的原因。
    pub fn with_reason(mut self, reason: impl Into<String>) -> Self {
        self.reason = Some(reason.into());
        self
    }

    /// 补缩进明细。
    pub fn with_lines<I, S>(mut self, lines: I) -> Self
    where
        I: IntoIterator<Item = S>,
        S: Into<String>,
    {
        self.lines = lines.into_iter().map(Into::into).collect();
        self
    }

    /// 补建议行。
    pub fn with_tips<I, S>(mut self, tips: I) -> Self
    where
        I: IntoIterator<Item = S>,
        S: Into<String>,
    {
        self.tips = tips.into_iter().map(Into::into).collect();
        self
    }

    /// 补 usage 行正文。
    pub fn with_usage(mut self, usage: impl Into<String>) -> Self {
        self.usage = Some(usage.into());
        self
    }

    /// 去掉末尾的帮助提示 (命令没有帮助选项时用)。
    pub fn without_hint(mut self) -> Self {
        self.hint = None;
        self
    }

    /// 进程退出码。
    pub fn exit_code(&self) -> i32 {
        2
    }
}

impl fmt::Display for UsageError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "error: {}", self.message)?;
        if let Some(reason) = &self.reason {
            write!(f, ": {reason}")?;
        }
        for line in &self.lines {
            write!(f, "\n  {line}")?;
        }
        if !self.tips.is_empty() {
            for tip in &self.tips {
                write!(f, "\n\n  tip: {tip}")?;
            }
        }
        if let Some(usage) = &self.usage {
            write!(f, "\n\nUsage: {usage}")?;
        }
        if let Some(hint) = &self.hint {
            write!(f, "\n\n{hint}")?;
        }
        Ok(())
    }
}

impl std::error::Error for UsageError {}

/// 解析入口的失败与控制流, 对应 Python 的 `UsageError` / `HelpRequested` / `VersionRequested`。
#[derive(Debug, Clone, PartialEq)]
pub enum ParseError {
    /// 命令行输入不合法, 退出码 2。
    Usage(Box<UsageError>),
    /// `-h` / `--help` 或 `help` 子命令, 退出码 0。
    Help {
        /// 走长帮助; `-h` 与不带参数的 `help` 为假, `--help` 为真。
        long: bool,
        /// 目标命令的全名, 供渲染层定位到对应命令。
        prog: String,
    },
    /// `-V` / `--version`, 退出码 0。
    Version {
        /// 已经拼好的 `prog version` 行。
        text: String,
    },
    /// 定义期错误在解析时才暴露 (命令链 `dest` 重复、引用未登记的参数), 退出码 1。
    ///
    /// Python 侧对应直接抛出的 `ValueError`, 不属于用法错误。
    Definition {
        /// 出错原因。
        message: String,
    },
}

impl ParseError {
    /// 进程退出码: 用法错误 2, 帮助与版本 0, 定义期错误 1。
    pub fn exit_code(&self) -> i32 {
        match self {
            Self::Usage(error) => error.exit_code(),
            Self::Help { .. } | Self::Version { .. } => 0,
            Self::Definition { .. } => 1,
        }
    }
}

impl From<UsageError> for ParseError {
    fn from(error: UsageError) -> Self {
        Self::Usage(Box::new(error))
    }
}

impl fmt::Display for ParseError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::Usage(error) => write!(f, "{error}"),
            Self::Help { long, .. } => f.write_str(if *long { "--help" } else { "-h" }),
            Self::Version { text } => f.write_str(text),
            Self::Definition { message } => f.write_str(message),
        }
    }
}

impl std::error::Error for ParseError {}

/// Jaro 相似度, 匹配窗口为 `max(长度) // 2 - 1`。
fn jaro(left: &str, right: &str) -> f64 {
    if left == right {
        return 1.0;
    }
    let left_chars: Vec<char> = left.chars().collect();
    let right_chars: Vec<char> = right.chars().collect();
    let left_len = left_chars.len();
    let right_len = right_chars.len();
    if left_len == 0 || right_len == 0 {
        return 0.0;
    }
    let window = (left_len.max(right_len) / 2).saturating_sub(1);
    let mut left_hit = vec![false; left_len];
    let mut right_hit = vec![false; right_len];
    let mut matches = 0usize;
    for (index, char) in left_chars.iter().enumerate() {
        let start = index.saturating_sub(window);
        let stop = (index + window + 1).min(right_len);
        for position in start..stop {
            if !right_hit[position] && right_chars[position] == *char {
                left_hit[index] = true;
                right_hit[position] = true;
                matches += 1;
                break;
            }
        }
    }
    if matches == 0 {
        return 0.0;
    }
    let mut transpositions = 0usize;
    let mut cursor = 0usize;
    for index in 0..left_len {
        if !left_hit[index] {
            continue;
        }
        while !right_hit[cursor] {
            cursor += 1;
        }
        if left_chars[index] != right_chars[cursor] {
            transpositions += 1;
        }
        cursor += 1;
    }
    transpositions /= 2;
    let matched = matches as f64;
    (matched / left_len as f64
        + matched / right_len as f64
        + (matched - transpositions as f64) / matched)
        / 3.0
}

/// Jaro-Winkler 相似度: 对相同前缀 (最多 4 字符) 加权。
pub(crate) fn jaro_winkler(left: &str, right: &str) -> f64 {
    let score = jaro(left, right);
    let mut prefix = 0usize;
    for (a, b) in left.chars().zip(right.chars()) {
        if a != b || prefix == 4 {
            break;
        }
        prefix += 1;
    }
    score + prefix as f64 * 0.1 * (1.0 - score)
}

/// 在候选里挑最像 `word` 的一个; 相似度不到阈值就返回 `None` (宁缺勿滥)。
///
/// 阈值与 clap 的 `did_you_mean` 一致 (Jaro-Winkler > 0.8), 与 Python spec 逐位对齐。
pub fn best_match<'a, S>(word: &str, candidates: &'a [S]) -> Option<&'a str>
where
    S: AsRef<str>,
{
    const THRESHOLD: f64 = 0.8;
    let mut best: Option<&str> = None;
    let mut best_score = THRESHOLD;
    for candidate in candidates {
        let candidate = candidate.as_ref();
        if candidate.is_empty() || candidate == word {
            continue;
        }
        let score = jaro_winkler(word, candidate);
        if score > best_score {
            best = Some(candidate);
            best_score = score;
        }
    }
    best
}

/// 把可选值拼成 `a, b, c` 形态。
pub fn format_items<T: fmt::Display>(values: &[T]) -> String {
    values
        .iter()
        .map(|value| value.to_string())
        .collect::<Vec<_>>()
        .join(", ")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn jaro_winkler_rewards_shared_prefix() {
        assert_eq!(jaro_winkler("--flag", "--flag"), 1.0);
        assert!(jaro_winkler("--flah", "--flag") > 0.8);
        // 期望值由 Python spec 生成: `jaro_winkler("--verbose", "--version")`
        assert!((jaro_winkler("--verbose", "--version") - 0.882_539_682_539_682_5).abs() < 1e-12);
        // 前缀只有 "--v" 相同时不越过阈值
        assert!(jaro_winkler("--vivid", "--version") < 0.8);
        assert_eq!(jaro_winkler("", "abc"), 0.0);
        assert_eq!(jaro_winkler("abc", ""), 0.0);
    }

    #[test]
    fn best_match_picks_nearest_above_threshold() {
        let candidates = ["--help", "--flag", "--output"];
        assert_eq!(best_match("--flah", &candidates), Some("--flag"));
        assert_eq!(best_match("--zzz", &candidates), None);
        assert_eq!(best_match("--output", &candidates), None);
        assert_eq!(best_match("--outpu", &candidates), Some("--output"));
    }

    #[test]
    fn format_items_joins_with_comma_space() {
        assert_eq!(format_items(&["a", "b", "c"]), "a, b, c");
        assert_eq!(format_items::<&str>(&[]), "");
        assert_eq!(format_items(&[1, 2]), "1, 2");
    }

    #[test]
    fn usage_error_renders_head_lines_tips_usage_and_hint() {
        let error = UsageError::new("invalid value 'x' for '--level <LEVEL>'")
            .with_kind(ErrorKind::InvalidValue)
            .with_reason("invalid digit found in string")
            .with_lines(["[possible values: 1, 2]"])
            .with_tips(["a similar value exists: '2'"])
            .with_usage("demo [OPTIONS]");
        let expected = concat!(
            "error: invalid value 'x' for '--level <LEVEL>': invalid digit found in string\n",
            "  [possible values: 1, 2]\n",
            "\n",
            "  tip: a similar value exists: '2'\n",
            "\n",
            "Usage: demo [OPTIONS]\n",
            "\n",
            "For more information, try '--help'."
        );
        assert_eq!(error.to_string(), expected);
        assert_eq!(error.exit_code(), 2);
    }

    #[test]
    fn usage_error_without_hint_drops_the_last_block() {
        let error = UsageError::new("unexpected argument '--x' found").without_hint();
        assert_eq!(error.to_string(), "error: unexpected argument '--x' found");
        assert_eq!(error.kind, ErrorKind::Other);
    }

    #[test]
    fn parse_error_exit_codes_follow_kind() {
        let usage = ParseError::from(UsageError::new("bad"));
        assert_eq!(usage.exit_code(), 2);
        let help = ParseError::Help {
            long: true,
            prog: "tool build".to_string(),
        };
        assert_eq!(help.exit_code(), 0);
        assert_eq!(help.to_string(), "--help");
        let version = ParseError::Version {
            text: "tool 1.0.0".to_string(),
        };
        assert_eq!(version.exit_code(), 0);
        assert_eq!(version.to_string(), "tool 1.0.0");
    }
}
