#include <quant1x/formula.h>

namespace formula {

    /**
     * @brief 使用xtensor实现通达信VALUEWHEN函数
     * @param condition 条件数组
     * @param value 取值数组
     * @return 返回结果数组，形状与输入相同
     */
    xt::xarray<double> value_when(const xt::xarray<bool>& condition, const xt::xarray<double>& value) {
        // 检查输入形状是否相同
        if (condition.shape() != value.shape()) {
            throw std::invalid_argument("Condition and value arrays must have the same shape");
        }

        // 创建结果数组
        xt::xarray<double> result = xt::empty_like(value);

        // 用于存储上一次满足条件的值
        std::optional<double> lastValidValue;

        // 遍历数组元素
        for (size_t i = 0; i < condition.size(); ++i) {
            if (condition.flat(i)) {
                lastValidValue = value.flat(i);
                result.flat(i) = value.flat(i);
            } else {
                if (lastValidValue.has_value()) {
                    result.flat(i) = lastValidValue.value();
                } else {
                    // 如果之前没有满足条件的值，返回NaN
                    result.flat(i) = std::numeric_limits<double>::quiet_NaN();
                }
            }
        }

        return result;
    }

} // namespace formula