//! FP-Growth 算法实现
//!
//! 纯Rust实现的FP-Growth频繁项集挖掘算法. 
//! 支持事务数据集的频繁模式挖掘. 
//!
//! # 示例
//!
//! ```
//! use quant1x::learn::fpgrowth::FPGrowth;
//!
//! // 创建FP-Growth挖掘器, 最小支持度为30%
//! let miner = FPGrowth::new(0.3);
//!
//! // 示例事务数据集(购物篮数据)
//! let transactions = vec![
//!     vec!["牛奶".to_string(), "面包".to_string(), "尿布".to_string()],
//!     vec!["面包".to_string(), "啤酒".to_string()],
//!     vec!["面包".to_string(), "黄油".to_string()],
//!     vec!["牛奶".to_string(), "面包".to_string(), "啤酒".to_string()],
//!     vec!["牛奶".to_string(), "黄油".to_string()],
//!     vec!["尿布".to_string(), "啤酒".to_string()],
//!     vec!["尿布".to_string(), "黄油".to_string()],
//!     vec!["牛奶".to_string(), "面包".to_string(), "尿布".to_string(), "啤酒".to_string()],
//! ];
//!
//! // 挖掘频繁模式
//! let patterns = miner.mine(&transactions);
//!
//! // 输出结果
//! println!("发现的频繁模式数量: {}", patterns.len());
//! for (pattern, support) in patterns {
//!     println!("模式 {:?} : 支持度 = {:.2}", pattern, support);
//! }
//! ```

use std::collections::HashMap;
use std::hash::Hash;
use std::marker::PhantomData;

// ==========================================
// FPGrowthCore: 核心实现 (处理 usize 类型)
// ==========================================

#[derive(Debug, Clone)]
struct FPNode {
    item_id: usize,
    count: usize,
    parent: Option<usize>,
    children: HashMap<usize, usize>, // item_id -> node_index
    next: Option<usize>,             // 相同项的链表
}

impl FPNode {
    fn new(item_id: usize, count: usize, parent: Option<usize>) -> Self {
        Self {
            item_id,
            count,
            parent,
            children: HashMap::new(),
            next: None,
        }
    }
}

#[derive(Debug, Clone)]
struct HeaderEntry {
    item_id: usize,
    support: usize,
    head: Option<usize>, // 指向第一个节点的索引
}

struct FPTree {
    nodes: Vec<FPNode>,
    root: usize,
}

impl FPTree {
    fn new() -> Self {
        let root_node = FPNode::new(0, 0, None);
        Self {
            nodes: vec![root_node],
            root: 0,
        }
    }

    fn insert(
        &mut self,
        transaction: &[usize],
        rank_map: &HashMap<usize, usize>,
        header_table: &mut [HeaderEntry],
        count: usize,
    ) {
        let mut current_idx = self.root;

        for &item in transaction {
            // 检查子节点是否存在
            let child_idx_opt = self.nodes[current_idx].children.get(&item).copied();

            if let Some(child_idx) = child_idx_opt {
                self.nodes[child_idx].count += count;
                current_idx = child_idx;
            } else {
                // 创建新节点
                let new_node_idx = self.nodes.len();
                let new_node = FPNode::new(item, count, Some(current_idx));
                self.nodes.push(new_node);
                self.nodes[current_idx].children.insert(item, new_node_idx);

                // 更新项头表
                let rank = rank_map[&item];
                let entry = &mut header_table[rank];

                // 插入到链表头部
                self.nodes[new_node_idx].next = entry.head;
                entry.head = Some(new_node_idx);

                current_idx = new_node_idx;
            }
        }
    }

    fn mine_patterns(
        &self,
        header_table: &[HeaderEntry],
        min_support: usize,
    ) -> Vec<(Vec<usize>, usize)> {
        let mut patterns = Vec::new();

        // 从项头表底部向上挖掘
        for entry in header_table.iter().rev() {
            // 1. 生成条件模式基
            let conditional_patterns =
                self.mine_conditional_patterns(header_table, entry.item_id, min_support);

            // 2. 生成包含当前项的频繁模式
            patterns.push((vec![entry.item_id], entry.support));

            // 3. 合并
            for (mut pat, count) in conditional_patterns {
                pat.push(entry.item_id);
                patterns.push((pat, count));
            }
        }

        patterns
    }

