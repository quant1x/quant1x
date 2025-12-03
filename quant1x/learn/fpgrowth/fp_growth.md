# FP Growth 算法 - C++ 实现

[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

## 📖 概述

FP Growth (Frequent Pattern Growth) 是一种高效的频繁项集挖掘算法，由韩家炜教授于2000年提出。该算法通过构建FP树(Frequent Pattern Tree)来避免Apriori算法中候选集生成的瓶颈问题，具有更好的性能和可扩展性。
本实现是Quant1X量化交易框架的一部分，提供了一个完整的、高性能的FP Growth算法C++实现。

## 🎯 核心特性

- **高效算法**：比传统Apriori算法快几个数量级
- **内存优化**：FP树结构避免了候选集的生成和存储
- **模板化设置**：支持不同数据类型的项标识符
- **灵活配置**：支持最小支持度比例和绝对计数两种配置方式
- **完整实现**：包含条件模式基挖掘和递归FP树构建

## 📊 算法原理

### FP树构建

1. 扫描数据集，统计各项的频率
2. 筛选出满足最小支持度的频繁项
3. 按支持度降序对事务进行排序
4. 构建FP树，将排序后的事务插入树中

### 频繁模式挖掘

1. 从项头表中选择支持度最低的项作为后缀
2. 提取该项的条件模式基（所有包含该项的事务中，该项前面的项）
3. 基于条件模式基构建条件FP树
4. 递归地在条件FP树上挖掘更长的频繁模式

## 🚀 使用方法

### 基本用法

```cpp
#include "quant1x/fp_growth.h"

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

    // 创建FP Growth实例，最小支持度30%
    quant1x::FPGrowth fpgrowth(0.3);

    // 挖掘频繁模式
    auto patterns = fpgrowth.mine(transactions);

    // 输出结果
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
```

### 配置选项

```cpp
// 使用最小支持度比例 (0.0-1.0)
quant1x::FPGrowth fpgrowth_ratio(0.1);  // 10%

// 使用最小支持度绝对计数
quant1x::FPGrowth fpgrowth_count(5);    // 至少出现5次
// 动态设置参数
fpgrowth.set_min_support(0.2);          // 设置为20%
fpgrowth.set_min_support_count(10);     // 设置为至少10次
```

## 📈 性能特点

### 时间复杂度

- **构建阶段**：O(N) - 线性时间扫描数据集
- **挖掘阶段**：通常比Apriori算法快2-3个数量级
- **空间复杂度**：O(频繁项的数量) - 远小于Apriori的候选集空间

### 优势对比

| 算法 | 时间复杂度 | 空间复杂度 | 适用场景 |
|------|-----------|-----------|----------|
| Apriori | O(2^D) | O(2^D) | 小数据集 |
| FP Growth | O(N) | O(频繁项数) | 大数据集 |

## 🏗️架构设计

### 核心类结构

```cpp
namespace quant1x {

class FPGrowth {
public:
    using ItemSet = std::vector<size_t>;
    using SupportCount = size_t;
    using FrequentPattern = std::pair<ItemSet, SupportCount>;
    using Transaction = std::vector<size_t>;
    using Transactions = std::vector<Transaction>;

    // 构造函数和主要接口
    FPGrowth(double min_support = 0.1);
    FPGrowth(size_t min_support_count);
    std::vector<FrequentPattern> mine(const Transactions& transactions);

private:
    // FP树节点
    struct FPNode {
        size_t item_id;
        size_t count;
        FPNode* parent;
        std::unordered_map<size_t, std::unique_ptr<FPNode>> children;
        FPNode* next;  // 相同项的链表
    };

    // 项头表
    struct HeaderEntry {
        size_t item_id;
        size_t support;
        FPNode* head;
    };

    // FP树类
    class FPTree {
        // 树构建和模式挖掘逻辑
    };

    // 配置参数
    double min_support_;
    size_t min_support_count_;
    bool use_count_threshold_;
};

} // namespace quant1x
```

### 关键组件

1. **FPNode**: FP树节点，包含项ID、计数、父子关系和链表指针
2. **HeaderEntry**: 项头表条目，维护每个频繁项的链表
3. **FPTree**: FP树类，负责树的构建和模式挖掘
4. **FPGrowth**: 主类，提供用户接口和参数配置

## 🧪 测试验证

### 运行测试

```bash
# 构建项目
cmake --build build --target catch2-test_fp_growth

# 运行测试
./build/tests/catch2-test_fp_growth.exe
```

### 示例输出

```bash
频繁模式挖掘结果 (最小支持度: 30%):
总事务数: 9
发现的频繁模式数量: 12

模式 {4} : 支持度 = 2
模式 {2, 4} : 支持度 = 2
模式 {5} : 支持度 = 2
模式 {1, 5} : 支持度 = 2
模式 {2, 1, 5} : 支持度 = 2
模式 {2, 5} : 支持度 = 2
模式 {1} : 支持度 = 6
模式 {2, 1} : 支持度 = 4
模式 {3, 1} : 支持度 = 4
模式 {3} : 支持度 = 6
模式 {2, 3} : 支持度 = 4
模式 {2} : 支持度 = 7
```

## 📚 应用场景

### 市场篮子分析

- 超市商品推荐
- 交叉销售策略
- 库存优化

### 量化交易

- 股票组合分析
- 交易模式识别
- 风险因子挖掘

### 数据挖掘

- 关联规则挖掘
- 序列模式分析
- 异常检测

## 🔧 编译和集成

### 依赖要求

- C++20 标准
- CMake 3.30+
- 支持的编译器：GCC 14.3+、Clang 18+、MSVC 14.3+

### 集成到项目

```cmake
# 在CMakeLists.txt中添加
add_executable(your_app main.cpp)
target_link_libraries(your_app quant1x)
```

## 📖 参考文献

1. Han, J., Pei, J., & Yin, Y. (2000). Mining frequent patterns without candidate generation. ACM SIGMOD Record, 29(2), 1-12.

2. Han, J., Pei, J., Yin, Y., & Mao, R. (2004). Mining frequent patterns without candidate generation: A frequent-pattern tree approach. Data Mining and Knowledge Discovery, 8(1), 53-87.

## 🤝 贡献

欢迎提交Issue和Pull Request来改进这个实现！

## 📄 许可证

本项目采用Apache 2.0许可证 - 查看[LICENSE](LICENSE)文件了解详情。
