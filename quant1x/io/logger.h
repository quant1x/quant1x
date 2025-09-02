#pragma once
#ifndef API_LOGGER_H
#define API_LOGGER_H 1

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include <filesystem>
#include <memory>

//class RenamingDailySink : public spdlog::sinks::base_sink<std::mutex> {
//public:
//    RenamingDailySink(const std::string& base_filename, int rotation_hour, int rotation_minute)
//            : base_filename_(base_filename),
//              rotation_hour_(rotation_hour),
//              rotation_minute_(rotation_minute) {
//        spdlog::flush_every(std::chrono::seconds(3));
//        // 初始创建sink
//        create_sink();
//    }
//
//protected:
//    void sink_it_(const spdlog::details::log_msg& msg) override {
//        current_sink_->log(msg);
//    }
//
//    void flush_() override {
//        current_sink_->flush();
//    }
//
//    void rotate_() {
//        // 1. 获取当前文件名
//        std::string current_file = base_filename_;
//
//        // 2. 计算带日期的新文件名
//        auto now = std::chrono::system_clock::now();
//        std::time_t t = std::chrono::system_clock::to_time_t(now);
//        std::tm tm = *std::localtime(&t);
//
//        char buffer[80];
//        strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm);
//        std::string new_file = fmt::format("{}_{}.log",
//                                           base_filename_.substr(0, base_filename_.find_last_of('.')),
//                                           buffer);
//
//        // 3. 关闭当前sink以释放文件句柄
//        current_sink_.reset();
//
//        // 4. 重命名旧文件
//        if (std::filesystem::exists(current_file)) {
//            std::filesystem::rename(current_file, new_file);
//        }
//
//        // 5. 创建新的sink
//        create_sink();
//    }
//
//private:
//    void create_sink() {
//        current_sink_ = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
//                base_filename_, rotation_hour_, rotation_minute_);
//    }
//
//    std::string base_filename_;
//    int rotation_hour_;
//    int rotation_minute_;
//    std::shared_ptr<spdlog::sinks::daily_file_sink_mt> current_sink_;
//};


#endif //API_LOGGER_H
