//
// Created by wangfeng on 2025/5/15.
//

#ifndef QUANT1X_SMA_H
#define QUANT1X_SMA_H

#include "ta.h"
#include "rolling.h"

namespace ta {

    // 中国式 SMA：Y = (X*M + Y_prev*(N-M))/N
    template<typename T>
    class SMA : public ITechnicalIndicator<T> {
    public:
        explicit SMA(size_t period, size_t weight = 1)
            : period_(period), weight_(weight), prev_sma_(T(0)) {}

        T update(T value) override {
            if (!initialized_) {
                prev_sma_ = value;
                initialized_ = true;
            } else {
                T alpha = static_cast<T>(weight_) / static_cast<T>(period_);
                prev_sma_ = alpha * value + (T(1) - alpha) * prev_sma_;
            }

            return prev_sma_;
        }

        std::vector<T> calculate(const std::vector<T>& data) override {
            if (data.empty()) return {};

            Periods<T> periods(period_);
            Rolling<T> rolling(data, periods);

            prev_sma_ = data[0]; // 初始化为第一个值
            auto sma_func = [this](T period, const T* block, size_t length) -> T {
                T alpha = static_cast<T>(weight_) / period_;
                T current_sma = prev_sma_;

                for (size_t i = 1; i < length; ++i) {
                    current_sma = alpha * block[i] + (T(1) - alpha) * current_sma;
                }

                prev_sma_ = current_sma;
                return current_sma;
            };

            // ✅ 使用 this 捕获，并且返回当前值作为默认值
            return rolling.template apply<T>(
                sma_func,
                [this](){ return this->prev_sma_; }); // ✅ 正确捕获类成员
        }

        std::string name() const override {
            return "SMA" + std::to_string(period_);
        }

    private:
        size_t period_;
        size_t weight_;
        T prev_sma_;
        bool initialized_ = false;
    };

} // namespace ta

#endif //QUANT1X_SMA_H
