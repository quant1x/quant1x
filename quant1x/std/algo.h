#pragma once
#ifndef API_ALGO_H
#define API_ALGO_H 1

#include <quant1x/std/numerics.h>

namespace algo {

    namespace detail {

        class WelfordStdDev {
        private:
            double mean = 0.0;   // 当前均值
            double M2 = 0.0;     // 平方差的累积和
            int count = 0;       // 已处理的数据点数量

        public:
            // 更新统计量（添加一个新数据点）
            void update(double newValue) {
                // 1. 更新数据点计数
                count++;

                // 2. 计算当前值与旧均值的差
                //    delta = xₙ - μₙ₋₁
                double delta = newValue - mean;

                // 3. 更新均值（递推公式）
                //    μₙ = μₙ₋₁ + (xₙ - μₙ₋₁)/n
                mean += delta / count;

                // 4. 计算当前值与新均值的差
                //    delta2 = xₙ - μₙ
                double delta2 = newValue - mean;

                // 5. 更新平方和（关键步骤）
                //    M2ₙ = M2ₙ₋₁ + (xₙ - μₙ₋₁)(xₙ - μₙ)
                M2 += delta * delta2;

                /* 数学解释：
                 * 这里使用 delta * delta2 而不是 delta2² 是为了数值稳定性
                 * 展开后：
                 * (xₙ - μₙ₋₁)(xₙ - μₙ)
                 * = (xₙ - μₙ₋₁)(xₙ - μₙ₋₁ - (μₙ - μₙ₋₁))
                 * = (xₙ - μₙ₋₁)² - (xₙ - μₙ₋₁)(δ/n)
                 * 其中 δ = xₙ - μₙ₋₁
                 * = δ² - δ²/n = δ²(1 - 1/n) = δ²((n-1)/n)
                 * 这种形式避免了直接计算大数的平方
                 */
            }

            // 获取当前均值
            double getMean() const {
                return (count > 0) ? mean : numerics::NaN; // 返回NaN如果无数据
            }

            // 获取总体方差
            double getVariancePopulation() const {
                return (count > 0) ? M2 / count : numerics::NaN;  // NaN如果无数据
            }

            // 获取样本方差（无偏估计）
            double getVarianceSample() const {
                return (count > 1) ? M2 / (count - 1) : numerics::NaN;  // NaN如果数据不足
            }

            // 获取总体标准差
            double getStdDevPopulation() const {
                return std::sqrt(getVariancePopulation());
            }

            // 获取样本标准差
            double getStdDevSample() const {
                return std::sqrt(getVarianceSample());
            }

            // 合并两个独立计算的统计量（用于并行计算）
            void combine(const WelfordStdDev& other) {
                if (other.count == 0) return;

                // 合并后的总数据点数
                int new_count = count + other.count;

                // 计算两个均值之间的差
                double delta = other.mean - mean;

                // 计算合并后的均值
                // μ_new = (n₁μ₁ + n₂μ₂)/(n₁+n₂)
                double new_mean = mean + delta * other.count / new_count;

                // 计算合并后的M2（关键步骤）
                // M2_new = M2₁ + M2₂ + δ²n₁n₂/(n₁+n₂)
                double new_M2 = M2 + other.M2 +
                                delta * delta * count * other.count / new_count;

                // 更新统计量
                count = new_count;
                mean = new_mean;
                M2 = new_M2;

                /* 数学解释：
                 * 合并公式来源于：
                 * 总平方和 = Σ(x - μ_new)²
                 *          = Σ₁(x - μ₁ + μ₁ - μ_new)² + Σ₂(x - μ₂ + μ₂ - μ_new)²
                 *          = [M2₁ + n₁(μ₁ - μ_new)²] + [M2₂ + n₂(μ₂ - μ_new)²]
                 * 代入 μ_new = (n₁μ₁ + n₂μ₂)/(n₁+n₂)
                 * 经过化简得到上述合并公式
                 */
            }
        };
    }

}

#endif // API_ALGO_H
