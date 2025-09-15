#include "cpu_info.h"

#include <thread>

namespace hw {
    // =============================
    // 获取 CPU 信息（主函数）
    // =============================
    cpu_info cpu_detect() {
        cpu_info info;

        // --- 获取逻辑核心数（通用） ---
        unsigned int lc = std::thread::hardware_concurrency();
        info.logical_cores = (lc == 0) ? 1 : static_cast<int>(lc);

        // --- 获取物理核心数 ---
#ifdef _WIN32
        DWORD buffer_size = 0;
        GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &buffer_size);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            info.physical_cores = 1;
        } else {
            std::vector<char> buffer(buffer_size);
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info_ex =
                reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data());
            if (GetLogicalProcessorInformationEx(RelationProcessorCore, info_ex, &buffer_size)) {
                DWORD offset = 0;
                while (offset < buffer_size) {
                    if (info_ex->Relationship == RelationProcessorCore) {
                        info.physical_cores++;
                    }
                    offset += info_ex->Size;
                    info_ex = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
                        reinterpret_cast<char*>(info_ex) + info_ex->Size);
                }
            } else {
                info.physical_cores = 1;
            }
        }

        // Windows 获取 CPU 厂商和型号（通过 CPUID，简化版）
        // 更精确需调用 RDTSCP / CPUID 指令，此处使用 WMI 或注册表略复杂
        // 简化：仅设置常见厂商
        info.vendor = "Intel/AMD"; // 实际可通过 WMI 获取详细型号
        info.model = "Windows CPU"; // 可选：从注册表 HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor\0 获取

#elif __linux__
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        std::set<std::pair<int, int>> cores; // (physical_id, core_id)
        int physical_id = -1, core_id = -1;
        std::string vendor_str = "", model_str = "";

        while (std::getline(cpuinfo, line)) {
            if (line.find("physical id") != std::string::npos) {
                physical_id = std::stoi(line.substr(line.find_last_of(':') + 1));
            } else if (line.find("core id") != std::string::npos) {
                core_id = std::stoi(line.substr(line.find_last_of(':') + 1));
                cores.insert({physical_id, core_id});
            } else if (line.find("vendor_id") != std::string::npos) {
                vendor_str = line.substr(line.find_first_of(':') + 1);
                vendor_str.erase(0, vendor_str.find_first_not_of(" \t")); // trim
                vendor_str.erase(vendor_str.find_last_not_of(" \t") + 1);
            } else if (line.find("model name") != std::string::npos) {
                model_str = line.substr(line.find_first_of(':') + 1);
                model_str.erase(0, model_str.find_first_not_of(" \t"));
                model_str.erase(model_str.find_last_not_of(" \t") + 1);
            }
        }

        info.physical_cores = static_cast<int>(cores.size());
        info.vendor = vendor_str;
        info.model = model_str;

#elif __APPLE__
        // macOS：物理核心数
        int pc = 0;
        size_t len = sizeof(pc);
        if (sysctlbyname("hw.physicalcpu", &pc, &len, nullptr, 0) == 0) {
            info.physical_cores = pc;
        } else {
            info.physical_cores = 1;
        }

        // 获取厂商和型号（Apple Silicon）
        info.vendor = "Apple";
        char model_buf[256] = {0};
        len = sizeof(model_buf);
        if (sysctlbyname("hw.model", model_buf, &len, nullptr, 0) == 0) {
            info.model = model_buf;
        } else {
            info.model = "Apple Silicon";
        }

#else
        info.physical_cores = 1;
        info.vendor = "Unknown";
        info.model = "Unknown Platform";
#endif

        // --- 计算 Socket 数 ---
#ifdef _WIN32
        buffer_size = 0;
        GetLogicalProcessorInformationEx(RelationProcessorPackage, nullptr, &buffer_size);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            info.sockets = 1;
        } else {
            std::vector<char> buffer(buffer_size);
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info_ex =
                reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data());
            if (GetLogicalProcessorInformationEx(RelationProcessorPackage, info_ex, &buffer_size)) {
                DWORD offset = 0;
                while (offset < buffer_size) {
                    if (info_ex->Relationship == RelationProcessorPackage) {
                        info.sockets++;
                    }
                    offset += info_ex->Size;
                    info_ex = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
                        reinterpret_cast<char*>(info_ex) + info_ex->Size);
                }
            } else {
                info.sockets = 1;
            }
        }

#elif __linux__
        std::ifstream f("/proc/cpuinfo");
        std::string line;
        std::set<int> socket_ids;
        while (std::getline(f, line)) {
            if (line.find("physical id") != std::string::npos) {
                int id = std::stoi(line.substr(line.find_last_of(':') + 1));
                socket_ids.insert(id);
            }
        }
        info.sockets = static_cast<int>(socket_ids.size());

#elif __APPLE__
        int sockets = 0;
        len = sizeof(sockets);
        if (sysctlbyname("hw.packages", &sockets, &len, nullptr, 0) == 0) {
            info.sockets = sockets;
        } else {
            info.sockets = 1;
        }

#else
        info.sockets = 1;
#endif

        // --- 判断是否启用超线程 ---
        info.hyperthreading = (info.logical_cores > info.physical_cores);

        // --- 预留：频率（Linux 可读取 /proc/cpuinfo 中的 cpu MHz，macOS 无标准接口）---
#ifdef __linux__
        std::ifstream freq_file("/proc/cpuinfo");
        std::string line;
        double max_freq_mhz = 0.0;
        while (std::getline(freq_file, line)) {
            if (line.find("cpu MHz") != std::string::npos) {
                double freq = std::stod(line.substr(line.find_last_of(':') + 1));
                if (freq > max_freq_mhz) max_freq_mhz = freq;
            }
        }
        info.frequency_ghz = max_freq_mhz / 1000.0;
#endif

        return info;
    }
} // namespace hw