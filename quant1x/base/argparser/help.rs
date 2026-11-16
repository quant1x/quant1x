// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 版式: usage 行、分节帮助、错误块与 ANSI 着色。
//!
//! 版式固定, 不随内容猜测: 章节顺序是 `Usage` → `Commands` → `Arguments` → `Options` → 自定义分组
//! → `epilog`; 左列超过半幅时说明文字换到下一行并对齐缩进。颜色只加在标题与选项字面量上,
//! `[default: ...]` / `[env: ...]` / `[possible values: ...]` 一律保留原样, 便于用例按文本断言。
//!
//! 宽度按字符计 (与 Python 的 `len()` 同口径), 因此中文说明与英文说明的折行位置一致。

use super::error::{UsageError, format_items};
use super::parser::Parser;
use super::spec::{Action, Argument, DefaultValue, Value};

const RESET: &str = "\x1b[0m";

/// ANSI 着色开关; 关闭时所有方法原样返回文本, 因此版式与宽度计算只看可见字符。
#[derive(Debug, Clone, Copy, Default)]
pub struct Style {
    enabled: bool,
}

impl Style {
    /// 按开关构造。
    pub fn new(enabled: bool) -> Self {
        Self { enabled }
    }

    /// 是否着色。
    pub fn enabled(&self) -> bool {
        self.enabled
    }

    /// 给一段文字套上 ANSI 颜色码; 空串或关闭着色时原样返回。
    fn wrap(&self, code: &str, text: &str) -> String {
        if !self.enabled || text.is_empty() {
            return text.to_string();
        }
        format!("{code}{text}{RESET}")
    }

    /// 章节标题与 `Usage:` / `error:` 前缀。
    pub fn heading(&self, text: &str) -> String {
        self.wrap("\x1b[1m\x1b[4m", text)
    }

    /// 强调。
    pub fn bold(&self, text: &str) -> String {
        self.wrap("\x1b[1m", text)
    }

    /// 选项与命令字面量。
    pub fn literal(&self, text: &str) -> String {
        self.wrap("\x1b[32m", text)
    }

    /// `error:` 前缀。
    pub fn error(&self, text: &str) -> String {
        self.wrap("\x1b[31m", text)
    }

    /// `tip:` 前缀。
    pub fn warning(&self, text: &str) -> String {
        self.wrap("\x1b[33m", text)
    }
}

/// 可见宽度: 字符个数, 与 Python 的 `len()` 同口径。
fn width_of(text: &str) -> usize {
    text.chars().count()
}

/// 单段文字按宽度折行; `width` 是**含缩进在内**的整行宽度。
fn wrap_one(
    paragraph: &str,
    width: usize,
    initial_indent: &str,
    subsequent_indent: &str,
) -> Vec<String> {
    let width = width.max(20);
    let mut lines = Vec::new();
    let mut line = String::from(initial_indent);
    let mut filled = false;
    for word in paragraph.split_whitespace() {
        if filled && width_of(&line) + 1 + width_of(word) > width {
            lines.push(std::mem::take(&mut line));
            line.push_str(subsequent_indent);
            filled = false;
        }
        if filled {
            line.push(' ');
        }
        line.push_str(word);
        filled = true;
    }
    if filled {
        lines.push(line);
    } else {
        lines.push(String::new());
    }
    lines
}

/// 按段落换行; 段间保留一个空行位。
fn wrap_paragraphs(
    text: &str,
    width: usize,
    indent: &str,
    first_indent: Option<&str>,
) -> Vec<String> {
    let mut lines = Vec::new();
    for (index, paragraph) in text.split('\n').enumerate() {
        let head = if index == 0 {
            first_indent.unwrap_or(indent)
        } else {
            indent
        };
        lines.extend(wrap_one(paragraph, width, head, indent));
    }
    lines
}

