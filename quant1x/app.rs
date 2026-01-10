// 小型适配模块，为 `src/main.rs` 提供应用级入口点。
// 这些实现有意保持最小且非侵入：提供 no-op 回退实现，
// 上层的 Rust 实现可在本 crate 中提供更完整的实现来覆盖它们。

pub fn global_init() {
    // 空实现（占位）
}

pub fn datasets_init() {
    // 初始化数据集并注册 Rust 实现的适配器
    if let Err(e) = std::panic::catch_unwind(|| {
        crate::data::init();
    }) {
        log::error!("data::init() panicked: {:?}", e);
    }
}

pub fn logger_set(_verbose: bool, _debug: bool) {
    // 初始化 log4rs，按级别拆分日志文件，并使用按天轮转的滚动策略。
    use log::LevelFilter;
    use log4rs::filter::threshold::ThresholdFilter;
    use std::path::PathBuf;

    // 自定义 EqualFilter，用于精确级别匹配，避免重复写入
    use log::Level;
    use log4rs::filter::{Filter, Response};

    #[derive(Debug)]
    struct EqualFilter {
        level: Level,
    }

    impl Filter for EqualFilter {
        fn filter(&self, record: &log::Record) -> Response {
            if record.level() == self.level {
                Response::Accept
            } else {
                Response::Reject
            }
        }
    }

    use std::sync::Once;

    static INIT: Once = Once::new();

    // Run initialization only once per process. Use stderr for reporting init errors
    // because logging may not yet be configured (or may already be configured by
    // the embedding application).
    INIT.call_once(|| {
        let logs_dir = crate::config::get_logs_path();
        // Ensure logs directory exists; report to stderr on failure.
        if let Err(e) = std::fs::create_dir_all(&logs_dir) {
            eprintln!("Failed to create logs dir {}: {}", logs_dir, e);
            // fall back to env_logger; ignore its error
            let _ = env_logger::try_init();
            return;
        }

        // Use date-based filenames so each day creates separate files.
        let date = chrono::Local::now().format("%Y-%m-%d").to_string();

        let pattern = "{d(%Y-%m-%d %H:%M:%S%.3f)} {l} {t} - {m}\n";
        let mut config = log4rs::config::Config::builder();

        // Create a file appender for each level, splitting by level.
        let levels = vec![
            ("trace", LevelFilter::Trace, Level::Trace),
            ("debug", LevelFilter::Debug, Level::Debug),
            ("info", LevelFilter::Info, Level::Info),
            ("warn", LevelFilter::Warn, Level::Warn),
            ("error", LevelFilter::Error, Level::Error),
        ];

        for (level_name, _level_filter, level) in levels {
            let dated_log_path = format!("{}/{}_{}.log", logs_dir, level_name, date);
            let app = match log4rs::append::file::FileAppender::builder()
                .encoder(Box::new(log4rs::encode::pattern::PatternEncoder::new(
                    pattern,
                )))
                .build(dated_log_path)
            {
                Ok(a) => a,
                Err(e) => {
                    eprintln!("Failed to build FileAppender for {}: {}", level_name, e);
                    let _ = env_logger::try_init();
                    return;
                }
            };
            let appender = log4rs::config::Appender::builder()
                .filter(Box::new(EqualFilter { level }))
                .build(level_name, Box::new(app));
            config = config.appender(appender);
        }

        let root = log4rs::config::Root::builder()
            .appender("trace")
            .appender("debug")
            .appender("info")
            .appender("warn")
            .appender("error")
            .build(LevelFilter::Info);

        match config.build(root) {
            Ok(cfg) => {
                if let Err(e) = log4rs::init_config(cfg) {
                    // If another logger is already initialized, just fall back silently.
                    eprintln!("Failed to initialize log4rs: {}", e);
                    let _ = env_logger::try_init();
                }
            }
            Err(e) => {
                eprintln!("Failed to build log4rs config: {}", e);
                let _ = env_logger::try_init();
            }
        }
    });
}

pub fn engine_init() {
    // no-op
}

