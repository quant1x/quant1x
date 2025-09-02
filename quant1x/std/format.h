#pragma once
#ifndef QUANT1X_STD_FORMAT_H
#define QUANT1X_STD_FORMAT_H 1

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <cstddef>

// 检测方法是否存在 ---
template <typename T, typename = void>
struct has_to_string : std::false_type {};
template <typename T>
struct has_to_string<T, std::void_t<decltype(std::declval<const T&>().to_string())>>
    : std::true_type {};

template <typename T, typename = void>
struct has_toString : std::false_type {};
template <typename T>
struct has_toString<T, std::void_t<decltype(std::declval<const T&>().toString())>>
    : std::true_type {};

template <typename T, typename = void>
struct has_ToString : std::false_type {};
template <typename T>
struct has_ToString<T, std::void_t<decltype(std::declval<const T&>().ToString())>>
    : std::true_type {};

// 优先调用字符串方法的输出适配器
template <typename T>
void outputWithPriority(std::ostream& os, const T& value) {
    if constexpr (has_to_string<T>::value) {
        os << value.to_string();
    }
    else if constexpr (has_toString<T>::value) {
        os << value.toString();
    }
    else if constexpr (has_ToString<T>::value) {
        os << value.ToString();
    }
    else {
        os << value; // 回退到常规 << 运算符
    }
}

// 您原有的 << 重载（修改版）
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
    os << "[";
    for (std::size_t i = 0; i < vec.size(); ++i) {
        outputWithPriority(os, vec[i]); // 关键修改点
        if (i != vec.size() - 1) os << ", ";
    }
    os << "]";
    return os;
}

#endif // QUANT1X_STD_FORMAT_H