/// 一行说明文字: 帮助正文 + 默认值 / 环境变量 / 可选值。
fn argument_text(argument: &Argument, long: bool) -> String {
    let mut text = String::new();
    let preferred = if long {
        argument.long_help.as_deref()
    } else {
        None
    };
    match preferred.filter(|body| !body.is_empty()) {
        Some(body) => text.push_str(body),
        None => {
            if let Some(body) = argument.help.as_deref() {
                text.push_str(body);
            }
        }
    }
    let mut extras: Vec<String> = Vec::new();
    let shows_default = matches!(
        argument.action,
        Action::Store | Action::Append | Action::Extend
    );
    if shows_default
        && !argument.hide_default
        && let DefaultValue::Value(value) = &argument.default
        && !matches!(value, Value::Null)
    {
        extras.push(format!("[default: {value}]"));
    }
    if let Some(env) = &argument.env {
        extras.push(format!("[env: {env}]"));
    }
    let choices = argument.choice_values().unwrap_or_default();
    if !choices.is_empty() {
        extras.push(format!("[possible values: {}]", format_items(&choices)));
    }
    if !extras.is_empty() {
        text = format!("{text} {}", extras.join(" ")).trim().to_string();
    }
    if long {
        let notes: Vec<String> = choices
            .iter()
            .filter_map(|value| {
                argument
                    .choice_help(value)
                    .map(|note| format!("- {value}: {note}"))
            })
            .collect();
        if !notes.is_empty() {
            let body = notes.join("\n");
            text = if text.is_empty() {
                body
            } else {
                format!("{text}\n{body}")
            };
        }
    }
    text
}

/// 本命令的选项 + 祖先命令下沉的 `global` 参数; 帮助 / 版本排最后 (与 clap 一致)。
fn visible_options(parser: &Parser) -> Vec<&Argument> {
    let mut options: Vec<&Argument> = parser
        .arguments()
        .iter()
        .filter(|argument| !argument.is_positional() && !argument.hide)
        .collect();
    options.extend(
        parser
            .global_arguments()
            .iter()
            .filter(|argument| !argument.hide),
    );
    let is_tail = |argument: &&&Argument| matches!(argument.action, Action::Help | Action::Version);
    let mut ordered: Vec<&Argument> = options.iter().copied().filter(|a| !is_tail(&a)).collect();
    ordered.extend(options.iter().copied().filter(|a| is_tail(&a)));
    ordered
}

/// `标题 + 两列表格`: 左列对齐, 右列折行并与右列首字符对齐。
fn table(
    title: &str,
    rows: &[(String, String)],
    width: usize,
    style: &Style,
    description: Option<&str>,
) -> String {
    if rows.is_empty() {
        return String::new();
    }
    let widest = rows
        .iter()
        .map(|(left, _)| width_of(left))
        .max()
        .unwrap_or(0);
    let left_width = widest.min(16.max(width / 3));
    let gutter = 2;
    let base = 2 + left_width + gutter;
    let mut lines: Vec<String> = vec![title.to_string()];
    if let Some(body) = description.filter(|body| !body.is_empty()) {
        lines.extend(wrap_paragraphs(body, width, "  ", Some("  ")));
        lines.push(String::new());
    }
    for (left, text) in rows {
        let head = style.literal(&format!("  {left}"));
        if text.is_empty() {
            lines.push(head);
            continue;
        }
        if width_of(left) > left_width {
            lines.push(head);
            lines.extend(wrap_paragraphs(text, width, "    ", None));
            continue;
        }
        let indent = " ".repeat(base);
        let chunks = wrap_paragraphs(text, width, &indent, Some(&indent));
        let pad = " ".repeat(base - 2 - width_of(left));
        lines.push(format!("{head}{pad}{}", chunks[0].trim_start()));
        lines.extend(chunks.into_iter().skip(1));
    }
    lines.join("\n")
}

/// 子命令章节的行: 名字 + 展示别名; 自动生成的 `help` 排在最后。
fn command_rows(parser: &Parser) -> Vec<(String, String)> {
    let Some(subparsers) = parser.subparsers() else {
        return Vec::new();
    };
    let mut rows: Vec<(String, String)> = subparsers
        .commands()
        .map(|(name, aliases, command)| {
            let head = if aliases.is_empty() {
                name.to_string()
            } else {
                format!("{name}, {}", aliases.join(", "))
            };
            (head, command.about().unwrap_or_default().to_string())
        })
        .collect();
    if !subparsers.has_command("help") {
        rows.push((
            "help".to_string(),
            "Print this message or the help of the given subcommand(s)".to_string(),
        ));
    }
    rows
}

