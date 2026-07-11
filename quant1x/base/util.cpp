#include <quant1x/base/util.h>

namespace util {

    uint64_t get_thread_id(const std::thread::id &tid) {
        return static_cast<uint64_t>(std::hash<std::thread::id>{}(tid));
    }
} // namespace util