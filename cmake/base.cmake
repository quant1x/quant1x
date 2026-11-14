# CMake policies
cmake_policy(SET CMP0022 NEW)
# On MacOS use @rpath/ for target's install name prefix path
if (POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif ()
# Clear VERSION variables when no VERSION is given to project()
if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()
# MSVC runtime library flags are selected by an abstraction.
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

# ============================================================
# 编译器策略（按平台固定）：
#   Windows -> MSVC（由 Visual Studio 生成器决定）
#   macOS   -> Clang
#   Linux / 其他 Unix -> GCC
# ============================================================
if (WIN32)
    # Windows 统一使用 MSVC，不在 CMake 中改写编译器，
    # 由生成器选择（-G "Visual Studio 17 2022"），见 compile_options.cmake 的校验。
elseif (APPLE)
    # macOS 强制使用 Clang
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
elseif (UNIX)
    # Linux / 其他 Unix 使用 GCC
    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_CXX_COMPILER "g++")
endif ()

# =============================
# 对齐输出宏定义, 可用于中间对齐的KV显示
# =============================
macro(pretty_print_label label value)
    set(max_len 36)
    string(LENGTH "${label}" len)
    math(EXPR padding "${max_len} - ${len}")
    if (padding GREATER 0)
        string(REPEAT " " ${padding} pad_str)
    else ()
        set(pad_str "")
    endif ()
    message(STATUS "${pad_str}${label} : ${value}")
endmacro()

# =============================
# 对齐输出宏定义
# =============================
macro(echo_lib_version label value)
    message("Found ${label}, version = ${value}")
endmacro()
