#pragma once
#ifndef QUANT1X_STD_NUMERICS_H
#define QUANT1X_STD_NUMERICS_H 1

#include <quant1x/std/api.h>

#include <cmath> // for std::abs
#include <ostream>

namespace numeric {

    constexpr double NaN    = std::numeric_limits<double>::quiet_NaN();  ///< NaN常量
    constexpr double Inf    = std::numeric_limits<double>::infinity();   ///< 正无穷常量
    constexpr double NegInf = -std::numeric_limits<double>::infinity();  ///< 负无穷常量

    constexpr double compare_epsilon_price = 1e-2;   ///< 价格比较, 用于价格(如金融, 交易)的比较. 精度到小数点后两位(如 0.01), 这是常见的货币单位(如美元, 人民币分). 
    constexpr double compare_epsilon_ta    = 1e-6;   ///< 技术比较, 用于技术分析(Technical Analysis)相关计算, 如指标, 均线等, 要求更高精度. 
    constexpr double compare_epsilon_large = 1e-10;  ///< 大数比较, 用于大数或高精度计算, 如科学计算, 统计, 机器学习等, 要求非常精确. 

    template <std::floating_point T>
    bool equal(T a, T b, T epsilon = static_cast<T>(compare_epsilon_ta)) {
        return std::abs(a - b) <= epsilon;
    }

    template <std::floating_point T>
    bool greater(T a, T b, T epsilon = static_cast<T>(compare_epsilon_ta)) {
        return (a - b) > epsilon;
    }

    template <std::floating_point T>
    bool less(T a, T b, T epsilon = static_cast<T>(compare_epsilon_ta)) {
        return (b - a) > epsilon;
    }

    /**
     * @brief 适用于高频高精度的银行家四舍五入
     * @param value double类型value
     * @param digits 保留几位小数点
     * @return 银行家四舍五入的double
     */
    f64 decimal(f64 value, int digits = 2);

    inline bool isEqual(f64 a, f64 b, f64 epsilon = 1e-10) {
        return std::fabs(a - b) < epsilon;
    }

    inline f64 ChangeRate(f64 base, f64 current) {
        return current / base;
    }

    inline f64 NetChangeRate(f64 base, f64 current) {
        auto chg = ChangeRate(base, current);
        return (chg - 1.00) * 100.00;
    }

    /**
     * @brief 数值范围
     * @tparam T
     */
    template <typename T>
    struct number_range {
        T min_;
        T max_;

        // 默认构造: [lowest, max]
        constexpr number_range() : min_(std::numeric_limits<T>::lowest()), max_(std::numeric_limits<T>::max()) {}

        // 双参数构造: [min, max]
        constexpr number_range(T min, T max) : min_(min), max_(max) {}

        // 单参数构造: [min, max()]
        constexpr explicit number_range(T min) : number_range(min, std::numeric_limits<T>::max()) {}

        // 新增: 从字符串构造(无异常)
        explicit number_range(const std::string &str) {
            min_ = std::numeric_limits<T>::lowest();
            max_ = std::numeric_limits<T>::max();

            std::string text = strings::trim(str);
            size_t      pos  = text.find('~');

            if (pos == std::string::npos) {
                // 情况1: 无分隔符, 视为最小值
                T val = strings::from_string(text, min_);
                min_  = val;
            } else {
                std::string s_min = strings::trim(text.substr(0, pos));
                std::string s_max = strings::trim(text.substr(pos + 1));

                // 情况4: 前后都为空
                if (s_min.empty() && s_max.empty()) {
                    // 已经是默认值
                    return;
                }

                // 情况2: 前空, 后为最大值
                if (s_min.empty()) {
                    T val = strings::from_string(s_max, max_);
                    max_  = val;
                }
                // 情况3: 后空, 前为最小值
                else if (s_max.empty()) {
                    T val = strings::from_string(s_min, min_);
                    min_  = val;
                }
                // 情况5: 前后都有值
                else {
                    T val_min = strings::from_string(s_min, min_);
                    T val_max = strings::from_string(s_max, max_);
                    min_      = val_min;
                    max_      = val_max;
                }
            }
        }

    public:
        // 验证值是否在范围内
        bool validate(double v) const {
            if (min_ == 0 && max_ == 0) {
                return true;
            }
            return v >= min_ && v < max_;
        }

        // 字符串表示
        std::string to_string() const { return "{min:" + std::to_string(min_) + ", max:" + std::to_string(max_) + "}"; }

        friend std::ostream &operator<<(std::ostream &os, const number_range &range) {
            os << range.to_string();
            return os;
        }
    };
}  // namespace numerics

#endif  // QUANT1X_STD_NUMERICS_H
