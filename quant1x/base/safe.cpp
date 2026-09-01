#include <quant1x/base/safe.h>
#include <cstdlib>

#if defined(_MSC_VER) || defined(__MINGW32__)
// Windows CRT 没有 aligned_alloc: _aligned_malloc / __mingw_aligned_malloc
// 的声明在 <malloc.h> 中 (分别对应 MSVC 与 MinGW-w64).
#  include <malloc.h>
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
}  // namespace safe