    fn mine_conditional_patterns(
        &self,
        header_table: &[HeaderEntry],
        suffix_item: usize,
        min_support: usize,
    ) -> Vec<(Vec<usize>, usize)> {
        // 找到后缀项的 HeaderEntry
        let suffix_entry = header_table.iter().find(|e| e.item_id == suffix_item);
        if suffix_entry.is_none() || suffix_entry.unwrap().head.is_none() {
            return Vec::new();
        }
        let suffix_entry = suffix_entry.unwrap();

        // 第一遍扫描: 统计条件模式基频率
        let mut conditional_counts: HashMap<usize, usize> = HashMap::new();
        let mut current_opt = suffix_entry.head;

        while let Some(curr_idx) = current_opt {
            let path_count = self.nodes[curr_idx].count;
            let mut parent_opt = self.nodes[curr_idx].parent;

            while let Some(parent_idx) = parent_opt {
                let parent_node = &self.nodes[parent_idx];
                if parent_node.item_id != 0 {
                    // 0 is root
                    *conditional_counts.entry(parent_node.item_id).or_insert(0) += path_count;
                    parent_opt = parent_node.parent;
                } else {
                    break;
                }
            }
            current_opt = self.nodes[curr_idx].next;
        }

        // 筛选
        let mut conditional_items: Vec<(usize, usize)> = conditional_counts
            .into_iter()
            .filter(|&(_, count)| count >= min_support)
            .collect();

        if conditional_items.is_empty() {
            return Vec::new();
        }

        // 排序
        conditional_items.sort_by(|a, b| {
            if a.1 == b.1 {
                b.0.cmp(&a.0) // count相等时按id降序 (match C++)
            } else {
                b.1.cmp(&a.1) // count降序
            }
        });

        // 构建 Order Map 和 Header Table
        let mut order_map = HashMap::new();
        let mut conditional_header_table = Vec::new();
        for (i, &(item, count)) in conditional_items.iter().enumerate() {
            order_map.insert(item, i);
            conditional_header_table.push(HeaderEntry {
                item_id: item,
                support: count,
                head: None,
            });
        }

        // 构建条件树
        let mut conditional_tree = FPTree::new();

        // 第二遍扫描: 插入路径
        current_opt = suffix_entry.head;
        while let Some(curr_idx) = current_opt {
            let path_count = self.nodes[curr_idx].count;
            let mut filtered_pattern = Vec::new();
            let mut parent_opt = self.nodes[curr_idx].parent;

            while let Some(parent_idx) = parent_opt {
                let parent_node = &self.nodes[parent_idx];
                if parent_node.item_id != 0 {
                    if order_map.contains_key(&parent_node.item_id) {
                        filtered_pattern.push(parent_node.item_id);
                    }
                    parent_opt = parent_node.parent;
                } else {
                    break;
                }
            }

            if !filtered_pattern.is_empty() {
                // 按频率排序
                filtered_pattern.sort_by(|a, b| order_map[a].cmp(&order_map[b]));
                conditional_tree.insert(
                    &filtered_pattern,
                    &order_map,
                    &mut conditional_header_table,
                    path_count,
                );
            }

            current_opt = self.nodes[curr_idx].next;
        }

        conditional_tree.mine_patterns(&conditional_header_table, min_support)
    }
}

struct FPGrowthCore {
    min_support: f64,
    min_support_count: usize,
    use_count_threshold: bool,
}

impl FPGrowthCore {
    fn new(min_support: f64) -> Self {
        Self {
            min_support,
            min_support_count: 0,
            use_count_threshold: false,
        }
    }

    fn new_with_count(min_support_count: usize) -> Self {
        Self {
            min_support: 0.0,
            min_support_count,
            use_count_threshold: true,
        }
    }

