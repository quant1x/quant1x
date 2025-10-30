cmake_minimum_required(VERSION 3.30)
set(CMAKE_VERBOSE_MAKEFILE ON)
# 加载基础 CMake 配置
include(cmake/base.cmake)

# ============================================================
# 检测 项目git仓库当前分支的最新tag, 用tag作为版本号
# ============================================================
# 通过git获取版本信息
include(cmake/git_tag_info.cmake)
get_latest_git_tag_info(VERSION_FROM_GIT LATEST_TAG TAG_COMMIT_HASH GIT_AUTHOR_NAME GIT_AUTHOR_EMAIL GIT_AUTHOR_DATE)

project(quant1x
        VERSION ${VERSION_FROM_GIT}
        LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # 为VSCode生成compile_commands.json
set(BUILD_SHARED_LIBS OFF)  # 默认构建为 STATIC

# ============================================================
# Fast cron test option
# 允许通过 -DFAST_CRON_TEST=ON 加速长耗时 cron 测试
# 如果未显式指定，保持默认 OFF（完整时序）
# ============================================================
option(FAST_CRON_TEST "Enable shortened durations in cron-v1 test" OFF)
if(FAST_CRON_TEST)
    message(STATUS "FAST_CRON_TEST enabled: cron-v1 test will use shortened timings")
    add_compile_definitions(FAST_CRON_TEST)
endif()

# 编译器选项
include(cmake/compile_options.cmake)

# ============================================================
# vcpkg Options/Dependencies
# ============================================================
add_library(third_libs INTERFACE)
# 可选：显式链接静态版 pthread
find_package(Threads REQUIRED)
target_link_libraries(third_libs INTERFACE Threads::Threads)
if (MINGW OR GNU)
    target_link_libraries(third_libs INTERFACE -static)
    # 静态链接所有库（包括 libwinpthread-1.dll）
    set(CMAKE_EXE_LINKER_FLAGS "-static -lpthread")
    set(CMAKE_SHARED_LINKER_FLAGS "-static -lpthread")
    set(CMAKE_MODULE_LINKER_FLAGS "-static -lpthread")
endif ()

message(STATUS "detect VCPKG")
# 通过环境变量获取全局路径
if (DEFINED ENV{VCPKG_ROOT})
    file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" VCPKG_ROOT)
    string(REGEX REPLACE "/$" "" VCPKG_ROOT "${VCPKG_ROOT}")
else ()
    message(FATAL_ERROR "请设置 VCPKG_ROOT 环境变量指向全局 vcpkg 目录")
endif ()

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
MESSAGE(STATUS "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")

# 修正VCPKG的installed目录
if(MSVC OR (WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    set(VCPKG_INSTALLED_DIR "${PROJECT_SOURCE_DIR}/vcpkg_installed")
else()
    set(VCPKG_INSTALLED_DIR "${VCPKG_ROOT}/installed")
endif()

# 在 Triplet 文件中添加
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CRT_LINKAGE static)
#if (MSVC)
#    message(STATUS "        MSVC_VERSION = ${MSVC_VERSION}")
#    message(STATUS "MSVC_TOOLSET_VERSION = ${MSVC_TOOLSET_VERSION}")
#endif ()
# 根据平台自动选择 Triplet
if (WIN32)
    message("triplet windows...")
    if (MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Default Triplet for Windows")
    else ()
        set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "Default Triplet for Windows")
    endif ()
elseif (UNIX AND NOT APPLE)
    set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "Default Triplet for Linux")
elseif (APPLE)
    set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "Default Triplet for macOS")
else ()
    message(status "other...")
endif ()
MESSAGE(STATUS "VCPKG_TARGET_TRIPLET = ${VCPKG_TARGET_TRIPLET}")

