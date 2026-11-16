// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 由命令定义生成 bash / zsh / fish 的补全脚本。
//!
//! 候选词只看「已敲定的子命令链」: 选项名、子命令名、位置参数的 `choices`;
//! 不区分「正在写取值」的语境, 也不补文件路径。

use crate::base::argparser::parser::Parser;

/// 支持的 shell, 按登记顺序。
pub const SHELLS: [&str; 3] = ["bash", "zsh", "fish"];

/// 生成 `shell` 的补全脚本; `prog` 缺省取 `parser.prog()`。
///
/// # Errors
///
/// `shell` 不是 `bash` / `zsh` / `fish` 之一时返回错误消息。
pub fn complete(parser: &Parser, shell: &str, prog: Option<&str>) -> Result<String, String> {
    if !SHELLS.contains(&shell) {
        return Err(format!("不支持的 shell: {shell}"));
    }
    let name = prog.unwrap_or_else(|| parser.prog());
    let mut nodes: Vec<(Vec<String>, &Parser)> = Vec::new();
    walk(parser, &mut Vec::new(), &mut nodes);
    match shell {
        "bash" => Ok(bash(name, &nodes)),
        "zsh" => Ok(zsh(name, &nodes)),
        _ => Ok(fish(name, &nodes)),
    }
}

