#include "learn/fpgrowth/fp_growth.h"
#include <iostream>
#include <vector>

int main() {
    // 示例数据集：购物篮分析
    std::vector<std::vector<size_t>> transactions = {
        {1, 2, 5},     // 牛奶、面包、尿布
        {2, 4},        // 面包、啤酒
        {2, 3},        // 面包、黄油
        {1, 2, 4},     // 牛奶、面包、啤酒
        {1, 3},        // 牛奶、黄油
        {2, 3},        // 面包、黄油
        {1, 3},        // 牛奶、黄油
        {1, 2, 3, 5},  // 牛奶、面包、黄油、尿布
        {1, 2, 3}      // 牛奶、面包、黄油
    };

    // 创建FP growth实例，最小支持度30% (3/10)
    quant1x::FPGrowth fpgrowth(0.3);

    // 挖掘频繁模式
    auto patterns = fpgrowth.mine(transactions);

    // 输出结果
    std::cout << "频繁模式挖掘结果 (最小支持度: 30%):\n";
    std::cout << "总事务数: " << transactions.size() << "\n";
    std::cout << "发现的频繁模式数量: " << patterns.size() << "\n\n";

    for (const auto& pattern : patterns) {
        std::cout << "模式 {";
        for (size_t i = 0; i < pattern.first.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << pattern.first[i];
        }
        std::cout << "} : 支持度 = " << pattern.second << "\n";
    }

    return 0;
}