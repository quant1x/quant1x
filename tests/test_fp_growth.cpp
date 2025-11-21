#include "learn/fpgrowth/fp_growth.h"
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <iomanip> // 引入 IO 操作符

int main() {
    // 设置输出格式：固定点表示法，保留2位小数
    std::cout << std::fixed << std::setprecision(2);

    // 1. 测试原始整数版本 (FPGrowth<size_t>)
    std::cout << "=== 测试 FPGrowth<size_t> (整数版本) ===\n";
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
    quant1x::FPGrowth<size_t> fpgrowth(0.3);

    // 挖掘频繁模式
    auto patterns = fpgrowth.mine(transactions);

    // 输出结果
    std::cout << "频繁模式挖掘结果 (最小支持度: 30%):\n";
    std::cout << "总事务数: " << transactions.size() << "\n";
    std::cout << "发现的频繁模式数量: " << patterns.size() << "\n";

    for (const auto& pattern : patterns) {
        std::cout << "模式 {";
        for (size_t i = 0; i < pattern.first.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << pattern.first[i];
        }
        std::cout << "} : 支持度 = " << pattern.second << "\n";
    }

    // 2. 测试泛型版本 (FPGrowth<std::string>)
    std::cout << "\n=== 测试 FPGrowth<std::string> (泛型版本) ===\n";
    std::vector<std::vector<std::string>> str_transactions = {
        {"牛奶", "面包", "黄油"},
        {"牛奶", "面包"},
        {"牛奶", "黄油"},
        {"面包", "黄油"},
        {"牛奶", "面包", "黄油", "鸡蛋"},
        {"鸡蛋", "黄油"},
        {"牛奶", "鸡蛋"},
        {"牛奶", "面包", "鸡蛋"},
        {"牛奶", "面包", "黄油", "鸡蛋", "果汁"},
        {"果汁", "面包"}
    };

    quant1x::FPGrowth<std::string> str_fpgrowth(0.3);
    auto str_patterns = str_fpgrowth.mine(str_transactions);

    std::cout << "频繁模式挖掘结果 (最小支持度: 30%):\n";
    std::cout << "总事务数: " << str_transactions.size() << "\n";
    std::cout << "发现的频繁模式数量: " << str_patterns.size() << "\n";

    for (const auto& pattern : str_patterns) {
        std::cout << "模式 {";
        for (size_t i = 0; i < pattern.first.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << pattern.first[i];
        }
        std::cout << "} : 支持度 = " << pattern.second << "\n";
    }

    return 0;
}