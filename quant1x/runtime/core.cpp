#include <quant1x/runtime/core.h>

#include <atomic>
#include <csignal>
#include <iostream>
#include <cstdlib>
#include <mutex>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

#include <filesystem>

#include <quant1x/base/filesystem.h>
#include <quant1x/runtime/crash.h>
#include <quant1x/runtime/scheduler.h>
#include <quant1x/base/except.h>
// router sink for per-level file routing
#include <quant1x/log/router_sink.h>
// lazy daily sink wrapper (creates files on first write)
#include <quant1x/log/lazy_daily_sink.h>

// rotating file sink
#include <spdlog/sinks/rotating_file_sink.h>
// daily file sink
#include <spdlog/sinks/daily_file_sink.h>

namespace runtime {
    std::atomic<bool> global_quit_flag(false);  // 全局退出标志
    std::atomic<bool> global_wait_flag(false);

    static void shutdown();

    void SetQuitFlag(bool flag) {
        global_quit_flag = flag;
    }

#ifdef _WIN32
    // Windows控制台事件处理函数
    BOOL WINAPI ConsoleHandler(DWORD event) {
        BOOL result = FALSE;
        switch (event) {
            case CTRL_C_EVENT:  // 必选事件: 用户按下 Ctrl+C. 
                spdlog::info("signal> Ctrl+C pressed. Exiting...");
                global_quit_flag = true;
                result           = TRUE;
                break;
            case CTRL_CLOSE_EVENT:  // 必选事件: 用户点击控制台窗口的关闭按钮(❌)
                spdlog::info("signal> Console closed. Saving state...");
                global_quit_flag = true;
                result           = TRUE;
                break;
            case CTRL_SHUTDOWN_EVENT:  // 必选事件: 系统即将关机或重启
                spdlog::info("signal> System shutting down. Cleaning up...");
                global_quit_flag = true;
                result           = TRUE;
                break;
            case CTRL_BREAK_EVENT:  // 可选事件: 用户按下 Ctrl+Break(或程序调用 GenerateConsoleCtrlEvent)
                spdlog::info("signal> Ctrl+Break pressed.");
                global_quit_flag = true;
                result           = TRUE;  // 不退出, 仅记录
                break;
            case CTRL_LOGOFF_EVENT:  // 可选事件: 用户注销(Logoff)或切换账户
                spdlog::info("signal> User logging off.");
                global_quit_flag = true;
                result           = TRUE;
                break;
            default:
                result = FALSE;
                break;
        }
        if (!global_wait_flag.load() && result) {
            shutdown();
        }
        return result;
    }

#else
    // Unix信号处理函数
    void SignalHandler(int signum) {
        (void)signum;
        spdlog::warn("signal: {}", signum);
        global_quit_flag.store(true);
        if (!global_wait_flag.load() && global_quit_flag) {
            shutdown();
        }
    }
#endif

    // 设置信号/事件处理函数
    void SetupSignalHandlers() {
#ifdef _WIN32
        SetConsoleCtrlHandler(ConsoleHandler, TRUE);
#else
        struct sigaction sa;
        sa.sa_handler = SignalHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        // 注册SIGINT(Ctrl+C)和SIGTERM(kill默认信号)
        sigaction(SIGINT, &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);
        sigaction(SIGHUP, &sa, nullptr);
#endif
    }

    // 防止在 terminate 处理器中递归/重入, 同时尽可能在不可恢复的上下文中
    // 做最小且不抛异常的日志 flush 与退出. 
    static std::atomic_flag terminate_in_progress = ATOMIC_FLAG_INIT;

    // 最小化, 安全的清理: 尽力刷新并关闭 spdlog, 但不调用可能抛或阻塞的高层代码. 
    static void safe_flush_and_exit(int exit_code) noexcept {
        // best-effort: flush default logger
        try {
            if (spdlog::default_logger()) {
                spdlog::default_logger()->flush();
            }
        } catch (...) {
            // 忽略所有错误(terminate 上下文不能抛)
        }
        // best-effort: shutdown spdlog resources
        try {
            spdlog::shutdown();
        } catch (...) {
        }
        std::_Exit(exit_code);
    }

    static void global_terminate_handler() noexcept {
        // 如果已在处理 terminate, 则直接快速退出, 避免死循环
        if (terminate_in_progress.test_and_set()) {
            std::fprintf(stderr, "terminate called recursively, aborting\n");
            std::_Exit(EXIT_FAILURE);
        }

        auto ex = std::current_exception();
        if (ex) {
            try {
                std::rethrow_exception(ex);
            } catch (const BaseException &e) {
                try {
                    if (spdlog::default_logger()) {
                        spdlog::error("Uncaught BaseException: file={} line={} msg={}",
                                      e.getFile(), e.getLine(), e.what());
                    } else {
                        std::fprintf(stderr,
                                     "Uncaught BaseException: file=%s line=%d msg=%s\n",
                                     e.getFile().c_str(), e.getLine(), e.what());
                    }
                } catch (...) {
                    std::fprintf(stderr,
                                 "Uncaught BaseException: file=%s line=%d msg=%s\n",
                                 e.getFile().c_str(), e.getLine(), e.what());
                }
            } catch (const std::exception &e) {
                try {
                    if (spdlog::default_logger()) {
                        spdlog::error("Uncaught std::exception: {}", e.what());
                    } else {
                        std::fprintf(stderr, "Uncaught std::exception: %s\n", e.what());
                    }
                } catch (...) {
                    std::fprintf(stderr, "Uncaught std::exception: %s\n", e.what());
                }
            } catch (...) {
                try {
                    if (spdlog::default_logger()) {
                        spdlog::error("Uncaught unknown exception");
                    } else {
                        std::fprintf(stderr, "Uncaught unknown exception\n");
                    }
                } catch (...) {
                    std::fprintf(stderr, "Uncaught unknown exception\n");
                }
            }
        } else {
            try {
                if (spdlog::default_logger()) {
                    spdlog::error("terminate called without active exception");
                } else {
                    std::fprintf(stderr, "terminate called without active exception\n");
                }
            } catch (...) {
                std::fprintf(stderr, "terminate called without active exception\n");
            }
        }

        // 尽力把日志写入磁盘并退出；使用非零退出码表示异常终止
        safe_flush_and_exit(EXIT_FAILURE);
    }

