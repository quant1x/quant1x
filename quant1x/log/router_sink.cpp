#include "quant1x/log/router_sink.h"

namespace quant1x::log {

void FirstMatchRouterSink::set_console_sink(const std::shared_ptr<spdlog::sinks::sink>& console) {
    console_sink_ = console;
}

void FirstMatchRouterSink::add_exact_route(spdlog::level::level_enum level, const std::shared_ptr<spdlog::sinks::sink>& sink) {
    routes_.emplace_back(level, sink);
}

void FirstMatchRouterSink::set_fallback_sink(const std::shared_ptr<spdlog::sinks::sink>& sink) {
    fallback_ = sink;
}

void FirstMatchRouterSink::sink_it_(const spdlog::details::log_msg &msg) {
    if (console_sink_) console_sink_->log(msg);
    for (auto &rt : routes_) {
        if (rt.first == msg.level) {
            rt.second->log(msg);
            return;
        }
    }
    if (fallback_) fallback_->log(msg);
}

void FirstMatchRouterSink::flush_() {
    if (console_sink_) console_sink_->flush();
    for (auto &rt : routes_) rt.second->flush();
    if (fallback_) fallback_->flush();
}

} // namespace quant1x::log
