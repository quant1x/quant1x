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
# 检测 确定使用哪个编译器
# ============================================================
if (WIN32)
    # Windows 下强制使用 GCC（MinGW）
    #set(CMAKE_C_COMPILER "gcc")
    #set(CMAKE_CXX_COMPILER "g++")
elseif (UNIX AND NOT APPLE)
    # Linux 下默认 GCC
    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_CXX_COMPILER "g++")
elseif (APPLE)
    # macOS 下强制使用 Clang
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
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
