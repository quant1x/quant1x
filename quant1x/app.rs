// Small shim module providing application-level entrypoints used by src/main.rs.
// These are intentionally minimal and non-intrusive: they provide no-op fallbacks
// which higher-level Rust implementations can override by providing fuller
// implementations in this crate.

pub fn global_init() {
    // no-op
}

pub fn datasets_init() {
    // no-op
}

pub fn logger_set(_verbose: bool, _debug: bool) {
    // initialize log4rs to write logs to <cache>/logs/quant1x.log with rolling policy.
    use std::path::PathBuf;
    use log::LevelFilter;

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
        .encoder(Box::new(log4rs::encode::pattern::PatternEncoder::new(pattern)))
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

    let level = if _debug { LevelFilter::Debug } else { LevelFilter::Info };
    let root = log4rs::config::Root::builder().appender("file").build(level);

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

pub fn engine_daemon(_action: &str, _pipe: bool) -> i32 {
    // default: not implemented -> return failure
    log::error!("engine_daemon not implemented in Rust library");
    1
}

// Library authors can extend with a function like:
// pub fn try_run_subcommand(name: &str, matches: &clap::ArgMatches) -> Result<bool, Box<dyn std::error::Error>>
// If provided, src/main.rs will call it via the crate public API. We don't implement
// it here to avoid pulling clap into the library surface.

/// A small built-in handler for a few top-level administrative commands.
/// Currently supports:
/// - "update": refresh calendar and/or server cache. Matches CLI flags `--calendar`, `--servers`, `--all`.
pub fn try_run_subcommand(name: &str, matches: &clap::ArgMatches) -> Result<bool, Box<dyn std::error::Error>> {
    if name != "update" { return Ok(false); }

    use indicatif::{ProgressBar, ProgressStyle};
    use std::time::{Instant, Duration};

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
    let do_calendar = if !only_calendar && !only_servers && !all && base_keys.is_empty() && features_keys.is_empty() { true } else { only_calendar || all || base_keys.iter().any(|k| k == "calendar") };
    let do_servers = if !only_calendar && !only_servers && !all && base_keys.is_empty() && features_keys.is_empty() { true } else { only_servers || all || base_keys.iter().any(|k| k == "servers") };

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

        spinner.finish_with_message(format!("Calendar ensured at {} (elapsed {:?})", cal_file, start.elapsed()));
    }

    if do_servers {
        log::info!("Detecting level1 servers (network probe)...");
        // We'll show a progress bar while probing the standard server list
        let servers = crate::level1::config::standard_server_list();
        let total = servers.len() as u64;
        let pb = ProgressBar::new(total);
        pb.set_style(ProgressStyle::with_template("{spinner:.green} [{elapsed_precise}] [{bar:40.cyan/blue}] {pos}/{len} {msg}")?.progress_chars("=> "));

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
        pb.finish_with_message(format!("Probe completed, {} responsive servers", found.len()));

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
    let want_base = if all || (!all && base_keys.is_empty() && features_keys.is_empty()) { true } else { !base_keys.is_empty() };
    if want_base {
        // Determine if we should run xdxr base update
        let should_xdxr = base_keys.is_empty() || base_keys.iter().any(|k| k.eq_ignore_ascii_case("xdxr"));
        if should_xdxr {
            let codes: Vec<String> = crate::exchange::get_code_list();
            if !codes.is_empty() {
                let total = codes.len() as u64;
                use indicatif::{ProgressBar, ProgressStyle};
                let pb = ProgressBar::new(total);
                pb.set_style(ProgressStyle::with_template("{spinner:.green} [{elapsed_precise}] {pos}/{len} {msg}").unwrap().progress_chars("=> "));

                // ensure xdxr dir exists
                let xdxr_dir = crate::config::get_xdxr_path();
                if let Err(e) = std::fs::create_dir_all(&xdxr_dir) {
                    log::error!("Failed to create xdxr dir {}: {}", xdxr_dir, e);
                }

                for code in codes.into_iter() {
                    pb.set_message(code.clone());
                    match crate::level1::xdxr::fetch_xdxr(&code) {
                        Some(resp) => {
                            // write CSV using C++ header order
                            use std::io::Write;
                            let filename = crate::config::get_xdxr_filename(&code);
                            let tmp = format!("{}.tmp", filename);

                            // header exactly as in datasets/xdxr.cpp
                            let header = "Date,Category,Name,FenHong,PeiGuJia,SongZhuanGu,PeiGu,SuoGu,QianLiuTong,HouLiuTong,QianZongGuBen,HouZongGuBen,FenShu,XingQuanJia\n";

                            // ensure parent directory for the per-code csv exists (e.g. <cache>/xdxr/sh600)
                            if let Some(parent) = std::path::Path::new(&filename).parent() {
                                if let Err(e) = std::fs::create_dir_all(parent) {
                                    log::error!("Failed to create parent dir {:?}: {}", parent, e);
                                }
                            }

                            match std::fs::File::create(&tmp) {
                                Ok(mut f) => {
                                    if let Err(e) = f.write_all(header.as_bytes()) {
                                        log::error!("Failed to write header to tmp file {}: {}", tmp, e);
                                    } else {
                                        // write each row
                                        for v in resp.list.iter() {
                                            // format floating values with default Debug formatting
                                            let row = format!("{},{},{},{},{},{},{},{},{},{},{},{},{},{}\n",
                                                v.date,
                                                v.category,
                                                v.name,
                                                v.fenhong,
                                                v.peigu_jia,
                                                v.songzhuan,
                                                v.peigu,
                                                v.suogu,
                                                v.qian_liutong,
                                                v.hou_liutong,
                                                v.qian_zonggu,
                                                v.hou_zonggu,
                                                v.fenshu,
                                                v.xingquan_jia);
                                            if let Err(e) = f.write_all(row.as_bytes()) {
                                                log::error!("Failed to write row to tmp file {}: {}", tmp, e);
                                                break;
                                            }
                                        }
                                        if let Err(e) = f.flush() {
                                            log::error!("Failed to flush tmp file {}: {}", tmp, e);
                                        } else if let Err(e) = std::fs::rename(&tmp, &filename) {
                                            log::error!("Failed to rename {} -> {}: {}", tmp, filename, e);
                                        }
                                    }
                                }
                                Err(e) => {
                                    log::error!("Failed to create tmp file {}: {}", tmp, e);
                                }
                            }
                        }
                        None => {
                            log::warn!("No XDXR response for {} (fetch failed)", code);
                        }
                    }
                    pb.inc(1);
                }
                pb.finish_with_message("XDXR update completed");
            }
        }
        // calendar base handled above (do_calendar)
        // servers base handled above (do_servers)
    }

    Ok(true)
}