    fn mine(&self, transactions: &[Vec<usize>]) -> Vec<(Vec<usize>, f64)> {
        if transactions.is_empty() {
            return Vec::new();
        }

        // 1. 统计频率
        let mut counts = HashMap::new();
        for tx in transactions {
            for &item in tx {
                *counts.entry(item).or_insert(0) += 1;
            }
        }

        let total_transactions = transactions.len();

        // 2. 计算最小支持度计数
        let min_count = if self.use_count_threshold {
            self.min_support_count
        } else {
            (self.min_support * total_transactions as f64).ceil() as usize
        };

        // 3. 获取频繁项并排序
        let mut frequent_items: Vec<(usize, usize)> = counts
            .into_iter()
            .filter(|&(_, count)| count >= min_count)
            .collect();

        if frequent_items.is_empty() {
            return Vec::new();
        }

        frequent_items.sort_by(|a, b| {
            if a.1 == b.1 {
                b.0.cmp(&a.0) // count相等时按id降序
            } else {
                b.1.cmp(&a.1) // count降序
            }
        });

        let mut header_table = Vec::new();
        let mut rank_map = HashMap::new();
        for (i, &(item, count)) in frequent_items.iter().enumerate() {
            header_table.push(HeaderEntry {
                item_id: item,
                support: count,
                head: None,
            });
            rank_map.insert(item, i);
        }

        // 4. 构建FP树
        let mut fp_tree = FPTree::new();
        for tx in transactions {
            let mut filtered_tx: Vec<usize> = tx
                .iter()
                .filter(|item| rank_map.contains_key(item))
                .cloned()
                .collect();

            if !filtered_tx.is_empty() {
                filtered_tx.sort_by(|a, b| rank_map[a].cmp(&rank_map[b]));
                fp_tree.insert(&filtered_tx, &rank_map, &mut header_table, 1);
            }
        }

        // 5. 挖掘
        let internal_patterns = fp_tree.mine_patterns(&header_table, min_count);

        // 6. 转换结果
        internal_patterns
            .into_iter()
            .map(|(items, count)| (items, count as f64 / total_transactions as f64))
            .collect()
    }
}

// ==========================================
// FPGrowth: 泛型接口
// ==========================================

pub struct FPGrowth<T> {
    core: FPGrowthCore,
    _marker: PhantomData<T>,
}

