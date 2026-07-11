#include <quant1x/base/safe.h>
#include <cstdlib>

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
}  // namespace safe