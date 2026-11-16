// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

use quant1x::argparser::parser::Parser;
use quant1x::argparser::spec::{Action, Argument, Choices, Namespace, Nargs, Value};

fn build_version_info() -> String {
    // 最小化的构建信息回退. 在 C++ 树中, 这由 build-info.h 生成.
    // 我们在这里保留简洁信息；项目可以通过构建脚本自行扩展.
    let mut s = String::new();
    s.push_str("quant1x - Rust edition\n");
    s.push_str(
        "--------------------------------------------------------------------------------\n",
    );
    s.push_str(&format!(
        "               Version : {}\n",
        env!("CARGO_PKG_VERSION")
    ));
    s.push_str(&format!(
        "                Author : {}\n",
        env!("CARGO_PKG_AUTHORS")
    ));
    s.push_str(
        "--------------------------------------------------------------------------------\n",
    );
    s
}

/// 取开关取值; 未给出或不是布尔时为 false
fn flag(namespace: &Namespace, key: &str) -> bool {
    namespace.get(key).and_then(Value::as_bool).unwrap_or(false)
}

/// 取文本取值; 未给出或不是字符串时为空串
fn text(namespace: &Namespace, key: &str) -> String {
    namespace
        .get(key)
        .and_then(Value::as_str)
        .unwrap_or_default()
        .to_string()
}

fn main() {
    // Initialize logger
    quant1x::app::logger_set(false, false);

    let program_name = std::env::current_exe()
        .ok()
        .and_then(|p| p.file_name().map(|n| n.to_string_lossy().to_string()))
        .unwrap_or_else(|| String::from("quant1x"));
    let program_version = build_version_info();

    let mut parser = Parser::new(&program_name);

    parser
        .add_argument(
            &["--version"],
            Argument {
                action: Action::StoreTrue,
                help: Some("Print build version information and exit".to_string()),
                ..Argument::default()
            },
        )
        .expect("register --version");
    parser
        .add_argument(
            &["--verbose"],
            Argument {
                action: Action::StoreTrue,
                help: Some("显示日志信息到终端".to_string()),
                ..Argument::default()
            },
        )
        .expect("register --verbose");
    parser
        .add_argument(
            &["--debug"],
            Argument {
                action: Action::StoreTrue,
                help: Some("打开日志的调试模式".to_string()),
                ..Argument::default()
            },
        )
        .expect("register --debug");

    let subparsers = parser
        .add_subparsers(Some("command"), "Commands", false)
        .expect("register subcommands");

    let service = subparsers
        .add_parser("service", "Manage the service.", &[], &[])
        .expect("register service");
    service
        .add_argument(
            &["action"],
            Argument {
                choices: Some(Choices::Values(vec![
                    Value::Str("install".to_string()),
                    Value::Str("uninstall".to_string()),
                    Value::Str("start".to_string()),
                    Value::Str("stop".to_string()),
                    Value::Str("status".to_string()),
                    Value::Str("run".to_string()),
                ])),
                help: Some("install | uninstall | start | stop | status | run".to_string()),
                ..Argument::default()
            },
        )
        .expect("register service action");
    service
        .add_argument(
            &["--pipe"],
            Argument {
                action: Action::StoreTrue,
                ..Argument::default()
            },
        )
        .expect("register service --pipe");
    service
        .add_argument(&["--elevated-out"], Argument::default())
        .expect("register service --elevated-out");
    service
        .add_argument(&["--elevated-pipe"], Argument::default())
        .expect("register service --elevated-pipe");

    let update = subparsers
        .add_parser("update", "Update cached data (base / features)", &[], &[])
        .expect("register update");
    update
        .add_argument(
            &["--calendar"],
            Argument {
                action: Action::StoreTrue,
                help: Some("Only update calendar cache".to_string()),
                ..Argument::default()
            },
        )
        .expect("register update --calendar");
    update
        .add_argument(
            &["--servers"],
            Argument {
                action: Action::StoreTrue,
                help: Some("Only detect and update level1 servers cache".to_string()),
                ..Argument::default()
            },
        )
        .expect("register update --servers");
    update
        .add_argument(
            &["--all"],
            Argument {
                action: Action::StoreTrue,
                help: Some("Update everything (default)".to_string()),
                ..Argument::default()
            },
        )
        .expect("register update --all");
    update
        .add_argument(
            &["--base"],
            Argument {
                nargs: Nargs::OneOrMore,
                help: Some("Update base data keys (e.g. xdxr)".to_string()),
                ..Argument::default()
            },
        )
        .expect("register update --base");
    update
        .add_argument(
            &["--features"],
            Argument {
                nargs: Nargs::OneOrMore,
                help: Some("Update feature datasets (derived data)".to_string()),
                ..Argument::default()
            },
        )
        .expect("register update --features");

    let argv: Vec<String> = std::env::args().skip(1).collect();
    let mut stdout = std::io::stdout();
    let mut stderr = std::io::stderr();
    let namespace = match parser.parse_or_exit(&argv, &mut stdout, &mut stderr) {
        Ok(Ok(namespace)) => namespace,
        Ok(Err(code)) => std::process::exit(code),
        Err(err) => {
            log::error!("解析命令行失败: {}", err);
            std::process::exit(1);
        }
    };

    // 如果请求, 打印构建/版本信息并提前退出
    if flag(&namespace, "version") {
        println!("{}", program_version);
        return;
    }

    let verbose = flag(&namespace, "verbose");
    let debug = flag(&namespace, "debug");

    // runtime::global_init() 等效调用 — 若 crate 提供则调用, 否则可选
    if let Err(e) = call_runtime_global_init() {
        log::error!("runtime::global_init failed: {}", e);
    }

    // data::init() 等效调用
    if let Err(e) = call_datasets_init() {
        log::error!("data::init failed: {}", e);
    }

    // 初始化日志系统
    let _ = call_logger_set(verbose, debug);

    // engine::init(可选地接受自定义初始化闭包)
    let _ = call_engine_init();

    let command = text(&namespace, "command");
    match command.as_str() {
        "service" => {
            let action = text(&namespace, "action");
            let pipe = flag(&namespace, "pipe");
            let elevated_out = text(&namespace, "elevated-out");
            let elevated_pipe = text(&namespace, "elevated-pipe");
            let code = call_engine_daemon(
                &action,
                pipe,
                (!elevated_out.is_empty()).then_some(elevated_out.as_str()),
                (!elevated_pipe.is_empty()).then_some(elevated_pipe.as_str()),
            );
            std::process::exit(code);
        }
        "update" => match lib_bridge::try_run_subcommand("update", &namespace) {
            Ok(true) => return,
            Ok(false) => {
                log::error!("Error: Command 'update' not implemented in Rust library.\n");
                let _ = parser.print_help(false, &mut std::io::stdout());
                std::process::exit(1);
            }
            Err(err) => {
                log::error!("Error running subcommand 'update': {}", err);
                std::process::exit(1);
            }
        },
        _ => {}
    }

    // 若执行到此处, 说明没有子命令被执行
    log::error!("Error: No command provided.");
    let _ = parser.print_help(false, &mut std::io::stdout());
    std::process::exit(1);
}

