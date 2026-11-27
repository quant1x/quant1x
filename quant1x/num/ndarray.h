// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_NDARRAY_H
#define QUANT1X_NUM_NDARRAY_H 1

/// ndarray —— 对标 numpy.ndarray 的一维可移植容器
///
/// 与 quant1x/num/ndarray.py、ndarray.rs 的方法表一致: 存 std::vector<double> 或
/// std::vector<bool>, 算子全部委托 quant1x::num 的自由函数。
///
/// 命名差异: Python 的 `std` 方法在 C++ 命名为 `stddev`(避免遮蔽 `std::` 命名空间)。

#include <quant1x/num/dtype.h>
#include <quant1x/num/elementwise.h>
#include <quant1x/num/ewm.h>
#include <quant1x/num/fillna.h>
#include <quant1x/num/logic.h>
#include <quant1x/num/matrix.h>
#include <quant1x/num/reduce.h>
#include <quant1x/num/rolling.h>
#include <quant1x/num/shift.h>
#include <quant1x/num/window.h>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace quant1x::num {

/// 一维数组: 浮点或布尔
class NdArray {
  public:
    NdArray() = default;

    explicit NdArray(std::vector<double> values)
        : is_bool_(false), floats_(std::move(values)) {}

    explicit NdArray(std::vector<bool> values) : is_bool_(true), bools_(std::move(values)) {}

    // -- 构造 -------------------------------------------------------------
    static NdArray zeros(std::size_t length) { return NdArray(std::vector<double>(length, 0.0)); }

    static NdArray ones(std::size_t length) { return NdArray(std::vector<double>(length, 1.0)); }

    static NdArray full(std::size_t length, double value) {
        return NdArray(std::vector<double>(length, value));
    }

    static NdArray arange(double start, double stop, double step) {
        return NdArray(quant1x::num::arange(start, stop, step));
    }

    static NdArray linspace(double start, double stop, std::size_t count) {
        return NdArray(quant1x::num::linspace(start, stop, count));
    }

    // -- 属性 -------------------------------------------------------------
    bool is_bool() const { return is_bool_; }

    const char *dtype() const { return is_bool_ ? "bool" : "float64"; }

    std::size_t size() const { return is_bool_ ? bools_.size() : floats_.size(); }

    bool empty() const { return size() == 0; }

    std::size_t ndim() const { return 1; }

    std::vector<std::size_t> shape() const { return {size()}; }

    /// 收敛为 float64 序列(布尔转 0/1)
    std::vector<double> to_vector() const {
        if (!is_bool_) {
            return floats_;
        }
        std::vector<double> out(bools_.size());
        for (std::size_t index = 0; index < bools_.size(); ++index) {
            out[index] = bools_[index] ? 1.0 : 0.0;
        }
        return out;
    }

    /// 收敛为布尔序列(非零且非 NaN 为真)
    std::vector<bool> to_bools() const {
        if (is_bool_) {
            return bools_;
        }
        std::vector<bool> out(floats_.size());
        for (std::size_t index = 0; index < floats_.size(); ++index) {
            out[index] = is_true(floats_[index]);
        }
        return out;
    }

    double get_f64(std::size_t index) const {
        return is_bool_ ? (bools_[index] ? 1.0 : 0.0) : floats_[index];
    }

    bool get_bool(std::size_t index) const {
        return is_bool_ ? bools_[index] : is_true(floats_[index]);
    }

    void set_f64(std::size_t index, double value) {
        if (!is_bool_ && index < floats_.size()) {
            floats_[index] = value;
        }
    }

    void set_bool(std::size_t index, bool value) {
        if (is_bool_ && index < bools_.size()) {
            bools_[index] = value;
        }
    }

    NdArray slice(std::size_t start, std::size_t end) const {
        if (is_bool_) {
            const std::size_t stop = end < bools_.size() ? end : bools_.size();
            const std::size_t begin = start < stop ? start : stop;
            return NdArray(std::vector<bool>(bools_.begin() + static_cast<std::ptrdiff_t>(begin),
                                             bools_.begin() + static_cast<std::ptrdiff_t>(stop)));
        }
        const std::size_t stop = end < floats_.size() ? end : floats_.size();
        const std::size_t begin = start < stop ? start : stop;
        return NdArray(std::vector<double>(floats_.begin() + static_cast<std::ptrdiff_t>(begin),
                                           floats_.begin() + static_cast<std::ptrdiff_t>(stop)));
    }

    NdArray copy() const { return *this; }

    NdArray astype(const std::string &target) const {
        if (target == "bool") {
            return NdArray(to_bools());
        }
        if (target == "float64") {
            return NdArray(to_vector());
        }
        throw std::invalid_argument("不支持的 dtype: " + target);
    }

    // -- 四则 -------------------------------------------------------------
    NdArray add(const NdArray &other) const {
        return NdArray(quant1x::num::add(to_vector(), other.to_vector()));
    }
    NdArray sub(const NdArray &other) const {
        return NdArray(quant1x::num::sub(to_vector(), other.to_vector()));
    }
    NdArray mul(const NdArray &other) const {
        return NdArray(quant1x::num::mul(to_vector(), other.to_vector()));
    }
    NdArray div(const NdArray &other) const {
        return NdArray(quant1x::num::div(to_vector(), other.to_vector()));
    }
    NdArray add_scalar(double value) const {
        return NdArray(quant1x::num::add_scalar(to_vector(), value));
    }
    NdArray sub_scalar(double value) const {
        return NdArray(quant1x::num::sub_scalar(to_vector(), value));
    }
    NdArray mul_scalar(double value) const {
        return NdArray(quant1x::num::mul_scalar(to_vector(), value));
    }
    NdArray div_scalar(double value) const {
        return NdArray(quant1x::num::div_scalar(to_vector(), value));
    }
    NdArray maximum(const NdArray &other) const {
        return NdArray(quant1x::num::maximum(to_vector(), other.to_vector()));
    }
    NdArray minimum(const NdArray &other) const {
        return NdArray(quant1x::num::minimum(to_vector(), other.to_vector()));
    }

