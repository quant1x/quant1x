# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

"""
核心数据结构定义(schema)

本包提供标准化、不可变的数据类(dataclass), 用于统一表示市场原始与衍生数据, 
包括 K 线(Bar)、分笔成交(Trade)、逐笔委托(Order)等。所有结构均设计为：
- 字段语义明确、类型严格
- 默认不可变(frozen=True), 保障数据一致性
- 可跨模块安全传递, 适用于回测、实盘、存储等场景

这些 schema 与 meta 中的元信息(如合约、频率、日历)协同工作, 共同构成量化系统的数据契约层。
"""
from .adjustment import XdxrInfo, XdxrEntry, XdxrCategory, CumulativeAdjustment
from .bar import Bar
from .trade import Direction, Transaction
from .sector import Sector

__all__ = [
    "XdxrInfo", "XdxrEntry", "XdxrCategory", "CumulativeAdjustment",
    "Bar",
    "Direction", "Transaction",
    "Sector",
]
