#pragma once
#ifndef QUANT1X_DATAFRAME_EXPONENTIAL_MOVING_WINDOWS_H
#define QUANT1X_DATAFRAME_EXPONENTIAL_MOVING_WINDOWS_H 1

#include <iostream>
#include <vector>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

//using Series = std::vector<double>;
namespace ta {
    template<typename T>
    class Series;
}

// 判断是否是 NaN
inline bool is_nan(double val) {
    return std::isnan(val);
}

// 设置 NaN 值
inline double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

enum class AlphaType {
    Alpha,     // 直接指定平滑因子 α
    Com,       // 根据质心 com 指定衰减 α = 1/(1 + com)
    Span,      // 根据跨度 span 指定衰减 α = 2/(span + 1)
    HalfLife   // 根据半衰期 halflife 指定衰减 α = 1 - exp(-ln(2)/halflife)
};

// EWMA (Factor) 指数加权(EW)计算Alpha 结构属性非0即为有效启动同名算法
struct ExponentialWeighting {
    double com{};         // 根据质心指定衰减
    double span{};        // 根据跨度指定衰减
    double half_life{};   // 根据半衰期指定衰减
    double alpha{};       // 直接指定的平滑因子α
    bool adjust = true;   // 是否调整权重
    bool ignore_na = false; // 是否忽略NaN
    std::function<double(int)> callback = nullptr; // 回调函数
};

// ExponentialMovingWindow 加权移动窗口
template <typename T>
class ExponentialMovingWindow {
private:
    std::shared_ptr<ta::Series<T >> data_; // 序列数据
    ExponentialWeighting parameter_;
    AlphaType type_;        // 计算方式: com/span/halflefe/alpha
    double param_;         // 参数值
    bool adjust_ = true;     // 默认为真
    bool ignore_na_ = false; // 默认为假
    int min_periods_ = 0;    // 最小观测数，默认为0
    int axis_ = 0;           // {0,1}, 默认为0, 0跨行计算, 1跨列计算（未在实现中使用）
    std::function<double(int)> call_back_; // 回调函数
public:

    template <typename EWType>
    ExponentialMovingWindow(ta::Series<T>& S, EWType&& param)
        : data_(std::make_shared<ta::Series<T>>(S)),
          parameter_(std::forward<EWType>(param)) {
            adjust_ = parameter_.adjust;
            ignore_na_= parameter_.ignore_na;
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

    // 计算指数加权移动平均
    ta::Series<T> mean() {
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

        return *data_;
    }

private:
    // 使用初始调整因子的指数加权均值
    void adjusted_mean(double alpha, bool ignore_na) {
        double weight = 1.0;
        auto values = data_->data(); // 获取 std::span<T>
        double last = values[0];

        alpha = 1.0 - alpha;

        for (size_t t = 1; t < values.size(); ++t) {
            double w = alpha * weight + 1.0;
            double x = values[t];

            if (is_nan(x)) {
                if (ignore_na) {
                    weight = w;
                }
                values[t] = last;
                continue;
            }

            last = last + (x - last) / w;
            weight = w;
            values[t] = last;
        }
    }

    // 不使用初始调整因子的指数加权均值
    void not_adjusted_mean(double alpha, bool ignore_na) {
        bool has_callback = static_cast<bool>(call_back_); // 判断是否有回调函数

        double beta = 1.0 - alpha;
        auto values = data_->data(); // 获取 std::span<T>
        double last = values[0];

        if (is_nan(last)) {
            last = 0.0;
            values[0] = last;
        }

        for (size_t t = 1; t < values.size(); ++t) {
            double x = values[t];

            if (is_nan(x)) {
                values[t] = last;
                continue;
            }

            if (has_callback) {
                alpha = call_back_(t);
                beta = 1.0 - alpha;
            }

            last = beta * last + alpha * x;

            if (is_nan(last)) {
                last = values[t - 1];
            }

            values[t] = last;
        }
    }
};

#endif //QUANT1X_DATAFRAME_EXPONENTIAL_MOVING_WINDOWS_H
