#ifndef QUANT1X_FP_GROWTH_H
#define QUANT1X_FP_GROWTH_H 1

#include <vector>
#include <unordered_map>
#include <memory>

namespace quant1x {

/**
 * @brief FP Growth算法实现 - 频繁模式挖掘
 *
 * FP-Growth (Frequent Pattern Growth) 是一种高效的频繁项集挖掘算法，
 * 特别适用于大数据集的关联规则挖掘。
 */
class FPGrowth {
public:
    /**
     * @brief 频繁项集
     */
    using ItemSet = std::vector<size_t>;

    /**
     * @brief 项集支持度
     */
    using SupportCount = size_t;

    /**
     * @brief 频繁模式：项集 -> 支持度
     */
    using FrequentPattern = std::pair<ItemSet, SupportCount>;

    /**
     * @brief 事务数据集：每个事务是一个项的集合
     */
    using Transaction = std::vector<size_t>;
    using Transactions = std::vector<Transaction>;

    /**
     * @brief 构造函数
     * @param min_support 最小支持度阈值 (0.0-1.0之间)
     */
    explicit FPGrowth(double min_support = 0.1);

    /**
     * @brief 构造函数
     * @param min_support_count 最小支持度计数
     */
    explicit FPGrowth(size_t min_support_count);

    /**
     * @brief 挖掘频繁模式
     * @param transactions 事务数据集
     * @return 频繁模式列表
     */
    std::vector<FrequentPattern> mine(const Transactions& transactions);

    /**
     * @brief 设置最小支持度
     */
    void set_min_support(double min_support) { min_support_ = min_support; }

    /**
     * @brief 设置最小支持度计数
     */
    void set_min_support_count(size_t count) { min_support_count_ = count; }

private:
    // FP树节点
    struct FPNode {
        size_t item_id;
        size_t count;
        FPNode* parent;
        std::unordered_map<size_t, std::unique_ptr<FPNode>> children;
        FPNode* next;  // 相同项的链表

        explicit FPNode(size_t item = 0, size_t cnt = 1, FPNode* p = nullptr)
            : item_id(item), count(cnt), parent(p), next(nullptr) {}
    };

    // 项头表
    struct HeaderEntry {
        size_t item_id;
        size_t support;
        FPNode* head;  // 指向第一个节点

        explicit HeaderEntry(size_t item = 0, size_t supp = 0)
            : item_id(item), support(supp), head(nullptr) {}
    };

    // FP树
    class FPTree {
    public:
        FPTree() : root_(std::make_unique<FPNode>()) {}
        ~FPTree() = default;

        void insert(const Transaction& transaction,
                   const std::vector<size_t>& item_order,
                   std::vector<HeaderEntry>& header_table);

        std::vector<FrequentPattern> mine_patterns(
            const std::vector<HeaderEntry>& header_table,
            size_t min_support);

    private:
        std::unique_ptr<FPNode> root_;

        void insert_single_path(FPNode* node, const Transaction& transaction,
                               size_t index, std::vector<HeaderEntry>& header_table);

        std::vector<FrequentPattern> mine_conditional_patterns(
            const std::vector<HeaderEntry>& header_table,
            size_t suffix_item,
            size_t min_support);
    };

    double min_support_;      // 最小支持度比例
    size_t min_support_count_; // 最小支持度计数
    bool use_count_threshold_; // 是否使用计数阈值

    // 辅助函数
    std::unordered_map<size_t, size_t> count_item_frequencies(
        const Transactions& transactions);

    std::vector<size_t> get_frequent_items(
        const std::unordered_map<size_t, size_t>& item_counts,
        size_t total_transactions);

    void sort_transaction_by_frequency(
        Transaction& transaction,
        const std::vector<size_t>& item_order);
};

} // namespace quant1x

#endif // QUANT1X_FP_GROWTH_H