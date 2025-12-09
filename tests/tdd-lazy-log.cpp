// unit test for lazy daily sink (self-contained)
#include <quant1x/test/test.h>
#include <quant1x/std/strings.h>
#include <quant1x/io/file.h>
#include <quant1x/logger/lazy_daily_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <chrono>
#include <cstdio>

#include "spdlog/spdlog.h"

using quant1x::log::make_lazy_daily_sink;

TEST_CASE("lazy-daily-log", "[spdlog][lazy]") {
    // 在项目目录下创建一个专用测试日志目录（避免使用全局配置或系统临时目录）
    const std::string logdir = "tests/logs";
    try { std::filesystem::create_directories(logdir); } catch(...) {}

    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    std::ostringstream ss;
    ss << "tdd_lazy_" << now << ".log";
    auto logfile = logdir + "/" + ss.str();

    // 确保不存在残留文件
    std::remove(logfile.c_str());

    // 局部 lazy sink + 局部 logger（不影响全局 spdlog 状态）
    auto lazy = make_lazy_daily_sink(logfile, 0, 0, false);
    auto sinks = spdlog::sinks_init_list{lazy};
    auto logger = std::make_shared<spdlog::logger>("tdd-lazy-local", sinks);
    logger->set_level(spdlog::level::trace);

    // 写入前文件不应存在
    {
        std::ifstream ifs(logfile, std::ios::binary);
        REQUIRE_FALSE(ifs.good());
    }

    // 写一条日志并 flush
    logger->info("lazy test message");
    logger->flush();

    // 释放局部 logger
    logger.reset();

    // 写入后文件应存在且非空
    {
        std::ifstream ifs(logfile, std::ios::binary | std::ios::ate);
        REQUIRE(ifs.good());
        auto sz = ifs.tellg();
        REQUIRE(sz > 0);
    }

    // 清理：删除日志文件（保留 logs 目录）
    std::remove(logfile.c_str());
}
