#include "affinity.h"

#include <atomic>
#include <climits>  
#include <system_error>
#include <thread>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <malloc.h>
#elif __APPLE__
#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#include <unistd.h>
#include <sys/sysctl.h>
#else
#include <sched.h>
#include <unistd.h>
#ifdef __linux__
#include <numa.h>
#include <numaif.h>
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

    // =============================================================================
    // NUMA拓扑发现实现
    // =============================================================================

    NumaTopology get_numa_topology(std::error_code &ec) {
        NumaTopology topology;
        ec.clear();

#ifdef _WIN32
        // Windows NUMA拓扑发现
        ULONG highest_node_number;
        if (!GetNumaHighestNodeNumber(&highest_node_number)) {
            ec = std::error_code(GetLastError(), std::system_category());
            return topology;
        }

        topology.node_count = highest_node_number + 1;
        topology.node_cpus.resize(topology.node_count);
        topology.node_memory_sizes.resize(topology.node_count);
        
        unsigned total_cpus = get_cpu_count(ec);
        if (ec) return topology;
        topology.cpu_to_node.resize(total_cpus, 0);

        for (ULONG node = 0; node <= highest_node_number; ++node) {
            // 获取CPU掩码
            ULONGLONG processor_mask;
            if (GetNumaNodeProcessorMask(node, &processor_mask)) {
                for (unsigned cpu = 0; cpu < 64 && cpu < total_cpus; ++cpu) {
                    if (processor_mask & (1ULL << cpu)) {
                        topology.node_cpus[node].push_back(cpu);
                        topology.cpu_to_node[cpu] = node;
                    }
                }
            }
            
            // 获取内存信息
            ULONGLONG available_bytes;
            if (GetNumaAvailableMemoryNode(node, &available_bytes)) {
                topology.node_memory_sizes[node] = available_bytes / (1024 * 1024); // 转换为MB
            }
        }
        topology.is_numa_available = true;

#elif defined(__linux__)
        // Linux NUMA拓扑发现
        if (numa_available() == -1) {
            topology.is_numa_available = false;
            ec = std::make_error_code(std::errc::function_not_supported);
            return topology;
        }

        int max_node = numa_max_node();
        if (max_node < 0) {
            topology.is_numa_available = false;
            ec = std::make_error_code(std::errc::no_such_device);
            return topology;
        }

        topology.node_count = max_node + 1;
        topology.node_cpus.resize(topology.node_count);
        topology.node_memory_sizes.resize(topology.node_count);
        
        int total_cpus = numa_num_possible_cpus();
        topology.cpu_to_node.resize(total_cpus, 0);

        for (int node = 0; node <= max_node; ++node) {
            // 获取CPU掩码
            struct bitmask* cpumask = numa_allocate_cpumask();
            if (numa_node_to_cpus(node, cpumask) == 0) {
                for (int cpu = 0; cpu < total_cpus; ++cpu) {
                    if (numa_bitmask_isbitset(cpumask, cpu)) {
                        topology.node_cpus[node].push_back(cpu);
                        topology.cpu_to_node[cpu] = node;
                    }
                }
            }
            numa_free_cpumask(cpumask);
            
            // 获取内存大小
            long long node_size = numa_node_size64(node, nullptr);
            if (node_size > 0) {
                topology.node_memory_sizes[node] = node_size / (1024 * 1024); // 转换为MB
            }
        }
        topology.is_numa_available = true;

#elif defined(__APPLE__)
        // macOS没有传统的NUMA，但有多个内存控制器
        // 简化处理：创建单节点拓扑
        topology.node_count = 1;
        topology.node_cpus.resize(1);
        topology.node_memory_sizes.resize(1);
        
        unsigned total_cpus = get_cpu_count(ec);
        if (ec) return topology;
        
        topology.cpu_to_node.resize(total_cpus, 0);
        for (unsigned cpu = 0; cpu < total_cpus; ++cpu) {
            topology.node_cpus[0].push_back(cpu);
            topology.cpu_to_node[cpu] = 0;
        }
        
        // 获取系统总内存
        int64_t memsize;
        size_t size = sizeof(memsize);
        if (sysctlbyname("hw.memsize", &memsize, &size, nullptr, 0) == 0) {
            topology.node_memory_sizes[0] = memsize / (1024 * 1024);
        }
        topology.is_numa_available = false; // macOS不是真正的NUMA

#else
        // 其他平台：fallback到单节点
        topology.node_count = 1;
        topology.node_cpus.resize(1);
        topology.node_memory_sizes.resize(1);
        
        unsigned total_cpus = get_cpu_count(ec);
        if (ec) return topology;
        
        topology.cpu_to_node.resize(total_cpus, 0);
        for (unsigned cpu = 0; cpu < total_cpus; ++cpu) {
            topology.node_cpus[0].push_back(cpu);
            topology.cpu_to_node[cpu] = 0;
        }
        topology.is_numa_available = false;
#endif

        return topology;
    }

    unsigned get_numa_node_of_memory(const void* ptr, std::error_code &ec) {
        (void)ptr; // 暂未实现内存地址检测
        ec.clear();
        
#ifdef _WIN32
        PROCESSOR_NUMBER proc_num;
        if (GetNumaProcessorNodeEx(&proc_num, nullptr)) {
            return proc_num.Group; // 简化处理
        }
        ec = std::error_code(GetLastError(), std::system_category());
        return 0;
        
#elif defined(__linux__)
        if (numa_available() == -1) {
            ec = std::make_error_code(std::errc::function_not_supported);
            return 0;
        }
        
        int node = -1;
        if (get_mempolicy(&node, nullptr, 0, const_cast<void*>(ptr), MPOL_F_NODE | MPOL_F_ADDR) == 0) {
            return static_cast<unsigned>(node);
        }
        ec = std::error_code(errno, std::system_category());
        return 0;
        
#else
        // 其他平台：始终返回节点0
        return 0;
#endif
    }

    unsigned get_current_numa_node(std::error_code &ec) {
        ec.clear();
        
#ifdef _WIN32
        PROCESSOR_NUMBER proc_num;
        GetCurrentProcessorNumberEx(&proc_num);
        
        USHORT node_number;
        if (GetNumaProcessorNodeEx(&proc_num, &node_number)) {
            return static_cast<unsigned>(node_number);
        }
        ec = std::error_code(GetLastError(), std::system_category());
        return 0;
        
#elif defined(__linux__)
        if (numa_available() == -1) {
            ec = std::make_error_code(std::errc::function_not_supported);
            return 0;
        }
        
        int cpu = sched_getcpu();
        if (cpu >= 0) {
            return numa_node_of_cpu(cpu);
        }
        ec = std::error_code(errno, std::system_category());
        return 0;
        
#else
        return 0;
#endif
    }

    unsigned get_current_cpu_id(std::error_code &ec) {
        ec.clear();
#ifdef _WIN32
        return GetCurrentProcessorNumber();
#elif defined(__linux__)
        int cpu = sched_getcpu();
        if (cpu < 0) {
            ec = std::error_code(errno, std::system_category());
            return 0;
        }
        return static_cast<unsigned>(cpu);
#else
        // macOS 和其他平台，返回 0
        return 0;
#endif
    }

    // =============================================================================
    // NumaAwareCpuAllocator实现
    // =============================================================================

    class NumaAwareCpuAllocator::Impl {
    public:
        NumaTopology topology;
        CpuAllocationStrategy strategy;
        std::unique_ptr<std::atomic<unsigned>[]> node_allocation_counters;
        unsigned node_count{0};
        std::atomic<unsigned> total_allocations{0};
        std::atomic<unsigned> isolated_allocations{0};
        std::vector<bool> isolated_cpus; // 标记哪些CPU被用作隔离
        
        explicit Impl(CpuAllocationStrategy default_strategy) : strategy(default_strategy) {
            std::error_code ec;
            topology = get_numa_topology(ec);
            if (!ec && topology.node_count > 0) {
                node_count = topology.node_count;
                node_allocation_counters = std::make_unique<std::atomic<unsigned>[]>(node_count);
                for (unsigned i = 0; i < node_count; ++i) {
                    node_allocation_counters[i].store(0, std::memory_order_relaxed);
                }
                isolated_cpus.resize(topology.cpu_to_node.size(), false);
                
                // 预留每个节点的最后一个CPU用于隔离
                for (unsigned node = 0; node < topology.node_count; ++node) {
                    if (!topology.node_cpus[node].empty()) {
                        unsigned last_cpu = topology.node_cpus[node].back();
                        if (last_cpu < isolated_cpus.size()) {
                            isolated_cpus[last_cpu] = true;
                        }
                    }
                }
            }
        }
        
        unsigned allocate_optimal_cpu_impl(ThreadPriority priority, const void* memory_hint) {
            total_allocations.fetch_add(1, std::memory_order_relaxed);
            
            if (!topology.is_numa_available || topology.node_count <= 1) {
                return get_next_cpu_index_simple();
            }
            
            unsigned target_node = 0;
            
            // 根据内存提示选择NUMA节点
            if (memory_hint) {
                std::error_code ec;
                target_node = get_numa_node_of_memory(memory_hint, ec);
                if (ec) target_node = 0;
            } else {
                // 选择负载最轻的节点
                target_node = get_least_loaded_node_impl();
            }
            
            // 高优先级线程可能需要特殊处理
            if (priority == ThreadPriority::CRITICAL_PATH || priority == ThreadPriority::HIGH_FREQUENCY) {
                unsigned isolated_cpu = try_allocate_isolated_cpu_on_node(target_node);
                if (isolated_cpu != UINT_MAX) {
                    isolated_allocations.fetch_add(1, std::memory_order_relaxed);
                    return isolated_cpu;
                }
            }
            
            // 在目标节点内分配CPU
            return allocate_cpu_on_node_impl(target_node);
        }
        
        unsigned allocate_cpu_on_node_impl(unsigned numa_node) {
            if (numa_node >= topology.node_count || topology.node_cpus[numa_node].empty()) {
                return get_next_cpu_index_simple();
            }
            
            const auto& node_cpus = topology.node_cpus[numa_node];
            unsigned local_counter = node_allocation_counters[numa_node].fetch_add(1, std::memory_order_relaxed);
            
            // 过滤掉已隔离的CPU
            std::vector<unsigned> available_cpus;
            for (unsigned cpu : node_cpus) {
                if (cpu < isolated_cpus.size() && !isolated_cpus[cpu]) {
                    available_cpus.push_back(cpu);
                }
            }
            
            if (available_cpus.empty()) {
                return node_cpus[local_counter % node_cpus.size()]; // fallback
            }
            
            return available_cpus[local_counter % available_cpus.size()];
        }
        
        unsigned try_allocate_isolated_cpu_on_node(unsigned numa_node) {
            if (numa_node >= topology.node_count) return UINT_MAX;
            
            const auto& node_cpus = topology.node_cpus[numa_node];
            for (auto it = node_cpus.rbegin(); it != node_cpus.rend(); ++it) {
                unsigned cpu = *it;
                if (cpu < isolated_cpus.size() && isolated_cpus[cpu]) {
                    return cpu;
                }
            }
            return UINT_MAX;
        }
        
        unsigned get_least_loaded_node_impl() const {
            if (topology.node_count <= 1) return 0;
            
            unsigned min_load_node = 0;
            unsigned min_load = node_allocation_counters[0].load(std::memory_order_relaxed);
            
            for (unsigned node = 1; node < topology.node_count; ++node) {
                unsigned load = node_allocation_counters[node].load(std::memory_order_relaxed);
                if (load < min_load) {
                    min_load = load;
                    min_load_node = node;
                }
            }
            return min_load_node;
        }
        
        unsigned get_next_cpu_index_simple() {
            // fallback到原始策略
            std::error_code ec;
            unsigned cpu_count = get_cpu_count(ec);
            if (ec || cpu_count == 0) return 0;
            
            static std::atomic<unsigned> next_cpu{0};
            return (cpu_count - 1 - (next_cpu.fetch_add(1, std::memory_order_relaxed) % cpu_count));
        }
    };

    NumaAwareCpuAllocator::NumaAwareCpuAllocator(CpuAllocationStrategy default_strategy) 
        : pimpl(std::make_unique<Impl>(default_strategy)) {
    }

    NumaAwareCpuAllocator::~NumaAwareCpuAllocator() = default;

    unsigned NumaAwareCpuAllocator::allocate_optimal_cpu(ThreadPriority priority, const void* memory_hint, std::error_code* ec) {
        if (ec) ec->clear();
        return pimpl->allocate_optimal_cpu_impl(priority, memory_hint);
    }

    unsigned NumaAwareCpuAllocator::allocate_cpu_on_node(unsigned numa_node, std::error_code &ec) {
        ec.clear();
        return pimpl->allocate_cpu_on_node_impl(numa_node);
    }

    unsigned NumaAwareCpuAllocator::allocate_isolated_cpu(std::error_code &ec) {
        ec.clear();
        unsigned node = pimpl->get_least_loaded_node_impl();
        unsigned cpu = pimpl->try_allocate_isolated_cpu_on_node(node);
        if (cpu != UINT_MAX) {
            pimpl->isolated_allocations.fetch_add(1, std::memory_order_relaxed);
            return cpu;
        }
        
        // fallback: 尝试其他节点
        for (unsigned n = 0; n < pimpl->topology.node_count; ++n) {
            if (n != node) {
                cpu = pimpl->try_allocate_isolated_cpu_on_node(n);
                if (cpu != UINT_MAX) {
                    pimpl->isolated_allocations.fetch_add(1, std::memory_order_relaxed);
                    return cpu;
                }
            }
        }
        
        ec = std::make_error_code(std::errc::resource_unavailable_try_again);
        return 0;
    }

    unsigned NumaAwareCpuAllocator::get_least_loaded_node() const {
        return pimpl->get_least_loaded_node_impl();
    }

    NumaAwareCpuAllocator::AllocationStats NumaAwareCpuAllocator::get_allocation_stats() const {
        AllocationStats stats;
        stats.total_allocations = pimpl->total_allocations.load(std::memory_order_relaxed);
        stats.isolated_allocations = pimpl->isolated_allocations.load(std::memory_order_relaxed);
        
        stats.node_allocations.resize(pimpl->topology.node_count);
        for (unsigned i = 0; i < pimpl->topology.node_count; ++i) {
            stats.node_allocations[i] = pimpl->node_allocation_counters[i].load(std::memory_order_relaxed);
        }
        
        return stats;
    }

    void NumaAwareCpuAllocator::reset_allocation_counters() {
        pimpl->total_allocations.store(0, std::memory_order_relaxed);
        pimpl->isolated_allocations.store(0, std::memory_order_relaxed);
        for (unsigned i = 0; i < pimpl->node_count; ++i) {
            pimpl->node_allocation_counters[i].store(0, std::memory_order_relaxed);
        }
    }

    // =============================================================================
    // Helper Functions
    // =============================================================================

}  // namespace affinity