/// 按前序收集 (子命令链, 命令); 链为空的是顶层命令。
fn walk<'a>(
    parser: &'a Parser,
    chain: &mut Vec<String>,
    nodes: &mut Vec<(Vec<String>, &'a Parser)>,
) {
    nodes.push((chain.clone(), parser));
    let Some(subs) = parser.subparsers() else {
        return;
    };
    let commands: Vec<(String, &'a Parser)> = subs
        .commands()
        .map(|(name, _, command)| (name.to_owned(), command))
        .collect();
    for (name, command) in commands {
        chain.push(name);
        walk(command, chain, nodes);
        chain.pop();
    }
}

/// 一个命令的候选词: 选项名、子命令名、位置参数的 `choices`, 按登记顺序去重。
fn words(parser: &Parser) -> Vec<String> {
    let mut collected: Vec<String> = Vec::new();
    for argument in parser.arguments().iter().chain(parser.global_arguments()) {
        if !argument.hide {
            collected.extend(argument.names.iter().cloned());
        }
    }
    if let Some(subs) = parser.subparsers() {
        collected.extend(subs.commands().map(|(name, _, _)| name.to_owned()));
    }
    for positional in parser
        .arguments()
        .iter()
        .filter(|argument| argument.is_positional())
    {
        if let Some(values) = positional.choice_values() {
            collected.extend(values.iter().map(ToString::to_string));
        }
    }
    let mut unique: Vec<String> = Vec::new();
    for word in collected {
        if word.is_empty() || word.chars().any(char::is_whitespace) || unique.contains(&word) {
            continue;
        }
        unique.push(word);
    }
    unique
}

/// 脚本里的函数名: 非标识符字符一律换成下划线。
fn function(name: &str) -> String {
    let mut text = String::from("_argparser_");
    for ch in name.chars() {
        text.push(if ch.is_alphanumeric() || ch == '_' {
            ch
        } else {
            '_'
        });
    }
    text
}

/// 状态机里的每条边: (键, 命中后的命令链文本)。键形如 `|alpha`、`alpha|beta`。
fn transitions(nodes: &[(Vec<String>, &Parser)]) -> Vec<(String, String)> {
    nodes
        .iter()
        .filter(|(chain, _)| !chain.is_empty())
        .map(|(chain, _)| {
            let key = format!(
                "{}|{}",
                chain[..chain.len() - 1].join(" "),
                chain[chain.len() - 1]
            );
            (key, chain.join(" "))
        })
        .collect()
}

/// 生成 bash 补全脚本: 先按已输入词推进到命令链, 再按链给出候选词。
fn bash(name: &str, nodes: &[(Vec<String>, &Parser)]) -> String {
    let function = function(name);
    let mut out = format!("{function}() {{\n    local cur cmd word\n");
    out.push_str("    cur=\"${COMP_WORDS[COMP_CWORD]}\"\n    cmd=\"\"\n");
    let edges = transitions(nodes);
    if !edges.is_empty() {
        out.push_str("    for word in \"${COMP_WORDS[@]:1:COMP_CWORD-1}\"; do\n");
        out.push_str("        case \"$cmd|$word\" in\n");
        for (key, chain) in &edges {
            out.push_str(&format!("            \"{key}\") cmd=\"{chain}\" ;;\n"));
        }
        out.push_str("        esac\n    done\n");
    }
    out.push_str("    case \"$cmd\" in\n");
    for (chain, parser) in nodes {
        out.push_str(&format!(
            "        \"{}\") COMPREPLY=( $(compgen -W \"{}\" -- \"$cur\") ) ;;\n",
            chain.join(" "),
            words(parser).join(" ")
        ));
    }
    out.push_str(&format!("    esac\n}}\ncomplete -F {function} {name}\n"));
    out
}

/// 生成 zsh 补全脚本: 结构同 bash, 候选走 `compadd`。
fn zsh(name: &str, nodes: &[(Vec<String>, &Parser)]) -> String {
    let function = function(name);
    let mut out = format!(
        "#compdef {name}\n\n{function}() {{\n    local -a candidates\n    local cmd=\"\" word\n"
    );
    let edges = transitions(nodes);
    if !edges.is_empty() {
        out.push_str("    for word in ${words[2,CURRENT-1]}; do\n");
        out.push_str("        case \"$cmd|$word\" in\n");
        for (key, chain) in &edges {
            out.push_str(&format!("            \"{key}\") cmd=\"{chain}\" ;;\n"));
        }
        out.push_str("        esac\n    done\n");
    }
    out.push_str("    case \"$cmd\" in\n");
    for (chain, parser) in nodes {
        out.push_str(&format!(
            "        \"{}\") candidates=({}) ;;\n",
            chain.join(" "),
            words(parser).join(" ")
        ));
    }
    out.push_str("    esac\n    compadd -- \"${candidates[@]}\"\n}\n\n");
    out.push_str(&format!("{function} \"$@\"\n"));
    out
}

/// 生成 fish 补全脚本: 每个选项一条 `complete -c`, 子命令用 `__fish_seen_subcommand_from` 限定。
fn fish(name: &str, nodes: &[(Vec<String>, &Parser)]) -> String {
    let mut out = String::new();
    for (chain, parser) in nodes {
        let condition = chain
            .last()
            .map(|command| format!(" -n '__fish_seen_subcommand_from {command}'"))
            .unwrap_or_default();
        for argument in parser.arguments() {
            if argument.hide || argument.is_positional() {
                continue;
            }
            let spelled: Vec<String> = argument
                .names
                .iter()
                .filter_map(|flag| fish_flag(flag))
                .collect();
            if spelled.is_empty() {
                continue;
            }
            let detail = argument
                .help
                .as_deref()
                .filter(|help| !help.is_empty())
                .map(|help| format!(" -d '{}'", quote(help)))
                .unwrap_or_default();
            out.push_str(&format!(
                "complete -c {name}{condition} {}{detail}\n",
                spelled.join(" ")
            ));
        }
        if let Some(subs) = parser.subparsers() {
            for (command, _, child) in subs.commands() {
                let detail = child
                    .about()
                    .filter(|about| !about.is_empty())
                    .map(|about| format!(" -d '{}'", quote(about)))
                    .unwrap_or_default();
                out.push_str(&format!(
                    "complete -c {name}{condition} -f -a {command}{detail}\n"
                ));
            }
        }
        for positional in parser
            .arguments()
            .iter()
            .filter(|argument| argument.is_positional())
        {
            if let Some(values) = positional.choice_values() {
                let spelled: Vec<String> = values
                    .iter()
                    .map(ToString::to_string)
                    .filter(|word| !word.chars().any(char::is_whitespace))
                    .collect();
                if !spelled.is_empty() {
                    out.push_str(&format!(
                        "complete -c {name}{condition} -f -a '{}'\n",
                        spelled.join(" ")
                    ));
                }
            }
        }
    }
    out
}

/// fish 的单选项拼法: `-h` → `-s h`, `--help` → `-l help`; 其余形态不补。
fn fish_flag(flag: &str) -> Option<String> {
    if let Some(long) = flag.strip_prefix("--").filter(|name| !name.is_empty()) {
        return Some(format!("-l {long}"));
    }
    if flag.starts_with('-') && flag.chars().count() == 2 {
        return flag.chars().nth(1).map(|short| format!("-s {short}"));
    }
    None
}

/// 把说明文字放进单引号: 反斜杠与单引号各转义一层。
fn quote(text: &str) -> String {
    text.replace('\\', "\\\\").replace('\'', "\\'")
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::base::argparser::spec::{Action, Argument, Choices, Value};

    const BASH: &str = r#"_argparser_demo() {
    local cur cmd word
    cur="${COMP_WORDS[COMP_CWORD]}"
    cmd=""
    for word in "${COMP_WORDS[@]:1:COMP_CWORD-1}"; do
        case "$cmd|$word" in
            "|alpha") cmd="alpha" ;;
            "|beta") cmd="beta" ;;
        esac
    done
    case "$cmd" in
        "") COMPREPLY=( $(compgen -W "-h --help -v --verbose --mode alpha beta x y" -- "$cur") ) ;;
        "alpha") COMPREPLY=( $(compgen -W "-h --help --file -v --verbose" -- "$cur") ) ;;
        "beta") COMPREPLY=( $(compgen -W "-h --help --count -v --verbose" -- "$cur") ) ;;
    esac
}
complete -F _argparser_demo demo
"#;

    const ZSH: &str = r#"#compdef demo