/// 单行用法文本。
pub fn render_usage(parser: &Parser) -> String {
    let style = Style::new(parser.color_enabled());
    format!("{} {}", style.heading("Usage:"), parser.usage_spec())
}

/// 整段帮助文本, 段间空一行。
pub fn render_help(parser: &Parser, long: bool) -> String {
    let style = Style::new(parser.color_enabled());
    let width = parser.width() as usize;
    let mut blocks: Vec<String> = Vec::new();
    let about = if long {
        parser.long_about().or_else(|| parser.about())
    } else {
        parser.about()
    };
    if let Some(body) = about.filter(|body| !body.is_empty()) {
        blocks.push(wrap_paragraphs(body, width, "", None).join("\n"));
    }
    blocks.push(render_usage(parser));
    if let Some(subparsers) = parser.subparsers() {
        let title = style.heading(&format!("{}:", subparsers.title()));
        let commands = table(
            &title,
            &command_rows(parser),
            width,
            &style,
            subparsers.help(),
        );
        if !commands.is_empty() {
            blocks.push(commands);
        }
    }
    let positionals: Vec<(String, String)> = parser
        .arguments()
        .iter()
        .filter(|argument| argument.is_positional() && !argument.hide)
        .map(|argument| (argument.help_names(), argument_text(argument, long)))
        .collect();
    if !positionals.is_empty() {
        blocks.push(table(
            &style.heading("Arguments:"),
            &positionals,
            width,
            &style,
            None,
        ));
    }
    let options = visible_options(parser);
    let default_options: Vec<(String, String)> = options
        .iter()
        .filter(|argument| match &argument.group {
            Some(group) => !parser.groups().iter().any(|(title, _)| title == group),
            None => true,
        })
        .map(|argument| (argument.help_names(), argument_text(argument, long)))
        .collect();
    if !default_options.is_empty() {
        blocks.push(table(
            &style.heading("Options:"),
            &default_options,
            width,
            &style,
            None,
        ));
    }
    for (title, description) in parser.groups() {
        let grouped: Vec<(String, String)> = options
            .iter()
            .filter(|argument| argument.group.as_deref() == Some(title.as_str()))
            .map(|argument| (argument.help_names(), argument_text(argument, long)))
            .collect();
        if grouped.is_empty() {
            continue;
        }
        blocks.push(table(
            &style.heading(&format!("{title}:")),
            &grouped,
            width,
            &style,
            description.as_deref(),
        ));
    }
    if let Some(body) = parser.epilog().filter(|body| !body.is_empty()) {
        blocks.push(wrap_paragraphs(body, width, "", None).join("\n"));
    }
    blocks.join("\n\n")
}

/// 错误块: 首行 `error:` + 缩进的细节 / 建议 + usage + 帮助提示。
pub fn render_error(error: &UsageError, color: bool) -> String {
    if !color {
        return error.to_string();
    }
    let style = Style::new(true);
    let mut body = format!("{} {}", style.error("error:"), error.message);
    if let Some(reason) = &error.reason {
        body.push_str(": ");
        body.push_str(reason);
    }
    for line in &error.lines {
        body.push_str("\n  ");
        body.push_str(line);
    }
    if !error.tips.is_empty() {
        body.push('\n');
        for tip in &error.tips {
            body.push_str("\n  ");
            body.push_str(&style.warning("tip:"));
            body.push(' ');
            body.push_str(tip);
        }
    }
    if let Some(usage) = &error.usage {
        body.push_str("\n\n");
        body.push_str(&style.heading("Usage:"));
        body.push(' ');
        body.push_str(usage);
    }
    if let Some(hint) = &error.hint {
        body.push_str("\n\n");
        body.push_str(hint);
    }
    body
}

#[cfg(test)]
mod tests {
    use super::super::error::ParseError;
    use super::super::parser::ColorChoice;
    use super::super::spec::{Choices, Nargs};
    use super::*;

