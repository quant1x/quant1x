#pragma once
#ifndef QUANT1X_BASE_SAFE_H
#define QUANT1X_BASE_SAFE_H 1

#include <cerrno>   // C标准库errno
#include <cstddef>  // size_t
#include <cstdio>   // snprintf
#include <cstring>  // C标准库字符串函数
#include <ctime>
#include <string>
#include <optional>

namespace safe {
    // 安全的 localtime 函数, 避免线程不安全
    std::tm localtime(std::time_t t) noexcept;

    // 安全的 gmtime 函数, 避免线程不安全
    std::tm gmtime(std::time_t t) noexcept;

    // 跨平台安全获取环境变量
    std::optional<std::string> getenv(const char *name);

    inline std::string strerror(int errnum) {
        constexpr size_t buf_size = 256;
        std::string      buf(buf_size, '\0');

#if defined(_WIN32) || defined(_MSC_VER)
        if (strerror_s(&buf[0], buf.size(), errnum) != 0) {
            std::snprintf(&buf[0], buf.size(), "Unknown error %d", errnum);
        }
#elif defined(__APPLE__) || ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE)
        if (strerror_r(errnum, &buf[0], buf.size()) != 0) {
            std::snprintf(&buf[0], buf.size(), "Unknown error %d", errnum);
        }
#else
        char *msg = strerror_r(errnum, &buf[0], buf.size());
        if (msg != &buf[0]) {
            std::strncpy(&buf[0], msg, buf.size());
            buf[buf.size() - 1] = '\0';
        }
#endif

        buf.resize(std::strlen(buf.c_str()));
        return buf;
    }

    // 按指定对齐分配内存, 参数顺序与 std::aligned_alloc 一致 (alignment, size).
    // 跨平台差异收敛(Windows CRT 不提供 aligned_alloc):
    //   - MSVC:    _aligned_malloc / _aligned_free
    //   - MinGW-w64: __mingw_aligned_malloc / __mingw_aligned_free
    //   - 其余(glibc / macOS arm64 g++ 等): std::aligned_alloc / std::free
    // 失败返回 nullptr(不抛异常), 分配/释放必须成对使用, 混用属 UB.
    void* aligned_alloc(size_t alignment, size_t size) noexcept;

    // 释放 aligned_alloc 分配的内存, 与分配函数成对匹配(见 aligned_alloc 说明)
    void aligned_free(void* p) noexcept;
}  // namespace safe

#endif  // QUANT1X_BASE_SAFE_H