#pragma once
#ifndef QUANT1X_TECHNICAL_ANALYSIS_H
#define QUANT1X_TECHNICAL_ANALYSIS_H 1

#include <vector>
#include <string>

namespace ta {

    // 抽象接口：所有指标必须实现 update() 和 name()
    template<typename T>
    class ITechnicalIndicator {
    public:
        virtual ~ITechnicalIndicator() = default;

        // 输入一个新值，返回当前指标值
        virtual T update(T value) = 0;

        // 对整个序列进行批量处理
        virtual std::vector<T> calculate(const std::vector<T>& data) {
            std::vector<T> result;
            for (T val : data)
                result.push_back(update(val));
            return result;
        }

        virtual std::string name() const = 0;
    };

    // 基类：封装通用缓冲逻辑（窗口管理）
    template<typename T>
    class VectorIndicator : public ITechnicalIndicator<T> {
    protected:
        size_t period_;
        std::vector<T> window_;

    public:
        explicit VectorIndicator(int period) : period_(period) {}

        virtual T update(T value) override {
            window_.push_back(value);
            if (window_.size() > period_) {
                window_.erase(window_.begin());
            }
            return compute();
        }

        virtual T compute() = 0; // 子类必须实现核心计算逻辑

        virtual std::string name() const override {
            return "Indicator<" + std::to_string(period_) + ">";
        }
    };

} // namespace ta

#endif //QUANT1X_TECHNICAL_ANALYSIS_H
