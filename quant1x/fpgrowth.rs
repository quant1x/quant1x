//! FP-Growth 算法实现
//!
//! 纯Rust实现的FP-Growth频繁项集挖掘算法。
//! 支持事务数据集的频繁模式挖掘。
//!
//! # 示例
//!
//! ```
//! use quant1x::FPGrowthMiner;
//!
//! // 创建FP-Growth挖掘器，最小支持度为30%
//! let miner = FPGrowthMiner::new(0.3);
//!
//! // 示例事务数据集（购物篮数据）
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
//!     println!("模式 {:?} : 支持度 = {:.1}%", pattern, (support as f64) * 100.0);
//! }
//! ```

use std::collections::HashMap;

/// FP树节点
#[derive(Debug, Clone)]
struct FPNode {
    item: String,
    count: usize,
    parent: Option<usize>,            // 父节点索引
    children: HashMap<String, usize>, // 子节点映射
    next: Option<usize>,              // 相同项的链表
}

impl FPNode {
    fn new(item: String, count: usize, parent: Option<usize>) -> Self {
        Self {
            item,
            count,
            parent,
            children: HashMap::new(),
            next: None,
        }
    }
}

/// FP树结构
#[derive(Debug)]
struct FPTree {
    nodes: Vec<FPNode>,
    header_table: HashMap<String, (usize, Option<usize>)>, // (总计数, 链表头)
    root: usize,
}

impl FPTree {
    fn new() -> Self {
        let mut nodes = Vec::new();
        nodes.push(FPNode::new("null".to_string(), 0, None)); // 根节点

        Self {
            nodes,
            header_table: HashMap::new(),
            root: 0,
        }
    }

    fn insert(&mut self, transaction: &[String], count: usize) {
        let mut current = self.root;

        for item in transaction {
            // 获取或创建子节点
            let child_idx = if let Some(&idx) = self.nodes[current].children.get(item) {
                idx
            } else {
                let new_idx = self.nodes.len();
                self.nodes.push(FPNode::new(item.clone(), 0, Some(current)));
                self.nodes[current].children.insert(item.clone(), new_idx);
                new_idx
            };

            // 增加计数
            self.nodes[child_idx].count += count;

            // 更新头表
            let entry = self.header_table.entry(item.clone()).or_insert((0, None));
            entry.0 += count;

            // 更新链表
            if let Some(last) = entry.1 {
                let mut next = self.nodes[last].next;
                while let Some(n) = next {
                    if self.nodes[n].next.is_none() {
                        self.nodes[n].next = Some(child_idx);
                        break;
                    }
                    next = self.nodes[n].next;
                }
            } else {
                entry.1 = Some(child_idx);
            }

            current = child_idx;
        }
    }

    fn get_conditional_pattern_base(&self, item: &str) -> Vec<(Vec<String>, usize)> {
        let mut patterns = Vec::new();

        if let Some(&(_, node_idx_opt)) = self.header_table.get(item) {
            let mut node_idx = node_idx_opt;
            while let Some(idx) = node_idx {
                let mut path = Vec::new();
                let mut current = idx;
                let count = self.nodes[idx].count;

                // 向上遍历到根节点
                while let Some(parent) = self.nodes[current].parent {
                    if parent != self.root {
                        path.push(self.nodes[parent].item.clone());
                    }
                    current = parent;
                }

                if !path.is_empty() {
                    path.reverse();
                    patterns.push((path, count));
                }

                node_idx = self.nodes[idx].next;
            }
        }

        patterns
    }
}

/// FP-Growth 挖掘器
pub struct FPGrowthMiner {
    min_support_ratio: f64,
}

impl FPGrowthMiner {
    /// 创建新的FP-Growth挖掘器
    ///
    /// # 参数
    /// * `min_support` - 最小支持度阈值 (0.0-1.0之间的小数)
    pub fn new(min_support: f64) -> Self {
        Self {
            min_support_ratio: min_support,
        }
    }