// 应用级的 Windows 服务名。在此处设置（不要从 CLI 或外部配置读取）。
// 如需匹配已安装的服务，请修改此常量。
const SERVICE_NAME: &str = "quant1x-rust";
// 面向用户的服务描述。将其保留在应用中以便安装程序和管理员在创建服务时参考。
const SERVICE_DESC: &str = "Quant1X background service for q1x operations";
// 在 Windows 服务管理器中显示的人类友好名称。
const SERVICE_DISPLAY_NAME: &str = "Quant1X Service(Rust)";

#[cfg(windows)]
fn normalize_to_utf8(b: &[u8]) -> Vec<u8> {
    // 检测常见编码并返回 UTF-8 字节。
    // 1) 带 BOM 的 UTF-16LE
    if b.len() >= 2 && b[0] == 0xFF && b[1] == 0xFE {
        let mut u16s = Vec::with_capacity(b.len() / 2);
        let mut i = 2; // skip BOM
        while i + 1 < b.len() {
            let lo = b[i] as u16;
            let hi = b[i + 1] as u16;
            u16s.push((hi << 8) | lo);
            i += 2;
        }
        return String::from_utf16_lossy(&u16s).into_bytes();
    }

    // 2) 启发式判断：大量的零字节 -> 很可能是无 BOM 的 UTF-16LE
    let zeros = b.iter().filter(|&&x| x == 0).count();
    if zeros * 2 > b.len() && b.len() > 2 {
        let mut u16s = Vec::with_capacity(b.len() / 2);
        let mut i = 0;
        while i + 1 < b.len() {
            let lo = b[i] as u16;
            let hi = b[i + 1] as u16;
            u16s.push((hi << 8) | lo);
            i += 2;
        }
        return String::from_utf16_lossy(&u16s).into_bytes();
    }

    // 3) 尝试 UTF-8
    if let Ok(s) = std::str::from_utf8(b) {
        return s.as_bytes().to_vec();
    }

    // 4) 回退方案：OEM 代码页 -> wide -> 通过 Win32 API 转为 UTF-8
    unsafe {
        // Use winapi functions directly to avoid adding new deps.
        use std::os::raw::c_char;

        let mb_bytes = b;
        // MultiByteToWideChar(CP_OEMCP, ...) -> wide
        let needed_wchars = winapi::um::stringapiset::MultiByteToWideChar(
            winapi::um::winnls::CP_OEMCP as u32,
            0,
            mb_bytes.as_ptr() as *const c_char,
            mb_bytes.len() as i32,
            std::ptr::null_mut(),
            0,
        );
        if needed_wchars <= 0 {
            return String::from_utf8_lossy(b).into_owned().into_bytes();
        }
        let mut wide: Vec<u16> = vec![0u16; needed_wchars as usize];
        let got = winapi::um::stringapiset::MultiByteToWideChar(
            winapi::um::winnls::CP_OEMCP as u32,
            0,
            mb_bytes.as_ptr() as *const c_char,
            mb_bytes.len() as i32,
            wide.as_mut_ptr(),
            needed_wchars,
        );
        if got == 0 {
            return String::from_utf8_lossy(b).into_owned().into_bytes();
        }

        // WideCharToMultiByte(CP_UTF8, ...) -> UTF-8 bytes
        let needed_utf8 = winapi::um::stringapiset::WideCharToMultiByte(
            winapi::um::winnls::CP_UTF8 as u32,
            0,
            wide.as_ptr(),
            got,
            std::ptr::null_mut(),
            0,
            std::ptr::null(),
            std::ptr::null_mut(),
        );
        if needed_utf8 <= 0 {
            return String::from_utf16_lossy(&wide[..got as usize]).into_bytes();
        }
        let mut out: Vec<u8> = vec![0u8; needed_utf8 as usize];
        let wrote = winapi::um::stringapiset::WideCharToMultiByte(
            winapi::um::winnls::CP_UTF8 as u32,
            0,
            wide.as_ptr(),
            got,
            out.as_mut_ptr() as *mut i8,
            needed_utf8,
            std::ptr::null(),
            std::ptr::null_mut(),
        );
        if wrote == 0 {
            return String::from_utf16_lossy(&wide[..got as usize]).into_bytes();
        }
        return out;
    }
}

