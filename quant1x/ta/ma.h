#pragma once
#ifndef QUANT1X_TECHNICAL_ANALYSIS_MA_H
#define QUANT1X_TECHNICAL_ANALYSIS_MA_H 1

#include <quant1x/ta/ta.h>
#include <quant1x/ta/rolling_window.h>
#include <numeric>
#include "rolling.h"

namespace ta {

    template<typename T>
    class MA : public ITechnicalIndicator<T> {
    public:
        explicit MA(size_t period)
            : period_(period),
              buffer_(period) {}

        T update(T value) override {
            buffer_.push(value);

            if (buffer_.size() < period_) {
                return type_default<T>();
            }

            T sum = T(0);
            for (size_t i = 0; i < period_; ++i)
                sum += buffer_[i];

            return sum / static_cast<T>(period_);
        }

        std::vector<T> calculate(const std::vector<T>& data) override {
            Periods<T> periods(period_);
            Rolling<T> rolling(data, periods);

            auto result = rolling.template apply<T>(
                [](T period, const T* block, size_t length) -> T {
                    T sum = T(0);
                    for (size_t i = 0; i < length; ++i)
                        sum += block[i];
                    return sum / static_cast<T>(length);
                },
                [](){ return type_default<T>(); });

            return result;
        }

        std::string name() const override {
            return "MA" + std::to_string(period_);
        }

    private:
        size_t period_;
        RollingWindow<T> buffer_;
    };

} // namespace ta

#endif //QUANT1X_TECHNICAL_ANALYSIS_MA_H