    // -- 一元 -------------------------------------------------------------
    NdArray abs() const { return NdArray(quant1x::num::abs(to_vector())); }
    NdArray sqrt() const { return NdArray(quant1x::num::sqrt(to_vector())); }
    NdArray log() const { return NdArray(quant1x::num::log(to_vector())); }
    NdArray pow(double exponent) const {
        return NdArray(quant1x::num::pow(to_vector(), exponent));
    }

    // -- 比较 -------------------------------------------------------------
    NdArray gt(const NdArray &other) const {
        return NdArray(quant1x::num::gt(to_vector(), other.to_vector()));
    }
    NdArray gte(const NdArray &other) const {
        return NdArray(quant1x::num::gte(to_vector(), other.to_vector()));
    }
    NdArray lt(const NdArray &other) const {
        return NdArray(quant1x::num::lt(to_vector(), other.to_vector()));
    }
    NdArray lte(const NdArray &other) const {
        return NdArray(quant1x::num::lte(to_vector(), other.to_vector()));
    }
    NdArray equal(const NdArray &other) const {
        return NdArray(quant1x::num::equal(to_vector(), other.to_vector()));
    }
    NdArray not_equal(const NdArray &other) const {
        return NdArray(quant1x::num::not_equal(to_vector(), other.to_vector()));
    }

    NdArray gt_scalar(double value) const { return compare_scalar(value, quant1x::num::gt); }
    NdArray lt_scalar(double value) const { return compare_scalar(value, quant1x::num::lt); }
    NdArray equal_scalar(double value) const { return compare_scalar(value, quant1x::num::equal); }

    // -- 逻辑 -------------------------------------------------------------
    NdArray logic_and(const NdArray &other) const {
        return NdArray(quant1x::num::logic_and(to_bools(), other.to_bools()));
    }
    NdArray logic_or(const NdArray &other) const {
        return NdArray(quant1x::num::logic_or(to_bools(), other.to_bools()));
    }
    NdArray logic_not() const { return NdArray(quant1x::num::logic_not(to_bools())); }

    // -- 归约 -------------------------------------------------------------
    double sum() const { return quant1x::num::sum_value(to_vector()); }
    double mean() const { return quant1x::num::mean(to_vector()); }
    double median() const { return quant1x::num::median(to_vector()); }
    double min_value() const { return quant1x::num::min_value(to_vector()); }
    double max_value() const { return quant1x::num::max_value(to_vector()); }
    std::int64_t argmin() const { return quant1x::num::argmin(to_vector()); }
    std::int64_t argmax() const { return quant1x::num::argmax(to_vector()); }
    double variance() const { return quant1x::num::variance(to_vector()); }
    /// 总体标准差(Python 的 std)
    double stddev() const { return quant1x::num::stddev(to_vector()); }
    NdArray cumsum() const { return NdArray(quant1x::num::cumsum(to_vector())); }

    // -- 移位 / 差分 / 填充 ----------------------------------------------
    NdArray shift(std::int64_t periods) const {
        return NdArray(quant1x::num::shift(to_vector(), periods));
    }
    NdArray diff(std::int64_t periods) const {
        return NdArray(quant1x::num::diff(to_vector(), periods));
    }
    NdArray fill_na(double value) const { return NdArray(quant1x::num::fill(to_vector(), value)); }

    // -- 滚动 / EWM -------------------------------------------------------
    NdArray rolling_sum(std::int64_t window) const {
        return NdArray(quant1x::num::rolling_sum(to_vector(), Window(window)));
    }
    NdArray rolling_mean(std::int64_t window) const {
        return NdArray(quant1x::num::rolling_mean(to_vector(), Window(window)));
    }
    NdArray rolling_std(std::int64_t window) const {
        return NdArray(quant1x::num::rolling_std(to_vector(), Window(window)));
    }
    NdArray rolling_min(std::int64_t window) const {
        return NdArray(quant1x::num::rolling_min(to_vector(), Window(window)));
    }
    NdArray rolling_max(std::int64_t window) const {
        return NdArray(quant1x::num::rolling_max(to_vector(), Window(window)));
    }
    NdArray rolling_avedev(std::int64_t window) const {
        return NdArray(quant1x::num::rolling_avedev(to_vector(), Window(window)));
    }

    template <typename Apply>
    NdArray rolling_apply(std::int64_t window, Apply apply) const {
        return NdArray(quant1x::num::rolling_apply(to_vector(), Window(window), apply));
    }

    NdArray ewm_mean(double alpha) const {
        return NdArray(quant1x::num::ewm_mean(to_vector(), alpha));
    }

  private:
    NdArray compare_scalar(double value, Bools (*op)(const Series &, const Series &)) const {
        const auto left = to_vector();
        return NdArray(op(left, std::vector<double>(left.size(), value)));
    }

    bool is_bool_ = false;
    std::vector<double> floats_;
    std::vector<bool> bools_;
};

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_NDARRAY_H