pub fn engine_daemon(
    _action: &str,
    _pipe: bool,
    _elevated_out: Option<&str>,
    _elevated_pipe: Option<&str>,
) -> i32 {
    // 默认实现：在非 Windows 平台上不提供 service 管理器；
    // 在 Windows 上尝试进行 UAC 提升，并在 `pipe` 设置时将提升后子进程的 stdout/stderr 回传给父进程。
    #[cfg(not(windows))]
    {
        log::error!("engine_daemon not implemented in Rust library (non-Windows)");
        return 1;
    }

    #[cfg(windows)]
    {
        use std::env;
        use std::io::{Read, Seek, SeekFrom, Write};
        use std::process::Command;
        use std::thread::sleep;
        use std::time::Duration;

        // Local bindings to rename underscore-prefixed parameters to the
        // names used throughout the Windows-specific implementation. This
        // avoids unused-variable warnings when compiling for non-Windows
        // targets while keeping the logic identical on Windows.
        let action = _action;
        let elevated_out = _elevated_out;
        let elevated_pipe = _elevated_pipe;

        // 如果用户请求直接 'run'，则在进程内运行（如果 crate 提供 runner）。
        if action == "run" {
            log::info!("service run requested; no in-process runner provided in this build");
            return 1;
        }

        // 对于 install/uninstall/start/stop/status 等操作通常需要提权。
        // 如果当前已经具有提权权限，则调用 crate 提供的实现。
        if is_current_process_elevated() {
            // 提权子进程模式：执行请求的操作（install/uninstall），
            // 并将命令输出通过命名管道回写给父进程（优先），或作为回退写入 elevated_out 文件。
            use std::ffi::OsStr;
            use std::os::windows::ffi::OsStrExt;
            use winapi::shared::minwindef::DWORD;
            use winapi::um::fileapi::{CreateFileW, WriteFile, OPEN_EXISTING};
            use winapi::um::handleapi::CloseHandle;
            use winapi::um::winnt::FILE_ATTRIBUTE_NORMAL;
            use winapi::um::winnt::GENERIC_WRITE;

            // Helper to write bytes to pipe or fallback file
            let write_bytes = |bytes: &[u8]| -> Result<(), String> {
                // Try pipe first
                if let Some(pipe_name) = elevated_pipe {
                    let wide: Vec<u16> = OsStr::new(pipe_name)
                        .encode_wide()
                        .chain(std::iter::once(0))
                        .collect();
                    // Try to open the pipe with short retries
                    let start = std::time::Instant::now();
                    let mut handle = std::ptr::null_mut();
                    while start.elapsed() < std::time::Duration::from_secs(5) {
                        unsafe {
                            handle = CreateFileW(
                                wide.as_ptr(),
                                GENERIC_WRITE,
                                0,
                                std::ptr::null_mut(),
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                std::ptr::null_mut(),
                            );
                        }
                        if !handle.is_null() && handle as isize != -1 {
                            break;
                        }
                        std::thread::sleep(std::time::Duration::from_millis(200));
                    }
                    if !handle.is_null() && handle as isize != -1 {
                        let mut written: DWORD = 0;
                        unsafe {
                            let ok = WriteFile(
                                handle,
                                bytes.as_ptr() as *const _,
                                bytes.len() as DWORD,
                                &mut written as *mut _,
                                std::ptr::null_mut(),
                            );
                            let _ = CloseHandle(handle as *mut _);
                            if ok == 0 {
                                return Err("WriteFile failed".to_string());
                            }
                        }
                        return Ok(());
                    }
                }

                // Fallback to file
                if let Some(path) = elevated_out {
                    let _ = std::fs::create_dir_all(
                        std::path::Path::new(path)
                            .parent()
                            .unwrap_or(std::path::Path::new(".")),
                    );
                    match std::fs::OpenOptions::new()
                        .create(true)
                        .append(true)
                        .open(path)
                    {
                        Ok(mut f) => {
                            if let Err(e) = f.write_all(bytes) {
                                return Err(format!("Failed writing to elevated-out file: {}", e));
                            }
                            let _ = f.flush();
                            return Ok(());
                        }
                        Err(e) => return Err(format!("Failed to open elevated-out file: {}", e)),
                    }
                }

                Err("No pipe or elevated-out available".to_string())
            };

            // Determine exe path for service binary
            let exe_path = match env::current_exe() {
                Ok(p) => p,
                Err(e) => {
                    let msg = format!("failed to determine executable path: {}\n", e);
                    let _ = write_bytes(msg.as_bytes());
                    return 1;
                }
            };

            // Run action-specific commands
            match action {
                "install" => {
                    // sc create <name> binPath= "<exe> service" DisplayName= "<display>" start= auto
                    let binarg = format!("binPath=\"{}\"", exe_path.display());
                    let disp = format!("DisplayName={}", SERVICE_DISPLAY_NAME);
                    let output = Command::new("sc")
                        .arg("create")
                        .arg(SERVICE_NAME)
                        .arg(binarg)
                        .arg(disp)
                        .arg("start=auto")
                        .output();
                    let mut combined = Vec::new();
                    match output {
                        Ok(o) => {
                            combined.extend_from_slice(&o.stdout);
                            combined.extend_from_slice(&o.stderr);
                        }
                        Err(e) => {
                            combined.extend_from_slice(
                                format!("failed to run sc create: {}\n", e).as_bytes(),
                            );
                        }
                    }
                    // set description
                    let desc_out = Command::new("sc")
                        .arg("description")
                        .arg(SERVICE_NAME)
                        .arg(SERVICE_DESC)
                        .output();
                    match desc_out {
                        Ok(o) => {
                            combined.extend_from_slice(&o.stdout);
                            combined.extend_from_slice(&o.stderr);
                        }
                        Err(e) => {
                            combined.extend_from_slice(
                                format!("failed to run sc description: {}\n", e).as_bytes(),
                            );
                        }
                    }

                    let _ = write_bytes(&normalize_to_utf8(&combined));
                    return 0;
                }
                "uninstall" => {
                    // sc delete <name>
                    let output = Command::new("sc").arg("delete").arg(SERVICE_NAME).output();
                    let mut combined = Vec::new();
                    match output {
                        Ok(o) => {
                            combined.extend_from_slice(&o.stdout);
                            combined.extend_from_slice(&o.stderr);
                        }
                        Err(e) => {
                            combined.extend_from_slice(
                                format!("failed to run sc delete: {}\n", e).as_bytes(),
                            );
                        }
                    }
                    let _ = write_bytes(&normalize_to_utf8(&combined));
                    return 0;
                }
                _ => {
                    // Unsupported elevated action: fall back to simple message
                    let msg = format!(
                        "Elevated, but action '{}' not implemented in this shim.\n",
                        action
                    );
                    let _ = write_bytes(msg.as_bytes());
                    return 1;
                }
            }
        }

        // Not elevated: re-launch elevated and capture output via a named pipe (preferred) or fallback to temp file.
        // If an elevated_pipe name was supplied (from caller), use it; otherwise generate one.
        let pipe_name = elevated_pipe.map(|s| s.to_string()).unwrap_or_else(|| {
            format!(
                r"\\.\pipe\quant1x-{}-{}",
                std::process::id(),
                chrono::Local::now().timestamp()
            )
        });

        // 尽力检查目标服务在尝试 `start`/`stop` 之前是否已安装。
        // 我们将使用可执行文件的文件名作为服务名候选（例如 'stock'）。
        if action == "start" || action == "stop" || action == "status" {
            // Use compile-time SERVICE_NAME constant as the Windows service name.
            let svc_name = SERVICE_NAME;
            let check_cmd = format!(
                "Get-Service -Name '{}' -ErrorAction SilentlyContinue",
                svc_name
            );
            if let Ok(out) = Command::new("powershell")
                .arg("-NoProfile")
                .arg("-Command")
                .arg(check_cmd)
                .output()
            {
                let stdout = String::from_utf8_lossy(&out.stdout).to_string();
                if stdout.trim().is_empty() {
                    eprintln!("Service '{}' not found ({}). Please install the service before calling '{}'.", svc_name, SERVICE_DESC, action);
                    eprintln!("Hint: use sc.exe create <{}> binPath= \"<path-to-exe>\" DisplayName= \"{}\"", svc_name, SERVICE_DISPLAY_NAME);
                    return 1;
                }
            }
        }

        // 在后台线程中创建一个命名管道服务端，接收一个客户端并将数据流打印到 stdout。
        let server_name = pipe_name.clone();
        let server_handle = std::thread::spawn(move || {
            use std::ffi::OsStr;
            use std::os::windows::ffi::OsStrExt;
            use winapi::shared::minwindef::DWORD;
            use winapi::um::fileapi::ReadFile;
            use winapi::um::handleapi::CloseHandle;
            use winapi::um::namedpipeapi::CreateNamedPipeW;
            use winapi::um::winbase::PIPE_ACCESS_INBOUND;
            use winapi::um::winbase::{
                PIPE_READMODE_BYTE, PIPE_TYPE_BYTE, PIPE_UNLIMITED_INSTANCES, PIPE_WAIT,
            };
            use winapi::um::winnt::GENERIC_READ;
            use winapi::um::winnt::HANDLE as WinHandle;
            use winapi::um::winnt::{FILE_SHARE_READ, FILE_SHARE_WRITE};

            let wide: Vec<u16> = OsStr::new(&server_name)
                .encode_wide()
                .chain(std::iter::once(0))
                .collect();

            unsafe {
                let handle: WinHandle = CreateNamedPipeW(
                    wide.as_ptr(),
                    PIPE_ACCESS_INBOUND,
                    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                    PIPE_UNLIMITED_INSTANCES,
                    4096,
                    4096,
                    0,
                    std::ptr::null_mut(),
                );

                if handle == winapi::um::handleapi::INVALID_HANDLE_VALUE as WinHandle {
                    log::error!("CreateNamedPipeW failed for {}", server_name);
                    return;
                }

                // 等待客户端连接。ConnectNamedPipe 会阻塞直到客户端连接。
                // 如返回失败，则检查是否为客户端已连接（ERROR_PIPE_CONNECTED），若是则继续处理。
                use winapi::shared::winerror::ERROR_PIPE_CONNECTED;
                use winapi::um::errhandlingapi::GetLastError;
                use winapi::um::namedpipeapi::ConnectNamedPipe;

                let conn = ConnectNamedPipe(handle, std::ptr::null_mut());
                if conn == 0 {
                    let err = GetLastError();
                    if err != ERROR_PIPE_CONNECTED {
                        log::error!("ConnectNamedPipe failed for {}: error {}", server_name, err);
                        let _ = CloseHandle(handle as *mut _);
                        return;
                    }
                    // else: ERROR_PIPE_CONNECTED means client already connected; proceed
                }

                let mut buf = [0u8; 4096];
                loop {
                    let mut read: DWORD = 0;
                    let ok = ReadFile(
                        handle,
                        buf.as_mut_ptr() as *mut _,
                        buf.len() as DWORD,
                        &mut read as *mut _,
                        std::ptr::null_mut(),
                    );
                    if ok != 0 && read > 0 {
                        let s = String::from_utf8_lossy(&buf[..read as usize]);
                        print!("{}", s);
                    } else {
                        break;
                    }
                }

                let _ = CloseHandle(handle as *mut _);
            }
        });

        // 如果提升后的子进程通过管道回连（以提升方式运行），子进程应打开该管道并写入日志。
        // 现在我们启动提升进程，服务端线程将接受连接并打印数据。

        // 构建参数：在原参数基础上附加标记 --elevated-pipe <pipename>
        let mut args: Vec<String> = env::args().collect();
        // append action if not present
        if !args.iter().any(|a| a == "service") {
            args.push("service".to_string());
            args.push(action.to_string());
        }
        args.push("--elevated-pipe".to_string());
        args.push(pipe_name.clone());

        // 为 Start-Process 组合参数列表
        let exe = match env::current_exe() {
            Ok(p) => p,
            Err(e) => {
                eprintln!("failed to determine executable path: {}", e);
                return 1;
            }
        };
        // Build -ArgumentList as a comma-separated list of quoted arguments
        let mut arg_items: Vec<String> = Vec::new();
        for a in args.iter().skip(1) {
            arg_items.push(format!("\"{}\"", a.replace("\"", "\\\"")));
        }
        let arglist = arg_items.join(", ");

        // 隐藏被启动的提升窗口以获得更简洁的用户体验；提升后的子进程将通过管道通信。
        let ps_cmd = format!(
            "Start-Process -FilePath \"{}\" -ArgumentList {} -Verb RunAs -WindowStyle Hidden",
            exe.display(),
            arglist
        );
        let spawn = Command::new("powershell")
            .arg("-NoProfile")
            .arg("-Command")
            .arg(ps_cmd)
            .spawn();

        match spawn {
            Ok(mut child) => {
                // Wait for helper to be launched; join the server thread when done.
                if let Ok(_) = child.wait() {
                    // child returned quickly (likely failure or Start-Process returned after launching)
                }
                // Wait for server thread to finish reading (it will exit on EOF)
                let _ = server_handle.join();
                return 0;
            }
            Err(e) => {
                log::error!("Failed to Spawn UAC helper: {}", e);
                return 1;
            }
        }
    }
}