# 设置cmake搜索路径
set(VCPKG_LIBS_PATH "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
message(STATUS "VCPKG_LIBS_PATH = ${VCPKG_LIBS_PATH}")
set(CMAKE_PREFIX_PATH "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
MESSAGE(STATUS "CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
include_directories(${CMAKE_PREFIX_PATH}/include)
link_directories(${CMAKE_PREFIX_PATH}/lib)
set(VCPKG_INCLUDE_DIRS ${VCPKG_LIBS_PATH}/include)
set(VCPKG_LIBRARY_DIRS ${VCPKG_LIBS_PATH}/lib)
#target_include_directories(third_libs INTERFACE ${VCPKG_INCLUDE_DIRS})
#target_link_directories(third_libs INTERFACE ${VCPKG_LIBRARY_DIRS})

if (DEFINED ENV{MSF_RUNTIME})
    file(TO_CMAKE_PATH "$ENV{MSF_RUNTIME}" RUNTIME)
    string(REGEX REPLACE "/$" "" RUNTIME "${RUNTIME}")
else ()
    message(FATAL_ERROR "NOT found ENV MSF_RUNTIME")
endif ()
MESSAGE(STATUS "RUNTIME = ${RUNTIME}")

# 项目内聚合的第三方库
set(inner_third_party_dir "${CMAKE_SOURCE_DIR}/third_party")
set(INNER_THIRD_PARTY_INCLUDE_DIRS ${inner_third_party_dir}/include)
set(INNER_THIRD_PARTY_LIBRARY_DIRS ${inner_third_party_dir}/lib)
if(WIN32)
    link_directories(${CMAKE_PREFIX_PATH}/lib ${INNER_THIRD_PARTY_LIBRARY_DIRS})
else ()
    link_directories(${CMAKE_PREFIX_PATH}/lib ${RUNTIME}/lib)
endif ()

message(STATUS "================== Crash Report Integration ==================")
#include(cmake/crash.cmake)
include(cmake/backward.cmake)

# check if compiler is nvcc or nvcc_wrapper
set(COMPILER_IS_NVCC false)
get_filename_component(COMPILER_NAME ${CMAKE_CXX_COMPILER} NAME)
if (COMPILER_NAME MATCHES "^nvcc")
    set(COMPILER_IS_NVCC true)
endif()

if (DEFINED ENV{OMPI_CXX} OR DEFINED ENV{MPICH_CXX})
    if ( ($ENV{OMPI_CXX} MATCHES "nvcc") OR ($ENV{MPICH_CXX} MATCHES "nvcc") )
        set(COMPILER_IS_NVCC true)
    endif()
endif()

# if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
#     if (NOT ${COMPILER_IS_NVCC})
#         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors")
#     endif()
#     if(quant1x_build_type STREQUAL "Debug")
#         target_compile_options(third_libs INTERFACE -O0 -fno-omit-frame-pointer)
#         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
#     endif()
# endif()

#add_library(backward_interface INTERFACE)
#set_target_properties(backward_interface PROPERTIES EXPORT_NAME Interface)
target_compile_definitions(third_libs INTERFACE ${BACKWARD_DEFINITIONS})
target_include_directories(third_libs INTERFACE ${BACKWARD_INCLUDE_DIRS})
if(BACKWARD_HAS_EXTERNAL_LIBRARIES)
    target_link_libraries(third_libs INTERFACE ${BACKWARD_LIBRARIES})
endif()

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Xclang -gcodeview -D_DEBUG -D_DLL -D_MT -Xclang --dependent-lib=msvcrtd")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Xclang -gcodeview -D_DEBUG -D_MT -Xclang --dependent-lib=msvcrtd")
message(STATUS "================== 基础功能库 ==================")
# OpenSSL
include(cmake/openssl.cmake)

# zlib
find_package(ZLIB REQUIRED)
target_link_libraries(third_libs INTERFACE ZLIB::ZLIB)
echo_lib_version(ZLIB ${ZLIB_VERSION})

# libiconv
include(cmake/iconv.cmake)

# argparse, argparse::argparse, header-only方式库, 收敛到项目内
#find_package(argparse CONFIG REQUIRED)
set(argparse_VERSION "3.2.0")
target_include_directories(third_libs INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/include>
    $<INSTALL_INTERFACE:include>
)
echo_lib_version(argparse ${argparse_VERSION})

# yaml-cpp yaml-cpp::yaml-cpp
find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(third_libs INTERFACE yaml-cpp::yaml-cpp)
echo_lib_version(yaml-cpp ${yaml-cpp_VERSION})

# tsl::robin_map(robin-map)
#find_package(tsl-robin-map CONFIG REQUIRED)
#target_link_libraries(main PRIVATE tsl::robin_map)
set(tsl-robin-map_VERSION "1.4.0")
echo_lib_version(tsl-robin-map ${tsl-robin-map_VERSION})

# croncpp
#find_package(croncpp CONFIG REQUIRED)
set(croncpp_VERSION "2023-03-30")
echo_lib_version(croncpp ${croncpp_VERSION})

# ==============================
# BS::thread_pool(bshoshany-thread-pool)
# find_path(BSHOSHANY_THREAD_POOL_INCLUDE_DIRS "BS_thread_pool.hpp")
# ==============================
include(cmake/threadpool.cmake)

#simd
include(cmake/simd.cmake)

## ==============================
## DataFrame
## find_package(Protobuf REQUIRED)
## ==============================
#find_package(DataFrame CONFIG REQUIRED)
#echo_lib_version(DataFrame ${DataFrame_VERSION})
#target_link_libraries(third_libs INTERFACE DataFrame::DataFrame)

message(STATUS "================== 编解码工具库 ==================")
## RapidJSON
#find_package(RapidJSON CONFIG REQUIRED)

# ==============================
# protobuf 3.21.11 固定版本, 3.21.12以上版本带有abseil库, 放弃高版本
# find_package(Protobuf REQUIRED)
# ==============================
include(cmake/protobuf.cmake)

# ==============================
# CapnProto
# find_package(CapnProto CONFIG REQUIRED)
# ==============================
# 查找 Cap'n Proto 包（需要 pkg-config 或 find_package）
find_package(CapnProto CONFIG REQUIRED)
#include_directories(${CAPNPROTO_INCLUDE_DIRS})
target_link_libraries(third_libs INTERFACE CapnProto::capnp CapnProto::capnp-json)
echo_lib_version(CapnProto ${CapnProto_VERSION})
if (MSVC)
    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_definitions(KJ_USE_FIBERS=0)
    endif ()
endif ()

find_package(unofficial-minizip CONFIG REQUIRED)
target_link_libraries(third_libs INTERFACE unofficial::minizip::minizip)
echo_lib_version(unofficial-minizip ${unofficial-minizip_VERSION})
message(STATUS "================== 底层 工具库 ==================")
# ASIO
include(cmake/network.cmake)

# mimalloc
# Allow selecting how mimalloc is enabled project-wide. Defaults keep the
# previous behavior (override C++ new/delete only).
option(ENABLE_MIMALLOC "Enable mimalloc support" OFF)
set(MIMALLOC_MODE "new_delete" CACHE STRING "mimalloc mode: new_delete or global")
set_property(CACHE MIMALLOC_MODE PROPERTY STRINGS "new_delete" "global")

# Optional tiny test program to validate mimalloc at runtime. Off by default.
option(ENABLE_MIMALLOC_CHECK "Build small mimalloc_check executable that prints mimalloc stats" OFF)

if(ENABLE_MIMALLOC)
    find_package(mimalloc CONFIG REQUIRED)
    # prefer static target when available (vcpkg provides mimalloc-static)
    target_link_libraries(third_libs INTERFACE $<IF:$<TARGET_EXISTS:mimalloc-static>,mimalloc-static,mimalloc>)

    if(MIMALLOC_MODE STREQUAL "global")
        # Global override: override the C allocation functions (malloc/free/etc.).
        # Note: on Windows/dynamic builds additional steps may be required
        # (see README / mimalloc docs). Use with care for ABI compatibility.
        target_compile_definitions(third_libs INTERFACE
            MI_OVERRIDE=1
        )
        message(STATUS "mimalloc: enabling global C allocator override (MI_OVERRIDE=1)")
    else()
        # Default: only override C++ operator new/delete and avoid interposing
        # other allocation functions (matches historical project behavior).
        target_compile_definitions(third_libs INTERFACE
           MI_OVERRIDE_NEW_DELETE=1  # 让mimalloc只重载operator new/delete
           MI_INTERPOSE=0            # 禁用拦截标准库函数
        )
        message(STATUS "mimalloc: enabling C++ new/delete override (MI_OVERRIDE_NEW_DELETE=1, MI_INTERPOSE=0)")
    endif()

    echo_lib_version(mimalloc ${mimalloc_VERSION})
else()
    message(STATUS "mimalloc support disabled (ENABLE_MIMALLOC=OFF)")
endif()

# spdlog
include(cmake/spdlog.cmake)

#message(STATUS "================== JavaScript工具库 ==================")
## MuJS
#find_package(PkgConfig)
#pkg_check_modules(MUJS REQUIRED IMPORTED_TARGET mujs)
# duktape
#find_package(unofficial-duktape CONFIG REQUIRED)
#target_link_libraries(third_libs INTERFACE unofficial::duktape::duktape)
#echo_lib_version(duktape ${unofficial-duktape_VERSION})

message(STATUS "================== 测试工具库 ==================")

# 查找 Google Benchmark 包
find_package(benchmark REQUIRED)
add_library(test-benchmark INTERFACE)
target_link_libraries(test-benchmark INTERFACE benchmark::benchmark benchmark::benchmark_main)
echo_lib_version(benchmark ${benchmark_VERSION})
# Catch2
find_package(Catch2 CONFIG REQUIRED)
echo_lib_version(Catch2 ${Catch2_VERSION})
add_library(test-catch2 INTERFACE)
target_link_libraries(test-catch2 INTERFACE Catch2::Catch2 Catch2::Catch2WithMain)
# 启用Catch2 Unicode支持
target_compile_definitions(test-catch2 INTERFACE
    CATCH_CONFIG_WINDOWS_SEH
    CATCH_CONFIG_USE_ANSI_COLOR_CODES
)
# GTest
find_package(GTest CONFIG REQUIRED)
add_library(test-gtest INTERFACE)
target_link_libraries(test-gtest INTERFACE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
echo_lib_version(GTest ${GTest_VERSION})

# ============================================================
# Language Standards Detection and Configuration
# ============================================================
# 设置构建信息变量
set(BUILD_TYPE "${CMAKE_BUILD_TYPE}")

# C 标准相关
set(C_STANDARD "${CMAKE_C_STANDARD}")
set(C_STANDARD_REQUIRED "${CMAKE_C_STANDARD_REQUIRED}")
set(C_EXTENSIONS "${CMAKE_C_EXTENSIONS}")

set(C_FLAGS "${CMAKE_C_FLAGS}")
set(C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

# C++ 标准相关
set(CXX_STANDARD "${CMAKE_CXX_STANDARD}")
set(CXX_STANDARD_REQUIRED "${CMAKE_CXX_STANDARD_REQUIRED}")
set(CXX_EXTENSIONS "${CMAKE_CXX_EXTENSIONS}")

set(CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

# 链接器参数
set(EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

# 编译器信息
set(C_COMPILER_ID "${CMAKE_C_COMPILER_ID}")
set(C_COMPILER_VERSION "${CMAKE_C_COMPILER_VERSION}")
set(C_COMPILER_PATH "${CMAKE_C_COMPILER}")

set(CXX_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}")
set(CXX_COMPILER_VERSION "${CMAKE_CXX_COMPILER_VERSION}")
set(CXX_COMPILER_PATH "${CMAKE_CXX_COMPILER}")

# CUDA 参数（如果启用）
if (ENABLE_CUDA)
    set(CUDA_FLAGS_DEBUG "${CMAKE_CUDA_FLAGS_DEBUG}")
    set(CUDA_FLAGS_RELEASE "${CMAKE_CUDA_FLAGS_RELEASE}")
else ()
    set(CUDA_FLAGS_DEBUG "N/A")
    set(CUDA_FLAGS_RELEASE "N/A")
endif ()

# 第三方库版本定义
set(DEPENDENCY_OPENSSL "v${OPENSSL_VERSION}")
set(DEPENDENCY_ASIO "v${ASIO_VERSION_FULL}")
set(DEPENDENCY_ICONV "v${Iconv_VERSION}")
set(DEPENDENCY_FMT "v${fmt_VERSION}")
set(DEPENDENCY_ZLIB "v${ZLIB_VERSION}")
set(DEPENDENCY_XSIMD "v${xsimd_VERSION}")
set(DEPENDENCY_BSHOSHANY_THREAD_POOL "v${BSHOSHANY_THREAD_POOL_VERSION}")
set(DEPENDENCY_CRONCPP "v${croncpp_VERSION}")
set(DEPENDENCY_TSL_ROBIN_MAP "v${tsl-robin-map_VERSION}")
set(DEPENDENCY_ARGPARSE "v${argparse_VERSION}")
#set(DEPENDENCY_RAPIDJSON "v${RapidJSON_VERSION}")
set(DEPENDENCY_YAML_CPP "v${yaml-cpp_VERSION}")
set(DEPENDENCY_PROTOBUF "v${Protobuf_VERSION}")
set(DEPENDENCY_CAPNP "v${CapnProto_VERSION}")
set(DEPENDENCY_MIMALLOC "v${mimalloc_VERSION}")
set(DEPENDENCY_SPDLOG "v${spdlog_VERSION}")
set(DEPENDENCY_CPR "v${cpr_VERSION}")
set(DEPENDENCY_DUKTAPE "v${unofficial-duktape_VERSION}")
set(DEPENDENCY_BENCHMARK "v${benchmark_VERSION}")
set(DEPENDENCY_CATCH2 "v${Catch2_VERSION}")

# ============================================================
# 量化基础设施
# ============================================================

set(quant1x-headers
    # 标准库
    quant1x/std/affinity.h
    quant1x/std/api.h
    quant1x/std/base.h
    quant1x/std/buffer.h
    quant1x/std/strings.h
    quant1x/std/util.h
    quant1x/std/format.h
    quant1x/std/feature_detection.h
    quant1x/std/numerics.h
    quant1x/std/safe.h
    quant1x/std/simd.h
    quant1x/std/cpu_info.h

    # 运行时
    quant1x/runtime/crash.h
    quant1x/runtime/service.h
    quant1x/runtime/cache1d.h
    quant1x/runtime/once.h
    quant1x/runtime/core.h
    quant1x/runtime/scheduler.h
    quant1x/runtime/ringbuffer.h
    

    # i/o 相关
    quant1x/io/csv-reader.h
    quant1x/io/csv-writer.h
    quant1x/io/file.h
    quant1x/io/http.h
    quant1x/log/router_sink.h
    quant1x/log/lazy_daily_sink.h

    # 编码相关
    quant1x/encoding/iconv.h
    quant1x/encoding/csv.h
    quant1x/encoding/json.h
    quant1x/encoding/yaml.h

    # 配置
    quant1x/config/data-cache.h
    quant1x/config/rule-parameter.h
    quant1x/config/strategy-parameter.h
    quant1x/config/trader-parameter.h
    quant1x/config/trading-session.h

    # 业务引擎
    quant1x/engine/action.h

    # 网络相关
    quant1x/net/base.h
    quant1x/net/connection_pool.h
    quant1x/net/endpoint.h
    quant1x/net/operation_handler.h

    # 数据帧, 列式存储
    quant1x/pandas/dataframe.h
    quant1x/pandas/periods.h
    quant1x/pandas/series.h
    quant1x/pandas/rule.h

    # 数据协议
    quant1x/proto/data.h
    quant1x/proto/chips.pb.h
    quant1x/proto/chips.pb.cc
    quant1x/proto/snapshot.capnp.h
    quant1x/proto/snapshot.capnp.c++
    quant1x/proto/xdxr.pb.h
    quant1x/proto/xdxr.pb.cc

    # 技术分析
    quant1x/ta/boll.h
    quant1x/ta/ta.h
    quant1x/ta/ma.h
    quant1x/ta/rsi.h
    quant1x/ta/macd.h
    quant1x/ta/type_default.h
    quant1x/ta/rsi.h
    quant1x/ta/rolling.h
    quant1x/ta/type_default.h
    quant1x/ta/sma.h

    # 数据集
    quant1x/datasets/base.h
    quant1x/datasets/xdxr.h
    quant1x/datasets/kline_raw.h
    quant1x/datasets/kline.h
    quant1x/datasets/trans.h
    quant1x/datasets/minute.h
    quant1x/datasets/chips.h
    quant1x/datasets/kline_minute.h

    # 实时模块
    quant1x/realtime/snapshot.h

    # 回测
    quant1x/backtest/backtest.h
    quant1x/backtest/order.h
    quant1x/backtest/trade.h
    quant1x/backtest/position.h

    # 交易所相关
    quant1x/exchange/blocks.h
    quant1x/exchange/calendar.h
    quant1x/exchange/code.h
    quant1x/exchange/margin-trading.h
    quant1x/exchange/session.h
    quant1x/exchange/timestamp.h

    # level1
    quant1x/level1/config.h
    quant1x/level1/minute_time.h
    quant1x/level1/client.h
    quant1x/level1/protocol.h

    # 策略
    quant1x/engine/strategy.h
    quant1x/engine/rule-engine.h
    quant1x/engine/rule-error.h
    quant1x/engine/rule-context.h

    # 模型

    # 插件
    quant1x/plugins/plugin-loader.h
    quant1x/plugins/plugin-api.h

    # 因子
    quant1x/factors/f10.h
    quant1x/factors/share-holder.h
    quant1x/factors/base.h
    quant1x/factors/notice.h
    quant1x/factors/financial_report.h
    quant1x/factors/safety-score.h
    quant1x/factors/history.h

    # Technical Analysis Patterns
    quant1x/ta/boll.h
    quant1x/ta/ema.h
    quant1x/ta/ma.h
    quant1x/ta/macd.h
    quant1x/ta/rolling.h
    quant1x/ta/rolling_window.h
    quant1x/ta/rsi.h
    quant1x/ta/ta.h
    quant1x/ta/type_default.h
    quant1x/ta/trend.h
    quant1x/ta/waves.h

    # 交易柜台
    quant1x/trader/fee.h
    quant1x/trader/order.h
    quant1x/trader/constants.h
    quant1x/trader/account.h
    quant1x/trader/trader.h
    quant1x/trader/holding.h
    quant1x/trader/order_cache.h
    quant1x/trader/position.h
    quant1x/trader/tracker.h
    quant1x/trader/order_state.h

    # 内嵌资源文件
    quant1x/resources/meta/blocks.h

    # 应用入口
    quant1x/quant1x.h

    # 数据处理功能 quant1x/data
    learn/fpgrowth/fp_growth.h
)

set(quant1x-sources
    # 标准库兼容
    quant1x/std/strings.cpp
    quant1x/std/time.cpp
    quant1x/std/util.cpp
    quant1x/std/numerics.cpp
    quant1x/std/safe.cpp
    quant1x/std/simd.cpp
    quant1x/std/cpu_info.cpp
    quant1x/std/affinity.cpp
    
    # 运行时
    quant1x/runtime/core.cpp
    quant1x/runtime/crash.cpp
    quant1x/runtime/scheduler.cpp

    # i/o 相关
    quant1x/io/file.cpp
    quant1x/io/http.cpp
    quant1x/log/router_sink.cpp
    quant1x/log/lazy_daily_sink.cpp

    # 编码相关
    quant1x/encoding/iconv.cpp
    quant1x/encoding/json.cpp

    # 数据帧处理
    quant1x/pandas/rule.cpp

    # 引擎
    quant1x/engine/adapter.cpp

    # level1
    quant1x/level1/config.cpp
    quant1x/level1/client.cpp
    quant1x/level1/protocol.cpp


    # 配置信息
    quant1x/config/config.cpp
    quant1x/config/trading-session.cpp
    quant1x/config/trader-parameter.cpp
    quant1x/config/strategy-parameter.cpp

    quant1x/cache.cpp
    quant1x/command.cpp
    quant1x/formula.cpp
    quant1x/exchange.cpp

    # 回测
    quant1x/backtest/backtest.cpp
    quant1x/backtest/position.cpp
    quant1x/backtest/order.cpp
    quant1x/backtest/stats.cpp
    quant1x/backtest/trade.cpp

    # 数据集
    quant1x/datasets/xdxr.cpp
    quant1x/datasets/kline.cpp
    quant1x/datasets/kline_raw.cpp
    quant1x/datasets/minute.cpp
    quant1x/datasets/trans.cpp
    quant1x/datasets/chips.cpp
    quant1x/datasets/kline_minute.cpp
    quant1x/datasets.cpp

    # 交易所相关
    quant1x/exchange/blocks.cpp
    quant1x/exchange/security.cpp
    quant1x/exchange/code.cpp
    quant1x/exchange/calendar.cpp
    quant1x/exchange/session.cpp
    quant1x/exchange/timestamp.cpp
    quant1x/exchange/margin-trading.cpp

    # 因子
    quant1x/factors/f10.cpp
    quant1x/factors/share-holder.cpp
    quant1x/factors/base.cpp
    quant1x/factors/history.cpp
    quant1x/factors/notice.cpp
    quant1x/factors/financial_report.cpp
    quant1x/factors/safety-score.cpp

    # Technical Analysis Patterns
    quant1x/ta/trend.cpp
    quant1x/ta/waves.cpp

    # 实时
    quant1x/realtime/snapshot.cpp

    # 插件
    quant1x/plugins/plugin-loader.cpp

    # 策略
    quant1x/engine/rule-engine.cpp
    quant1x/engine/strategy.cpp

    # 交易柜台
    quant1x/trader/fee.cpp
    quant1x/trader/order.cpp
    quant1x/trader/account.cpp
    quant1x/trader/trader.cpp
    quant1x/trader/holding.cpp
    quant1x/trader/order_cache.cpp
    quant1x/trader/position.cpp
    quant1x/trader/tracker.cpp
    quant1x/trader/order_state.cpp

    # 应用入口
    quant1x/quant1x.cpp

    # 数据处理功能 quant1x/data
    learn/fpgrowth/fp_growth.cpp
)

# If mimalloc is enabled and we're using the C++ new/delete override mode,
# compile a tiny TU that includes the mimalloc new/delete header so the
# operator overrides are emitted into the library and linked into executables.
if(ENABLE_MIMALLOC)
    # Always compile the anchor TU when mimalloc is enabled. The TU is
    # safe to compile in both modes because it only includes
    # <mimalloc-new-delete.h> when the corresponding compile-time macros
    # are defined. This guarantees the anchor symbol
    # `quant1x_mimalloc_force_symbol` exists for link-time forcing.
    list(APPEND quant1x-sources quant1x/platform/mimalloc_force.cpp)
endif()

if (WIN32)
    list(APPEND quant1x-sources quant1x/runtime/daemon_service.cpp)
elseif (APPLE)
    list(APPEND quant1x-sources quant1x/runtime/daemon_macosx.cpp)
elseif (UNIX)
    # LINUX 是非官方变量，我们手动检测
    set(LINUX FALSE)
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(LINUX TRUE)
    endif ()

    if (LINUX)
        list(APPEND quant1x-sources quant1x/runtime/daemon_systemd.cpp)
    else ()
        message(WARNING "Unsupported Unix OS: ${CMAKE_SYSTEM_NAME}")
    endif ()
else ()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif ()

# 添加测试策略
list(APPEND quant1x-headers user/no0.h)
list(APPEND quant1x-sources user/no0.cpp)
list(APPEND quant1x-headers user/strategy-no0.h)
list(APPEND quant1x-sources user/strategy-no0.cpp)

# 检查编译器选项
get_target_property(_include_dirs third_libs INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(_library_dirs third_libs INTERFACE_LIBRARY_DIRECTORIES)
get_target_property(_libs         third_libs INTERFACE_LINK_LIBRARIES)
get_target_property(_defines      third_libs INTERFACE_COMPILE_DEFINITIONS)
get_target_property(_options      third_libs INTERFACE_COMPILE_OPTIONS)

message(STATUS "third_libs Include dirs: ${_include_dirs}")
message(STATUS "third_libs Library dirs: ${_library_dirs}")
message(STATUS "third_libs     Libs: ${_libs}")
message(STATUS "third_libs  Defines: ${_defines}")
message(STATUS "third_libs  Options: ${_options}")

add_library(quant1x STATIC
    ${quant1x-headers}
    ${quant1x-sources}
)
target_include_directories(quant1x
    SYSTEM        # 关键选项：标记为系统头文件路径
    PUBLIC        # 仅当前目标可见（或 PUBLIC/INTERFACE）
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/third_party/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(quant1x PUBLIC
    third_libs global_compile_options
)

get_target_property(_libs quant1x INTERFACE_LINK_LIBRARIES)
message(STATUS "Library 'quant1x' requires: ${_libs}")

MESSAGE(STATUS "RUNTIME = ${RUNTIME}")
set(CMAKE_INSTALL_PREFIX ${RUNTIME})

STRING(TOUPPER "${CMAKE_BUILD_TYPE}" buildtype)
message(STATUS "")
message(STATUS "")
message(STATUS "quant1x configuration summary:")
message(STATUS "")
message(STATUS "                              Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "                        Operating System : ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "                            Build System : ${CMAKE_SYSTEM}")
message(STATUS "                  Processor Architecture : ${CMAKE_SYSTEM_PROCESSOR}")
# C 编译选项
message(STATUS "------------------------------ C Standard Configuration ------------------------------")
message(STATUS "                              C_STANDARD : ${CMAKE_C_STANDARD}")
message(STATUS "                     C_STANDARD_REQUIRED : ${CMAKE_C_STANDARD_REQUIRED}")
message(STATUS "                            C_EXTENSIONS : ${CMAKE_C_EXTENSIONS}")
message(STATUS "                         C flags(CFLAGS) : ${CMAKE_C_FLAGS}")
message(STATUS "                           C flags debug : ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "                         C flags release : ${CMAKE_C_FLAGS_RELEASE}")
# C++ 编译选项
message(STATUS "------------------------------ C++ Standard Configuration ------------------------------")
message(STATUS "                            CXX_STANDARD : ${CMAKE_CXX_STANDARD}")
message(STATUS "                   CXX_STANDARD_REQUIRED : ${CMAKE_CXX_STANDARD_REQUIRED}")
message(STATUS "                          CXX_EXTENSIONS : ${CMAKE_CXX_EXTENSIONS}")
message(STATUS "                       C++ flags(CFLAGS) : ${CMAKE_CXX_FLAGS}")
message(STATUS "                         C++ flags debug : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "                       C++ flags release : ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "------------------------------ C++ linker ------------------------------")
message(STATUS "                  CMAKE_EXE_LINKER_FLAGS : ${CMAKE_EXE_LINKER_FLAGS}")
# 输出当前编译器信息
message(STATUS "------------------------------ C compiler ------------------------------")
message(STATUS "                           C compiler ID : ${CMAKE_C_COMPILER_ID}")
message(STATUS "                      C compiler version : ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "                         C compiler path : ${CMAKE_C_COMPILER}")
message(STATUS "------------------------------ C++ compiler ------------------------------")
message(STATUS "                         C++ compiler ID : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "                    C++ compiler version : ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "                       C++ compiler path : ${CMAKE_CXX_COMPILER}")
message(STATUS "------------------------------ CUDA compiler ------------------------------")
message(STATUS "                  CMAKE_CUDA_FLAGS_DEBUG : ${CMAKE_CUDA_FLAGS_DEBUG}")
message(STATUS "                CMAKE_CUDA_FLAGS_RELEASE : ${CMAKE_CUDA_FLAGS_RELEASE}")
message(STATUS "")
message(STATUS "")

message(STATUS "   Build type .......................... : ${CMAKE_BUILD_TYPE}")
message(STATUS "   Install .pdb (if available).......... : ${INSTALL_PDB}")
message(STATUS "   Install prefix ...................... : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "     Directory for binary files ........ : PREFIX/${QUANT1X_INSTALL_BIN_DIR}")
message(STATUS "     Directory for library files ....... : PREFIX/${QUANT1X_INSTALL_LIB_DIR}")
message(STATUS "     Directory for include files ....... : PREFIX/${QUANT1X_INSTALL_INCLUDE_DIR}")
message(STATUS "   C compiler .......................... : ${CMAKE_C_COMPILER}")
message(STATUS "   CXX compiler ........................ : ${CMAKE_CXX_COMPILER}")
message(STATUS "")
message(STATUS "                                 OpenSSL : v${OPENSSL_VERSION}")
message(STATUS "                                    asio : v${ASIO_VERSION_FULL}")
message(STATUS "                         Iconv(libiconv) : v${Iconv_VERSION}")
message(STATUS "                                     fmt : v${fmt_VERSION}")
message(STATUS "                                    zlib : v${ZLIB_VERSION}")
message(STATUS "                                    inja : v${inja_VERSION}")
message(STATUS "                           nlohmann_json : v${nlohmann_json_VERSION}")
message(STATUS "                                     xtl : v${xtl_VERSION}")
message(STATUS "                                   xsimd : v${xsimd_VERSION}")
message(STATUS "                                 xtensor : v${xtensor_VERSION}")
message(STATUS "  BS::thread_pool(bshoshany-thread-pool) : v${BSHOSHANY_THREAD_POOL_VERSION}")
message(STATUS "                                 croncpp : v${croncpp_VERSION}")
message(STATUS "            tsl::robin_map(robin-map)(√) : v${tsl-robin-map_VERSION}")
message(STATUS "                                argparse : v${argparse_VERSION}")
#message(STATUS "                               RapidJSON : v${RapidJSON_VERSION}")
message(STATUS "                                yaml-cpp : v${yaml-cpp_VERSION}")
message(STATUS "                                Protobuf : v${Protobuf_VERSION}")
message(STATUS "                             Cap'n Proto : v${CapnProto_VERSION}")
message(STATUS "                                mimalloc : v${mimalloc_VERSION}")
message(STATUS "                                  spdlog : v${spdlog_VERSION}")
message(STATUS "     C++ Requests Library (Cpr for curl) : v${cpr_VERSION}")
#message(STATUS "                   unofficial-duktape(√) : v${unofficial-duktape_VERSION}")
message(STATUS "                               benchmark : v${benchmark_VERSION}")
message(STATUS "                                  Catch2 : v${Catch2_VERSION}")
message(STATUS "                                   GTest : v${GTest_VERSION}")

#message(STATUS "quant1x found: ${quant1x_FOUND}")
#message(STATUS "quant1x version: ${quant1x_VERSION}")
#message(STATUS "quant1x include dirs: ${quant1x_INCLUDE_DIRS}")
#message(STATUS "quant1x library dirs: ${quant1x_LIBRARY_DIRS}")
#get_target_property(quant1x_opts quant1x COMPILE_OPTIONS)
#get_target_property(quant1x_defs quant1x COMPILE_DEFINITIONS)
#message(STATUS "继承的编译选项: ${quant1x_opts}")
#message(STATUS "继承的宏定义: ${quant1x_defs}")

# 检查编译器选项
## Query the `quant1x` target so we see the aggregated includes/libs/defines
## (global_compile_options typically only carries compile flags/options).
get_target_property(compile_includes quant1x INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(compile_libs     quant1x INTERFACE_LINK_LIBRARIES)
get_target_property(compile_defines  quant1x INTERFACE_COMPILE_DEFINITIONS)
get_target_property(compile_options  global_compile_options INTERFACE_COMPILE_OPTIONS)
get_target_property(link_options     global_compile_options INTERFACE_LINK_OPTIONS)

# If `quant1x` has no INTERFACE_COMPILE_DEFINITIONS, collect from common
# providers (third_libs, global_compile_options) and any local definitions.
if(compile_defines STREQUAL "compile_defines-NOTFOUND" OR "${compile_defines}" STREQUAL "")
    set(_collected_defs "")
    get_target_property(_third_defs third_libs INTERFACE_COMPILE_DEFINITIONS)
    if(NOT _third_defs STREQUAL "_third_defs-NOTFOUND")
        list(APPEND _collected_defs ${_third_defs})
    endif()
    get_target_property(_global_defs global_compile_options INTERFACE_COMPILE_DEFINITIONS)
    if(NOT _global_defs STREQUAL "_global_defs-NOTFOUND")
        list(APPEND _collected_defs ${_global_defs})
    endif()
    # Also include any non-interface compile definitions attached directly to quant1x
    get_target_property(_local_defs quant1x COMPILE_DEFINITIONS)
    if(NOT _local_defs STREQUAL "_local_defs-NOTFOUND")
        list(APPEND _collected_defs ${_local_defs})
    endif()
    list(REMOVE_DUPLICATES _collected_defs)
    if(_collected_defs)
        string(REPLACE ";" ";" compile_defines "${_collected_defs}")
    endif()
endif()

message(STATUS "quant1x (C)Includes: ${compile_includes}")
message(STATUS "quant1x (C)    Libs: ${compile_libs}")
message(STATUS "quant1x (C) Defines: ${compile_defines}")
message(STATUS "quant1x (C) Options: ${compile_options}")
message(STATUS "quant1x (L) Options: ${link_options}")

function(get_all_linked_targets target result_var)
    set(result "")

    # 获取该目标直接链接的库
    get_target_property(libs ${target} LINK_LIBRARIES)

    foreach(lib IN LISTS libs)
        if(TARGET ${lib})
            list(APPEND result ${lib})
            # 递归处理子依赖
            get_all_linked_targets(${lib} sub_result)
            list(APPEND result ${sub_result})
        endif()
    endforeach()

    list(REMOVE_DUPLICATES result)
    set(${result_var} ${result} PARENT_SCOPE)
endfunction()

# 使用函数
get_all_linked_targets(quant1x ALL_LINKED_TARGETS)
message(STATUS "All linked targets (recursive): ${ALL_LINKED_TARGETS}")

# 清除DLL文件的前缀
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_IMPORT_LIBRARY_PREFIX "")
# 为静态库和动态库分别设置目标名称，避免冲突
set_target_properties(quant1x PROPERTIES OUTPUT_NAME "quant1x" PREFIX "")

# Generate build-info.h from build-info.h.in
if(ENABLE_MIMALLOC)
    set(HAVE_MIMALLOC 1)
else()
    set(HAVE_MIMALLOC 0)
endif()

set(MIMALLOC_MODE "${MIMALLOC_MODE}")

configure_file(
    "${CMAKE_SOURCE_DIR}/quant1x/build-info.h.in"
    "${CMAKE_BINARY_DIR}/include/private/build-info.h"
)

add_executable(q1x
    main.cpp
)
target_include_directories(q1x
    SYSTEM        # 关键选项：标记为系统头文件路径
    PUBLIC       # 仅当前目标可见（或 PUBLIC/INTERFACE）
    ${CMAKE_SOURCE_DIR}/include  # 头文件目录
    ${CMAKE_SOURCE_DIR}/third_party/include  # 头文件目录
    ${CMAKE_BINARY_DIR}/include  # 私有头文件
)

target_link_libraries(q1x PRIVATE quant1x)

# If user requested global mimalloc (override C allocator), link mimalloc
# directly into the q1x executable to ensure it is initialized as early as
# possible on Windows (prefer static target if available).
if(ENABLE_MIMALLOC AND MIMALLOC_MODE STREQUAL "global")
    # prefer static target when available (vcpkg provides mimalloc-static)
    # Prefer the static target when available. Also define MI_OVERRIDE so
    # mimalloc will attempt to override the C allocator at runtime.
    target_link_libraries(q1x PRIVATE $<IF:$<TARGET_EXISTS:mimalloc-static>,mimalloc-static,mimalloc>)
    target_compile_definitions(q1x PRIVATE $<$<STREQUAL:${MIMALLOC_MODE},global>:MI_OVERRIDE=1>)
    message(STATUS "q1x: linking mimalloc into executable to enable global override")

    # On MSVC we need to force the linker to pull the mimalloc object that
    # performs early initialization. We use the anchor symbol emitted by
    # quant1x/platform/mimalloc_force.cpp and force its inclusion via
    # /INCLUDE:quant1x_mimalloc_force_symbol. This helps ensure mimalloc's
    # redirect code runs before CRT setup.
    if(MSVC)
        # Use a link option that is only applied on MSVC
        target_link_options(q1x PRIVATE "/INCLUDE:quant1x_mimalloc_force_symbol")
    endif()
endif()


# Add optional mimalloc_check utility to validate runtime behavior
if(EXISTS "${CMAKE_SOURCE_DIR}/tools/mimalloc_check.cpp" AND ENABLE_MIMALLOC_CHECK)
    add_executable(mimalloc_check tools/mimalloc_check.cpp)
    target_include_directories(mimalloc_check PRIVATE ${CMAKE_SOURCE_DIR})
    if(ENABLE_MIMALLOC)
        # link mimalloc into the check binary so it can exercise the allocator
        target_link_libraries(mimalloc_check PRIVATE $<IF:$<TARGET_EXISTS:mimalloc-static>,mimalloc-static,mimalloc>)
        target_compile_definitions(mimalloc_check PRIVATE $<$<STREQUAL:${MIMALLOC_MODE},global>:MI_OVERRIDE=1>)
    endif()
    message(STATUS "mimalloc_check: added (ENABLE_MIMALLOC_CHECK=ON). Build with --target mimalloc_check")
endif()

# 设置安装路径变量
include(GNUInstallDirs)
MESSAGE(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
set(THIRD_PARTY_INSTALL_INCDIR "${CMAKE_INSTALL_INCLUDEDIR}/third_party")

# 设置编译模式
string(TOLOWER ${quant1x_build_type} lib_build_type)
# 设置编译器
string(TOLOWER ${CMAKE_CXX_COMPILER_ID} lib_compiler)
# 设置 Debug 版本后缀, 并标记为高级选项(默认不显示在 CMake GUI 中)
set(lib_suffix "-${lib_build_type}-${lib_compiler}" CACHE STRING "Default debug/release postfix")
if (WIN32)
    set(lib_suffix "${lib_suffix}-win")
elseif(APPLE)
    set(lib_suffix "${lib_suffix}-darwin")
endif ()
set(quant1x_DEBUG_POSTFIX ${lib_suffix})
message(STATUS ${quant1x_DEBUG_POSTFIX})

mark_as_advanced(quant1x_DEBUG_POSTFIX)
set_target_properties(quant1x PROPERTIES
    DEBUG_POSTFIX "${quant1x_DEBUG_POSTFIX}"
)
# 将第三方头文件声明为安装目标
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party/include/
    DESTINATION ${THIRD_PARTY_INSTALL_INCDIR}
    #FILES_MATCHING
    #PATTERN "*.h"
    #PATTERN "*.hpp"
    #PATTERN "*.inl"
)

# 安装配置（可选）
install(TARGETS quant1x third_libs xtensor_optimize global_compile_options EXPORT quant1x
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

# 安装主库头文件
install(
    DIRECTORY quant1x/
    DESTINATION include/quant1x
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    PATTERN "*.inl"
    PATTERN "__pycache__" EXCLUDE  # 忽略 Python 缓存目录（安全防护）
    PATTERN ".git" EXCLUDE         # 可选：忽略 .git 目录
)

# 安装主库头文件
install(
    DIRECTORY learn/
    DESTINATION include/learn
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    PATTERN "*.inl"
    PATTERN "__pycache__" EXCLUDE  # 忽略 Python 缓存目录（安全防护）
    PATTERN ".git" EXCLUDE         # 可选：忽略 .git 目录
)

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/include/private/
    DESTINATION include/quant1x
)

# 安装 CMake 配置文件
install(EXPORT quant1x
        FILE quant1x.cmake
        NAMESPACE quant1x::
        DESTINATION lib/cmake/quant1x)

include(CMakePackageConfigHelpers)

# 生成版本文件
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/quant1x-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(EXPORT quant1x
    FILE quant1x-targets.cmake
    NAMESPACE quant1x::
    DESTINATION lib/cmake/quant1x
)

# 安装配置文件
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/quant1x-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/quant1x-config.cmake"
    INSTALL_DESTINATION lib/cmake/quant1x
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/quant1x-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/quant1x-config-version.cmake"
    DESTINATION lib/cmake/quant1x
)

install(TARGETS q1x
    DESTINATION bin)
# 单元测试
# Ensure testing is enabled at the top-level so that add_test calls in
# subdirectories (e.g. `tests/`) are properly registered with CTest and
# a top-level CTestTestfile.cmake is generated.
include(CTest)
enable_testing()

add_subdirectory(tests)
# 演示代码
add_subdirectory(examples)