#pragma once
#ifndef API_BASE_H
#define API_BASE_H 1

#include "feature_detection.h"

// 网络相关头文件（Windows 特定）
#if OS_IS_WINDOWS
#ifndef WINVER
#define WINVER 0x0A00
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>       // 最后包含
#undef max            // for std::max
#undef min            // for std::min
#endif

// 标准库头文件
#include <vector>
#include <string>
#include <memory>
#include <ostream>

#if TARGET_CPP_AT_LEAST(20)
#define std_cplusplus 20
#include <format>
#elif
#define std_cplusplus 17
#endif

#include <fmt/core.h>
#include <tsl/robin_map.h>

#include <chrono>
#include <cstdint>  // uint16_t
#include <cstring>
#include <fstream>
#include <regex>      // std::regex
#include <stdexcept>  // std::invalid_argument, std::out_of_range
#include <tuple> // _, std::ignore
#include <version>
#include <algorithm>
#include <string>
#include <string_view>
#include <functional>
#include <numeric> // std::accumulate
#include <variant>
#include <ranges> // for filter
#include <boost/pfr.hpp>
#include <magic_enum/magic_enum.hpp> // for enum_name

// 基础数据类型简写别名

typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

// 常量定义
constexpr const char *const INVALID_SECURITY_CODE_MSG           = "证券代码长度不足8位";
constexpr const char *const INVALID_DATE_FORMAT_YMD_MSG         = "日期格式非 YYYY-mm-dd";
constexpr const char *const INVALID_DATE_FORMAT_YMD_COMPACT_MSG = "日期格式非 YYYYmmdd";

#define ASSERT(cond, msg)                                                                                   \
    do {                                                                                                    \
        if (!(cond)) {                                                                                      \
            std::cerr << "Assertion failed: " << msg << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            std::abort();                                                                                   \
        }                                                                                                   \
    } while (false)

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
// GCC/Clang 等编译器直接支持 __BYTE_ORDER__
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ENDIAN_LITTLE 1
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define ENDIAN_BIG 1
#endif
#elif OS_IS_WINDOWS
// Windows 系统默认小端
#define ENDIAN_LITTLE 1
#elif defined(__LITTLE_ENDIAN__) || defined(__i386__) || defined(__x86_64__) || defined(__ARMEL__)
// 其他小端架构
#define ENDIAN_LITTLE 1
#elif defined(__BIG_ENDIAN__) || defined(__ARMEB__) || defined(__MIPSEB__)
// 其他大端架构
#define ENDIAN_BIG 1
#else
// 无法确定时，触发编译错误
#error "Unknown endianness"
#endif

#define api_inline FORCE_INLINE

#include "format.h"

#endif  // API_BASE_H
