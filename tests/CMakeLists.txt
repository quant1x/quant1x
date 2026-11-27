# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT
#
# C++ 测试构建：
#   - Catch2 用例按模块合并成 4 个可执行文件（num / data / runtime / contrib），
#     不再一文件一可执行：每个可执行都要过一遍 1 GB 级的 quant1x 静态库，65 次链接是主要浪费；
#     用例粒度不受影响，catch_discover_tests 仍按单条 TEST_CASE 注册 ctest 条目。
#   - 全部测试目标 EXCLUDE_FROM_ALL：默认构建只编库与 q1x，跑测试时构建 `tests` 目标。

# 测试目标收集，供末尾的 `tests` 聚合目标使用
set(QUANT1X_TEST_TARGETS "")

# 一般性测试可执行文件（自带 main）
macro(add_app_executable source_file)
    get_filename_component(test_name ${source_file} NAME_WE)
    set(target_name "app-${test_name}")

    add_executable(${target_name} EXCLUDE_FROM_ALL ${source_file})
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    )
    target_link_libraries(${target_name} PRIVATE
        quant1x
    )

    list(APPEND QUANT1X_TEST_TARGETS ${target_name})
endmacro()

include(GoogleTest)

# Catch2 的用例发现模块（根 CMakeLists 已完成 find_package(Catch2)）；
# 没有它 catch_discover_tests 不存在，ctest 只能拿到整包一个条目。
include(Catch OPTIONAL)

# Ensure CTest is available and tests are enabled so test discovery (or add_test)
# entries are registered into the generated build tree. Without this, `ctest`
# may report "No tests were found" even when test executables exist.
include(CTest)
enable_testing()

# GTest 可执行文件
macro(add_gtest_executable source_file)
    get_filename_component(test_name ${source_file} NAME_WE)
    set(target_name "gtest-${test_name}")

    add_executable(${target_name} EXCLUDE_FROM_ALL ${source_file})
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    )
    target_link_libraries(${target_name} PRIVATE
        quant1x test-gtest
    )

    list(APPEND QUANT1X_TEST_TARGETS ${target_name})
endmacro()

# benchmark 基准测试可执行文件（不并入 tests 目标）
macro(add_benchmark_executable source_file)
    get_filename_component(test_name ${source_file} NAME_WE)
    set(target_name "benchmark-${test_name}")

    add_executable(${target_name} ${source_file})
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    )
    target_link_libraries(${target_name} PRIVATE
        quant1x test-benchmark
    )
endmacro()

# ==============================
# Catch2 用例分组
# ==============================
set(CATCH2_NUM_SOURCES
    # 公式 / 指标 / 数值
    tdd-formula.cpp
    tdd-num-ndarray.cpp
    tdd-rolling.cpp
    tdd-series.cpp
    tdd-ta-ma.cpp
    tdd-ta-dbscan.cpp
    tdd-linear-regression.cpp
    tdd-number-range.cpp
    tdd-peaks.cpp
    tdd-waves.cpp
    tdd-patterns-wave-release.cpp
    tdd-std-util.cpp
    tdd-globmatch.cpp
    # DataFrame
    tdd-dataframe-ewm.cpp
    tdd-dataframe-data.cpp
    tdd-dataframe-release.cpp
)

set(CATCH2_DATA_SOURCES
    # 行情与数据契约
    tdd-bars.cpp
    tdd-datasets.cpp
    tdd-chips.cpp
    tdd-xdxr.cpp
    tdd-snapshot.cpp
    # 编解码
    tdd-encoding-csv.cpp
    tdd-encoding-json.cpp
    tdd-json.cpp
    tdd-yaml-deserialize.cpp
    tdd-yaml-deserialize-config.cpp
    tdd-charsets.cpp
    tdd-iconv.cpp
    # 协议
    tdd-level1.cpp
    tdd-level1-detect-server.cpp
    tdd-level1-encoding.cpp
    # 时间与日历
    tdd-calendar.cpp
    test-calender.cpp
    tdd-timestamp.cpp
    tdd-times.cpp
)

set(CATCH2_RUNTIME_SOURCES
    # 基础组件
    tdd-error.cpp
    tdd-homedir.cpp
    tdd-config.cpp
    tdd-strings.cpp
    tdd-all.cpp
    tdd-session.cpp
    tdd-touch.cpp
    tdd-filelock.cpp
    # 日志
    tdd-logger.cpp
    tdd-logger-dist.cpp
    tdd-lazy-log.cpp
    # 网络与调度
    tdd-http.cpp
    tdd-asio.cpp
    tdd-cron.cpp
    tdd-distributed-id.cpp
)

