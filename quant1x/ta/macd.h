#pragma once
#ifndef QUANT1X_TECHNICAL_ANALYSIS_MACD_H
#define QUANT1X_TECHNICAL_ANALYSIS_MACD_H 1

#include "ta.h"
#include "ema.h"
#include "rolling.h"

namespace ta {

    template<typename T>
    class MACD : public ITechnicalIndicator<T> {
    public:
        MACD(size_t fastPeriod = 12, size_t slowPeriod = 26, size_t signalPeriod = 9)
            : fastPeriod_(fastPeriod), slowPeriod_(slowPeriod), signalPeriod_(signalPeriod) {}

        T update(T value) override {
            fast_ = fastEma_->update(value);
            slow_ = slowEma_->update(value);
            macd_ = fast_ - slow_;
            signal_ = signalEma_->update(macd_);
            hist_ = macd_ - signal_;
            return macd_;
        }

        std::vector<T> calculate(const std::vector<T> &data) override {
            Periods<T> fast_periods(fastPeriod_);
            Periods<T> slow_periods(slowPeriod_);
            Periods<T> signal_periods(signalPeriod_);

            Rolling<T> fast_rolling(data, fast_periods);
            Rolling<T> slow_rolling(data, slow_periods);

            auto fast_ema = fast_rolling.template apply<T>(
                [](T period, const T *block, size_t length) -> T {
                    T alpha = T(2) / (static_cast<T>(period) + T(1));
                    T ema = block[0];
                    for (size_t i = 1; i < length; ++i) {
                        ema = alpha * block[i] + (T(1) - alpha) * ema;
                    }
                    return ema;
                });

            auto slow_ema = slow_rolling.template apply<T>(
                [](T period, const T *block, size_t length) -> T {
                    T alpha = T(2) / (static_cast<T>(period) + T(1));
                    T ema = block[0];
                    for (size_t i = 1; i < length; ++i) {
                        ema = alpha * block[i] + (T(1) - alpha) * ema;
                    }
                    return ema;
                });

            std::vector<T> result(data.size());
            for (size_t i = 0; i < data.size(); ++i) {
                result[i] = fast_ema[i] - slow_ema[i];
            }

            return result;
        }

        std::string name() const override {
            return "MACD";
        }

    private:
        size_t fastPeriod_;
        size_t slowPeriod_;
        size_t signalPeriod_;

        std::unique_ptr<EMA<T>> fastEma_, slowEma_, signalEma_;
        T fast_, slow_, macd_, signal_, hist_;
        bool initialized_ = false;
    };
}

#endif // QUANT1X_TECHNICAL_ANALYSIS_MACD_H