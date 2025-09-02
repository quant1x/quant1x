#pragma once
#ifndef QUANT1X_Q1X_QUANT1X_H
#define QUANT1X_Q1X_QUANT1X_H 1

#include <utility>
#include <argparse/argparse.hpp>

namespace q1x::engine {

    /**
     * @brief 初始化, 接受一个回到函数
     * @tparam Callback
     * @param cb
     */
    template<typename Callback>
    void init(Callback&& cb) {
        std::forward<Callback>(cb)();
    }

    /**
     * @brief 守护进程入口
     * @param service_command
     */
    int daemon(const argparse::ArgumentParser& cmd);
}

#endif //QUANT1X_Q1X_QUANT1X_H
