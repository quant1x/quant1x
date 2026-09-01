# C++ 基准测试 (对应 Rust 版 benchmarks, 见 benches/vyukov_bench.rs)
# 依赖根 CMakeLists.txt 中的 test-benchmark INTERFACE 库 (Google Benchmark)

# 宏: 创建 benchmark-<name> 可执行文件 (与 tests/CMakeLists.txt 中同名宏对称)
macro(add_benchmark_executable source_file)
    get_filename_component(test_name ${source_file} NAME_WE)
    set(target_name "benchmark-${test_name}")

    add_executable(${target_name} ${source_file})

    # 头文件路径: 仓库根 (quant1x/ 下所有头文件) 与 third_party
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/third_party/include
    )

    # 链接 quant1x (BUILD_INTERFACE include) 与 Google Benchmark
    target_link_libraries(${target_name} PRIVATE
        quant1x test-benchmark
    )
endmacro()

# Vyukov MPMC 队列基准 (8P8C, 与 benches/vyukov_bench.rs 参数对齐)
add_benchmark_executable(vyukov_bench.cpp)
