#pragma once
#ifndef API_CRASH_HANDLER_H
#define API_CRASH_HANDLER_H 1

#include <string>
#include <functional>

namespace crash {

    const char * const crash_logger_name = "crash_handler";

    // 初始化全局崩溃处理器
    void InitCrashHandler();

} // namespace crash

#endif //API_CRASH_HANDLER_H
