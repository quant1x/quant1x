#pragma once
#ifndef QUANT1X_TECHNICAL_ANALYSIS_RSI_H
#define QUANT1X_TECHNICAL_ANALYSIS_RSI_H 1

#include "ta.h"
#include "rolling.h"

namespace ta {

    template<typename T>
    class RSI : public ITechnicalIndicator<T> {
    public:
        explicit RSI(size_t period = 14)
            : period_(period) {}

        T update(T value) override {
            if (!initialized_) {
                prev_value_ = value;
                result_ = T(50); // 初始值
                initialized_ = true;
            } else {
                diff_ = value - prev_value_;
                prev_value_ = value;

                gain_ = diff_ > T(0) ? diff_ : T(0);
                loss_ = diff_ < T(0) ? -diff_ : T(0);

                avg_gain_ = gain_ema_->update(gain_);
                avg_loss_ = loss_ema_->update(loss_);

                if (avg_gain_ + avg_loss_ == T(0)) {
                    result_ = T(50);
                } else {
                    result_ = avg_gain_ / (avg_gain_ + avg_loss_) * T(100);
                }
            }

            return result_;
        }

        std::vector<T> calculate(const std::vector<T>& data) override {
            Periods<T> periods(period_);
            Rolling<T> rolling(data, periods);

            auto rsi_func = [this](T period, const T* block, size_t length) -> T {
                T gain_sum = T(0), loss_sum = T(0);
                for (size_t i = 1; i < length; ++i) {
                    T diff = block[i] - block[i - 1];
                    if (diff > T(0))
                        gain_sum += diff;
                    else
                        loss_sum -= diff;
                }

                if (gain_sum + loss_sum == T(0)) {
                    return T(50); // 默认中间值
                }

                return gain_sum / (gain_sum + loss_sum) * T(100);
            };

            return rolling.template apply<T>(rsi_func, [](){ return T(50); });
        }

        std::string name() const override {
            return "RSI";
        }

    private:
        size_t period_;
        std::unique_ptr<EMA<T>> gain_ema_, loss_ema_;

        T prev_value_ = NAN;
        T diff_;
        T gain_, loss_;
        T avg_gain_ = T(0), avg_loss_ = T(0);
        T result_ = T(50);
        bool initialized_ = false;
    };

} // namespace ta

#endif // QUANT1X_TECHNICAL_ANALYSIS_RSI_H