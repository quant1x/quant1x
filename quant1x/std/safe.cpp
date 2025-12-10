#include <quant1x/std/safe.h>

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
}  // namespace safe