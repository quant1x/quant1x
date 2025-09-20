#include <gtest/gtest.h>
#include "affinity.h"
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <numeric>
#include <iomanip>

using namespace affinity;
using namespace std::chrono;

class NumaAffinityTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::error_code ec;
        topology = get_numa_topology(ec);
        
        // 如果获取拓扑失败，创建一个最小的mock拓扑用于测试
        if (ec || topology.node_count == 0) {
            topology.node_count = 1;
            topology.node_cpus.resize(1);
            topology.cpu_to_node.resize(std::thread::hardware_concurrency(), 0);
            topology.node_memory_sizes.resize(1, 1024); // 1GB
            topology.is_numa_available = false;
            
            for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
                topology.node_cpus[0].push_back(i);
            }
        }
    }
    
    NumaTopology topology;
};

// 测试基础CPU亲和性功能
TEST_F(NumaAffinityTest, BasicCpuAffinity) {
    std::error_code ec;
    
    // 测试绑定当前线程到最优CPU
    bool result = bind_current_thread_to_optimal_cpu(ec);
    EXPECT_TRUE(result);
    EXPECT_FALSE(ec) << "绑定当前线程失败: " << ec.message();
    
    // 测试绑定到指定CPU（如果有多个CPU）
    if (topology.cpu_to_node.size() > 1) {
        unsigned target_cpu = 1;
        result = bind_current_thread_to_cpu(target_cpu, ec);
        EXPECT_TRUE(result);
        EXPECT_FALSE(ec) << "绑定到CPU " << target_cpu << " 失败: " << ec.message();
    }
}

// 测试NUMA拓扑发现
TEST_F(NumaAffinityTest, NumaTopologyDiscovery) {
    EXPECT_GT(topology.node_count, 0) << "NUMA节点数应该大于0";
    EXPECT_EQ(topology.node_cpus.size(), topology.node_count) << "节点CPU列表大小不匹配";
    EXPECT_EQ(topology.node_memory_sizes.size(), topology.node_count) << "节点内存大小列表不匹配";
    
    // 验证每个CPU都有对应的NUMA节点映射
    for (size_t cpu = 0; cpu < topology.cpu_to_node.size(); ++cpu) {
        unsigned node = topology.cpu_to_node[cpu];
        EXPECT_LT(node, topology.node_count) << "CPU " << cpu << " 映射到无效节点 " << node;
        
        // 验证该CPU在对应节点的CPU列表中
        const auto& node_cpus = topology.node_cpus[node];
        EXPECT_TRUE(std::find(node_cpus.begin(), node_cpus.end(), cpu) != node_cpus.end())
            << "CPU " << cpu << " 不在节点 " << node << " 的CPU列表中";
    }
    
    std::cout << "检测到 " << topology.node_count << " 个NUMA节点" << std::endl;
    for (unsigned node = 0; node < topology.node_count; ++node) {
        std::cout << "节点 " << node << ": " << topology.node_cpus[node].size() 
                  << " CPUs, " << topology.node_memory_sizes[node] << " MB" << std::endl;
    }
}

// 测试NumaAwareCpuAllocator
TEST_F(NumaAffinityTest, NumaAwareCpuAllocator) {
    NumaAwareCpuAllocator allocator(CpuAllocationStrategy::NUMA_LOCAL);
    std::error_code ec;
    
    // 测试普通线程分配
    unsigned cpu1 = allocator.allocate_optimal_cpu(ThreadPriority::NORMAL, nullptr, &ec);
    EXPECT_FALSE(ec) << "分配普通线程CPU失败: " << ec.message();
    EXPECT_LT(cpu1, topology.cpu_to_node.size()) << "分配的CPU ID无效";
    
    // 测试高优先级线程分配
    unsigned cpu2 = allocator.allocate_optimal_cpu(ThreadPriority::HIGH_FREQUENCY, nullptr, &ec);
    EXPECT_FALSE(ec) << "分配高频线程CPU失败: " << ec.message();
    EXPECT_LT(cpu2, topology.cpu_to_node.size()) << "分配的CPU ID无效";
    
    // 测试隔离CPU分配
    unsigned isolated_cpu = allocator.allocate_isolated_cpu(ec);
    if (!ec) {  // 隔离CPU可能不可用，这是正常的
        EXPECT_LT(isolated_cpu, topology.cpu_to_node.size()) << "隔离CPU ID无效";
        std::cout << "成功分配隔离CPU: " << isolated_cpu << std::endl;
    }
    
    // 验证分配统计
    auto stats = allocator.get_allocation_stats();
    EXPECT_GE(stats.total_allocations, 2) << "总分配次数应该至少为2";
    EXPECT_EQ(stats.node_allocations.size(), topology.node_count) << "节点分配统计大小不匹配";
    
    std::cout << "CPU分配统计: 总计=" << stats.total_allocations 
              << ", 隔离=" << stats.isolated_allocations << std::endl;
}