    /// 取某个章节标题之后、下一个空行之前的行。
    fn section(text: &str, title: &str) -> Vec<String> {
        let lines: Vec<&str> = text.split('\n').collect();
        let start = match lines.iter().position(|line| *line == title) {
            Some(index) => index + 1,
            None => return Vec::new(),
        };
        lines[start..]
            .iter()
            .take_while(|line| !line.trim().is_empty())
            .map(|line| (*line).to_string())
            .collect()
    }

    #[test]
    fn usage_composition() {
        let bare = Parser::new("bare").with_color(ColorChoice::Never);
        assert_eq!(bare.usage_spec(), "bare");

        let mut opt = Parser::new("opt").with_color(ColorChoice::Never);
        opt.add_argument(
            &["-a", "--all"],
            Argument {
                action: Action::StoreTrue,
                help: Some("全部".to_string()),
                ..Argument::default()
            },
        )
        .unwrap();
        opt.add_argument(
            &["--out"],
            Argument {
                required: true,
                help: Some("输出".to_string()),
                ..Argument::default()
            },
        )
        .unwrap();
        opt.add_argument(
            &["files"],
            Argument {
                nargs: Nargs::OneOrMore,
                help: Some("文件".to_string()),
                ..Argument::default()
            },
        )
        .unwrap();
        assert_eq!(opt.usage_spec(), "opt --out <OUT> [OPTIONS] <FILES>...");

        let mut cmd = Parser::new("cmd").with_color(ColorChoice::Never);
        cmd.add_subparsers(None, "Commands", true).unwrap();
        assert_eq!(cmd.usage_spec(), "cmd <COMMAND>");

        let mut optional = Parser::new("cmd2").with_color(ColorChoice::Never);
        optional.add_subparsers(None, "Commands", false).unwrap();
        assert_eq!(optional.usage_spec(), "cmd2 [COMMAND]");

        let mut own = Parser::new("own")
            .with_color(ColorChoice::Never)
            .with_usage("own <自定义>");
        own.add_argument(
            &["--x"],
            Argument {
                help: Some("忽略".to_string()),
                ..Argument::default()
            },
        )
        .unwrap();
        assert_eq!(own.usage_spec(), "own <自定义>");
        assert!(own.format_usage().starts_with("Usage: own <自定义>"));
    }

