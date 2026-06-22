#pragma once
#ifndef QUANT1X_APP_H
#define QUANT1X_APP_H 1

#include <utility>
#include <argparse/argparse.hpp>

namespace quant1x::app {

    /**
     * @brief 初始化, 接受一个回到函数
     * @tparam Callback
     * @param cb
     */
    template<typename Callback>
    void init(Callback&& cb) {
        std::forward<Callback>(cb)();
    }

    void init_datasource();

    /**
     * @brief 守护进程入口
     * @param cmd
     */
    // (TODO: API migration — quant1x.cpp masked, stub provided)
    int daemon(const argparse::ArgumentParser& cmd);
} // namespace quant1x::app

#endif // QUANT1X_APP_H