// 测试NUMA感知内存分配器
TEST_F(NumaAffinityTest, NumaAwareAllocator) {
    NumaAwareAllocator<int> allocator;
    std::error_code ec;
    
    const size_t test_size = 1000;
    
    // 测试本地内存分配
    int* local_memory = allocator.allocate_local(test_size, ec);
    if (!ec) {
        EXPECT_NE(local_memory, nullptr) << "本地内存分配返回nullptr";
        
        // 测试写入数据
        for (size_t i = 0; i < test_size; ++i) {
            local_memory[i] = static_cast<int>(i);
        }
        
        // 验证数据
        for (size_t i = 0; i < test_size; ++i) {
            EXPECT_EQ(local_memory[i], static_cast<int>(i)) << "内存数据验证失败";
        }
        
        // 释放内存
        allocator.deallocate_numa(local_memory, test_size, ec);
        EXPECT_FALSE(ec) << "释放内存失败: " << ec.message();
        
        std::cout << "本地内存分配测试通过 (" << test_size << " 个int)" << std::endl;
    }
    
    // 测试指定节点内存分配
    if (topology.node_count > 0) {
        int* node_memory = allocator.allocate_on_numa_node(test_size, 0, ec);
        if (!ec) {
            EXPECT_NE(node_memory, nullptr) << "节点内存分配返回nullptr";
            allocator.deallocate_numa(node_memory, test_size, ec);
            EXPECT_FALSE(ec) << "释放节点内存失败: " << ec.message();
            
            std::cout << "节点0内存分配测试通过" << std::endl;
        }
    }
    
    // 测试标准allocator接口兼容性
    try {
        int* std_memory = allocator.allocate(test_size);
        EXPECT_NE(std_memory, nullptr) << "标准分配接口返回nullptr";
        
        // 测试数据写入
        std_memory[0] = 42;
        std_memory[test_size - 1] = 99;
        EXPECT_EQ(std_memory[0], 42);
        EXPECT_EQ(std_memory[test_size - 1], 99);
        
        allocator.deallocate(std_memory, test_size);
        std::cout << "标准allocator接口兼容性测试通过" << std::endl;
        
    } catch (const std::bad_alloc&) {
        // 内存不足是可能的，不算失败
        std::cout << "标准allocator接口内存不足（正常情况）" << std::endl;
    }
}

