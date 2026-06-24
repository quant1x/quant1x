#include <quant1x/test/test.h>
#include <quant1x/config/config.h>

#include "quant1x/runtime/core.h"
#include "quant1x/std/cpu_info.h"

namespace config = quant1x::config;

// 从YAML加载配置
config::TraderParameter LoadConfigFromYAML(const std::string& filename) {
    config::TraderParameter config;
    try {
        YAML::Node yaml = YAML::LoadFile(filename);
        config = yaml["trader"].as<config::TraderParameter>();
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML解析错误: " << e.what() << std::endl;
    }
    return config;
}

TEST_CASE("config-basic", "[config]") {
    const std::string& cfg_filename = config::config_filename();
    std::cout << cfg_filename << std::endl;
    config::TraderParameter traderParameter{};
    // 加载配置
    config::TraderParameter config = LoadConfigFromYAML(cfg_filename);
    traderParameter = config;
    // 打印配置
    std::cout << traderParameter << std::endl;
}

TEST_CASE("config-base", "[config]") {
    // 加载配置
    auto const & config = config::TraderConfig();

    // 打印配置
    std::cout << *config << std::endl;
}

TEST_CASE("config-data", "[config]") {
    runtime::global_init();
    // 加载配置
    auto const & config = config::global_config();

    // 打印配置
    std::cout << config << std::endl;
}

#include <iostream>

#ifdef _WIN32
#   include <windows.h>
#elif __linux__
#   include <fstream>
#   include <string>
#   include <set>
#elif __APPLE__
#   include <sys/sysctl.h>
#endif

int getPhysicalCPUCount() {
#ifdef _WIN32
    DWORD buffer_size = 0;
    GetLogicalProcessorInformationEx(RelationProcessorPackage, nullptr, &buffer_size);

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return 1;
    }

    std::vector<char> buffer(buffer_size);
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info =
        reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data());

    if (!GetLogicalProcessorInformationEx(RelationProcessorPackage, info, &buffer_size)) {
        return 1;
    }

    int socket_count = 0;
    DWORD offset = 0;
    while (offset < buffer_size) {
        if (info->Relationship == RelationProcessorPackage) {
            socket_count++;
        }
        offset += info->Size;
        info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
            reinterpret_cast<char*>(info) + info->Size);
    }
    return socket_count;

#elif __linux__
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    std::set<int> physical_ids;

    while (std::getline(cpuinfo, line)) {
        if (line.find("physical id") != std::string::npos) {
            int id = std::stoi(line.substr(line.find_last_of(':') + 1));
            physical_ids.insert(id);
        }
    }
    return physical_ids.size();

#elif __APPLE__
    int numSockets = 0;
    size_t len = sizeof(numSockets);
    if (sysctlbyname("hw.packages", &numSockets, &len, nullptr, 0) == -1) {
        return 1;
    }
    return numSockets;

#else
    return 1; // 不支持平台, 默认1颗
#endif
}

TEST_CASE("devices-v1", "[config]") {
    int cpu_sockets = getPhysicalCPUCount();
    std::cout << "物理 CPU 颗数: " << cpu_sockets << std::endl;

    // 如果你也想知道总核心数, 可以一起输出: 
    unsigned int total_cores = std::thread::hardware_concurrency();
    if (total_cores == 0) total_cores = 1;
    std::cout << "总逻辑核心数: " << total_cores << std::endl;
}

TEST_CASE("cpu-info", "[config]") {
    hw::cpu_info cpu = hw::cpu_detect();
    cpu.print(); // 自动格式化输出

    // 你也可以单独访问字段
    std::cout << "\n程序中使用: \n";
    std::cout << "创建线程池大小建议: " << cpu.physical_cores << "(推荐用物理核)\n";
}
