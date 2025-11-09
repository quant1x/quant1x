#include "affinity.h"

#include <atomic>
#include <system_error>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <cerrno>
#include <pthread.h>
#include <unistd.h>
#if defined(__APPLE__)
#include <mach/mach.h>
#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#else
#include <sched.h>
#endif
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
                long count = sysconf(_SC_NPROCESSORS_ONLN);
                if (count <= 0) {
                    return 0;
                }
                return static_cast<unsigned>(count);
#endif
            }();

            if (cpu_count == 0) {
                ec = std::make_error_code(std::errc::no_such_device);
            } else {
                ec.clear();
            }
            return cpu_count;
        }

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
            mach_port_t thread_mach = static_cast<mach_port_t>(handle);
            thread_affinity_policy policy{};
            policy.affinity_tag = static_cast<integer_t>(cpu_index + 1);
            kern_return_t kr = thread_policy_set(
                thread_mach,
                THREAD_AFFINITY_POLICY,
                reinterpret_cast<thread_policy_t>(&policy),
                THREAD_AFFINITY_POLICY_COUNT);
            if (kr != KERN_SUCCESS) {
                ec = std::error_code(kr, std::system_category());
                return false;
            }
#else
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(cpu_index, &cpuset);
            if (pthread_setaffinity_np(reinterpret_cast<pthread_t>(handle), sizeof(cpu_set_t), &cpuset) != 0) {
                ec = std::error_code(errno, std::system_category());
                return false;
            }
#endif
            ec.clear();
            return true;
        }

        unsigned get_next_cpu_index(std::error_code &ec) {
            unsigned cpu_count = get_cpu_count(ec);
            if (ec) {
                return 0;
            }

            static std::atomic<unsigned> next_cpu{0};
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

    unsigned get_current_cpu_id(std::error_code &ec) {
        ec.clear();
#ifdef _WIN32
        return GetCurrentProcessorNumber();
#elif defined(__APPLE__)
        return 0;
#else
        int cpu = sched_getcpu();
        if (cpu < 0) {
            ec = std::error_code(errno, std::system_category());
            return 0;
        }
        return static_cast<unsigned>(cpu);
#endif
    }

}  // namespace affinity