// Small shim module providing application-level entrypoints used by src/main.rs.
// These are intentionally minimal and non-intrusive: they provide no-op fallbacks
// which higher-level Rust implementations can override by providing fuller
// implementations in this crate.

pub fn global_init() {
    // no-op
}

pub fn datasets_init() {
    // Initialize datasets and register adapters implemented in Rust
    if let Err(e) = std::panic::catch_unwind(|| {
        crate::datasets::init();
    }) {
        log::error!("datasets::init() panicked: {:?}", e);
    }
}

pub fn logger_set(_verbose: bool, _debug: bool) {
    // initialize log4rs to write logs to <cache>/logs/quant1x.log with rolling policy.
    use log::LevelFilter;
    use std::path::PathBuf;

    let logs_dir = crate::config::get_logs_path();
    // ensure logs dir exists
    if let Err(e) = std::fs::create_dir_all(&logs_dir) {
        log::error!("Failed to create logs dir {}: {}", logs_dir, e);
    }

    let mut log_path = PathBuf::from(&logs_dir);
    log_path.push("quant1x.log");

    // Use a date-stamped logfile name so each day produces a separate file.
    let date = chrono::Local::now().format("%Y-%m-%d").to_string();
    let mut dated_log_path = PathBuf::from(&logs_dir);
    dated_log_path.push(format!("quant1x-{}.log", date));

    let pattern = "{d} {l} {t} - {m}\n";
    let app = match log4rs::append::file::FileAppender::builder()
        .encoder(Box::new(log4rs::encode::pattern::PatternEncoder::new(
            pattern,
        )))
        .build(dated_log_path.clone())
    {
        Ok(a) => a,
        Err(e) => {
            log::error!("Failed to build FileAppender: {}", e);
            let _ = env_logger::try_init();
            return;
        }
    };

    let mut config = log4rs::config::Config::builder();
    let appender = log4rs::config::Appender::builder().build("file", Box::new(app));
    config = config.appender(appender);

    let level = if _debug {
        LevelFilter::Debug
    } else {
        LevelFilter::Info
    };
    let root = log4rs::config::Root::builder()
        .appender("file")
        .build(level);

    match config.build(root) {
        Ok(cfg) => {
            if let Err(e) = log4rs::init_config(cfg) {
                log::error!("Failed to initialize log4rs: {}", e);
                let _ = env_logger::try_init();
            }
        }
        Err(e) => {
            log::error!("Failed to build log4rs config: {}", e);
            let _ = env_logger::try_init();
        }
    }
}

pub fn engine_init() {
    // no-op
}

// Application-level Windows service name. Set here (do not read from CLI or external config).
// Change this constant to match the installed service name for your application.
const SERVICE_NAME: &str = "quant1x-rust";
// Human-readable service description. Keep this in the application so installers
// and admins can refer to it when creating the Windows service.
const SERVICE_DESC: &str = "Quant1X background service for q1x operations";
// Human-friendly display name shown in Windows service manager.
const SERVICE_DISPLAY_NAME: &str = "Quant1X Service(Rust)";

