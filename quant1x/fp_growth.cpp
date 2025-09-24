#include "fp_growth.h"
#include <algorithm>

namespace quant1x {

FPGrowth::FPGrowth(double min_support)
    : min_support_(min_support),
      min_support_count_(0),
      use_count_threshold_(false) {}

FPGrowth::FPGrowth(size_t min_support_count)
    : min_support_(0.0),
      min_support_count_(min_support_count),
      use_count_threshold_(true) {}

std::vector<FPGrowth::FrequentPattern> FPGrowth::mine(const Transactions& transactions) {
    if (transactions.empty()) {
        return {};
    }

    // 计算项频
    auto item_counts = count_item_frequencies(transactions);
    size_t total_transactions = transactions.size();

    // 计算最小支持度计数
    size_t min_count = use_count_threshold_ ?
        min_support_count_ :
        static_cast<size_t>(min_support_ * static_cast<double>(total_transactions));

    // 获取频繁项
    auto frequent_items = get_frequent_items(item_counts, min_count);

    if (frequent_items.empty()) {
        return {};
    }

    // 构建FP树
    FPTree fp_tree;
    std::vector<HeaderEntry> header_table;

    // 初始化项头表
    header_table.reserve(frequent_items.size());
    for (size_t item : frequent_items) {
        header_table.emplace_back(item, item_counts[item]);
    }

    // 插入事务到FP树
    for (const auto& transaction : transactions) {
        Transaction filtered_transaction;
        for (size_t item : transaction) {
            if (item_counts[item] >= min_count) {
                filtered_transaction.push_back(item);
            }
        }

        if (!filtered_transaction.empty()) {
            sort_transaction_by_frequency(filtered_transaction, frequent_items);
            fp_tree.insert(filtered_transaction, frequent_items, header_table);
        }
    }

    // 从FP树中挖掘模式
    return fp_tree.mine_patterns(header_table, min_count);
}

std::unordered_map<size_t, size_t> FPGrowth::count_item_frequencies(
    const Transactions& transactions) {

    std::unordered_map<size_t, size_t> counts;
    for (const auto& transaction : transactions) {
        for (size_t item : transaction) {
            counts[item]++;
        }
    }
    return counts;
}

std::vector<size_t> FPGrowth::get_frequent_items(
    const std::unordered_map<size_t, size_t>& item_counts,
    size_t min_support) {

    std::vector<std::pair<size_t, size_t>> items;
    for (const auto& pair : item_counts) {
        if (pair.second >= min_support) {
            items.emplace_back(pair.second, pair.first); // 按支持度降序
        }
    }

    std::sort(items.rbegin(), items.rend()); // 降序排序

    std::vector<size_t> result;
    result.reserve(items.size());
    for (const auto& item : items) {
        result.push_back(item.second);
    }

    return result;
}

void FPGrowth::sort_transaction_by_frequency(
    Transaction& transaction,
    const std::vector<size_t>& item_order) {

    // 创建项到顺序的映射
    std::unordered_map<size_t, size_t> order_map;
    for (size_t i = 0; i < item_order.size(); ++i) {
        order_map[item_order[i]] = i;
    }

    // 按频率排序（支持度高的项排在前面）
    std::sort(transaction.begin(), transaction.end(),
              [&order_map](size_t a, size_t b) {
                  return order_map[a] < order_map[b];
              });
}

// ===== FPTree 实现 =====

void FPGrowth::FPTree::insert(
    const Transaction& transaction,
    [[maybe_unused]] const std::vector<size_t>& item_order,
    std::vector<HeaderEntry>& header_table) {

    insert_single_path(root_.get(), transaction, 0, header_table);
}

void FPGrowth::FPTree::insert_single_path(
    FPNode* node,
    const Transaction& transaction,
    size_t index,
    std::vector<HeaderEntry>& header_table) {

    if (index >= transaction.size()) {
        return;
    }

    size_t item = transaction[index];
    auto it = node->children.find(item);

    if (it == node->children.end()) {
        // 创建新节点
        auto new_node = std::make_unique<FPNode>(item, 1, node);
        auto* new_node_ptr = new_node.get();
        node->children[item] = std::move(new_node);

        // 更新项头表
        for (auto& entry : header_table) {
            if (entry.item_id == item) {
                new_node_ptr->next = entry.head;
                entry.head = new_node_ptr;
                break;
            }
        }

        // 递归插入剩余项
        insert_single_path(new_node_ptr, transaction, index + 1, header_table);
    } else {
        // 增加计数
        it->second->count++;

        // 递归插入剩余项
        insert_single_path(it->second.get(), transaction, index + 1, header_table);
    }
}

std::vector<FPGrowth::FrequentPattern> FPGrowth::FPTree::mine_patterns(
    const std::vector<HeaderEntry>& header_table,
    size_t min_support) {

    std::vector<FrequentPattern> patterns;

    // 从支持度最低的项开始挖掘
    for (auto it = header_table.rbegin(); it != header_table.rend(); ++it) {
        const auto& entry = *it;

        // 生成条件模式基
        auto conditional_patterns = mine_conditional_patterns(
            header_table, entry.item_id, min_support);

        // 添加单项模式
        patterns.emplace_back(ItemSet{entry.item_id}, entry.support);

        // 添加条件模式
        for (auto& pattern : conditional_patterns) {
            pattern.first.push_back(entry.item_id);
            patterns.push_back(std::move(pattern));
        }
    }

    return patterns;
}

std::vector<FPGrowth::FrequentPattern> FPGrowth::FPTree::mine_conditional_patterns(
    [[maybe_unused]] const std::vector<HeaderEntry>& header_table,
    [[maybe_unused]] size_t suffix_item,
    [[maybe_unused]] size_t min_support) {

    // 这里简化实现，实际应该构建条件FP树
    // 为了完整性，这里返回空结果
    return {};
}

} // namespace quant1x