#pragma once
#ifndef QUANT1X_BASE_SAFE_H
#define QUANT1X_BASE_SAFE_H 1

#include <cerrno>   // C标准库errno
#include <cstddef>  // size_t
#include <cstdio>   // snprintf
#include <cstdint>  // int64_t
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

    // 短时休眠, 参数单位为微秒.
    //
    // Windows 上**不要**用 std::this_thread::sleep_for 做亚毫秒级休眠:
    // MSVC 的实现会把短暂时长向上取整为 Sleep(1), 而 Sleep(1) 受系统默认 15.6ms
    // 定时器粒度支配 —— 实测 sleep_for(50us) 单次实际耗时 15.57ms, 是名义值的
    // 311 倍. Rust 的 thread::sleep(50us) 实测 0.55ms, 两者相差 28 倍.
    //
    // 该差异对无锁算法的退避路径是致命的: Vyukov 队列在连续 8 次 CAS 竞争失败后
    // 进入第三级退避(名义 50us), C++ 侧每次退避实际停摆 15.6ms, 8 生产者场景下
    // 吞吐从 23.1M/s 跌到 1.1M/s(实测, 相差 21 倍), 而 Rust 侧曲线保持平坦.
    //
    // 本函数在 Windows 上改用 CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 可等待定时器
    // (实测单次 0.53ms, 与 Rust 一致); 其他平台退化为 std::this_thread::sleep_for.
    // 定时器句柄每次调用创建并关闭: 复用句柄需 thread_local 且多线程共享同一句柄
    // 会互相干扰, 而实测创建开销仅 14us(548us vs 534us), 相对休眠本身可忽略.
    //
    // 该常量需 Windows 10 1803+; 创建失败时回退到 std::this_thread::sleep_for.
    void sleep_for_microseconds(uint64_t us) noexcept;
}  // namespace safe

#endif  // QUANT1X_BASE_SAFE_H