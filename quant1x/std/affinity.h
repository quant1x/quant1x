#pragma once
#ifndef QUANT1X_STD_AFFINITY_H
#define QUANT1X_STD_AFFINITY_H 1

#include "base.h"
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <errno.h>
#include <cstdlib>

#if defined(__linux__)
#include <numaif.h>
#endif

// CPU亲和性和NUMA感知优化
namespace affinity {

    // =============================================================================
    // 基础CPU亲和性API（保持向后兼容）
    // =============================================================================
    
    // 绑定当前线程到最优CPU（智能选择）
    bool bind_current_thread_to_optimal_cpu(std::error_code &ec);
    
    // 绑定当前线程到指定CPU
    bool bind_current_thread_to_cpu(unsigned cpu_index, std::error_code &ec);
    
    // 绑定指定线程到CPU
    bool bind_thread_to_cpu(std::thread &thread, unsigned cpu_index, std::error_code &ec);
    
    // 绑定指定线程到最优CPU
    bool bind_thread_to_optimal_cpu(std::thread &thread, std::error_code &ec);

    // NUMA拓扑信息结构体
    struct NumaTopology {
        unsigned node_count;                           // NUMA节点数量
        std::vector<std::vector<unsigned>> node_cpus;  // 每个NUMA节点的CPU列表  
        std::vector<unsigned> cpu_to_node;             // CPU到NUMA节点的映射
        std::vector<size_t> node_memory_sizes;         // 每个节点的内存大小(MB)
        bool is_numa_available;                        // 系统是否支持NUMA
        
        NumaTopology() : node_count(0), is_numa_available(false) {}
    };

    // CPU分配策略枚举
    enum class CpuAllocationStrategy {
        ROUND_ROBIN,        // 轮询分配（原始策略）
        NUMA_LOCAL,         // NUMA本地节点优先
        LOAD_BALANCED,      // 负载均衡
        ISOLATED_CRITICAL   // 关键线程隔离
    };

    // 线程优先级分类
    enum class ThreadPriority {
        NORMAL,           // 普通线程
        HIGH_FREQUENCY,   // 高频交易线程
        MARKET_DATA,      // 市场数据线程  
        CRITICAL_PATH     // 关键路径线程
    };

    // =============================================================================
    // NUMA感知增强API
    // =============================================================================
    
    // 获取系统NUMA拓扑信息
    NumaTopology get_numa_topology(std::error_code &ec);
    
    // 获取指定内存地址所在的NUMA节点
    unsigned get_numa_node_of_memory(const void* ptr, std::error_code &ec);
    
    // 获取当前线程运行的NUMA节点
    unsigned get_current_numa_node(std::error_code &ec);
    
    // 获取当前线程运行的CPU ID
    unsigned get_current_cpu_id(std::error_code &ec);

    // =============================================================================
    // 智能CPU分配器类
    // =============================================================================
    
    class NumaAwareCpuAllocator {
    public:
        explicit NumaAwareCpuAllocator(CpuAllocationStrategy default_strategy = CpuAllocationStrategy::NUMA_LOCAL);
        ~NumaAwareCpuAllocator();

        // 为线程分配最优CPU
        unsigned allocate_optimal_cpu(ThreadPriority priority = ThreadPriority::NORMAL, 
                                     const void* memory_hint = nullptr,
                                     std::error_code* ec = nullptr);
        
        // 为特定NUMA节点分配CPU
        unsigned allocate_cpu_on_node(unsigned numa_node, std::error_code &ec);
        
        // 为关键线程分配隔离CPU
        unsigned allocate_isolated_cpu(std::error_code &ec);
        
        // 获取负载最轻的NUMA节点
        unsigned get_least_loaded_node() const;
        
        // 获取分配统计信息
        struct AllocationStats {
            std::vector<unsigned> node_allocations;  // 每个节点的分配次数
            unsigned total_allocations;              // 总分配次数
            unsigned isolated_allocations;           // 隔离分配次数
        };
        AllocationStats get_allocation_stats() const;
        
        // 重置分配器状态
        void reset_allocation_counters();

    private:
        class Impl;
        std::unique_ptr<Impl> pimpl;
    };

    // =============================================================================
    // NUMA感知内存分配器
    // =============================================================================
    
    template<typename T>
    class NumaAwareAllocator {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        template<typename U>
        struct rebind {
            using other = NumaAwareAllocator<U>;
        };
        
        NumaAwareAllocator() = default;
        template<typename U> NumaAwareAllocator(const NumaAwareAllocator<U>&) {}
        
        // 在指定CPU对应的NUMA节点上分配内存
        T* allocate_on_cpu_node(size_t count, unsigned cpu_id, std::error_code &ec);
        
        // 在指定NUMA节点上分配内存
        T* allocate_on_numa_node(size_t count, unsigned numa_node, std::error_code &ec);
        
        // 分配本地NUMA节点内存（基于当前线程CPU）
        T* allocate_local(size_t count, std::error_code &ec);
        
        // 释放NUMA感知分配的内存
        void deallocate_numa(T* ptr, size_t count, std::error_code &ec);
        
        // 标准allocator接口兼容
        T* allocate(size_t count);
        void deallocate(T* ptr, size_t count);
        
        // 比较操作符
        template<typename U>
        bool operator==(const NumaAwareAllocator<U>&) const noexcept { return true; }
        template<typename U>
        bool operator!=(const NumaAwareAllocator<U>&) const noexcept { return false; }
    };

