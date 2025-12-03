#include "fp_growth_core.h"
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <cmath>

namespace quant1x {

// FPGrowthCore 实现
FPGrowthCore::FPGrowthCore(double min_support)
    : min_support_(min_support),
      min_support_count_(0),
      use_count_threshold_(false) {}

FPGrowthCore::FPGrowthCore(size_t min_support_count)
    : min_support_(0.0),
      min_support_count_(min_support_count),
      use_count_threshold_(true) {}

FPGrowthCore::~FPGrowthCore() = default;

FPGrowthCore::FPGrowthCore(FPGrowthCore&&) noexcept = default;
FPGrowthCore& FPGrowthCore::operator=(FPGrowthCore&&) noexcept = default;

std::vector<FPGrowthCore::FrequentPattern> FPGrowthCore::mine(const Transactions& transactions) {
    if (transactions.empty()) {
        return {};
    }

    // 1. 第一步：计算所有项的频率
    auto item_counts = count_item_frequencies(transactions);
    size_t total_transactions = transactions.size();

    // 2. 第二步：计算最小支持度计数阈值
    size_t min_count = use_count_threshold_ ?
        min_support_count_ :
        static_cast<size_t>(std::ceil(min_support_ * static_cast<double>(total_transactions)));

    // 3. 第三步：获取满足最小支持度的频繁项，并按频率降序排序
    auto frequent_items = get_frequent_items(item_counts, min_count);

    if (frequent_items.empty()) {
        return {};
    }

    // 4. 第四步：构建FP树
    FPTree fp_tree;
    std::vector<HeaderEntry> header_table;
    std::unordered_map<size_t, size_t> rank_map;

    // 4.1 初始化项头表和排名映射（用于快速查找和排序）
    header_table.reserve(frequent_items.size());
    for (size_t i = 0; i < frequent_items.size(); ++i) {
        size_t item = frequent_items[i];
        header_table.emplace_back(item, item_counts[item]);
        rank_map[item] = i;
    }

    // 4.2 扫描事务数据库，将过滤后的事务插入FP树
    for (const auto& transaction : transactions) {
        Transaction filtered_transaction;
        filtered_transaction.reserve(transaction.size()); // 预分配内存
        for (size_t item : transaction) {
            if (item_counts[item] >= min_count) {
                filtered_transaction.push_back(item);
            }
        }

        if (!filtered_transaction.empty()) {
            sort_transaction_by_frequency(filtered_transaction, rank_map);
            fp_tree.insert(filtered_transaction, rank_map, header_table);
        }
    }

    // 5. 第五步：递归挖掘FP树，获取频繁模式
    auto internal_patterns = fp_tree.mine_patterns(header_table, min_count);

    // 6. 转换结果：将支持度计数转换为比率
    std::vector<FrequentPattern> patterns;
    patterns.reserve(internal_patterns.size());
    for (auto& p : internal_patterns) {
        double support_ratio = static_cast<double>(p.second) / total_transactions;
        patterns.emplace_back(std::move(p.first), support_ratio);
    }

    return patterns;
}

std::unordered_map<size_t, size_t> FPGrowthCore::count_item_frequencies(
    const Transactions& transactions) {

    std::unordered_map<size_t, size_t> counts;
    for (const auto& transaction : transactions) {
        for (size_t item : transaction) {
            counts[item]++;
        }
    }
    return counts;
}

std::vector<size_t> FPGrowthCore::get_frequent_items(
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

void FPGrowthCore::sort_transaction_by_frequency(
    Transaction& transaction,
    const std::unordered_map<size_t, size_t>& rank_map) {

    // 按频率降序排序（支持度高的项排在前面）
    // rank_map 中存储了项的排名，rank 越小，表示频率越高
    std::sort(transaction.begin(), transaction.end(),
              [&rank_map](size_t a, size_t b) {
                  return rank_map.at(a) < rank_map.at(b);
              });
}

// ===== FPTree 实现 =====

void FPGrowthCore::FPTree::insert(
    const Transaction& transaction,
    const std::unordered_map<size_t, size_t>& rank_map,
    std::vector<HeaderEntry>& header_table,
    size_t count) {

    FPNode* current = root_.get();
    for (size_t item : transaction) {
        auto it = current->children.find(item);
        if (it != current->children.end()) {
            // 如果子节点已存在，直接增加计数
            it->second->count += count;
            current = it->second.get();
        } else {
            // 如果子节点不存在，创建新节点
            auto new_node = std::make_unique<FPNode>(item, count, current);
            FPNode* new_node_ptr = new_node.get();
            current->children[item] = std::move(new_node);

            // 更新项头表（Header Table）的链表结构
            // 使用 rank_map 快速定位对应的 HeaderEntry
            size_t rank = rank_map.at(item);
            // header_table 的顺序与 rank_map 一致，直接通过下标访问
            auto& entry = header_table[rank];
            
            // 将新节点插入到链表头部
            new_node_ptr->next = entry.head;
            entry.head = new_node_ptr;

            current = new_node_ptr;
        }
    }
}

std::vector<FPGrowthCore::FPTree::InternalFrequentPattern> FPGrowthCore::FPTree::mine_patterns(
    const std::vector<HeaderEntry>& header_table,
    size_t min_support) {

    std::vector<InternalFrequentPattern> patterns;

    // 从项头表底部（支持度最低的项）开始向上挖掘
    for (auto it = header_table.rbegin(); it != header_table.rend(); ++it) {
        const auto& entry = *it;

        // 1. 生成当前项的条件模式基（Conditional Pattern Base）
        auto conditional_patterns = mine_conditional_patterns(
            header_table, entry.item_id, min_support);

        // 2. 生成包含当前项的频繁模式
        // 添加单项模式（当前项本身）
        patterns.emplace_back(ItemSet{entry.item_id}, entry.support);

        // 3. 将当前项合并到条件模式基挖掘出的模式中
        for (auto& pattern : conditional_patterns) {
            pattern.first.push_back(entry.item_id);
            patterns.push_back(std::move(pattern));
        }
    }

    return patterns;
}

std::vector<FPGrowthCore::FPTree::InternalFrequentPattern> FPGrowthCore::FPTree::mine_conditional_patterns(
    const std::vector<HeaderEntry>& header_table,
    size_t suffix_item,
    size_t min_support) {

    // 找到后缀项在项头表中的位置（Find suffix item in Header Table）
    const HeaderEntry* suffix_entry = nullptr;
    for (const auto& entry : header_table) {
        if (entry.item_id == suffix_item) {
            suffix_entry = &entry;
            break;
        }
    }

    if (!suffix_entry || !suffix_entry->head) {
        return {};  // 没有找到后缀项或没有节点
    }

    // 第一遍扫描：统计条件模式基中各项的频率
    // 遍历后缀项在FP树中的链表，向上回溯路径
    std::unordered_map<size_t, size_t> conditional_counts;
    FPNode* current = suffix_entry->head;
    while (current) {
        size_t path_count = current->count;
        FPNode* node = current->parent;
        while (node && node->item_id != 0) {  // 根节点的item_id为0
            conditional_counts[node->item_id] += path_count;
            node = node->parent;
        }
        current = current->next;
    }

    // 筛选出满足最小支持度的条件频繁项
    std::vector<std::pair<size_t, size_t>> conditional_items;
    for (const auto& pair : conditional_counts) {
        if (pair.second >= min_support) {
            conditional_items.emplace_back(pair.second, pair.first);
        }
    }

    if (conditional_items.empty()) {
        return {};
    }

    // 按支持度降序排序
    std::sort(conditional_items.rbegin(), conditional_items.rend());

    // 创建条件项的顺序映射
    std::vector<size_t> conditional_item_order;
    std::unordered_map<size_t, size_t> order_map;
    for (size_t i = 0; i < conditional_items.size(); ++i) {
        size_t item = conditional_items[i].second;
        conditional_item_order.push_back(item);
        order_map[item] = i;
    }

    // 构建条件FP树
    FPTree conditional_tree;
    std::vector<HeaderEntry> conditional_header_table;
    conditional_header_table.reserve(conditional_item_order.size());

    for (size_t item : conditional_item_order) {
        conditional_header_table.emplace_back(item, conditional_counts[item]);
    }

    // 第二遍扫描：构建条件FP树
    // 再次遍历后缀项的链表，将路径插入到新的条件FP树中
    current = suffix_entry->head;
    while (current) {
        size_t path_count = current->count;
        
        Transaction filtered_pattern;
        FPNode* node = current->parent;
        while (node && node->item_id != 0) {
            if (conditional_counts[node->item_id] >= min_support) {
                filtered_pattern.push_back(node->item_id);
            }
            node = node->parent;
        }

        if (!filtered_pattern.empty()) {
            // 按条件项的频率排序
            FPGrowthCore::sort_transaction_by_frequency(filtered_pattern, order_map);

            conditional_tree.insert(filtered_pattern, order_map,
                                   conditional_header_table, path_count);
        }
        current = current->next;
    }

    // 递归挖掘条件FP树
    return conditional_tree.mine_patterns(conditional_header_table, min_support);
}

} // namespace quant1x
