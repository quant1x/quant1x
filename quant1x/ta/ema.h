#pragma once
#ifndef QUANT1X_TECHNICAL_ANALYSIS_EMA_H
#define QUANT1X_TECHNICAL_ANALYSIS_EMA_H 1

#include "ta.h"
#include "rolling_window.h"
#include <stdexcept>

namespace ta {

    /**
     * @brief 指数移动平均线
     * @tparam T
     */
    template<typename T>
    class EMA : public ITechnicalIndicator<T> {
    public:
        explicit EMA(size_t period)
            : period_(period) {}

        T update(T value) override {
            // 实现增量更新逻辑
            if (!initialized_) {
                prev_ema_ = value;
                initialized_ = true;
            } else {
                prev_ema_ = alpha_ * value + (T(1) - alpha_) * prev_ema_;
            }

            return prev_ema_;
        }

        std::vector<T> calculate(const std::vector<T>& data) override {
            Periods<T> periods(period_);
            Rolling<T> rolling(data, periods);

            auto ema_func = [this](T period, const T* block, size_t length) -> T {
                T alpha = T(2) / (static_cast<T>(period_) + T(1));
                T prev_ema = block[0];

                for (size_t i = 1; i < length; ++i) {
                    prev_ema = alpha * block[i] + (T(1) - alpha) * prev_ema;
                }

                return prev_ema;
            };

            return rolling.template apply<T>(ema_func, [](){ return type_default<T>(); });
        }

        std::string name() const override {
            return "EMA" + std::to_string(period_);
        }

    private:
        size_t period_;
        T alpha_ = T(2) / (period_ + T(1));
        T prev_ema_;
        bool initialized_ = false;
    };

} // namespace ta

#endif //QUANT1X_TECHNICAL_ANALYSIS_EMA_H