// 测试多线程CPU绑定
TEST_F(NumaAffinityTest, MultiThreadCpuBinding) {
    const int num_threads = std::min(4, static_cast<int>(std::thread::hardware_concurrency()));
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> error_count{0};
    
    NumaAwareCpuAllocator allocator;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&allocator, &success_count, &error_count, i]() {
            std::error_code ec;
            
            // 每个线程分配自己的CPU
            unsigned cpu = allocator.allocate_optimal_cpu(ThreadPriority::NORMAL, nullptr, &ec);
            if (!ec) {
                // 尝试绑定到分配的CPU
                if (bind_current_thread_to_cpu(cpu, ec)) {
                    success_count.fetch_add(1);
                    
                    // 模拟一些工作
                    auto start = high_resolution_clock::now();
                    volatile int sum = 0;
                    for (int j = 0; j < 1000000; ++j) {
                        sum += j;
                    }
                    auto end = high_resolution_clock::now();
                    
                    auto duration = duration_cast<microseconds>(end - start).count();
                    (void)duration; // 避免未使用变量警告
                } else {
                    error_count.fetch_add(1);
                }
            } else {
                error_count.fetch_add(1);
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "多线程测试结果: 成功=" << success_count.load() 
              << ", 失败=" << error_count.load() << std::endl;
    
    // 至少应该有一些成功的绑定
    EXPECT_GT(success_count.load(), 0) << "没有线程成功绑定CPU";
}

// 性能基准测试
TEST_F(NumaAffinityTest, PerformanceBenchmark) {
    const size_t iterations = 10000;
    const size_t data_size = 1024;
    
    // 测试无CPU绑定的性能
    auto start_unbound = high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        std::vector<int> data(data_size);
        std::iota(data.begin(), data.end(), 0);
        volatile int sum = std::accumulate(data.begin(), data.end(), 0);
        (void)sum; // 避免优化掉
    }
    auto end_unbound = high_resolution_clock::now();
    auto unbound_duration = duration_cast<microseconds>(end_unbound - start_unbound).count();
    
    // 测试CPU绑定后的性能
    std::error_code ec;
    NumaAwareCpuAllocator allocator;
    unsigned cpu = allocator.allocate_optimal_cpu(ThreadPriority::HIGH_FREQUENCY, nullptr, &ec);
    
    if (!ec) {
        bind_current_thread_to_cpu(cpu, ec);
        
        auto start_bound = high_resolution_clock::now();
        for (size_t i = 0; i < iterations; ++i) {
            std::vector<int> data(data_size);
            std::iota(data.begin(), data.end(), 0);
            volatile int sum = std::accumulate(data.begin(), data.end(), 0);
            (void)sum; // 避免优化掉
        }
        auto end_bound = high_resolution_clock::now();
        auto bound_duration = duration_cast<microseconds>(end_bound - start_bound).count();
        
        std::cout << "性能对比:" << std::endl;
        std::cout << "  未绑定: " << unbound_duration << " μs" << std::endl;
        std::cout << "  CPU绑定: " << bound_duration << " μs" << std::endl;
        
        if (bound_duration > 0) {
            double improvement = static_cast<double>(unbound_duration) / bound_duration;
            std::cout << "  性能提升: " << std::fixed << std::setprecision(2) 
                      << improvement << "x" << std::endl;
        }
        
        // 绑定后的性能不应该显著变差（允许10%的波动）
        EXPECT_LE(bound_duration, unbound_duration * 1.1) 
            << "CPU绑定后性能显著下降";
    }
}

// 错误处理测试
TEST_F(NumaAffinityTest, ErrorHandling) {
    std::error_code ec;
    
    // 测试无效CPU绑定
    unsigned invalid_cpu = topology.cpu_to_node.size() + 100;
    EXPECT_FALSE(bind_current_thread_to_cpu(invalid_cpu, ec));
    EXPECT_TRUE(ec) << "绑定无效CPU应该返回错误";
    
    // 测试NUMA内存分配器的错误处理
    NumaAwareAllocator<int> allocator;
    
    // 测试零大小分配
    int* zero_alloc = allocator.allocate_on_numa_node(0, 0, ec);
    EXPECT_EQ(zero_alloc, nullptr) << "零大小分配应该返回nullptr";
    
    // 测试无效NUMA节点
    unsigned invalid_node = topology.node_count + 10;
    int* invalid_alloc = allocator.allocate_on_numa_node(100, invalid_node, ec);
    if (topology.is_numa_available) {
        EXPECT_TRUE(ec) << "无效NUMA节点应该返回错误";
    }
    // 清理内存以避免泄漏
    if (invalid_alloc) {
        allocator.deallocate_numa(invalid_alloc, 100, ec);
    }
    
    std::cout << "错误处理测试完成" << std::endl;
}

