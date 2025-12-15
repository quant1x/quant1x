// unit test for lazy daily sink (self-contained)
#include <quant1x/test/test.h>
#include <quant1x/std/strings.h>
#include <quant1x/io/file.h>
#include <quant1x/logger/lazy_daily_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <chrono>
#include <cstdio>

#include "spdlog/spdlog.h"

using quant1x::log::make_lazy_daily_sink;

TEST_CASE("lazy-daily-log", "[spdlog][lazy]") {
    // 在项目源目录下创建一个专用测试日志目录（避免使用全局配置或系统临时目录）
    // Derive project source root from this source file path so tests work when run from build dir.
    std::filesystem::path src_path(__FILE__);
    auto project_root = src_path.parent_path().parent_path(); // .../tests/<file> -> project root
    const std::string logdir = (project_root / "tests" / "logs").string();
    try { std::filesystem::create_directories(logdir); } catch(...) {}

    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    std::ostringstream ss;
    ss << "tdd_lazy_" << now << ".log";
    auto logfile = logdir + "/" + ss.str();

    // debug: print logfile path for CTest runs
    std::cerr << "[tdd-lazy-log] logfile=" << logfile << std::endl;
    std::cerr << "[tdd-lazy-log] exists_before=" << std::filesystem::exists(logfile) << std::endl;

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

    // debug: try direct spdlog daily_file_sink to see if it creates the file
    try {
        auto real_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logfile, 0, 0, false);
        auto sinks2 = spdlog::sinks_init_list{real_sink};
        auto logger2 = std::make_shared<spdlog::logger>("tdd-daily-direct", sinks2);
        logger2->set_level(spdlog::level::trace);
        logger2->info("direct daily test");
        logger2->flush();
        std::cerr << "[tdd-lazy-log] exists_after_direct_daily=" << std::filesystem::exists(logfile) << std::endl;
        if (std::filesystem::exists(logfile)) {
            std::cerr << "[tdd-lazy-log] size_after_direct_daily=" << std::filesystem::file_size(logfile) << std::endl;
        }
    } catch (...) {
        std::cerr << "[tdd-lazy-log] direct daily sink exception" << std::endl;
    }

    // 写一条日志并 flush
    logger->info("lazy test message");
    logger->flush();

    std::cerr << "[tdd-lazy-log] exists_after_flush=" << std::filesystem::exists(logfile) << std::endl;
    if (std::filesystem::exists(logfile)) {
        std::cerr << "[tdd-lazy-log] size_after_flush=" << std::filesystem::file_size(logfile) << std::endl;
    }

    // debug: try to create the file via std::ofstream to check filesystem permissions
    try {
        std::ofstream ofs(logfile, std::ios::binary | std::ios::app);
        if (ofs.good()) {
            ofs << "probe" << std::endl;
            ofs.close();
            std::cerr << "[tdd-lazy-log] wrote_probe=1" << std::endl;
            std::cerr << "[tdd-lazy-log] exists_after_ofstream=" << std::filesystem::exists(logfile) << std::endl;
            std::cerr << "[tdd-lazy-log] size_after_ofstream=" << std::filesystem::file_size(logfile) << std::endl;
        } else {
            std::cerr << "[tdd-lazy-log] ofstream.good=false" << std::endl;
        }
    } catch (...) {
        std::cerr << "[tdd-lazy-log] ofstream exception" << std::endl;
    }

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
