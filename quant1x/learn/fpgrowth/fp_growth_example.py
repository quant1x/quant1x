#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

"""
FP Growth 示例：挖掘频繁项集

FP Growth (Frequent Pattern Growth) 是一种高效的频繁项集挖掘算法，
适用于关联规则挖掘和市场篮子分析。

本示例使用 mlxtend 库实现 FP Growth。
"""

import pandas as pd
from mlxtend.frequent_patterns import fpgrowth
from mlxtend.preprocessing import TransactionEncoder

# 示例数据集：超市购物篮
transactions = [
    ['牛奶', '面包', '黄油'],
    ['牛奶', '面包'],
    ['牛奶', '黄油'],
    ['面包', '黄油'],
    ['牛奶', '面包', '黄油', '鸡蛋'],
    ['鸡蛋', '黄油'],
    ['牛奶', '鸡蛋'],
    ['牛奶', '面包', '鸡蛋'],
    ['牛奶', '面包', '黄油', '鸡蛋', '果汁'],
    ['果汁', '面包']
]

print("原始交易数据:")
for i, tx in enumerate(transactions, 1):
    print(f"交易 {i}: {tx}")

# 数据预处理：转换为 one-hot 编码
te = TransactionEncoder()
te_ary = te.fit(transactions).transform(transactions)
df = pd.DataFrame(te_ary, columns=te.columns_)

print("\nOne-hot 编码后的数据:")
print(df.head())

# 设置最小支持度 (min_support)
min_support = 0.3  # 30% 的交易中出现

print(f"\n使用 FP Growth 挖掘频繁项集 (min_support={min_support}):")

# 应用 FP Growth
frequent_itemsets = fpgrowth(df, min_support=min_support, use_colnames=True)

print("频繁项集:")
print(frequent_itemsets.sort_values('support', ascending=False))

# 可选：生成关联规则
from mlxtend.frequent_patterns import association_rules

if not frequent_itemsets.empty:
    print("\n关联规则 (min_confidence=0.7):")
    rules = association_rules(frequent_itemsets, metric="confidence", min_threshold=0.7)
    print(rules[['antecedents', 'consequents', 'support', 'confidence', 'lift']])

print("\nFP Growth 示例完成。")
