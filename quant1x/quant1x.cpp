#include <quant1x/quant1x.h>
#include <quant1x/std/api.h>
#include <argparse/argparse.hpp>
#include <quant1x/runtime/service.h>
#include <spdlog/spdlog.h>
#include <quant1x/runtime/core.h>
#include <quant1x/exchange/timestamp.h>
#include <quant1x/realtime/snapshot.h>
#include <quant1x/cache.h>
#include <quant1x/trader/tracker.h>

namespace quant1x::engine {

    int daemon(const argparse::ArgumentParser& cmd) {
        auto action = cmd.get<std::string>("action");
        spdlog::warn("service: {}", action);
#if OS_IS_WINDOWS
        std::ofstream pipeFile;
        if(cmd.is_used("--pipe")) {
            auto pipe = cmd.get<std::string>("--pipe");
            spdlog::warn("service runas: [{}]", pipe);
            if (!pipe.empty()) {
                pipeFile.open(pipe, std::ios::binary);
                if (!pipeFile) {
                    spdlog::error("无法打开输出文件[{}]", pipe);
                    return 1;
                }
                std::cout.rdbuf(pipeFile.rdbuf()); // 重定向stdout到文件
                if (std::cout.rdbuf() != pipeFile.rdbuf()) {
                    spdlog::error("重定向 stdout 失败");
                }
                std::cerr.rdbuf(pipeFile.rdbuf()); // 重定向stderr到文件
                if (std::cerr.rdbuf() != pipeFile.rdbuf()) {
                    spdlog::error("重定向 stdout 失败");
                }
            }
        }
#endif
        if (action == "install") {
            service::install();
        } else if (action == "uninstall") {
            service::uninstall();
        } else if (action == "start") {
            service::start();
        } else if (action == "stop") {
            service::stop();
        } else if (action == "status") {
            service::query_status();
        } else if (action == "run") {
            spdlog::info("进入服务运行");
            // 盘中快照
            auto task_snapshot = runtime::add_task("realtime-snapshot", "*/1 * 9-15 * * ?", [] {
                exchange::timestamp now = exchange::timestamp::now();
                auto ts = exchange::check_trading_timestamp(now);
                spdlog::info("realtime update: {}", ts.updateInRealTime);
                if(ts.updateInRealTime) {
                    realtime::sync_snapshots();
                }
            });
            spdlog::info("realtime-snapshot, task id={}", task_snapshot);

            // 日常更新数据
            auto task_update = runtime::add_task("update-all", "*/1 * 15-22 * * ?", [] {
                cache::update_all();
            });
            spdlog::info("data-update, task id={}", task_update);

            // 盘中交易
            auto task_trader = runtime::add_task("realtime-trader", "*/1 * 9-15 * * ?", [] {
                exchange::timestamp now = exchange::timestamp::now();
                auto ts = exchange::check_trading_timestamp(now);
                spdlog::info("realtime trade status: {}", magic_enum::enum_name(ts.status));
                if((ts.status & exchange::MaskOrder) == exchange::MaskOrder) {
                    trader::tracker();
                }
            });
            spdlog::info("task_trader, task id={}", task_trader);

            service::run_daemon();
        }
#if OS_IS_WINDOWS
        if(pipeFile.is_open()) {
            pipeFile.flush();
            pipeFile.close();
        }
#endif
        spdlog::default_logger()->flush();
        return 0;
    }
} // namespace quant1x::engine