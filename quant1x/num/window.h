// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_WINDOW_H
#define QUANT1X_NUM_WINDOW_H 1

/// 窗口参数 —— 常量窗口与逐位置窗口统一成 int64 序列。
/// 无效窗口 (<= 0) 由滚动算子统一返回 NaN。

#include <quant1x/num/dtype.h>

#include <cstdint>
#include <utility>
#include <vector>

namespace quant1x::num {

/// 无效窗口标记
inline constexpr std::int64_t INVALID_WINDOW = -1;

/// 窗口参数: 常量或逐位置序列。
class Window {
  public:
    Window(std::int64_t value) : scalar_(value), series_(), is_scalar_(true) {}
    Window(std::vector<std::int64_t> values)
        : scalar_(INVALID_WINDOW), series_(std::move(values)), is_scalar_(false) {}
    Window(std::vector<double> values);

    [[nodiscard]] bool is_scalar() const { return is_scalar_; }
    [[nodiscard]] std::int64_t scalar() const { return scalar_; }
    [[nodiscard]] const std::vector<std::int64_t> &series() const { return series_; }

  private:
    std::int64_t scalar_;
    std::vector<std::int64_t> series_;
    bool is_scalar_;
};

/// 浮点窗口转 int64: NaN 记为 INVALID_WINDOW。
inline Window::Window(std::vector<double> values) : scalar_(INVALID_WINDOW), is_scalar_(false) {
    series_.reserve(values.size());
    for (const double value : values) {
        series_.push_back(is_nan(value) ? INVALID_WINDOW : static_cast<std::int64_t>(value));
    }
}

/// 把窗口参数归一化成与序列等长的数组。
///
/// 逐位置窗口长度不一致时按位置截断或补 INVALID_WINDOW。
inline std::vector<std::int64_t> as_window(const Window &window, std::size_t length) {
    if (window.is_scalar()) {
        return std::vector<std::int64_t>(length, window.scalar());
    }
    std::vector<std::int64_t> out(length, INVALID_WINDOW);
    const auto &values = window.series();
    const std::size_t count = values.size() < length ? values.size() : length;
    for (std::size_t index = 0; index < count; ++index) {
        out[index] = values[index];
    }
    return out;
}

/// 取某位置的窗口长度。
inline std::int64_t window_at(const Window &window, std::size_t index) {
    if (window.is_scalar()) {
        return window.scalar();
    }
    const auto &values = window.series();
    return index < values.size() ? values[index] : INVALID_WINDOW;
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_WINDOW_H
