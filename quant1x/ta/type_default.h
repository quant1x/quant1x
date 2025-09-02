#pragma once
#ifndef QUANT1X_TECHNICAL_ANALYSIS_TYPE_DEFAULT_H
#define QUANT1X_TECHNICAL_ANALYSIS_TYPE_DEFAULT_H 1

#include <cmath>
#include <limits>
#include <string>
#include <cstdint>

namespace ta {

    template<typename T>
    struct TypeDefault {
        static T value() { return T{}; }
    };

    // 特化浮点数为 NaN
    template<>
    struct TypeDefault<float> {
        static float value() { return NAN; }
    };

    template<>
    struct TypeDefault<double> {
        static double value() { return NAN; }
    };

    // 整数类型为 0
    template<>
    struct TypeDefault<int8_t> { static int8_t value() { return 0; } };
    template<>
    struct TypeDefault<uint8_t> { static uint8_t value() { return 0; } };
    template<>
    struct TypeDefault<int16_t> { static int16_t value() { return 0; } };
    template<>
    struct TypeDefault<uint16_t> { static uint16_t value() { return 0; } };
    template<>
    struct TypeDefault<int32_t> { static int32_t value() { return 0; } };
    template<>
    struct TypeDefault<uint32_t> { static uint32_t value() { return 0; } };
    template<>
    struct TypeDefault<int64_t> { static int64_t value() { return 0; } };
    template<>
    struct TypeDefault<uint64_t> { static uint64_t value() { return 0; } };

    // bool 默认 false
    template<>
    struct TypeDefault<bool> { static bool value() { return false; } };

    // string 默认空字符串
    template<>
    struct TypeDefault<std::string> { static std::string value() { return ""; } };

    // 模板函数封装
    template<typename T>
    T type_default() {
        return TypeDefault<T>::value();
    }

} // namespace ta

#endif // QUANT1X_TECHNICAL_ANALYSIS_TYPE_DEFAULT_H