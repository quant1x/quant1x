// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_PANDAS_EXPONENTIAL_MOVING_WINDOWS_H
#define QUANT1X_PANDAS_EXPONENTIAL_MOVING_WINDOWS_H 1

/// 指数加权窗口（com / span / halflife / alpha 四种参数化）
///
/// 对齐 quant1x/pandas/ewm.py 与 pandas 的 EWM 口径；输入/输出用
/// quant1x::pandas::Series。`Series::ewm(alpha)` 是简化的 spec 门面，
/// 本文件的 ExponentialMovingWindow 支持更细的参数化与 adjust / ignore_na。

#include <quant1x/pandas/series.h>

#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace quant1x::pandas {

/// 平滑因子来源
enum class AlphaType {
    Alpha,    // 直接指定平滑因子 α
    Com,      // 质心 com: α = 1/(1+com)
    Span,     // 跨度 span: α = 2/(span+1)
    HalfLife  // 半衰期 halflife: α = 1 - exp(-ln2/halflife)
};

/// 指数加权参数（非零项即启用同名算法）
struct ExponentialWeighting {
    double com{};
    double span{};
    double half_life{};
    double alpha{};
    bool adjust = true;
    bool ignore_na = false;
    std::function<double(int)> callback = nullptr;
};

/// 指数加权移动窗口
class ExponentialMovingWindow {
  public:
    ExponentialMovingWindow(const Series &series, ExponentialWeighting parameter)
        : values_(series.values()),
          name_(series.name()),
          parameter_(std::move(parameter)) {
        adjust_ = parameter_.adjust;
        ignore_na_ = parameter_.ignore_na;
        call_back_ = parameter_.callback;
        type_ = AlphaType::Alpha;
        if (parameter_.com != 0) {
            type_ = AlphaType::Com;
            param_ = parameter_.com;
        } else if (parameter_.span != 0) {
            type_ = AlphaType::Span;
            param_ = parameter_.span;
        } else if (parameter_.half_life != 0) {
            type_ = AlphaType::HalfLife;
            param_ = parameter_.half_life;
        } else {
            type_ = AlphaType::Alpha;
            param_ = parameter_.alpha;
        }
    }

    /// 计算指数加权移动平均
    Series mean() {
        if (values_.empty()) {
            return Series(std::move(values_), std::move(name_));
        }
        double alpha = 0.0;
        switch (type_) {
            case AlphaType::Alpha:
                if (param_ <= 0) {
                    throw std::invalid_argument("alpha must be > 0");
                }
                alpha = param_;
                break;
            case AlphaType::Com:
                if (param_ <= 0) {
                    throw std::invalid_argument("com must be >= 0");
                }
                alpha = 1.0 / (1.0 + param_);
                break;
            case AlphaType::Span:
                if (param_ < 1) {
                    throw std::invalid_argument("span must be >= 1");
                }
                alpha = 2.0 / (param_ + 1.0);
                break;
            case AlphaType::HalfLife:
                if (param_ <= 0) {
                    throw std::invalid_argument("halflife must be > 0");
                }
                alpha = 1.0 - std::exp(-std::log(2.0) / param_);
                break;
        }
        if (adjust_) {
            adjusted_mean(alpha, ignore_na_);
        } else {
            not_adjusted_mean(alpha, ignore_na_);
        }
        return Series(std::move(values_), std::move(name_));
    }

  private:
    static bool is_nan_value(double value) { return std::isnan(value); }

    void adjusted_mean(double alpha, bool ignore_na) {
        double weight = 1.0;
        double last = values_[0];
        alpha = 1.0 - alpha;
        for (std::size_t t = 1; t < values_.size(); ++t) {
            const double w = alpha * weight + 1.0;
            const double x = values_[t];
            if (is_nan_value(x)) {
                if (ignore_na) {
                    weight = w;
                }
                values_[t] = last;
                continue;
            }
            last = last + (x - last) / w;
            weight = w;
            values_[t] = last;
        }
    }

    void not_adjusted_mean(double alpha, bool ignore_na) {
        (void)ignore_na;
        const bool has_callback = static_cast<bool>(call_back_);
        double beta = 1.0 - alpha;
        double last = values_[0];
        if (is_nan_value(last)) {
            last = 0.0;
            values_[0] = last;
        }
        for (std::size_t t = 1; t < values_.size(); ++t) {
            const double x = values_[t];
            if (is_nan_value(x)) {
                values_[t] = last;
                continue;
            }
            if (has_callback) {
                alpha = call_back_(static_cast<int>(t));
                beta = 1.0 - alpha;
            }
            last = beta * last + alpha * x;
            if (is_nan_value(last)) {
                last = values_[t - 1];
            }
            values_[t] = last;
        }
    }

    std::vector<double> values_;
    std::string name_;
    ExponentialWeighting parameter_;
    AlphaType type_ = AlphaType::Alpha;
    double param_ = 0.0;
    bool adjust_ = true;
    bool ignore_na_ = false;
    std::function<double(int)> call_back_;
};

/// 便捷入口：对 Series 做一次指数加权均值
inline Series ewm(const Series &series, ExponentialWeighting parameter) {
    ExponentialMovingWindow window(series, std::move(parameter));
    return window.mean();
}

}  // namespace quant1x::pandas

#endif  // QUANT1X_PANDAS_EXPONENTIAL_MOVING_WINDOWS_H
