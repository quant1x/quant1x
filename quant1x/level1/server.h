#pragma once
#ifndef QUANT1X_LEVEL1_SERVER_H
#define QUANT1X_LEVEL1_SERVER_H 1

#include <quant1x/level1/encoding.h>
#include <quant1x/net/base.h>

namespace level1 {

    constexpr int     _max_connections  = 10;                                      ///< 最大连接数
    constexpr int64_t _max_elapsed_time = std::chrono::milliseconds(100).count();  ///< 最大连接耗时

    // 定义Server结构体
    struct Server {
        cista::offset::string Source;
        cista::offset::string Name;
        cista::offset::string Host;
        u16                   Port;
        i64                   CrossTime;
        // 定义 Cista 反射
        constexpr auto serialize() { return std::tie(Source, Name, Host, Port, CrossTime); }
    };

    // 标准行情服务器列表
    cista::offset::vector<Server> detect(i64 elapsed_time = _max_elapsed_time, int conn_limit = _max_connections, int connect_timeout_milliseconds = 1000);

}  // namespace level1

#endif  // QUANT1X_LEVEL1_SERVER_H
