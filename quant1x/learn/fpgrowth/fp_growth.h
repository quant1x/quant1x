#ifndef QUANT1X_FP_GROWTH_H
#define QUANT1X_FP_GROWTH_H 1

#include <vector>
#include <memory>
#include <string>

namespace quant1x {

// 前置声明, 不暴露具体定义
class FPGrowthCore;

/**
 * @brief 泛型 FP Growth 算法实现
 * 
 * 自动处理类型 T 到 size_t 的映射, 支持任意可哈希类型(如 std::string)
 * 注意: 目前仅支持 std::string, size_t, int, long 等常见类型的显式实例化. 
 */
template <typename T>
class FPGrowth {
public:
    using ItemSet = std::vector<T>;
    using SupportCount = size_t;
    using Support = double;
    using FrequentPattern = std::pair<ItemSet, Support>;
    using Transaction = std::vector<T>;
    using Transactions = std::vector<Transaction>;

    explicit FPGrowth(double min_support = 0.1);
    explicit FPGrowth(size_t min_support_count);
    ~FPGrowth();

    // 移动构造和赋值
    FPGrowth(FPGrowth&&) noexcept;
    FPGrowth& operator=(FPGrowth&&) noexcept;

    // 禁止拷贝
    FPGrowth(const FPGrowth&) = delete;
    FPGrowth& operator=(const FPGrowth&) = delete;

    std::vector<FrequentPattern> mine(const Transactions& transactions);

private:
    std::unique_ptr<FPGrowthCore> core_;
};

} // namespace quant1x


#endif // QUANT1X_FP_GROWTH_H