    // Template implementation (must be in header for proper instantiation)
    template<typename T>
    T* NumaAwareAllocator<T>::allocate_on_cpu_node(size_t count, unsigned cpu_id, std::error_code &ec) {
        ec.clear();
        
        // 获取 NUMA 拓扑信息
        auto topology = get_numa_topology(ec);
        if (ec || !topology.is_numa_available || cpu_id >= topology.cpu_to_node.size()) {
            // 如果NUMA不可用或CPU ID无效，回退到普通分配
            return allocate(count);
        }
        
        unsigned numa_node = topology.cpu_to_node[cpu_id];
        return allocate_on_numa_node(count, numa_node, ec);
    }

    template<typename T>
    T* NumaAwareAllocator<T>::allocate_local(size_t count, std::error_code &ec) {
        ec.clear();
        
        // 获取当前线程绑定的CPU
        unsigned current_cpu = get_current_cpu_id(ec);
        if (ec) {
            // 如果无法获取当前CPU，回退到普通分配
            return allocate(count);
        }
        
        return allocate_on_cpu_node(count, current_cpu, ec);
    }

    template<typename T>
    T* NumaAwareAllocator<T>::allocate(size_t count) {
        std::error_code ec;
        T* result = allocate_local(count, ec);
        if (ec) {
            // 如果NUMA分配失败，回退到标准分配
            size_t size = count * sizeof(T);
            void* ptr = std::malloc(size);
            if (!ptr) {
                throw std::bad_alloc();
            }
            return static_cast<T*>(ptr);
        }
        return result;
    }

    template<typename T>
    T* NumaAwareAllocator<T>::allocate_on_numa_node(size_t count, unsigned numa_node, std::error_code &ec) {
        ec.clear();
        
        size_t size = count * sizeof(T);
        void* ptr = nullptr;
        
#ifdef _WIN32
        // Windows NUMA 内存分配
        ptr = VirtualAllocExNuma(
            GetCurrentProcess(),
            nullptr,
            size,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE,
            numa_node
        );
        
        if (!ptr) {
            ec = std::error_code(GetLastError(), std::system_category());
            return nullptr;
        }
#elif defined(__linux__)
        // Linux NUMA 内存分配
        if (posix_memalign(&ptr, 64, size) != 0) {
            ec = std::error_code(errno, std::system_category());
            return nullptr;
        }
        
        if (ptr) {
            unsigned long nodemask = 1UL << numa_node;
            if (mbind(ptr, size, MPOL_BIND, &nodemask, sizeof(nodemask) * 8, MPOL_MF_STRICT) != 0) {
                free(ptr);
                ec = std::error_code(errno, std::system_category());
                return nullptr;
            }
        }
#else
        (void)numa_node;
        // macOS 或其他平台，回退到普通分配
        if (posix_memalign(&ptr, 64, size) != 0) {
            ec = std::error_code(errno, std::system_category());
            return nullptr;
        }
#endif
        
        return static_cast<T*>(ptr);
    }

    template<typename T>
    void NumaAwareAllocator<T>::deallocate_numa(T* ptr, size_t count, std::error_code &ec) {
        ec.clear();
        
        if (!ptr) return;
        
        // 抑制未使用参数警告
        (void)count;
        
#ifdef _WIN32
        if (!VirtualFree(ptr, 0, MEM_RELEASE)) {
            ec = std::error_code(GetLastError(), std::system_category());
        }
#else
        free(ptr);
#endif
    }

    template<typename T>
    void NumaAwareAllocator<T>::deallocate(T* ptr, size_t count) {
        if (!ptr) return;
        
        std::error_code ec;
        deallocate_numa(ptr, count, ec);
        if (ec) {
            // 如果NUMA释放失败，尝试标准释放
            std::free(ptr);
        }
    }

    // =============================================================================
    // 高频交易优化工具类
    // =============================================================================
    
    class HighFrequencyOptimizer {
    public:
        struct OptimizationConfig {
            bool enable_cpu_isolation;
            bool enable_numa_binding;
            bool enable_memory_pinning;
            unsigned reserved_cpus_per_node;
            
            OptimizationConfig() 
                : enable_cpu_isolation(true)
                , enable_numa_binding(true) 
                , enable_memory_pinning(true)
                , reserved_cpus_per_node(1) {}
        };
        
        explicit HighFrequencyOptimizer(const OptimizationConfig& config = {});
        
        // 为市场数据线程优化CPU和内存
        bool optimize_market_data_thread(std::error_code &ec);
        
        // 为交易执行线程优化
        bool optimize_trading_thread(const void* shared_memory_ptr, std::error_code &ec);
        
        // 为策略计算线程优化
        bool optimize_strategy_thread(std::error_code &ec);
        
        // 获取优化建议
        struct OptimizationReport {
            unsigned recommended_cpu;
            unsigned numa_node;
            size_t local_memory_mb;
            double expected_latency_improvement_pct;
            std::string optimization_summary;
        };
        OptimizationReport analyze_current_thread(std::error_code &ec);

    private:
        OptimizationConfig config_;
        std::unique_ptr<NumaAwareCpuAllocator> allocator_;
    };

} // namespace affinity

#endif // QUANT1X_STD_AFFINITY_H