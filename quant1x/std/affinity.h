#pragma once
#ifndef QUANT1X_STD_AFFINITY_H
#define QUANT1X_STD_AFFINITY_H 1

#include "base.h"
#include <system_error>
#include <thread>

// CPU亲和性相关接口
namespace affinity {

    // 绑定当前线程到最优CPU（循环分配避免热点）
    bool bind_current_thread_to_optimal_cpu(std::error_code &ec);

    // 绑定当前线程到指定CPU
    bool bind_current_thread_to_cpu(unsigned cpu_index, std::error_code &ec);

    // 绑定指定线程到指定CPU
    bool bind_thread_to_cpu(std::thread &thread, unsigned cpu_index, std::error_code &ec);

    // 绑定指定线程到最优CPU
    bool bind_thread_to_optimal_cpu(std::thread &thread, std::error_code &ec);

    // 获取当前线程运行的CPU ID
    unsigned get_current_cpu_id(std::error_code &ec);

} // namespace affinity

#endif // QUANT1X_STD_AFFINITY_H