#[cfg(windows)]
fn is_current_process_elevated() -> bool {
    // 使用 PowerShell 检查当前进程是否具有提升权限。这避免引入原生 Windows crate，保持此适配层轻量。
    use std::process::Command;
    let check = r#"[bool](([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator))"#;
    let out = Command::new("powershell")
        .arg("-NoProfile")
        .arg("-Command")
        .arg(check)
        .output();
    if let Ok(o) = out {
        if let Ok(s) = String::from_utf8(o.stdout) {
            return s.trim().eq_ignore_ascii_case("True");
        }
    }
    false
}

// 库作者可以实现如下函数以扩展功能：
// pub fn try_run_subcommand(name: &str, matches: &clap::ArgMatches) -> Result<bool, Box<dyn std::error::Error>>
// 如果提供了该函数，`src/main.rs` 将通过 crate 的公共 API 调用它。我们在此未实现该函数，
// 以避免将 clap 泄露到库的公共接口中。

/// 一个内置的简易处理器，用于处理若干顶层管理命令。
/// 当前支持：
/// - "update"：刷新日历和/或服务器缓存。对应 CLI 标志 `--calendar`、`--servers`、`--all`。
pub fn try_run_subcommand(
    name: &str,
    matches: &clap::ArgMatches,
) -> Result<bool, Box<dyn std::error::Error>> {
    if name != "update" {
        return Ok(false);
    }

    use indicatif::{ProgressBar, ProgressStyle};
    use std::time::{Duration, Instant};

    // 确定请求的更新范围
    let only_calendar = matches.get_flag("calendar");
    let only_servers = matches.get_flag("servers");
    let all = matches.get_flag("all");

    // base and features keys (may be multi-valued)
    let base_keys: Vec<String> = match matches.get_many::<String>("base") {
        Some(vals) => vals.map(|s| s.to_string()).collect(),
        None => Vec::new(),
    };
    let features_keys: Vec<String> = match matches.get_many::<String>("features") {
        Some(vals) => vals.map(|s| s.to_string()).collect(),
        None => Vec::new(),
    };

    // 默认行为：若未设置任何标志或 key，更新所有（base + features）
    let do_calendar = if !only_calendar
        && !only_servers
        && !all
        && base_keys.is_empty()
        && features_keys.is_empty()
    {
        true
    } else {
        only_calendar || all || base_keys.iter().any(|k| k == "calendar")
    };
    let do_servers = if !only_calendar
        && !only_servers
        && !all
        && base_keys.is_empty()
        && features_keys.is_empty()
    {
        true
    } else {
        only_servers || all || base_keys.iter().any(|k| k == "servers")
    };

    if do_calendar {
        log::info!("正在更新日历缓存...");
        let spinner = ProgressBar::new_spinner();
        spinner.set_style(ProgressStyle::with_template("{spinner} {msg}").unwrap());
        spinner.enable_steady_tick(Duration::from_millis(80));
        spinner.set_message("Downloading/updating calendar...");

        let start = Instant::now();
        // Ensure calendar cache exists and trigger any necessary lazy loading.
        if let Err(e) = crate::exchange::calendar::ensure_calendar_cache() {
            spinner.finish_with_message(format!(
                "Failed to ensure calendar cache: {} (path {})",
                e,
                crate::config::get_calendar_filename()
            ));
        }

        spinner.finish_with_message(format!(
            "Calendar ensured at {} (elapsed {:?})",
            crate::config::get_calendar_filename(),
            start.elapsed()
        ));
    }

    if do_servers {
        log::info!("正在探测 level1 服务器（握手探测）...");
        let start = Instant::now();
        let detected = crate::level1::config::detect(
            crate::level1::config::MAX_ELAPSED_TIME_MS,
            crate::level1::config::MAX_CONNECTIONS,
            crate::level1::config::DEFAULT_CONNECT_TIMEOUT_MS,
        );

        if detected.is_empty() {
            log::warn!("未探测到可用服务器。");
        } else {
            for srv in detected.iter() {
                log::info!(
                    "{} {} => {}:{} ({} ms)",
                    srv.source,
                    srv.name,
                    srv.host,
                    srv.port,
                    srv.latency_ms
                );
            }
            crate::level1::config::save_cached_servers(&detected);
            log::info!(
                "Saved {} servers to cache (elapsed {:?}).",
                detected.len(),
                start.elapsed()
            );
        }
    }

    // Base data updates (example keys: "xdxr", "calendar", "servers").
    // If the user supplied --base keys, update those; if no keys were supplied
    // and no other flags were given, default is to update all base keys.
    let want_base = if all || (!all && base_keys.is_empty() && features_keys.is_empty()) {
        true
    } else {
        !base_keys.is_empty()
    };
    if want_base {
        // If the user specified base keys, select plugins with those keys; otherwise update all base data plugins
        if base_keys.is_empty() {
            // update all base adapters
            let _count = crate::data::update_all_mask(
                crate::data::PLUGIN_MASK_BASE_DATA,
                None,
                crate::exchange::last_trading_day(crate::Timestamp::now()),
            );
            log::info!("Updated {} base adapters", _count);
        } else {
            // update only named base adapters
            let ks: Vec<String> = base_keys.clone();
            let _count = crate::data::update_all_mask(
                crate::data::PLUGIN_MASK_BASE_DATA,
                Some(&ks),
                crate::exchange::last_trading_day(crate::Timestamp::now()),
            );
            log::info!("Updated {} selected base adapters", _count);
        }
    }

    // Feature data updates (example keys: "kline", etc.).
    // If the user supplied --features keys, update those; if no keys were supplied
    // and no other flags were given, default is to update all feature data.
    let want_features = if all || (!all && base_keys.is_empty() && features_keys.is_empty()) {
        true
    } else {
        !features_keys.is_empty()
    };
    if want_features {
        // If the user specified feature keys, select plugins with those keys; otherwise update all feature data plugins
        if features_keys.is_empty() {
            // update all feature adapters
            let _count = crate::data::update_all_mask(
                crate::data::PLUGIN_MASK_FEATURE,
                None,
                crate::exchange::last_trading_day(crate::Timestamp::now()),
            );
            log::info!("Updated {} feature adapters", _count);
        } else {
            // update only named feature adapters
            let ks: Vec<String> = features_keys.clone();
            let _count = crate::data::update_all_mask(
                crate::data::PLUGIN_MASK_FEATURE,
                Some(&ks),
                crate::exchange::last_trading_day(crate::Timestamp::now()),
            );
            log::info!("Updated {} selected feature adapters", _count);
        }
    }

    Ok(true)
}