    /// `tool` 命令: 简介、分组、子命令齐全, 供后面几节复用。
    fn build_tool() -> Parser {
        let mut parser = Parser::new("tool")
            .with_about("工具集")
            .with_long_about("工具集的完整说明, 只在 --help 里出现。")
            .with_epilog("示例: tool build -j 8")
            .with_version("1.0.0")
            .with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["-v", "--verbose"],
                Argument {
                    action: Action::Count,
                    help: Some("输出更详细".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--level"],
                Argument {
                    choices: Some(Choices::Values(vec![
                        Value::Str("debug".to_string()),
                        Value::Str("info".to_string()),
                        Value::Str("warn".to_string()),
                    ])),
                    default: DefaultValue::Value(Value::Str("info".to_string())),
                    help: Some("日志级别".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--jobs"],
                Argument {
                    value_parser: Some(super::super::spec::parse_int(None, None)),
                    default: DefaultValue::Value(Value::Int(4)),
                    env: Some("TOOL_JOBS".to_string()),
                    help: Some("并发数".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--secret"],
                Argument {
                    hide: true,
                    help: Some("不展示".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--quiet"],
                Argument {
                    action: Action::StoreTrue,
                    hide_default: true,
                    help: Some("安静".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["src"],
                Argument {
                    nargs: Nargs::ZeroOrMore,
                    help: Some("源文件".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        let subparsers = parser
            .add_subparsers(Some("command"), "Commands", false)
            .unwrap();
        subparsers.add_parser("build", "构建", &[], &["b"]).unwrap();
        subparsers.add_parser("run", "运行", &[], &[]).unwrap();
        parser
    }

    #[test]
    fn sections_and_content() {
        let parser = build_tool();
        let text = parser.format_help(false);
        assert!(text.starts_with("工具集\n"), "{text}");
        let usage = text.find("Usage:").unwrap();
        let commands = text.find("Commands:").unwrap();
        let arguments = text.find("Arguments:").unwrap();
        let options = text.find("Options:").unwrap();
        assert!(usage < commands && commands < arguments && arguments < options);
        assert!(text.trim_end().ends_with("示例: tool build -j 8"));
        assert!(!text.contains("--secret"));
        assert!(text.contains("-v, --verbose..."));
        assert!(text.contains("[default: info]"));
        assert!(text.contains("[env: TOOL_JOBS]"));
        assert!(text.contains("[possible values: debug, info, warn]"));
        assert!(text.contains("安静"));
        assert!(!text.contains("[default: false]"));
        assert!(text.contains("-V, --version"));
        assert!(text.contains("-h, --help"));

        let long_text = parser.format_help(true);
        assert!(long_text.starts_with("工具集的完整说明"));
        assert_eq!(long_text.matches("工具集").count(), 1);

        let rows = section(&text, "Options:");
        assert_eq!(rows.len(), 6, "{rows:?}");
        assert!(rows[rows.len() - 2].contains("Print help"), "{rows:?}");
        assert!(rows[rows.len() - 1].contains("Print version"), "{rows:?}");
        assert!(!text.contains("[default: true]"));

        let rows = section(&text, "Commands:");
        assert!(rows[0].trim_start().starts_with("build, b"), "{rows:?}");
        assert!(rows[1].trim_start().starts_with("run"), "{rows:?}");
        assert!(
            rows[rows.len() - 1].trim_start().starts_with("help"),
            "{rows:?}"
        );
        assert!(rows[rows.len() - 1].contains("Print this message"));

        let rows = section(&text, "Arguments:");
        assert!(rows[0].trim_start().starts_with("[SRC]..."), "{rows:?}");
    }

    #[test]
    fn long_help_lists_choice_notes() {
        let mut parser = Parser::new("level").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["--level"],
                Argument {
                    choices: Some(Choices::Described(vec![
                        (Value::Str("debug".to_string()), "逐条打印".to_string()),
                        (Value::Str("info".to_string()), "常规".to_string()),
                    ])),
                    default: DefaultValue::Value(Value::Str("info".to_string())),
                    help: Some("级别".to_string()),
                    long_help: Some("日志级别, 影响输出量".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        let short_text = parser.format_help(false);
        assert!(!short_text.contains("影响输出量"));
        assert!(short_text.contains("级别"));
        let long_text = parser.format_help(true);
        assert!(long_text.contains("影响输出量"));
        assert!(long_text.contains("- debug: 逐条打印"));
    }

    #[test]
    fn groups_render_their_own_section() {
        let mut parser = Parser::new("grouped").with_color(ColorChoice::Never);
        parser.add_group("Database", Some("数据库相关"));
        parser
            .add_argument(
                &["--host"],
                Argument {
                    group: Some("Database".to_string()),
                    default: DefaultValue::Value(Value::Str("localhost".to_string())),
                    help: Some("主机".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--port"],
                Argument {
                    group: Some("Database".to_string()),
                    value_parser: Some(super::super::spec::parse_int(None, None)),
                    default: DefaultValue::Value(Value::Int(5432)),
                    help: Some("端口".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--debug"],
                Argument {
                    action: Action::StoreTrue,
                    help: Some("调试".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        let text = parser.format_help(false);
        assert!(text.contains("Database:"));
        assert!(text.contains("数据库相关"));
        assert!(text.find("--host").unwrap() > text.find("Database:").unwrap());
        assert!(text.find("--debug").unwrap() < text.find("Database:").unwrap());
    }

    #[test]
    fn alignment_and_wrapping() {
        let mut parser = Parser::new("align").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["-a", "--all"],
                Argument {
                    help: Some("全部".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--beta"],
                Argument {
                    metavar: Some("BETA".to_string()),
                    help: Some("乙".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        let text = parser.format_help(false);
        let rows = section(&text, "Options:");
        assert_eq!(rows[0].find("全部"), rows[1].find("乙"), "{rows:?}");
        assert!(rows[0].starts_with("  -a, --all"), "{rows:?}");

        let mut wrap = Parser::new("wrap")
            .with_color(ColorChoice::Never)
            .with_max_width(44);
        wrap.add_argument(
            &["--long"],
            Argument {
                metavar: Some("VALUE".to_string()),
                help: Some("这是一段足够长的说明文字, 必须在窄幅下折行并保持对齐缩进".to_string()),
                ..Argument::default()
            },
        )
        .unwrap();
        let narrow = wrap.format_help(false);
        let rows = section(&narrow, "Options:");
        assert!(rows.len() >= 2, "{rows:?}");
        let continuation = rows[1..]
            .iter()
            .find(|line| !line.trim().is_empty())
            .expect("窄幅下应当折行");
        assert!(continuation.starts_with(&" ".repeat(18)), "{rows:?}");
        assert!(
            narrow.split('\n').all(|line| width_of(line) <= 44),
            "{narrow}"
        );
    }

    #[test]
    fn subcommand_help_inherits_globals() {
        let mut parser = Parser::new("tool").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["-q", "--quiet"],
                Argument {
                    action: Action::StoreTrue,
                    global: true,
                    help: Some("安静模式".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        let subparsers = parser
            .add_subparsers(Some("command"), "Commands", false)
            .unwrap();
        let build = subparsers.add_parser("build", "构建", &[], &[]).unwrap();
        build
            .add_argument(
                &["-j", "--jobs"],
                Argument {
                    value_parser: Some(super::super::spec::parse_int(None, None)),
                    default: DefaultValue::Value(Value::Int(4)),
                    help: Some("并发数".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        let child = match parser.subparsers().and_then(|sp| sp.lookup("build")) {
            Some(child) => child,
            None => panic!("子命令 build 未登记"),
        };
        let child_text = child.format_help(false);
        assert!(
            child_text.contains("Usage: tool build [OPTIONS]"),
            "{child_text}"
        );
        assert!(child_text.contains("-q, --quiet"), "{child_text}");
        assert!(child_text.contains("-j, --jobs"), "{child_text}");

        let mut nested = Parser::new("nested").with_color(ColorChoice::Never);
        let nested_commands = nested
            .add_subparsers(Some("command"), "Commands", false)
            .unwrap();
        let nested_build = nested_commands
            .add_parser("build", "构建", &[], &[])
            .unwrap();
        let targets = nested_build
            .add_subparsers(Some("target"), "Commands", false)
            .unwrap();
        targets.add_parser("release", "发布版", &[], &[]).unwrap();
        let nested_text = nested_build.format_help(false);
        assert!(nested_text.contains("Print this message"));

        let long_help = parser.parse(&["build".to_string(), "--help".to_string()]);
        match long_help {
            Err(ParseError::Help { long, prog }) => {
                assert!(long);
                assert_eq!(prog, "tool build");
            }
            other => panic!("应当请求长帮助: {other:?}"),
        }
        let short_help = parser.parse(&["-h".to_string()]);
        match short_help {
            Err(ParseError::Help { long, prog }) => {
                assert!(!long);
                assert_eq!(prog, "tool");
            }
            other => panic!("应当请求短帮助: {other:?}"),
        }
    }

    #[test]
    fn error_block_layout() {
        let mut parser = Parser::new("demo")
            .with_about("演示")
            .with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["--level"],
                Argument {
                    choices: Some(Choices::Values(vec![
                        Value::Str("debug".to_string()),
                        Value::Str("info".to_string()),
                        Value::Str("warn".to_string()),
                    ])),
                    help: Some("级别".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        let text = match parser.parse(&["--level".to_string(), "warnn".to_string()]) {
            Err(ParseError::Usage(error)) => error.to_string(),
            other => panic!("应当报用法错误: {other:?}"),
        };
        assert!(text.starts_with("error: invalid value 'warnn' for '--level <LEVEL>'"));
        assert!(text.contains("\n  [possible values: debug, info, warn]"));
        assert!(text.contains("\n  tip: a similar value exists: 'warn'"));
        assert!(text.contains("\nUsage: demo [OPTIONS]"));
        assert!(text.ends_with("For more information, try '--help'."));

        let error = UsageError::new("自定义")
            .with_reason("原因")
            .with_lines(["明细"])
            .with_tips(["建议"])
            .with_usage("demo");
        assert!(error.to_string().starts_with("error: 自定义: 原因"));
        assert!(render_error(&error, false).starts_with("error: 自定义: 原因"));
        assert!(render_error(&error, true).contains("\x1b[31merror:\x1b[0m"));
    }

    #[test]
    fn color_switches_layout_only() {
        let mut colored = Parser::new("color").with_color(ColorChoice::Always);
        colored
            .add_argument(
                &["--x"],
                Argument {
                    help: Some("甲".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        let mut mono = Parser::new("color").with_color(ColorChoice::Never);
        mono.add_argument(
            &["--x"],
            Argument {
                help: Some("甲".to_string()),
                ..Argument::default()
            },
        )
        .unwrap();
        assert!(colored.format_help(false).contains("\x1b["));
        let plain = mono.format_help(false);
        assert!(!plain.contains("\x1b["));
        assert_eq!(strip_ansi(&colored.format_help(false)), plain);
        assert!(
            render_error(&UsageError::new("甲"), false)
                .find("\x1b[")
                .is_none()
        );
    }

    #[test]
    fn print_help_and_parse_or_exit() {
        let mut parser = Parser::new("demo")
            .with_version("1.0.0")
            .with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["--x"],
                Argument {
                    help: Some("甲".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();

        let mut printed = Vec::new();
        parser.print_help(false, &mut printed).unwrap();
        assert_eq!(
            String::from_utf8(printed).unwrap(),
            parser.format_help(false) + "\n"
        );

        let mut out = Vec::new();
        let mut err = Vec::new();
        let exit = parser
            .parse_or_exit(&["--help".to_string()], &mut out, &mut err)
            .unwrap();
        assert_eq!(exit, Err(0));
        assert_eq!(
            String::from_utf8(out).unwrap(),
            parser.format_help(true) + "\n"
        );
        assert!(err.is_empty());

        let mut out = Vec::new();
        let mut err = Vec::new();
        let exit = parser
            .parse_or_exit(&["-V".to_string()], &mut out, &mut err)
            .unwrap();
        assert_eq!(exit, Err(0));
        assert_eq!(String::from_utf8(out).unwrap(), "demo 1.0.0\n");

        let mut out = Vec::new();
        let mut err = Vec::new();
        let exit = parser
            .parse_or_exit(&["--y".to_string()], &mut out, &mut err)
            .unwrap();
        assert_eq!(exit, Err(2));
        assert!(out.is_empty());
        let failure = String::from_utf8(err).unwrap();
        assert!(failure.starts_with("error:"), "{failure}");
        assert!(failure.ends_with('\n'), "{failure}");

        let mut out = Vec::new();
        let mut err = Vec::new();
        assert!(
            parser
                .parse_or_exit(&["--x".to_string(), "1".to_string()], &mut out, &mut err)
                .unwrap()
                .is_ok()
        );
        assert!(out.is_empty() && err.is_empty());

        let mut tool = Parser::new("tool").with_color(ColorChoice::Never);
        let subparsers = tool
            .add_subparsers(Some("command"), "Commands", false)
            .unwrap();
        subparsers.add_parser("build", "构建", &[], &[]).unwrap();
        let child = tool
            .subparsers()
            .and_then(|sp| sp.lookup("build"))
            .expect("子命令 build 未登记");
        let expected = child.format_help(true) + "\n";
        let mut out = Vec::new();
        let mut err = Vec::new();
        let exit = tool
            .parse_or_exit(
                &["build".to_string(), "--help".to_string()],
                &mut out,
                &mut err,
            )
            .unwrap();
        assert_eq!(exit, Err(0));
        assert_eq!(String::from_utf8(out).unwrap(), expected);
    }

    /// 去掉 ANSI 转义, 便于比较可见版式。
    fn strip_ansi(text: &str) -> String {
        let mut body = String::new();
        let mut rest = text;
        while let Some(index) = rest.find('\x1b') {
            body.push_str(&rest[..index]);
            rest = &rest[index..];
            match rest.find('m') {
                Some(end) => rest = &rest[end + 1..],
                None => {
                    rest = "";
                }
            }
        }
        body.push_str(rest);
        body
    }
}
