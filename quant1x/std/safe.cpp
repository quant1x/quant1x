#include <quant1x/std/safe.h>

namespace q1x {

    namespace safe {
        std::tm localtime(std::time_t t) noexcept {
            std::tm result{};
#ifdef _WIN32
            localtime_s(&result, &t);
#else
            localtime_r(&t, &result);
#endif
            return result;
        }

        std::tm gmtime(std::time_t t) noexcept {
            std::tm result{};
#ifdef _WIN32
            gmtime_s(&result, &t);
#else
            gmtime_r(&t, &result);
#endif
            return result;
        }
    }  // namespace safe
    
}  // namespace q1x