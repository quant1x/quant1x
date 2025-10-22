# 定义一个宏，用于创建 一般性的测试 可执行文件
macro(add_app_executable source_file)
    # 获取源文件名（不含扩展名）
    get_filename_component(test_name ${source_file} NAME_WE)

    # 添加前缀 "app-"
    set(target_name "app-${test_name}")

    # 创建带前缀的可执行文件
    add_executable(${target_name} ${source_file})

    # 添加头文件路径
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/include
    )

    # 链接依赖库（例如你提到的 utils）
    target_link_libraries(${target_name} PRIVATE
        quant1x
    )
endmacro()

include(GoogleTest)

# Ensure CTest is available and tests are enabled so test discovery (or add_test)
# entries are registered into the generated build tree. Without this, `ctest`
# may report "No tests were found" even when test executables exist.
include(CTest)
enable_testing()

# 定义一个宏，用于创建 Catch2单元测试 可执行文件
macro(add_catch2_executable source_file)
    # 获取源文件名（不含扩展名）
    get_filename_component(test_name ${source_file} NAME_WE)

    # 添加前缀 "catch2-"
    set(target_name "catch2-${test_name}")

    # 创建带前缀的可执行文件
    add_executable(${target_name} ${source_file})

    # 自动为该可执行中的每个 Catch2 测试用例创建 CTest 条目
    # 如果项目已经包含 Catch2 的 CMake helpers，则使用 catch_discover_tests，
    # 否则退回到为整个可执行添加一个单独的 add_test（可通过 exe 参数过滤具体测试）
    if(COMMAND catch_discover_tests)
        catch_discover_tests(${target_name}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            PROPERTIES TIMEOUT 60
        )
    else()
        add_test(NAME ${target_name} COMMAND $<TARGET_FILE:${target_name}>)
    endif()

    # 添加头文件路径
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/include
    )

    # 链接依赖库（例如你提到的 utils）
    target_link_libraries(${target_name} PRIVATE
        quant1x test-catch2 test-benchmark
    )
    # Fallback: register a single CTest entry for the whole executable only if
    # a test with the same name hasn't been registered already. This avoids
    # duplicate add_test errors when discovery helpers have already added tests
    # or when the same macro is processed multiple times for any reason.
    get_property(_registered_tests DIRECTORY PROPERTY TESTS)
    list(FIND _registered_tests ${target_name} _test_index)
    if(_test_index EQUAL -1)
        add_test(NAME ${target_name} COMMAND $<TARGET_FILE:${target_name}>)
    endif()
endmacro()

# 定义一个宏，用于创建 GTest单元测试 可执行文件
macro(add_gtest_executable source_file)
    # 获取源文件名（不含扩展名）
    get_filename_component(test_name ${source_file} NAME_WE)

    # 添加前缀 "gtest-"
    set(target_name "gtest-${test_name}")

    # 创建带前缀的可执行文件
    add_executable(${target_name} ${source_file})

    # 添加头文件路径
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/include
    )

    # 链接依赖库（例如你提到的 utils）
    target_link_libraries(${target_name} PRIVATE
        quant1x test-gtest
    )

    # 自动为 GoogleTest 可执行注册 CTest 条目（如果可用则使用 gtest_discover_tests）
    if(COMMAND gtest_discover_tests)
        gtest_discover_tests(${target_name}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            PROPERTIES TIMEOUT 60
        )
    else()
        add_test(NAME ${target_name} COMMAND $<TARGET_FILE:${target_name}>)
    endif()
endmacro()

# 定义一个宏，用于创建 benchmark基准测试 可执行文件
macro(add_benchmark_executable source_file)
    # 获取源文件名（不含扩展名）
    get_filename_component(test_name ${source_file} NAME_WE)

    # 添加前缀 "benchmark-"
    set(target_name "benchmark-${test_name}")

    # 创建带前缀的可执行文件
    add_executable(${target_name} ${source_file})

    # 添加头文件路径
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/include
    )

    # 链接依赖库（例如你提到的 utils）
    target_link_libraries(${target_name} PRIVATE
        quant1x test-benchmark
    )