_argparser_demo() {
    local -a candidates
    local cmd="" word
    for word in ${words[2,CURRENT-1]}; do
        case "$cmd|$word" in
            "|alpha") cmd="alpha" ;;
            "|beta") cmd="beta" ;;
        esac
    done
    case "$cmd" in
        "") candidates=(-h --help -v --verbose --mode alpha beta x y) ;;
        "alpha") candidates=(-h --help --file -v --verbose) ;;
        "beta") candidates=(-h --help --count -v --verbose) ;;
    esac
    compadd -- "${candidates[@]}"
}

_argparser_demo "$@"
"#;

    const FISH: &str = r#"complete -c demo -s h -l help -d 'Print help'
complete -c demo -s v -l verbose -d 'verbose output'
complete -c demo -l mode -d 'mode'
complete -c demo -f -a alpha -d 'alpha command'
complete -c demo -f -a beta -d 'beta command'
complete -c demo -f -a 'x y'
complete -c demo -n '__fish_seen_subcommand_from alpha' -s h -l help -d 'Print help'
complete -c demo -n '__fish_seen_subcommand_from alpha' -l file -d 'input file'
complete -c demo -n '__fish_seen_subcommand_from beta' -s h -l help -d 'Print help'
complete -c demo -n '__fish_seen_subcommand_from beta' -l count
"#;

    fn demo() -> Parser {
        let mut parser = Parser::new("demo").with_about("演示");
        parser
            .add_argument(
                &["-v", "--verbose"],
                Argument {
                    action: Action::StoreTrue,
                    help: Some("verbose output".to_owned()),
                    global: true,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--mode"],
                Argument {
                    help: Some("mode".to_owned()),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--secret"],
                Argument {
                    hide: true,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["kind"],
                Argument {
                    choices: Some(Choices::Values(vec![
                        Value::Str("x".to_owned()),
                        Value::Str("y".to_owned()),
                    ])),
                    ..Argument::default()
                },
            )
            .unwrap();
        let subparsers = parser.add_subparsers(None, "Commands", false).unwrap();
        subparsers
            .add_parser("alpha", "alpha command", &[], &[])
            .unwrap()
            .add_argument(
                &["--file"],
                Argument {
                    help: Some("input file".to_owned()),
                    ..Argument::default()
                },
            )
            .unwrap();
        subparsers
            .add_parser("beta", "beta command", &[], &[])
            .unwrap()
            .add_argument(&["--count"], Argument::default())
            .unwrap();
        parser
    }

    #[test]
    fn scripts_match_the_python_sample() {
        assert_eq!(complete(&demo(), "bash", None).unwrap(), BASH);
        assert_eq!(complete(&demo(), "zsh", None).unwrap(), ZSH);
        assert_eq!(complete(&demo(), "fish", None).unwrap(), FISH);
    }

    #[test]
    fn prog_override_and_function_name_are_normalized() {
        let script = complete(&demo(), "bash", Some("my-tool")).unwrap();
        assert!(script.contains("complete -F _argparser_my_tool my-tool"));
        assert!(script.starts_with("_argparser_my_tool() {\n"));
    }

    #[test]
    fn unknown_shell_is_rejected() {
        let error = complete(&demo(), "powershell", None).unwrap_err();
        assert!(error.contains("powershell"), "{error}");
    }

    #[test]
    fn command_without_subparsers_has_no_state_machine() {
        let script = complete(&Parser::new("solo"), "bash", None).unwrap();
        assert!(!script.contains("$cmd|$word"), "{script}");
        assert!(script.contains("        \"\") COMPREPLY="), "{script}");
    }

    #[test]
    fn empty_help_writes_no_detail() {
        let mut parser = Parser::new("blank");
        parser
            .add_argument(
                &["--flag"],
                Argument {
                    help: Some(String::new()),
                    ..Argument::default()
                },
            )
            .unwrap();
        let subparsers = parser.add_subparsers(None, "Commands", false).unwrap();
        subparsers.add_parser("child", "", &[], &[]).unwrap();
        let script = complete(&parser, "fish", None).unwrap();
        assert!(!script.contains("-d ''"), "{script}");
    }
}
