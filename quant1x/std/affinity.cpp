#include <quant1x/std/affinity.h>

#include <atomic>
#include <climits>  // 添加头文件用于LONG_MAX
#include <system_error>
#include <thread>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#include <unistd.h>
#else
#include <sched.h>
#include <unistd.h>
#endif

// CPU亲和性
namespace affinity {

    namespace {

        uintptr_t get_current_thread_handle() {
#ifdef _WIN32
            return reinterpret_cast<uintptr_t>(GetCurrentThread());
#else
            return reinterpret_cast<uintptr_t>(pthread_self());
#endif
        }

        unsigned get_cpu_count(std::error_code &ec) {
            static const unsigned cpu_count = []() -> unsigned {
#ifdef _WIN32
                SYSTEM_INFO sysinfo;
                GetSystemInfo(&sysinfo);
                return static_cast<unsigned>(sysinfo.dwNumberOfProcessors);
#else
                // 使用long暂存结果以便检查错误
                long count = sysconf(_SC_NPROCESSORS_ONLN);
                // 检查返回值的有效性
                if (count <= 0 || count > LONG_MAX) {
                    return 0;
                }
                return static_cast<unsigned>(count);
#endif
            }();

            // 检查CPU核心数是否有效
            if (cpu_count == 0) {
                ec = std::make_error_code(std::errc::no_such_device);
            } else {
                ec.clear();
            }
            return cpu_count;
        }

        // 更安全地接受 uintptr_t，兼容 Windows HANDLE 和 Linux pthread_t
        bool set_affinity(uintptr_t handle, unsigned int cpu_index, std::error_code &ec) {
            unsigned count = get_cpu_count(ec);
            if (ec) {
                return false;
            }
            if (cpu_index >= count) {
                ec = std::make_error_code(std::errc::invalid_argument);
                return false;
            }

#ifdef _WIN32
            DWORD_PTR mask = (static_cast<DWORD_PTR>(1) << cpu_index);
            if (!SetThreadAffinityMask(reinterpret_cast<HANDLE>(handle), mask)) {
                ec = std::error_code(GetLastError(), std::system_category());
                return false;
            }
#elif defined(__APPLE__)
            mach_port_t                   thread_mach = static_cast<mach_port_t>(handle);
            struct thread_affinity_policy policy;
            policy.affinity_tag = cpu_index + 1;
            kern_return_t kr    = thread_policy_set(
                thread_mach, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, THREAD_AFFINITY_POLICY_COUNT);
            if (kr != KERN_SUCCESS) {
                ec = std::error_code(kr, std::system_category());
                return false;
            }
#else
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(cpu_index, &cpuset);
            if (pthread_setaffinity_np(static_cast<pthread_t>(handle), sizeof(cpu_set_t), &cpuset) != 0) {
                ec = std::error_code(errno, std::system_category());
                return false;
            }
#endif
            ec.clear();
            return true;
        }

        // 获取下一个CPU核心索引（循环分配）
        unsigned get_next_cpu_index(std::error_code &ec) {
            unsigned cpu_count = get_cpu_count(ec);
            if (ec) {
                // 确保有错误代码（可能是从get_cpu_count传播的）
                if (!ec)
                    ec = std::make_error_code(std::errc::no_such_device);
                return 0;
            }

            // 使用静态原子变量保证线程安全
            static std::atomic<unsigned> next_cpu{0};

            // 正确计算反向递减的核心索引
            // 策略：从最后一个核心开始向前循环分配
            // 目的是避免与大多数默认绑定到低编号CPU的应用程序争抢资源
            // 实际上，许多程序/内核线程默认运行在CPU0或其他前面的核心上
            // 因此我们优先使用后面的核心，以获得更干净、独立的执行环境
            return (cpu_count - 1 - (next_cpu.fetch_add(1, std::memory_order_relaxed) % cpu_count));
        }
    }  // namespace

    bool bind_current_thread_to_cpu(unsigned cpu_index, std::error_code &ec) {
        return set_affinity(get_current_thread_handle(), cpu_index, ec);
    }

    bool bind_current_thread_to_optimal_cpu(std::error_code &ec) {
        unsigned cpu_index = get_next_cpu_index(ec);
        if (ec) {
            return false;
        }
        return set_affinity(get_current_thread_handle(), cpu_index, ec);
    }

    bool bind_thread_to_cpu(std::thread &thread, unsigned cpu_index, std::error_code &ec) {
        return set_affinity(reinterpret_cast<uintptr_t>(thread.native_handle()), cpu_index, ec);
    }

    bool bind_thread_to_optimal_cpu(std::thread &thread, std::error_code &ec) {
        unsigned cpu_index = get_next_cpu_index(ec);
        if (ec) {
            return false;
        }
        return set_affinity(reinterpret_cast<uintptr_t>(thread.native_handle()), cpu_index, ec);
    }

}  // namespace affinity