// 下面的函数是对 crate 中实现的轻量包装(shim), 如果相应符号存在就调用它们.
// 它们作为弱连接实现, 便于 crate 逐步实现 `engine`, `runtime`, `datasets` 和 `run_subcommand`.

fn call_runtime_global_init() -> Result<(), String> {
    // Rust 端可能没有 runtime::global_init；如果存在则调用它.
    // 我们尝试调用 quant1x::global_init()(crate 提供).
    // 目前调用的是 crate 提供的 no-op(占位实现).
    quant1x::global_init();
    Ok(())
}

fn call_datasets_init() -> Result<(), String> {
    quant1x::datasets_init();
    Ok(())
}

fn call_logger_set(verbose: bool, debug: bool) -> Result<(), String> {
    quant1x::logger_set(verbose, debug);
    Ok(())
}

fn call_engine_init() -> Result<(), String> {
    quant1x::engine_init();
    Ok(())
}

fn call_engine_daemon(
    action: &str,
    pipe: bool,
    elevated_out: Option<&str>,
    elevated_pipe: Option<&str>,
) -> i32 {
    quant1x::engine_daemon(action, pipe, elevated_out, elevated_pipe)
}

mod lib_bridge {
    use quant1x::argparser::spec::Namespace;
    use std::error::Error;

    // 如果存在, 转发到 crate 提供的实现. crate 在其 app 模块中导出 `quant1x::try_run_subcommand`.
    pub fn try_run_subcommand(name: &str, namespace: &Namespace) -> Result<bool, Box<dyn Error>> {
        // 如果可用, 则调用 quant1x::try_run_subcommand
        quant1x::try_run_subcommand(name, namespace)
    }
}
