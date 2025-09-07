#include <quant1x/test/test.h>
#include <quant1x/std/strings.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <quant1x/io/file.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

auto make_spdlog_sink(const std::string& name) {
    std::string level_name = "logs";
    level_name.append("/");
    level_name.append(strings::trim(name));
    level_name.append(".log");
    return std::make_shared<spdlog::sinks::daily_file_sink_mt>(
        level_name, 0, 0, false);
}

class HybridRouterSink : public spdlog::sinks::base_sink<std::mutex> {
public:

    // 添加控制台输出（新增）
    void add_console_sink(std::shared_ptr<spdlog::sinks::sink> console_sink) {
        console_sink_ = console_sink;
    }

    // 添加精确路由规则（仅接受严格等于该级别的日志）
    void add_exact_route(spdlog::level::level_enum level,
                         std::shared_ptr<spdlog::sinks::sink> sink) {
        exact_routes_[level] = sink;
    }

    // 设置默认降级目标（必须调用）
    void set_fallback_sink(std::shared_ptr<spdlog::sinks::sink> sink) {
        fallback_sink_ = sink;
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        // 0. 始终输出到控制台（新增）
        if (console_sink_) {
            console_sink_->log(msg);
        }

        // 1. 精确匹配文件路由
        auto exact_it = exact_routes_.find(msg.level);
        if (exact_it != exact_routes_.end()) {
            exact_it->second->log(msg);
            return;
        }

        // 2. 默认降级文件
        if (fallback_sink_) {
            fallback_sink_->log(msg);
        }
    }

    void flush_() override {
        for (auto& [level, sink] : exact_routes_) {
            sink->flush();
        }
        if (fallback_sink_) fallback_sink_->flush();
    }

private:
    std::map<spdlog::level::level_enum, std::shared_ptr<spdlog::sinks::sink>> exact_routes_;
    std::shared_ptr<spdlog::sinks::sink> fallback_sink_;
    std::shared_ptr<spdlog::sinks::sink> console_sink_; // 新增
};


TEST_CASE("logger-filter", "[spdlog]") {
#ifdef _WIN32
    // 设置控制台输出和输入代码页为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    //std::locale::global(std::locale(".65001"));
#endif
    // 1. 创建混合路由Sink
    auto router = std::make_shared<HybridRouterSink>();

    // 2. 配置精确路由（仅接受严格等于该级别的日志）
    router->add_exact_route(  // info级别严格匹配
        spdlog::level::info,
        make_spdlog_sink("info")
    );
    router->add_exact_route(  // error级别严格匹配
        spdlog::level::err,
        make_spdlog_sink("error")
    );
    router->add_exact_route(  // debug级别严格匹配
        spdlog::level::debug,
        make_spdlog_sink("debug")
    );

    router->add_console_sink(
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
    );

    // 3. 设置默认降级目标（未配置的级别都进这里）
    router->set_fallback_sink(
        make_spdlog_sink("trace")
    );

    // 4. 创建Logger并设置全局级别过滤
    std::string application_name = io::executable_name();
    auto logger = std::make_shared<spdlog::logger>(application_name, router);
    logger->set_level(spdlog::level::trace);  // 原生>=过滤：接收所有级别

    // 5. 测试日志路由
    logger->trace("1. Trace -> trace.log (fallback)");    // 未配置 → trace.log
    logger->debug("2. Debug -> trace.log (fallback)");   // 未配置 → trace.log
    logger->info("3. Info -> info.log (exact match)");    // 精确匹配info.log
    logger->warn("4. Warn -> trace.log (fallback)");      // 未配置 → trace.log
    logger->error("5. Error -> errors.log (exact match)"); // 精确匹配errors.log
    logger->critical("6. Critical -> trace.log");        // 未配置 → trace.log

    logger->flush();
    spdlog::shutdown();
    std::cout << "测试中文" << std::endl;
}