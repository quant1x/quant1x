// lazy_daily_sink.h
#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/sink.h>
#include <memory>
#include <string>

namespace quant1x::log {

class lazy_daily_sink : public spdlog::sinks::base_sink<std::mutex> {
public:
    lazy_daily_sink(std::string filename, int hour = 0, int minute = 0, bool truncate = false)
        : filename_(std::move(filename)), hour_(hour), minute_(minute), truncate_(truncate) {}

    ~lazy_daily_sink() override = default;

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override;
    void flush_() override;

private:
    std::string filename_;
    int hour_;
    int minute_;
    bool truncate_;
    // underlying sink will be created lazily on first write
    std::shared_ptr<spdlog::sinks::sink> sink_;
};

inline std::shared_ptr<spdlog::sinks::sink> make_lazy_daily_sink(const std::string &filename, int hour = 0,
                                                                  int minute = 0, bool truncate = false) {
    return std::make_shared<lazy_daily_sink>(filename, hour, minute, truncate);
}

} // namespace quant1x::log
