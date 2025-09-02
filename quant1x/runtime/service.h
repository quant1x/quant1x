#pragma once
#ifndef API_OS_SERVICE_H
#define API_OS_SERVICE_H 1

#include <string>

namespace service {

    // 服务配置
    struct ServiceConfig {
        std::string service_name = "Quant1X-Q2X";
        std::string display_name = "Quant1X V2.0(C++)";
        std::string description  = "Quant1X C++版本的数据服务";
    };

    // 安装服务
    void install();
    // 卸载服务
    void uninstall();
    // 启动服务
    void start();
    // 关闭服务
    void stop();
    // 查询状态
    void query_status();
    // 运行服务
    void run_daemon();
}

#endif //API_OS_SERVICE_H
