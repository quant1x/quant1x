#include "fp_growth.h"
#include "fp_growth_core.h"

namespace quant1x {

// FPGrowth<T> 实现
template <typename T>
FPGrowth<T>::FPGrowth(double min_support) 
    : core_(std::make_unique<FPGrowthCore>(min_support)) {}

template <typename T>
FPGrowth<T>::FPGrowth(size_t min_support_count) 
    : core_(std::make_unique<FPGrowthCore>(min_support_count)) {}

template <typename T>
FPGrowth<T>::~FPGrowth() = default;

template <typename T>
FPGrowth<T>::FPGrowth(FPGrowth&&) noexcept = default;

template <typename T>
FPGrowth<T>& FPGrowth<T>::operator=(FPGrowth&&) noexcept = default;

template <typename T>
std::vector<typename FPGrowth<T>::FrequentPattern> FPGrowth<T>::mine(const Transactions& transactions) {
    if (transactions.empty()) {
        return {};
    }

    // 1. 建立映射 T -> size_t
    std::unordered_map<T, size_t> item_to_id;
    std::vector<T> id_to_item;
    id_to_item.push_back(T()); // 0号ID保留

    FPGrowthCore::Transactions core_transactions;
    core_transactions.reserve(transactions.size());

    for (const auto& tx : transactions) {
        FPGrowthCore::Transaction core_tx;
        core_tx.reserve(tx.size());
        for (const auto& item : tx) {
            if (item_to_id.find(item) == item_to_id.end()) {
                size_t new_id = id_to_item.size();
                item_to_id[item] = new_id;
                id_to_item.push_back(item);
            }
            core_tx.push_back(item_to_id[item]);
        }
        core_transactions.push_back(std::move(core_tx));
    }

    // 2. 调用核心算法
    auto core_patterns = core_->mine(core_transactions);

    // 3. 映射回 T
    std::vector<FrequentPattern> patterns;
    patterns.reserve(core_patterns.size());

    for (const auto& p : core_patterns) {
        ItemSet itemset;
        itemset.reserve(p.first.size());
        for (size_t id : p.first) {
            itemset.push_back(id_to_item[id]);
        }
        patterns.emplace_back(std::move(itemset), p.second);
    }

    return patterns;
}

// 显式实例化
template class FPGrowth<std::string>;
template class FPGrowth<size_t>;
template class FPGrowth<int>;
template class FPGrowth<long>;

} // namespace quant1x

