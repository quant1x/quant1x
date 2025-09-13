#pragma once
#ifndef QUANT1X_TECHNICAL_ANALYSIS_ROLLING_H
#define QUANT1X_TECHNICAL_ANALYSIS_ROLLING_H 1

#include <quant1x/pandas/series.h>
#include <quant1x/pandas/periods.h>
#include "rolling_window.h"
#include <vector>
#include <functional>
#include <cmath>
#include <memory>

namespace ta {

    template<typename T>
    class Rolling {
    public:
        Rolling(const std::vector<T>& data, const Periods<T>& periods)
            : data_(data), periods_(periods) {
            max_period_ = 0;
            for (size_t i = 0; i < data.size(); ++i) {
                auto [p, ok] = periods.at(i);
                if (ok) {
                    max_period_ = std::max(max_period_, static_cast<size_t>(p));
                }
            }

            window_ = std::make_unique<RollingWindow<T>>(max_period_);
        }

        template<typename R>
        std::vector<R> apply(
            std::function<R(T period, const T* block, size_t length)> func,
            std::function<R()> default_value = [](){ return type_default<R>(); }) {

            std::vector<R> result(data_.size());

            for (size_t i = 0; i < data_.size(); ++i) {
                auto [p, ok] = periods_.at(i);
                if (!ok) {
                    result[i] = default_value();
                    continue;
                }

                window_->push(data_[i]);

                if (window_->size() < static_cast<size_t>(p)) {
                    result[i] = default_value();
                    continue;
                }

                const T* block = window_->data();
                result[i] = func(p, block, static_cast<size_t>(p));
            }

            return result;
        }

    private:
        const std::vector<T>& data_;
        Periods<T> periods_;
        size_t max_period_;
        std::unique_ptr<RollingWindow<T>> window_;
    };

} // namespace ta

#endif // QUANT1X_TECHNICAL_ANALYSIS_ROLLING_H