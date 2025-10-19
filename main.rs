use clap::{Arg, ArgAction, Command};

fn build_version_info() -> String {
    // 最小化的构建信息回退。在 C++ 树中，这由 build-info.h 生成。
    // 我们在这里保留简洁信息；项目可以通过构建脚本自行扩展。
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

fn main() {
    // Initialize logger
    quant1x::app::logger_set(false, false);

    let program_name = std::env::current_exe()
        .ok()
        .and_then(|p| p.file_name().map(|n| n.to_string_lossy().to_string()))
        .unwrap_or_else(|| String::from("quant1x"));
    let program_version = build_version_info();

    let service_sub = Command::new("service")
        .about("Manage the service.")
        .arg(
            Arg::new("action")
                .value_parser(["install", "uninstall", "start", "stop", "status", "run"])
                .required(true),
        )
        .arg(Arg::new("pipe").long("pipe").action(ArgAction::SetTrue))
        .arg(Arg::new("elevated-out").long("elevated-out").num_args(1))
        .arg(Arg::new("elevated-pipe").long("elevated-pipe").num_args(1));

    let update_sub = Command::new("update")
        .about("Update cached data (base / features)")
        .arg(
            Arg::new("calendar")
                .long("calendar")
                .action(ArgAction::SetTrue)
                .help("Only update calendar cache"),
        )
        .arg(
            Arg::new("servers")
                .long("servers")
                .action(ArgAction::SetTrue)
                .help("Only detect and update level1 servers cache"),
        )
        .arg(
            Arg::new("all")
                .long("all")
                .action(ArgAction::SetTrue)
                .help("Update everything (default)"),
        )
        .arg(
            Arg::new("base")
                .long("base")
                .num_args(1..)
                .value_name("KEY")
                .help("Update base data keys (e.g. xdxr)")
                .value_parser(clap::builder::NonEmptyStringValueParser::new()),
        )
        .arg(
            Arg::new("features")
                .long("features")
                .num_args(1..)
                .value_name("KEY")
                .help("Update feature datasets (derived data)")
                .value_parser(clap::builder::NonEmptyStringValueParser::new()),
        );

    // 对于生命周期短的 CLI 流程，将这些字符串字面量泄露为静态是可接受的，
    // 这样可以避免与 clap 要求 'static str 的生命周期冲突。
    // 这也与 C++ 中动态设置程序名/版本号的行为一致。
    let program_name_static: &'static str = Box::leak(program_name.into_boxed_str());
    let program_version_static: &'static str = Box::leak(program_version.into_boxed_str());

    let mut app = Command::new(program_name_static)
        .long_about(program_version_static)
        .arg(
            Arg::new("version")
                .long("version")
                .action(ArgAction::SetTrue)
                .help("Print build version information and exit"),
        )
        .arg(
            Arg::new("verbose")
                .long("verbose")
                .action(ArgAction::SetTrue)
                .help("显示日志信息到终端"),
        )
        .arg(
            Arg::new("debug")
                .long("debug")
                .action(ArgAction::SetTrue)
                .help("打开日志的调试模式"),
        )
        .subcommand_required(false)
        .subcommand(service_sub)
        .subcommand(update_sub);

    // 注意：C++ 实现会动态注册来自 `quant1x::subcommands` 的子命令。
    // Rust 移植版可能尚未暴露该列表。
    // 我们提供了一个最小的兼容层：如果 crate 提供了
    // `quant1x::run_subcommand(name, matches)`，可以在这里接入。
    // 注意：C++ 实现会动态注册来自 `quant1x::subcommands` 的子命令。
    // Rust 移植版可能尚未暴露完整的子命令列表。我们在此提供一个最小兼容层：如果 crate
    // 提供了 `quant1x::run_subcommand(name, matches)`，则可以从这里转发调用。

    let matches = app.clone().get_matches();

    // 如果请求，打印构建/版本信息并提前退出
    if matches.get_flag("version") {
        // program_version_static was created from build_version_info()
        println!("{}", program_version_static);
        return;
    }

    let verbose = matches.get_flag("verbose");
    let debug = matches.get_flag("debug");

    // runtime::global_init() 等效调用 — 若 crate 提供则调用，否则可选
    if let Err(e) = call_runtime_global_init() {
        log::error!("runtime::global_init failed: {}", e);
    }

    // datasets::init() 等效调用
    if let Err(e) = call_datasets_init() {
        log::error!("datasets::init failed: {}", e);
    }

    // 初始化日志系统
    let _ = call_logger_set(verbose, debug);

    // engine::init（可选地接受自定义初始化闭包）
    let _ = call_engine_init();

    // service 子命令的特殊处理
    if let Some((name, subm)) = matches.subcommand() {
        if name == "service" {
            // 若可用，调用 engine::daemon(service_matches)
            let action = subm
                .get_one::<String>("action")
                .map(|s| s.as_str())
                .unwrap_or("");
            let pipe = subm.get_flag("pipe");
            let elevated_out = subm
                .get_one::<String>("elevated-out")
                .map(|s| s.to_string());
            let elevated_pipe = subm
                .get_one::<String>("elevated-pipe")
                .map(|s| s.to_string());
            let code = call_engine_daemon(
                action,
                pipe,
                elevated_out.as_deref(),
                elevated_pipe.as_deref(),
            );
            std::process::exit(code);
        }
    }

    // 尝试分发由库提供的动态子命令。
    // 库可能暴露 `quant1x::subcommands()` 或 `quant1x::run_subcommand`。
    // 我们将尽力进行动态派发：查找用户使用的单个子命令，
    // 并在该符号存在时调用 `quant1x::run_subcommand(name, &ArgMatches)`。

    // 找出被使用的子命令（排除 service）
    if let Some((name, subm)) = matches.subcommand() {
        if name != "service" {
            // attempt to call into library
            match lib_bridge::try_run_subcommand(name, subm) {
                Ok(true) => {
                    // ran successfully
                    return;
                }
                Ok(false) => {
                    log::error!(
                        "Error: Command '{}' not implemented in Rust library.\n",
                        name
                    );
                    app.print_help().ok();
                    std::process::exit(1);
                }
                Err(err) => {
                    log::error!("Error running subcommand '{}': {}", name, err);
                    std::process::exit(1);
                }
            }
        }
    }

    // 若执行到此处，说明没有子命令被执行
    log::error!("Error: No command provided.");
    app.print_help().ok();
    std::process::exit(1);
}

// 下面的函数是对 crate 中实现的轻量包装（shim），如果相应符号存在就调用它们。
// 它们作为弱连接实现，便于 crate 逐步实现 `engine`、`runtime`、`datasets` 和 `run_subcommand`。

fn call_runtime_global_init() -> Result<(), String> {
    // Rust 端可能没有 runtime::global_init；如果存在则调用它。
    // 我们尝试调用 quant1x::global_init()（crate 提供）。
    // 目前调用的是 crate 提供的 no-op（占位实现）。
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
    use clap::ArgMatches;
    use std::error::Error;

    // 如果存在，转发到 crate 提供的实现。crate 在其 app 模块中导出 `quant1x::try_run_subcommand`。
    pub fn try_run_subcommand(name: &str, matches: &ArgMatches) -> Result<bool, Box<dyn Error>> {
        // 如果可用，则调用 quant1x::try_run_subcommand
        quant1x::try_run_subcommand(name, matches)
    }
}
