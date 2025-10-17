#pragma once
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/base_sink.h>
#include <memory>
#include <vector>
#include <mutex>

namespace quant1x::log {

class FirstMatchRouterSink : public spdlog::sinks::base_sink<std::mutex> {
public:
    FirstMatchRouterSink() = default;
    ~FirstMatchRouterSink() override = default;

    // set optional console sink (not considered file duplication)
    void set_console_sink(const std::shared_ptr<spdlog::sinks::sink>& console);

    // add an exact-level route (checked in insertion order)
    void add_exact_route(spdlog::level::level_enum level, const std::shared_ptr<spdlog::sinks::sink>& sink);

    // optional fallback sink for unmatched levels
    void set_fallback_sink(const std::shared_ptr<spdlog::sinks::sink>& sink);

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override;
    void flush_() override;

private:
    std::vector<std::pair<spdlog::level::level_enum, std::shared_ptr<spdlog::sinks::sink>>> routes_;
    std::shared_ptr<spdlog::sinks::sink> fallback_;
    std::shared_ptr<spdlog::sinks::sink> console_sink_;
};

} // namespace quant1x::log