endmacro()

# crash测试
if (WIN32)
    add_app_executable(crash-windows.cpp)
elseif (LINUX)
    add_app_executable(crash-linux.cpp)
endif ()
add_app_executable(test-crash.cpp)

# 错误码测试
add_catch2_executable(tdd-error.cpp)

# 测试宿主目录
add_catch2_executable(tdd-homedir.cpp)
# 字符集测试
add_catch2_executable(tdd-iconv.cpp)
# 测试配置
add_catch2_executable(tdd-config.cpp)

# io测试
add_catch2_executable(tdd-logger-dist.cpp)
add_catch2_executable(tdd-lazy-log.cpp)

# cron测试
add_catch2_executable(tdd-cron.cpp)

# 网络测试
add_catch2_executable(tdd-http.cpp)

# 基础组件测试
add_app_executable(test-ringbuffer.cpp)

# 编解码测试
add_catch2_executable(tdd-encoding-csv.cpp)
add_catch2_executable(tdd-encoding-json.cpp)

add_catch2_executable(tdd-yaml-deserialize.cpp)
add_catch2_executable(tdd-yaml-deserialize-config.cpp)

# 测试缓存
add_catch2_executable(tdd-calendar.cpp)
add_catch2_executable(test-calender.cpp)

# 测试协议
add_catch2_executable(tdd-level1.cpp)
add_catch2_executable(tdd-level1-detect-server.cpp)
add_catch2_executable(tdd-level1-encoding.cpp)

# ta 测试
add_gtest_executable(tdd-ta.cpp)
add_gtest_executable(test_security_quote_implicit_spread.cpp)
add_catch2_executable(tdd-formula.cpp)
add_catch2_executable(tdd-rolling.cpp)
add_catch2_executable(tdd-series.cpp)
add_catch2_executable(tdd-ta-ma.cpp)
add_catch2_executable(tdd-ta-dbscan.cpp)

# DataFrame 测试
add_catch2_executable(tdd-dataframe-ewm.cpp)
add_catch2_executable(tdd-dataframe-data.cpp)
add_catch2_executable(tdd-dataframe-release.cpp)

# numerics 数值类功能测试
add_catch2_executable(tdd-number-range.cpp)

# 时间功能 测试
add_catch2_executable(tdd-session.cpp)
add_catch2_executable(tdd-timestamp.cpp)
add_catch2_executable(tdd-times.cpp)

# 文件锁/落地标识
add_catch2_executable(tdd-touch.cpp)
add_catch2_executable(tdd-filelock.cpp)

# 基础数据测试
add_catch2_executable(tdd-datasets.cpp)
add_catch2_executable(tdd-klines.cpp)

# f10
add_catch2_executable(tdd-f10-notices.cpp)
add_catch2_executable(tdd-f10-safety-score.cpp)
add_catch2_executable(tdd-f10-margin-trading.cpp)
add_catch2_executable(tdd-f10-capital.cpp)

# 筹码分布
add_catch2_executable(tdd-chips.cpp)

# 技术形态
add_catch2_executable(tdd-peaks.cpp)
add_catch2_executable(tdd-waves.cpp)
add_catch2_executable(tdd-linear-regression.cpp)
add_catch2_executable(tdd-patterns-wave-release.cpp)

# 回测框架测试
add_catch2_executable(tdd-backtest-release.cpp)

# round-trip matching unit test
add_catch2_executable(tdd-backtest-roundtrip.cpp)

# additional round-trip edge cases
add_catch2_executable(tdd-backtest-roundtrip-extra.cpp)
add_catch2_executable(tdd-backtest-result-compat.cpp)

# FP growth测试
add_catch2_executable(test_fp_growth.cpp)

