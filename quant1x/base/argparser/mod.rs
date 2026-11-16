// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 命令行解析: Python `argparse` 的语义 + Rust `clap` 的交互。
//!
//! 语义以 `quant1x/base/argparser/*.py` 为准。公开路径即语义路径, 不做再导出:
//! `quant1x::base::argparser::spec::Argument`、`quant1x::base::argparser::error::UsageError`。
//!
//! - [`spec`] —— 参数本体: 取值、动作、数量形态、转换器、参数定义与命名空间;
//! - [`error`] —— 失败口径: 用法错误类别、帮助 / 版本控制流、相似项建议;
//! - [`parser`] —— 命令定义与解析引擎: 选项匹配、位置参数分配、子命令、校验;
//! - [`help`] —— 版式: usage 行、分节帮助、错误块与 ANSI 着色;
//! - [`complete`] —— 补全: 由命令定义生成 bash / zsh / fish 脚本。

pub mod complete;
pub mod error;
pub mod help;
pub mod parser;
pub mod spec;
