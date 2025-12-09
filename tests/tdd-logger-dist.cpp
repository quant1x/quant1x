#include <quant1x/test/test.h>
#include <quant1x/std/strings.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/dist_sink.h>
#include <filesystem>

#include "spdlog/spdlog.h"
// include reusable router sink
#include "quant1x/logger/router_sink.h"

using namespace std::literals;

static std::shared_ptr<spdlog::sinks::sink> make_spdlog_sink_local(const std::string& name) {
    std::string level_name = "logs";
    level_name.append("/");
    level_name.append(strings::trim(name));
    level_name.append(".log");
    return std::make_shared<spdlog::sinks::daily_file_sink_mt>(level_name, 0, 0, false);
}

// Use the shared FirstMatchRouterSink from quant1x::log (defined in quant1x/log/router_sink.h/.cpp)

TEST_CASE("logger-first-match-router", "[spdlog]") {
    namespace fs = std::filesystem;
    try { fs::create_directories("logs"); } catch (...) {}

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_info = make_spdlog_sink_local("info");
    auto file_err = make_spdlog_sink_local("error");
    auto file_debug = make_spdlog_sink_local("debug");
    auto file_trace = make_spdlog_sink_local("trace");

    // use reusable router sink from quant1x::log
    auto router = std::make_shared<quant1x::log::FirstMatchRouterSink>();
    router->set_console_sink(console_sink);
    router->add_exact_route(spdlog::level::info, file_info);
    router->add_exact_route(spdlog::level::err, file_err);
    router->add_exact_route(spdlog::level::debug, file_debug);
    // add warn & critical as separate files per request
    auto file_warn = make_spdlog_sink_local("warn");
    auto file_critical = make_spdlog_sink_local("critical");
    router->add_exact_route(spdlog::level::warn, file_warn);
    router->add_exact_route(spdlog::level::critical, file_critical);

    // fallback: capture any unmatched levels to trace.log
    router->set_fallback_sink(file_trace);

    auto logger = std::make_shared<spdlog::logger>("router_logger_separate", router);
    logger->set_level(spdlog::level::trace);

    logger->trace("[router] Trace");
    logger->debug("[router] Debug");
    logger->info("[router] Info");
    logger->warn("[router] Warn");
    logger->error("[router] Error");
    logger->critical("[router] Critical");

    logger->flush();
    spdlog::shutdown();
}