set(CATCH2_CONTRIB_SOURCES
    # 特色数据
    tdd-f10-notices.cpp
    tdd-f10-safety-score.cpp
    tdd-f10-margin-trading.cpp
    tdd-f10-capital.cpp
    # 回测
    tdd-backtest-roundtrip.cpp
    tdd-backtest-roundtrip-extra.cpp
    tdd-backtest-result-compat.cpp
    tdd-backtest-order.cpp
    # 策略与实盘
    tdd-strategies.cpp
    tdd-trader-order.cpp
    tdd-trader-price.cpp
    tdd-history.cpp
)

# 一个分组 = 一个可执行文件；用例仍按 TEST_CASE 逐条注册到 ctest
function(add_catch2_group group_name)
    set(target_name "catch2-${group_name}")

    add_executable(${target_name} EXCLUDE_FROM_ALL ${ARGN})
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    )
    target_link_libraries(${target_name} PRIVATE
        quant1x test-catch2 test-benchmark
    )

    if(COMMAND catch_discover_tests)
        catch_discover_tests(${target_name}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            PROPERTIES TIMEOUT 60
        )
    else()
        add_test(NAME ${target_name} COMMAND $<TARGET_FILE:${target_name}>)
    endif()

    set(QUANT1X_TEST_TARGETS "${QUANT1X_TEST_TARGETS};${target_name}" PARENT_SCOPE)
endfunction()

add_catch2_group(num ${CATCH2_NUM_SOURCES})
add_catch2_group(data ${CATCH2_DATA_SOURCES})
add_catch2_group(runtime ${CATCH2_RUNTIME_SOURCES})
add_catch2_group(contrib ${CATCH2_CONTRIB_SOURCES})

# ==============================
# 自带 main 的测试程序
# ==============================
# crash 测试
if (WIN32)
    add_app_executable(crash-windows.cpp)
elseif (LINUX)
    add_app_executable(crash-linux.cpp)
endif ()
add_app_executable(test-crash.cpp)

# 基础组件
add_app_executable(test-ringbuffer.cpp)
add_app_executable(test-highest_one_bit.cpp)

# 命令行解析
add_app_executable(test-argparser-parser.cpp)
add_app_executable(test-argparser-complete.cpp)

# 跨语言对拍：输出即结论，不并入 Catch2
add_app_executable(tdd-num-parity.cpp)
add_test(NAME app-tdd-num-parity COMMAND app-tdd-num-parity WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_app_executable(tdd-formula-parity.cpp)
add_test(NAME app-tdd-formula-parity COMMAND app-tdd-formula-parity WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_app_executable(tdd-indicators.cpp)
add_test(NAME app-tdd-indicators COMMAND app-tdd-indicators WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# A股回测应用测试
add_app_executable(tdd-backtest-ashare.cpp)

# 自带 main 的独立程序（不属于 Catch2 用例集）
add_app_executable(tdd-backtest-release.cpp)
add_test(NAME app-tdd-backtest-release COMMAND app-tdd-backtest-release WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_app_executable(test_fp_growth.cpp)
add_test(NAME app-test_fp_growth COMMAND app-test_fp_growth WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# 其他应用测试
add_app_executable(debug_snake_case.cpp)
add_app_executable(mmap-read.cpp)
add_app_executable(mmap-write.cpp)
add_app_executable(test-iconv.cpp)
add_app_executable(test-multibar.cpp)
add_app_executable(test_go_strings_port.cpp)

# ==============================
# GTest 用例（框架不同，单独可执行）
# ==============================
# gtest-tdd-ta 仍按旧 ta::Series 接口编写（ta/rolling.h 编译失败），待迁移后再启用
# add_gtest_executable(tdd-ta.cpp)
add_gtest_executable(test_security_quote_implicit_spread.cpp)

# ==============================
# 基准测试
# ==============================
add_benchmark_executable(test-benchmark.cpp)

# ==============================
# 聚合目标：cmake --build . --target tests
# ==============================
add_custom_target(tests DEPENDS ${QUANT1X_TEST_TARGETS})
