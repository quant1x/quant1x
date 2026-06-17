#pragma once
#ifndef QUANT1X_STD_CPU_INFO_H
#define QUANT1X_STD_CPU_INFO_H 1

#include <iostream>
#include <string>
#include <vector>
#ifdef _WIN32
#   include <windows.h>
#elif __linux__
#   include <fstream>
#   include <string>
#   include <set>
#elif __APPLE__
#   include <sys/sysctl.h>
#endif

namespace hw {
    // =============================
    // CPU 信息结构体 —— 可扩展设计
    // =============================
    struct cpu_info {
        int sockets;          // 物理 CPU 颗数(Socket 数量)
        int physical_cores;   // 物理核心总数(不含超线程)
        int logical_cores;    // 逻辑核心总数(含超线程)
        bool hyperthreading;  // 是否开启超线程(logical > physical 时为 true)

        // 🔧 扩展字段(预留, 可后续添加)
        std::string vendor;   // 厂商: Intel, AMD, Apple Silicon 等
        std::string model;    // 型号: 如 "Intel(R) Core(TM) i7-13700H"
        double frequency_ghz; // 最大睿频(GHz), 若无法获取则为 0.0

        // 构造函数(默认初始化)
        cpu_info()
            : sockets(0), physical_cores(0), logical_cores(0),
              hyperthreading(false), frequency_ghz(0.0) {}

        // 打印函数(方便调试)
        void print() const {
            std::cout << "=== CPU 信息 ===\n";
            std::cout << "物理 CPU 颗数: " << sockets << "\n";
            std::cout << "物理核心数: " << physical_cores << "\n";
            std::cout << "逻辑核心数: " << logical_cores << "\n";
            std::cout << "是否启用超线程: " << (hyperthreading ? "是" : "否") << "\n";
            if (!vendor.empty()) std::cout << "厂商: " << vendor << "\n";
            if (!model.empty()) std::cout << "型号: " << model << "\n";
            if (frequency_ghz > 0.0) std::cout << "最大频率: " << frequency_ghz << " GHz\n";
            std::cout << "================\n";
        }
    };

    cpu_info cpu_detect();
} // namespace hw

#endif //QUANT1X_STD_CPU_INFO_H