// 专门测试 NUMA 内存分配器的功能
TEST_F(NumaAffinityTest, NumaMemoryAllocatorTest) {
    std::error_code ec;
    
    std::cout << "=== 测试 NUMA 内存分配器功能 ===" << std::endl;
    
    // 测试标准分配器接口
    {
        NumaAwareAllocator<double> allocator;
        
        // 测试标准 allocate/deallocate
        constexpr size_t count = 1000;
        double* ptr = allocator.allocate(count);
        ASSERT_NE(ptr, nullptr) << "标准分配应该成功";
        
        // 写入数据验证内存可用
        for (size_t i = 0; i < count; ++i) {
            ptr[i] = static_cast<double>(i) * 1.5;
        }
        
        // 验证数据正确性
        for (size_t i = 0; i < count; ++i) {
            EXPECT_DOUBLE_EQ(ptr[i], static_cast<double>(i) * 1.5);
        }
        
        allocator.deallocate(ptr, count);
        std::cout << "标准分配器接口测试完成" << std::endl;
    }
    
    // 测试 NUMA 感知分配
    if (topology.is_numa_available && topology.node_count > 0) {
        NumaAwareAllocator<int> numa_allocator;
        
        // 测试在指定NUMA节点上分配
        constexpr size_t numa_count = 500;
        for (unsigned node = 0; node < std::min(topology.node_count, 2u); ++node) {
            int* numa_ptr = numa_allocator.allocate_on_numa_node(numa_count, node, ec);
            
            if (!ec && numa_ptr) {
                // 写入和验证数据
                for (size_t i = 0; i < numa_count; ++i) {
                    numa_ptr[i] = static_cast<int>(i + node * 1000);
                }
                
                for (size_t i = 0; i < numa_count; ++i) {
                    EXPECT_EQ(numa_ptr[i], static_cast<int>(i + node * 1000));
                }
                
                numa_allocator.deallocate_numa(numa_ptr, numa_count, ec);
                EXPECT_FALSE(ec) << "NUMA释放应该成功";
                
                std::cout << "NUMA节点 " << node << " 分配测试完成" << std::endl;
            }
        }
    }
    
    // 测试基于CPU的本地NUMA分配
    {
        NumaAwareAllocator<char> local_allocator;
        constexpr size_t local_count = 256;
        
        char* local_ptr = local_allocator.allocate_local(local_count, ec);
        if (!ec && local_ptr) {
            // 写入测试数据
            for (size_t i = 0; i < local_count; ++i) {
                local_ptr[i] = static_cast<char>('A' + (i % 26));
            }
            
            // 验证数据
            for (size_t i = 0; i < local_count; ++i) {
                EXPECT_EQ(local_ptr[i], static_cast<char>('A' + (i % 26)));
            }
            
            local_allocator.deallocate_numa(local_ptr, local_count, ec);
            std::cout << "本地NUMA分配测试完成" << std::endl;
        } else {
            std::cout << "本地NUMA分配跳过（可能不支持或CPU绑定失败）" << std::endl;
        }
    }
    
    // 测试多线程并发分配
    {
        constexpr int num_threads = 4;
        constexpr size_t alloc_size = 100;
        std::vector<std::thread> threads;
        std::atomic<int> success_count{0};
        std::atomic<int> failure_count{0};
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&, i]() {
                NumaAwareAllocator<float> thread_allocator;
                std::error_code thread_ec;
                
                float* ptr = thread_allocator.allocate(alloc_size);
                if (ptr) {
                    // 写入线程特定数据
                    float base_value = static_cast<float>(i * 100);
                    for (size_t j = 0; j < alloc_size; ++j) {
                        ptr[j] = base_value + static_cast<float>(j);
                    }
                    
                    // 验证数据完整性
                    bool data_valid = true;
                    for (size_t j = 0; j < alloc_size; ++j) {
                        if (ptr[j] != base_value + static_cast<float>(j)) {
                            data_valid = false;
                            break;
                        }
                    }
                    
                    thread_allocator.deallocate(ptr, alloc_size);
                    
                    if (data_valid) {
                        success_count.fetch_add(1);
                    } else {
                        failure_count.fetch_add(1);
                    }
                } else {
                    failure_count.fetch_add(1);
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        EXPECT_EQ(success_count.load(), num_threads) << "所有线程应该成功分配和验证内存";
        EXPECT_EQ(failure_count.load(), 0) << "不应该有失败的分配";
        
        std::cout << "多线程并发分配测试完成，成功: " << success_count.load() 
                  << ", 失败: " << failure_count.load() << std::endl;
    }
    
    std::cout << "=== NUMA 内存分配器测试全部完成 ===" << std::endl;
}