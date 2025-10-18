// lazy_daily_sink.cpp
#include "quant1x/log/lazy_daily_sink.h"
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/details/os.h>

namespace quant1x::log {

void lazy_daily_sink::sink_it_(const spdlog::details::log_msg &msg) {
    // create underlying sink lazily; base_sink already holds mutex_ when calling sink_it_
    if (!sink_) {
        // construct the real spdlog daily file sink now
        sink_ = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filename_, hour_, minute_, truncate_);
    }
    // forward message to underlying sink
    if (sink_) {
        sink_->log(msg);
    }
}

void lazy_daily_sink::flush_() {
    if (sink_) {
        sink_->flush();
    }
}

} // namespace quant1x::log