    /// 隐藏全局初始化函数
    namespace {
        // 全局调度器, 智能指针动态分配
        // static std::unique_ptr<AsyncScheduler> global_scheduler = nullptr;
        AsyncScheduler *global_scheduler() {
            static AsyncScheduler scheduler;
            return &scheduler;
        }

        // 注册全部组件
        void init_all_components() {
            // make sure logs directory exists
            try {
                std::filesystem::create_directories(quant1x::config::get_logs_path());
            } catch (...) {
                // best-effort; fall back to letting spdlog fail if path unusable
            }

            // Build a first-match router that writes each level into its own daily file
            auto router = std::make_shared<quant1x::log::FirstMatchRouterSink>();

            // use lazy daily files (rotate every day) — keep false for truncate=false
            using quant1x::log::make_lazy_daily_sink;
            auto info_sink = make_lazy_daily_sink(quant1x::config::get_logs_path() + "/info.log", 0, 0, false);
            auto debug_sink = make_lazy_daily_sink(quant1x::config::get_logs_path() + "/debug.log", 0, 0, false);
            auto warn_sink = make_lazy_daily_sink(quant1x::config::get_logs_path() + "/warn.log", 0, 0, false);
            auto err_sink = make_lazy_daily_sink(quant1x::config::get_logs_path() + "/error.log", 0, 0, false);
            auto critical_sink = make_lazy_daily_sink(quant1x::config::get_logs_path() + "/critical.log", 0, 0, false);
            auto trace_sink = make_lazy_daily_sink(quant1x::config::get_logs_path() + "/trace.log", 0, 0, false);

            router->add_exact_route(spdlog::level::info, info_sink);
            router->add_exact_route(spdlog::level::debug, debug_sink);
            router->add_exact_route(spdlog::level::warn, warn_sink);
            router->add_exact_route(spdlog::level::err, err_sink);
            router->add_exact_route(spdlog::level::critical, critical_sink);
            router->set_fallback_sink(trace_sink);

            std::string application_name = filesystem::executable_name();
            auto        combined_logger  = std::make_shared<spdlog::logger>(application_name, router);
            // default to INFO level; logger_set(debug=true) will raise it to DEBUG
            combined_logger->set_level(spdlog::level::info);
            // register as default logger
            spdlog::set_default_logger(combined_logger);

            // 现在可以直接使用 spdlog::info(), spdlog::error() 等
            spdlog::info("quant1x init");
            std::atexit(shutdown);
            std::set_terminate(global_terminate_handler);
            // 每3秒自动刷新一次(单位: 秒)
            spdlog::flush_every(std::chrono::seconds(3));
            console_set_utf8();
            SetupSignalHandlers();
        }
    }  // namespace

    // 懒加载标志
    std::once_flag global_task_once;

    void console_set_utf8(void) {
#ifdef _WIN32
        // 设置控制台输出和输入代码页为UTF-8
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        // std::locale::global(std::locale(".65001"));
#endif
    }

    /// 全局初始化, 注册退出清理函数
    void global_init() {
        std::call_once(global_task_once, [&] {
            init_all_components();
            crash::InitCrashHandler();
        });
    }

    // 设置日志模块, debug模式及控制台显示
    void logger_set(bool verbose, bool debug) {
        global_init();
        if (verbose) {
            std::vector<spdlog::sink_ptr> &tmp_sinks = spdlog::default_logger()->sinks();
            // 创建控制台 sink
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            tmp_sinks.push_back(console_sink);
            // logger = spdlog::logger("multi", sinks);
        }
        if (debug) {
            spdlog::set_level(spdlog::level::debug);
        }
    }

    // 追加一个任务到全局任务调度器
    task_id add_task(const std::string &name, const std::string &cron_expr, std::function<void()> task) {
        global_init();
        auto id = global_scheduler()->schedule_cron(name, cron_expr, std::move(task));
        return id;
    }

    // 取消一个任务
    void cancel_task(task_id id) {
        global_init();
        global_scheduler()->cancel(id);
    }

    // 一般性退出, 包括正常退出和异常
    void shutdown() {
        global_init();
        spdlog::info("刷新日志");
        spdlog::default_logger()->flush();
        spdlog::shutdown();
        _exit(0);
        // std::exit(0);
    }

    // 等待结束信号, 守护进程使用
    void wait_for_exit() {
        global_init();
        global_wait_flag = true;
        while (!global_quit_flag.load()) {
            // spdlog::warn("wait for exit...");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        shutdown();
    }
}  // namespace runtime