#[cfg(windows)]
fn normalize_to_utf8(b: &[u8]) -> Vec<u8> {
    // Detect common encodings and return UTF-8 bytes.
    // 1) UTF-16LE with BOM
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

    // 2) Heuristic: many zero bytes -> likely UTF-16LE without BOM
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

    // 3) Try UTF-8
    if let Ok(s) = std::str::from_utf8(b) {
        return s.as_bytes().to_vec();
    }

    // 4) Fallback: OEM code page -> wide -> UTF-8 via Win32 APIs
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
    action: &str,
    _pipe: bool,
    elevated_out: Option<&str>,
    elevated_pipe: Option<&str>,
) -> i32 {
    // Default implementation: on non-Windows platforms we don't provide a
    // service manager; on Windows try to perform UAC elevation and relay
    // elevated child stdout/stderr back to the parent when `pipe` is set.
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

        // If user asked to 'run' directly, just run in-process if crate exposes a runner.
        if action == "run" {
            log::info!("service run requested; no in-process runner provided in this build");
            return 1;
        }

        // For install/uninstall/start/stop/status we typically need elevation.
        // If already elevated, call into crate-provided implementations.
        if is_current_process_elevated() {
            // Elevated child mode: perform the requested action (install/uninstall)
            // and write command output back to the parent via named pipe (preferred)
            // or append to elevated_out file as fallback.
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

        // Best-effort: check whether the target service appears installed before attempting `start`/`stop`.
        // We'll use the executable file stem as the service name candidate (e.g. 'stock').
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

        // Create a named pipe server in a background thread that will accept one client and stream data to stdout.
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

                // Wait for a client to connect. ConnectNamedPipe will block until
                // a client connects. If it returns failure, check whether the
                // client already connected (ERROR_PIPE_CONNECTED) and continue.
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

        // If the elevated child connects back by pipe (when it runs elevated), the child should open the pipe and write logs.
        // We now launch the elevated process and let the server thread accept the connection and print data.

        // Build args: original args plus marker --elevated-pipe <pipename>
        let mut args: Vec<String> = env::args().collect();
        // append action if not present
        if !args.iter().any(|a| a == "service") {
            args.push("service".to_string());
            args.push(action.to_string());
        }
        args.push("--elevated-pipe".to_string());
        args.push(pipe_name.clone());

        // Compose argument list for Start-Process
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

        // Hide the spawned elevated window for a cleaner UX; the elevated child will communicate via the pipe.
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
    // Use PowerShell to ask whether current process is elevated. This avoids
    // pulling in native Windows crates and keeps the shim lightweight.
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

// Library authors can extend with a function like:
// pub fn try_run_subcommand(name: &str, matches: &clap::ArgMatches) -> Result<bool, Box<dyn std::error::Error>>
// If provided, src/main.rs will call it via the crate public API. We don't implement
// it here to avoid pulling clap into the library surface.

/// A small built-in handler for a few top-level administrative commands.
/// Currently supports:
/// - "update": refresh calendar and/or server cache. Matches CLI flags `--calendar`, `--servers`, `--all`.
pub fn try_run_subcommand(
    name: &str,
    matches: &clap::ArgMatches,
) -> Result<bool, Box<dyn std::error::Error>> {
    if name != "update" {
        return Ok(false);
    }

    use indicatif::{ProgressBar, ProgressStyle};
    use std::time::{Duration, Instant};

    // determine requested scopes
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

    // default behavior: if no flags/keys set, update all (base + features)
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
        log::info!("Updating calendar cache...");
        let spinner = ProgressBar::new_spinner();
        spinner.set_style(ProgressStyle::with_template("{spinner} {msg}").unwrap());
        spinner.enable_steady_tick(Duration::from_millis(80));
        spinner.set_message("Downloading/updating calendar...");

        let start = Instant::now();
        // Ensure calendar cache file exists (placeholder). If a real calendar
        // downloader is available in the crate it should replace this logic.
        let cal_file = crate::get_calendar_filename();
        if !std::path::Path::new(&cal_file).exists() {
            let _ = std::fs::File::create(&cal_file);
        }

        spinner.finish_with_message(format!(
            "Calendar ensured at {} (elapsed {:?})",
            cal_file,
            start.elapsed()
        ));
    }

    if do_servers {
        log::info!("Detecting level1 servers (network probe)...");
        // We'll show a progress bar while probing the standard server list
        let servers = crate::level1::config::standard_server_list();
        let total = servers.len() as u64;
        let pb = ProgressBar::new(total);
        pb.set_style(
            ProgressStyle::with_template(
                "{spinner:.green} [{elapsed_precise}] [{bar:40.cyan/blue}] {pos}/{len} {msg}",
            )?
            .progress_chars("=> "),
        );

        let mut found: Vec<crate::level1::config::ServerInfo> = Vec::new();
        use std::net::ToSocketAddrs;
        for s in servers.into_iter() {
            pb.set_message(s.desc.clone());
            // Try to connect with a short timeout to measure reachability
            let start = Instant::now();
            let addr = s.addr();
            let timeout = std::time::Duration::from_millis(250);
            if let Ok(mut addrs) = addr.to_socket_addrs() {
                if let Some(sock) = addrs.find(|_| true) {
                    if let Ok(res) = std::net::TcpStream::connect_timeout(&sock, timeout) {
                        let latency = start.elapsed().as_millis() as i64;
                        let mut si = s.clone();
                        si.latency_ms = latency;
                        let _ = res.shutdown(std::net::Shutdown::Both);
                        found.push(si);
                    }
                }
            }
            pb.inc(1);
        }
        pb.finish_with_message(format!(
            "Probe completed, {} responsive servers",
            found.len()
        ));

        if !found.is_empty() {
            // sort and trim similar to detect()
            found.sort_by_key(|s| s.latency_ms);
            let limit = std::cmp::min(found.len(), 8);
            let saved = found.into_iter().take(limit).collect::<Vec<_>>();
            crate::level1::config::save_cached_servers(&saved);
            log::info!("Saved {} best servers to cache.", saved.len());
        } else {
            log::info!("No responsive servers discovered.");
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
            let _count = crate::cache::update_all_mask(
                crate::cache::PLUGIN_MASK_BASE_DATA,
                None,
                crate::exchange::last_trading_day(crate::Timestamp::now()),
            );
            log::info!("Updated {} base adapters", _count);
        } else {
            // update only named base adapters
            let ks: Vec<String> = base_keys.clone();
            let _count = crate::cache::update_all_mask(
                crate::cache::PLUGIN_MASK_BASE_DATA,
                Some(&ks),
                crate::exchange::last_trading_day(crate::Timestamp::now()),
            );
            log::info!("Updated {} selected base adapters", _count);
        }
    }

    Ok(true)
}
