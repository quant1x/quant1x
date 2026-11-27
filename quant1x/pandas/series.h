// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_PANDAS_SERIES_H
#define QUANT1X_PANDAS_SERIES_H 1

/// Series —— 一列同构数据的可移植容器
///
/// 对齐 quant1x/pandas/series.py：Python 用 pandas 承载是 spec，本类以
/// quant1x::num::NdArray 为底座（float64 / bool），数值算子委托 quant1x::num。
///
/// 与 Python 的命名差异：`std` 方法在 C++ 叫 `stddev`（避免遮蔽 `std::`）。

#include <quant1x/num/ndarray.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace quant1x::pandas {

/// 滚动窗口
class Rolling;

/// 指数加权窗口
class Ewm;

/// 一列同构数据：值 + 名称
class Series {
  public:
    Series() = default;

    explicit Series(std::vector<double> values) : data_(std::move(values)) {}

    explicit Series(std::vector<bool> values) : data_(std::move(values)) {}

    Series(std::vector<double> values, std::string name)
        : data_(std::move(values)), name_(std::move(name)) {}

    Series(std::vector<bool> values, std::string name)
        : data_(std::move(values)), name_(std::move(name)) {}

    Series(num::NdArray data, std::string name) : data_(std::move(data)), name_(std::move(name)) {}

    // -- 属性 -------------------------------------------------------------
    std::size_t size() const { return data_.size(); }

    bool empty() const { return data_.empty(); }

    bool is_bool() const { return data_.is_bool(); }

    const char *dtype() const { return data_.dtype(); }

    const std::string &name() const { return name_; }

    Series &set_name(std::string name) {
        name_ = std::move(name);
        return *this;
    }

    /// 收敛为 float64 序列（布尔转 0/1）
    std::vector<double> values() const { return data_.to_vector(); }

    /// 收敛为布尔序列
    std::vector<bool> to_bools() const { return data_.to_bools(); }

    double get_f64(std::size_t index) const { return data_.get_f64(index); }

    bool get_bool(std::size_t index) const { return data_.get_bool(index); }

    void set_f64(std::size_t index, double value) { data_.set_f64(index, value); }

    void set_bool(std::size_t index, bool value) { data_.set_bool(index, value); }

    Series copy() const { return Series(data_.copy(), name_); }

    Series slice(std::size_t start, std::size_t end) const {
        return Series(data_.slice(start, end), name_);
    }

    // -- 变换 -------------------------------------------------------------
    Series shift(std::int64_t periods) const { return Series(data_.shift(periods), name_); }

    Series diff(std::int64_t periods) const { return Series(data_.diff(periods), name_); }

    Series cumsum() const { return Series(data_.cumsum(), name_); }

    Series fill_na(double value) const { return Series(data_.fill_na(value), name_); }

    Series apply(const std::function<double(double)> &func) const {
        const auto input = data_.to_vector();
        std::vector<double> out(input.size());
        for (std::size_t index = 0; index < input.size(); ++index) {
            out[index] = func(input[index]);
        }
        return Series(std::move(out), name_);
    }

    // -- 四则 -------------------------------------------------------------
    Series add(const Series &other) const { return Series(data_.add(other.data_), name_); }
    Series sub(const Series &other) const { return Series(data_.sub(other.data_), name_); }
    Series mul(const Series &other) const { return Series(data_.mul(other.data_), name_); }
    Series div(const Series &other) const { return Series(data_.div(other.data_), name_); }
    Series add_scalar(double value) const { return Series(data_.add_scalar(value), name_); }
    Series sub_scalar(double value) const { return Series(data_.sub_scalar(value), name_); }
    Series mul_scalar(double value) const { return Series(data_.mul_scalar(value), name_); }
    Series div_scalar(double value) const { return Series(data_.div_scalar(value), name_); }

    // -- 比较（返回布尔 Series） ------------------------------------------
    Series gt(const Series &other) const { return Series(data_.gt(other.data_), name_); }
    Series gte(const Series &other) const { return Series(data_.gte(other.data_), name_); }
    Series lt(const Series &other) const { return Series(data_.lt(other.data_), name_); }
    Series lte(const Series &other) const { return Series(data_.lte(other.data_), name_); }
    Series equal(const Series &other) const { return Series(data_.equal(other.data_), name_); }
    Series not_equal(const Series &other) const { return Series(data_.not_equal(other.data_), name_); }
    Series gt_scalar(double value) const { return Series(data_.gt_scalar(value), name_); }
    Series lt_scalar(double value) const { return Series(data_.lt_scalar(value), name_); }
    Series equal_scalar(double value) const { return Series(data_.equal_scalar(value), name_); }

