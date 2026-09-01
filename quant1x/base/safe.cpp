#include <quant1x/base/safe.h>
#include <chrono>
#include <cstdlib>
#include <thread>

#if defined(_MSC_VER) || defined(__MINGW32__)
// Windows CRT 没有 aligned_alloc: _aligned_malloc / __mingw_aligned_malloc
// 的声明在 <malloc.h> 中 (分别对应 MSVC 与 MinGW-w64).
#  include <malloc.h>
#endif

#if defined(_WIN32)
// 仅在本 .cpp 内包含 windows.h: 避免其宏(min/max 等)泄漏到所有包含 safe.h 的
// 翻译单元 —— safe.h 被全项目广泛包含, 泄漏会破坏大量调用点.
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

namespace safe {
    std::tm localtime(std::time_t t) noexcept {
        std::tm result{};
#ifdef _WIN32
        if (localtime_s(&result, &t) != 0) {
            result = {};
        }
#else
        if (localtime_r(&t, &result) == nullptr) {
            result = {};
        }
#endif
        return result;
    }

    std::tm gmtime(std::time_t t) noexcept {
        std::tm result{};
#ifdef _WIN32
        if (gmtime_s(&result, &t) != 0) {
            result = {};
        }
#else
        if (gmtime_r(&t, &result) == nullptr) {
            result = {};
        }
#endif
        return result;
    }

    std::optional<std::string> getenv(const char *name) {
#ifdef _WIN32
        char  *value = nullptr;
        size_t len   = 0;
        if (_dupenv_s(&value, &len, name) == 0 && value != nullptr) {
            std::string result(value);
            free(value);
            return result;
        }
        return std::nullopt;
#else
        const char *value = std::getenv(name);
        if (value) {
            return std::string(value);
        }
        return std::nullopt;
#endif
    }

    void* aligned_alloc(size_t alignment, size_t size) noexcept {
        // 注意 __mingw_aligned_malloc / _aligned_malloc 的参数顺序是 (size, alignment),
        // 与 std::aligned_alloc 的 (alignment, size) 相反, 这里统一为 (alignment, size).
#if defined(_MSC_VER)
        return _aligned_malloc(size, alignment);
#elif defined(__MINGW32__)
        return __mingw_aligned_malloc(size, alignment);
#else
        return std::aligned_alloc(alignment, size);
#endif
    }

    void aligned_free(void* p) noexcept {
#if defined(_MSC_VER)
        _aligned_free(p);
#elif defined(__MINGW32__)
        __mingw_aligned_free(p);
#else
        std::free(p);
#endif
    }

    void sleep_for_microseconds(uint64_t us) noexcept {
        // CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 需 Windows 10 1803+ 的 SDK 声明;
        // 该常量由 <windows.h> 按 _WIN32_WINNT 提供, 若目标 SDK 过旧(或未定义
        // _WIN32_WINNT 导致取到低默认值)则缺失, 此时编译期退化为 sleep_for ——
        // 语义不变, 只是精度回落到 15.6ms 粒度.
#if defined(_WIN32) && defined(CREATE_WAITABLE_TIMER_HIGH_RESOLUTION)
        if (us == 0) {
            return;
        }
        // 高精度可等待定时器: 绕开系统默认 15.6ms 粒度(详见 safe.h 中的说明).
        // 需要 Windows 10 1803+; 旧系统返回 NULL, 回退到 std::this_thread::sleep_for.
        HANDLE timer = ::CreateWaitableTimerExW(nullptr, nullptr,
                                               CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
                                               TIMER_ALL_ACCESS);
        if (timer == nullptr) {
            std::this_thread::sleep_for(std::chrono::microseconds(us));
            return;
        }
        LARGE_INTEGER due;
        // 负值表示相对时间, 单位为 100ns; 溢出防护: us 上限取 1 小时
        constexpr uint64_t kMaxUs = 3600ULL * 1000ULL * 1000ULL;
        if (us > kMaxUs) {
            us = kMaxUs;
        }
        due.QuadPart = -static_cast<LONGLONG>(us * 10ULL);
        ::SetWaitableTimer(timer, &due, 0, nullptr, nullptr, FALSE);
        ::WaitForSingleObject(timer, INFINITE);
        ::CloseHandle(timer);
#else
        std::this_thread::sleep_for(std::chrono::microseconds(us));
#endif
    }
}  // namespace safe