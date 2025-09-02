#pragma once
#ifndef QUANT1X_TECHNICAL_ANALYSIS_BOLL_H
#define QUANT1X_TECHNICAL_ANALYSIS_BOLL_H 1

#include "ta.h"
#include "rolling_window.h"
#include <cmath>

namespace ta {

    template<typename T>
    class BollingerBands : public ITechnicalIndicator<T> {
    public:
        explicit BollingerBands(size_t period = 20, T nbDevUp = T(2), T nbDevDn = T(2))
            : period_(period), upperBand_(nbDevUp), lowerBand_(nbDevDn) {}

        std::string name() const override {
            return "BOLL" + std::to_string(period_);
        }

        T update(T value) override {
            window_.push(value);

            if (window_.size() < period_) {
                mean_ = T(0);
                std_ = T(0);
                return value;
            }

            // 计算均值和标准差
            mean_ = T(0);
            for (size_t i = 0; i < period_; ++i)
                mean_ += window_[i];
            mean_ /= static_cast<T>(period_);

            T var = T(0);
            for (size_t i = 0; i < period_; ++i)
                var += (window_[i] - mean_) * (window_[i] - mean_);
            std_ = std::sqrt(var / static_cast<T>(period_));

            upper_ = mean_ + upperBand_ * std_;
            lower_ = mean_ - lowerBand_ * std_;

            return mean_;
        }

        std::vector<T> calculate(const std::vector<T>& data) override {
            std::vector<T> result;
            for (auto val : data)
                result.push_back(update(val));
            return result;
        }

        T getUpperBand() const { return upper_; }
        T getLowerBand() const { return lower_; }

    private:
        size_t period_;
        T upperBand_, lowerBand_;
        RollingWindow<T> window_;
        T mean_, std_, upper_, lower_;
    };

} // namespace ta

#endif // QUANT1X_TECHNICAL_ANALYSIS_BOLL_H