    /// 从事务数据挖掘频繁模式
    ///
    /// # 参数
    /// * `transactions` - 事务数据集，每个事务是一个字符串向量
    ///
    /// # 返回
    /// 频繁模式及其支持度计数的向量
    pub fn mine(&self, transactions: &[Vec<String>]) -> Vec<(Vec<String>, usize)> {
        let total_transactions = transactions.len();
        let min_support_count =
            (self.min_support_ratio * total_transactions as f64).ceil() as usize;

        // 统计项频
        let mut item_counts = HashMap::new();
        for transaction in transactions {
            for item in transaction {
                *item_counts.entry(item.clone()).or_insert(0) += 1;
            }
        }

        // 筛选频繁项
        let mut frequent_items: Vec<_> = item_counts
            .into_iter()
            .filter(|&(_, count)| count >= min_support_count)
            .collect();

        // 按支持度降序排序
        frequent_items.sort_by(|a, b| b.1.cmp(&a.1));

        // 构建FP树
        let mut fp_tree = FPTree::new();
        for transaction in transactions {
            let mut sorted_transaction: Vec<String> = transaction
                .iter()
                .filter(|item| {
                    frequent_items
                        .iter()
                        .any(|(freq_item, _)| freq_item == *item)
                })
                .cloned()
                .collect();

            // 按频繁项顺序排序
            sorted_transaction.sort_by(|a, b| {
                let a_pos = frequent_items
                    .iter()
                    .position(|(item, _)| item == a)
                    .unwrap_or(usize::MAX);
                let b_pos = frequent_items
                    .iter()
                    .position(|(item, _)| item == b)
                    .unwrap_or(usize::MAX);
                a_pos.cmp(&b_pos)
            });

            if !sorted_transaction.is_empty() {
                fp_tree.insert(&sorted_transaction, 1);
            }
        }

        // 挖掘频繁模式
        let mut patterns = Vec::new();

        // 添加单个频繁项
        for (item, count) in &frequent_items {
            patterns.push((vec![item.clone()], *count));
        }

        // 递归挖掘
        for (item, _) in frequent_items.iter().rev() {
            let conditional_patterns = fp_tree.get_conditional_pattern_base(item);
            if !conditional_patterns.is_empty() {
                let sub_miner = FPGrowthMiner::new(self.min_support_ratio);
                let sub_patterns = sub_miner.mine_from_patterns(&conditional_patterns);

                // 添加后缀
                for (mut pattern, count) in sub_patterns {
                    pattern.push(item.clone());
                    patterns.push((pattern, count));
                }
            }
        }

        // 去重和排序
        let mut unique_patterns = HashMap::new();
        for (pattern, count) in patterns {
            let key = pattern.clone();
            unique_patterns.insert(key, count);
        }

        let mut result: Vec<_> = unique_patterns.into_iter().collect();
        result.sort_by(|a, b| b.1.cmp(&a.1)); // 按支持度降序

        result
    }

    /// 从条件模式基挖掘频繁模式（辅助方法）
    fn mine_from_patterns(
        &self,
        conditional_patterns: &[(Vec<String>, usize)],
    ) -> Vec<(Vec<String>, usize)> {
        let mut patterns = Vec::new();

        // 转换为事务格式
        let transactions: Vec<Vec<String>> = conditional_patterns
            .iter()
            .map(|(pattern, _)| pattern.clone())
            .collect();

        // 递归挖掘
        let sub_patterns = self.mine(&transactions);

        // 合并计数
        for (pattern, _) in sub_patterns {
            let total_count: usize = conditional_patterns.iter().map(|(_, count)| *count).sum();

            patterns.push((pattern, total_count));
        }

        patterns
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// 测试基本的频繁模式挖掘功能
    #[test]
    fn test_basic_frequent_patterns() {
        let miner = FPGrowthMiner::new(0.3);

        // 示例数据集：购物篮分析
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
                assert_eq!(*support, 6, "牛奶的支持度应该是6");
            } else if pattern == &vec!["面包".to_string()] {
                found_bread = true;
                assert_eq!(*support, 7, "面包的支持度应该是7");
            } else if pattern == &vec!["黄油".to_string()] {
                found_butter = true;
                assert_eq!(*support, 6, "黄油的支持度应该是6");
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
        let miner_low = FPGrowthMiner::new(0.3);
        let patterns_low = miner_low.mine(&transactions);

        // 较高的支持度
        let miner_high = FPGrowthMiner::new(0.8);
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
        let miner = FPGrowthMiner::new(0.1);
        let transactions: Vec<Vec<String>> = vec![];

        let patterns = miner.mine(&transactions);
        assert!(patterns.is_empty(), "空数据集应该返回空结果");
    }

    /// 测试单个事务
    #[test]
    fn test_single_transaction() {
        let miner = FPGrowthMiner::new(0.1);
        let transactions = vec![vec!["A".to_string(), "B".to_string(), "C".to_string()]];

        let patterns = miner.mine(&transactions);

        // 应该找到所有单个项和组合
        assert!(!patterns.is_empty(), "应该找到频繁模式");

        // 验证所有项的支持度都是1
        for (_, support) in &patterns {
            assert_eq!(*support, 1, "单个事务中所有模式的支持度应该是1");
        }
    }

    /// 测试最小支持度边界情况
    #[test]
    fn test_min_support_boundary() {
        let miner = FPGrowthMiner::new(1.0); // 100% 支持度
        let transactions = vec![
            vec!["A".to_string(), "B".to_string()],
            vec!["A".to_string()],
        ];

        let patterns = miner.mine(&transactions);

        // 只有出现在所有事务中的项才会保留
        for (pattern, _) in &patterns {
            if pattern.contains(&"A".to_string()) && pattern.len() == 1 {
                // A 应该被找到，因为它出现在所有事务中
                continue;
            }
            if pattern.contains(&"B".to_string()) && pattern.len() == 1 {
                panic!("B 不应该被找到，因为它没有出现在所有事务中");
            }
        }
    }
}
