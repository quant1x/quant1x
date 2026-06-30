#include <quant1x/app.h>
#include <quant1x/std/api.h>
#include <argparse/argparse.hpp>
#include "data/meta/exchange.h"
#include <quant1x/runtime/service.h>
#include <spdlog/spdlog.h>
#include <quant1x/runtime/core.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/realtime/snapshot.h>
#include <quant1x/cache.h>
#include <quant1x/trader/tracker.h>
#include <quant1x/contrib/data/tdx/bar.h>
#include <quant1x/contrib/data/tdx/xdxr.h>
#include <quant1x/contrib/data/tdx/bar_raw.h>
#include <quant1x/contrib/data/tdx/bar.h>
#include <quant1x/contrib/data/tdx/minute.h>
#include <quant1x/contrib/data/tdx/chips.h>
#include <quant1x/contrib/data/tdx/trans.h>
#include <quant1x/contrib/data/tdx/bar_minute.h>
#include <quant1x/factors/f10.h>
#include <quant1x/factors/history.h>
#include <quant1x/pandas/rule.h>

namespace quant1x::app {
    using namespace quant1x::data;
    
    quant1x::config::MinuteKLineConfig get_minute_bar_config() {
        quant1x::config::MinuteKLineConfig config{};
        auto const &local_cfg = config::global_config().data.cache.kline;
        if (local_cfg.size() > 1) {
            throw std::runtime_error("kline config size must be exactly one");
        }
        if (local_cfg.empty()) {
            return config;
        }
        const auto minute_bar_config = local_cfg.begin();
        const auto key = minute_bar_config->first;
        const auto value = minute_bar_config->second;
        const auto d = pandas::parse_time_rule(key);
        const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(d);
        config.minutes = minutes.count();
        config.frequency = key;
        config.enabled = value;
        return config;
    }

    void init_datasource() {
       using namespace quant1x::contrib::data;
        // 基础数据
        // 除权除息
        data::Register(std::make_unique<tdx::DataXdxr>());
        // 日线 - 未除权
        data::Register(std::make_unique<tdx::DataKLineRaw>());
        // 日线 - 除权
        data::Register(std::make_unique<tdx::DataKLine>());
        // 分时数据
        data::Register(std::make_unique<tdx::DataMinute>());
        // 分笔成交
        data::Register(std::make_unique<tdx::DataTrans>());
        // 筹码分布
        data::Register(std::make_unique<tdx::DataChips>());
        // 分钟级别K线
        auto const &mkc = get_minute_bar_config();
        if (mkc.enabled) {
            data::Register(std::make_unique<tdx::DataMinuteKLine>());
        }

        // 特征数据
        // F10
        data::Register(std::make_unique<::F10Feature>());
        // 通用历史数据
        data::Register(std::make_unique<::HistoryFeature>());
    }

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
                meta::Timestamp now = meta::Timestamp::now();
                auto ts = meta::check_trading_timestamp(meta::Exchange::SSE, now);
                spdlog::info("realtime update: {}", ts.update_in_real_time);
                if(ts.update_in_real_time) {
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
                meta::Timestamp now = meta::Timestamp::now();
                auto ts = meta::check_trading_timestamp(meta::Exchange::SSE, now);
                spdlog::info("realtime trade status: {}", magic_enum::enum_name(ts.status));
                if((ts.status & 0) == 0) {
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
} // namespace quant1x::app