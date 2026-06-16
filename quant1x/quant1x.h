#pragma once
#ifndef QUANT1X_Q1X_QUANT1X_H
#define QUANT1X_Q1X_QUANT1X_H 1

#include <utility>
#include <argparse/argparse.hpp>

namespace quant1x::engine {

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
     * @param cmd
     */
    // (TODO: API migration — quant1x.cpp masked, stub provided)
    inline int daemon(const argparse::ArgumentParser& cmd) {
        (void)cmd;
        return 1;
    }
} // namespace quant1x::engine

#endif //QUANT1X_Q1X_QUANT1X_H