impl<T> FPGrowth<T>
where
    T: Hash + Eq + Clone + Ord,
{
    pub fn new(min_support: f64) -> Self {
        Self {
            core: FPGrowthCore::new(min_support),
            _marker: PhantomData,
        }
    }

    pub fn new_with_count(min_support_count: usize) -> Self {
        Self {
            core: FPGrowthCore::new_with_count(min_support_count),
            _marker: PhantomData,
        }
    }

    pub fn mine(&self, transactions: &[Vec<T>]) -> Vec<(Vec<T>, f64)> {
        if transactions.is_empty() {
            return Vec::new();
        }

        // 1. 映射 T -> usize
        let mut item_to_id = HashMap::new();
        let mut id_to_item = Vec::new();
        // 0号保留给Root
        id_to_item.push(None);

        let mut core_transactions = Vec::with_capacity(transactions.len());

        for tx in transactions {
            let mut core_tx = Vec::with_capacity(tx.len());
            for item in tx {
                let id = if let Some(&id) = item_to_id.get(item) {
                    id
                } else {
                    let new_id = id_to_item.len();
                    item_to_id.insert(item.clone(), new_id);
                    id_to_item.push(Some(item.clone()));
                    new_id
                };
                core_tx.push(id);
            }
            core_transactions.push(core_tx);
        }

        // 2. 调用核心算法
        let core_patterns = self.core.mine(&core_transactions);

        // 3. 映射回 T
        core_patterns
            .into_iter()
            .map(|(ids, support)| {
                let items: Vec<T> = ids
                    .into_iter()
                    .map(|id| id_to_item[id].as_ref().unwrap().clone())
                    .collect();
                (items, support)
            })
            .collect()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// 测试基本的频繁模式挖掘功能
    #[test]
    fn test_basic_frequent_patterns() {
        let miner = FPGrowth::new(0.3);

        // 示例数据集: 购物篮分析
        let transactions = vec![
            vec!["牛奶".to_string(), "面包".to_string(), "尿布".to_string()],
            vec!["面包".to_string(), "啤酒".to_string()],
            vec!["面包".to_string(), "黄油".to_string()],
            vec!["牛奶".to_string(), "面包".to_string(), "啤酒".to_string()],
            vec!["牛奶".to_string(), "黄油".to_string()],
            vec!["面包".to_string(), "黄油".to_string()],
            vec!["牛奶".to_string(), "黄油".to_string()],
            vec![
                "牛奶".to_string(),
                "面包".to_string(),
                "黄油".to_string(),
                "尿布".to_string(),
            ],
            vec!["牛奶".to_string(), "面包".to_string(), "黄油".to_string()],
        ];

        let patterns = miner.mine(&transactions);

        // 验证结果
        assert!(!patterns.is_empty(), "应该找到频繁模式");

        // 检查一些期望的模式
        let mut found_milk = false;
        let mut found_bread = false;
        let mut found_butter = false;

        for (pattern, support) in &patterns {
            if pattern == &vec!["牛奶".to_string()] {
                found_milk = true;
                assert!((support - 6.0 / 9.0).abs() < 1e-5, "牛奶的支持度应该是6/9");
            } else if pattern == &vec!["面包".to_string()] {
                found_bread = true;
                assert!((support - 7.0 / 9.0).abs() < 1e-5, "面包的支持度应该是7/9");
            } else if pattern == &vec!["黄油".to_string()] {
                found_butter = true;
                assert!((support - 6.0 / 9.0).abs() < 1e-5, "黄油的支持度应该是6/9");
            }
        }

        assert!(found_milk, "应该找到牛奶模式");
        assert!(found_bread, "应该找到面包模式");
        assert!(found_butter, "应该找到黄油模式");
    }

    /// 测试不同最小支持度阈值的影响
    #[test]
    fn test_different_min_support() {
        let transactions = vec![
            vec!["A".to_string(), "B".to_string()],
            vec!["A".to_string(), "C".to_string()],
            vec!["A".to_string(), "B".to_string(), "C".to_string()],
        ];

        // 较低的支持度
        let miner_low = FPGrowth::new(0.3);
        let patterns_low = miner_low.mine(&transactions);

        // 较高的支持度
        let miner_high = FPGrowth::new(0.8);
        let patterns_high = miner_high.mine(&transactions);

        // 较低支持度应该找到更多模式
        assert!(
            patterns_low.len() >= patterns_high.len(),
            "较低支持度应该找到更多或相等数量的模式"
        );
    }

    /// 测试空数据集
    #[test]
    fn test_empty_dataset() {
        let miner = FPGrowth::<String>::new(0.1);
        let transactions: Vec<Vec<String>> = vec![];

        let patterns = miner.mine(&transactions);
        assert!(patterns.is_empty(), "空数据集应该返回空结果");
    }

    /// 测试单个事务
    #[test]
    fn test_single_transaction() {
        let miner = FPGrowth::new(0.1);
        let transactions = vec![vec!["A".to_string(), "B".to_string(), "C".to_string()]];

        let patterns = miner.mine(&transactions);

        // 应该找到所有单个项和组合
        assert!(!patterns.is_empty(), "应该找到频繁模式");

        // 验证所有项的支持度都是1.0
        for (_, support) in &patterns {
            assert!(
                (support - 1.0).abs() < 1e-5,
                "单个事务中所有模式的支持度应该是1.0"
            );
        }
    }

    /// 测试最小支持度边界情况
    #[test]
    fn test_min_support_boundary() {
        let miner = FPGrowth::new(1.0); // 100% 支持度
        let transactions = vec![
            vec!["A".to_string(), "B".to_string()],
            vec!["A".to_string()],
        ];

        let patterns = miner.mine(&transactions);

        // 只有出现在所有事务中的项才会保留
        for (pattern, _) in &patterns {
            if pattern.contains(&"A".to_string()) && pattern.len() == 1 {
                // A 应该被找到, 因为它出现在所有事务中
                continue;
            }
            if pattern.contains(&"B".to_string()) && pattern.len() == 1 {
                panic!("B 不应该被找到, 因为它没有出现在所有事务中");
            }
        }
    }
}
