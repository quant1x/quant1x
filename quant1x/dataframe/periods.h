#pragma once
#ifndef QUANT1X_DATAFRAME_PERIODS_H
#define QUANT1X_DATAFRAME_PERIODS_H 1

#include <vector>
#include <string>

namespace ta {
    /**
     * @brief 动态周期
     * @tparam T
     */
    template<typename T>
    class Periods {
    public:
        explicit Periods(T fixed)
            : is_fixed_(true), fixed_val_(fixed) {}

        explicit Periods(const std::vector<T>& variable)
            : is_fixed_(false), variable_vals_(variable) {}

        /**
         * @brief 获取下标为index的元素, 如果i超过切片V的长度, 则直接返回常量C, 附带越界检查 boundaryExceeded
         * @param index
         * @return
         */
        std::pair<T, bool> at(size_t index) const {
            T n = is_fixed_ ? fixed_val_ : (index < variable_vals_.size()
                                            ? variable_vals_[index]
                                            : (variable_vals_.empty() ? type_default<T>() : variable_vals_.back()));

            auto offset = static_cast<size_t>(n);
            bool valid = !std::isnan(n) && offset > 0 && offset <= index + 1;

            return {n, valid};
        }

        // 是否全部是固定值
        [[nodiscard]] bool is_const() const {
            return is_fixed_ || variable_vals_.empty();
        }

    private:
        bool is_fixed_;
        T fixed_val_;
        std::vector<T> variable_vals_;
    };
}

#endif //QUANT1X_DATAFRAME_PERIODS_H
