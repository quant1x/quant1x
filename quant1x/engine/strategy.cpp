#include "strategy.h"

std::string StrategyInfo::QmtStrategyName() const {
    std::string result = "S";
    char buffer[20];  // 最大支持 uint64_t 字符数（如 18446744073709551615）

    auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), Code());
    if (ec == std::errc()) {
        result.append(buffer, ptr - buffer);
    } else {
        // 错误处理（可选），例如抛出异常或返回空字符串
        result += "invalid";
    }

    return result;
}