    // -- 逻辑 -------------------------------------------------------------
    Series logic_and(const Series &other) const { return Series(data_.logic_and(other.data_), name_); }
    Series logic_or(const Series &other) const { return Series(data_.logic_or(other.data_), name_); }
    Series logic_not() const { return Series(data_.logic_not(), name_); }

    // -- 归约 -------------------------------------------------------------
    double sum() const { return data_.sum(); }
    double mean() const { return data_.mean(); }
    double median() const { return data_.median(); }
    double min_value() const { return data_.min_value(); }
    double max_value() const { return data_.max_value(); }
    std::int64_t argmin() const { return data_.argmin(); }
    std::int64_t argmax() const { return data_.argmax(); }
    double variance() const { return data_.variance(); }
    /// 总体标准差（Python 的 std）
    double stddev() const { return data_.stddev(); }

    // -- 滚动 / EWM -------------------------------------------------------
    Rolling rolling(std::int64_t window) const;

    Ewm ewm(double alpha) const;

    Ewm ewm_span(std::int64_t span) const;

    // -- 运算符糖（映射到上面的方法） ------------------------------------
    Series operator+(const Series &other) const { return add(other); }
    Series operator-(const Series &other) const { return sub(other); }
    Series operator*(const Series &other) const { return mul(other); }
    Series operator/(const Series &other) const { return div(other); }
    Series operator+(double value) const { return add_scalar(value); }
    Series operator-(double value) const { return sub_scalar(value); }
    Series operator*(double value) const { return mul_scalar(value); }
    Series operator/(double value) const { return div_scalar(value); }
    Series operator>(const Series &other) const { return gt(other); }
    Series operator>=(const Series &other) const { return gte(other); }
    Series operator<(const Series &other) const { return lt(other); }
    Series operator<=(const Series &other) const { return lte(other); }
    Series operator==(const Series &other) const { return equal(other); }
    Series operator!=(const Series &other) const { return not_equal(other); }
    Series operator&(const Series &other) const { return logic_and(other); }
    Series operator|(const Series &other) const { return logic_or(other); }
    Series operator~() const { return logic_not(); }

  private:
    friend class Rolling;
    friend class Ewm;

    num::NdArray data_;
    std::string name_;
};

/// 滚动窗口
class Rolling {
  public:
    Rolling(const Series &series, std::int64_t window) : series_(&series), window_(window) {}

    Series sum() const { return Series(series_->data_.rolling_sum(window_), series_->name()); }
    Series mean() const { return Series(series_->data_.rolling_mean(window_), series_->name()); }
    Series stddev() const { return Series(series_->data_.rolling_std(window_), series_->name()); }
    Series min_value() const { return Series(series_->data_.rolling_min(window_), series_->name()); }
    Series max_value() const { return Series(series_->data_.rolling_max(window_), series_->name()); }

    template <typename Apply>
    Series apply(Apply func) const {
        return Series(series_->data_.rolling_apply(window_, func), series_->name());
    }

  private:
    const Series *series_;
    std::int64_t window_;
};

/// 指数加权窗口
class Ewm {
  public:
    Ewm(const Series &series, double alpha) : series_(&series), alpha_(alpha) {}

    Series mean() const { return Series(series_->data_.ewm_mean(alpha_), series_->name()); }

  private:
    const Series *series_;
    double alpha_;
};

inline Rolling Series::rolling(std::int64_t window) const { return Rolling(*this, window); }

inline Ewm Series::ewm(double alpha) const { return Ewm(*this, alpha); }

inline Ewm Series::ewm_span(std::int64_t span) const {
    const double period = span <= 0 ? 1.0 : static_cast<double>(span);
    return Ewm(*this, 2.0 / (period + 1.0));
}

}  // namespace quant1x::pandas

#endif  // QUANT1X_PANDAS_SERIES_H
