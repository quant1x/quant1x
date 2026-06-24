#pragma once
#ifndef QUANT1X_LOG_LOGGER_H
#define QUANT1X_LOG_LOGGER_H 1

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>

// 使用 spdlog 作为日志库
namespace logger = spdlog;


#endif // QUANT1X_LOG_LOGGER_H
