# ==============================
# xsimd / xtensor / xtl —— 由 Conan 提供（conanfile.py 声明）
# 不再随 third_party 分发，避免仓库内嵌一份与 Conan 版本漂移的副本。
# ==============================
find_package(xsimd CONFIG REQUIRED)
find_package(xtensor CONFIG REQUIRED)
find_package(xtl CONFIG REQUIRED)

if (NOT xsimd_VERSION)
    set(xsimd_VERSION "unknown")
endif ()
if (NOT xtensor_VERSION)
    set(xtensor_VERSION "unknown")
endif ()
if (NOT xtl_VERSION)
    set(xtl_VERSION "unknown")
endif ()
echo_lib_version(xsimd ${xsimd_VERSION})
echo_lib_version(xtensor ${xtensor_VERSION})
echo_lib_version(xtl ${xtl_VERSION})

# AVX2
if (MSVC)
    # Microsoft compiler
    # AVX2 for Release
    string(APPEND CMAKE_C_FLAGS_RELEASE " /arch:AVX2")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " /arch:AVX2")

    # AVX for Debug
    string(APPEND CMAKE_C_FLAGS_DEBUG " /arch:AVX")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /arch:AVX")
else ()
    # gcc, clang, ...
    # arm64: NEON is auto-enabled by -march=native; AVX flags are x86-only
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -march=native -mtune=native")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native -mtune=native")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -march=native -mtune=native")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -mtune=native")
    else ()
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mavx -mavx2 -march=native -mtune=native")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mavx -mavx2 -march=native -mtune=native")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mavx -mavx2 -march=native -mtune=native")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mavx -mavx2 -march=native -mtune=native")
    endif ()
endif ()

# Conan 的 CMakeDeps 可能生成带命名空间或不带命名空间的两套 target，二者都接受
if (TARGET xsimd::xsimd)
    set(_xsimd_target xsimd::xsimd)
elseif (TARGET xsimd)
    set(_xsimd_target xsimd)
else ()
    message(FATAL_ERROR "xsimd: 未找到可用的 CMake target")
endif ()

if (TARGET xtensor::xtensor)
    set(_xtensor_target xtensor::xtensor)
elseif (TARGET xtensor)
    set(_xtensor_target xtensor)
else ()
    message(FATAL_ERROR "xtensor: 未找到可用的 CMake target")
endif ()

add_library(xtensor_optimize INTERFACE)
if(MSVC)
    target_compile_options(xtensor_optimize INTERFACE /EHsc)
    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(global_compile_options INTERFACE
            /MP           # 多进程编译
        )
    endif ()
    target_compile_options(xtensor_optimize INTERFACE /bigobj)
else()
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG(-march=native arch_native_supported)
    if(arch_native_supported)
        target_compile_options(xtensor_optimize INTERFACE -march=native)
    endif()
endif()

target_link_libraries(third_libs INTERFACE ${_xtensor_target} ${_xsimd_target} xtensor_optimize)
