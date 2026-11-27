# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

"""ndarray 用例: 容器语义与 numpy 数组对齐."""

import math
import unittest

import numpy as np

from quant1x.num import NDArray


class ConstructTest(unittest.TestCase):
    """构造与属性."""

    def test_basic(self):
        """**用例**: 构造、长度、形状、类型
        """
        a = NDArray([1, 2, 3])
        self.assertEqual(a.size, 3)
        self.assertEqual(len(a), 3)
        self.assertEqual(a.shape, (3,))
        self.assertEqual(a.ndim, 1)
        self.assertEqual(a.dtype, "float64")
        self.assertEqual(a.to_list(), [1.0, 2.0, 3.0])

    def test_factories(self):
        """**用例**: zeros / ones / full / arange / linspace
        """
        self.assertEqual(NDArray.zeros(3).to_list(), [0.0, 0.0, 0.0])
        self.assertEqual(NDArray.ones(2).to_list(), [1.0, 1.0])
        self.assertEqual(NDArray.full(2, 5.0).to_list(), [5.0, 5.0])
        self.assertEqual(NDArray.arange(0, 5, 1).to_list(), [0.0, 1.0, 2.0, 3.0, 4.0])
        self.assertEqual(NDArray.linspace(0, 1, 3).to_list(), [0.0, 0.5, 1.0])

    def test_bool_dtype(self):
        """**用例**: 布尔列保留 bool 类型
        """
        b = NDArray([True, False])
        self.assertEqual(b.dtype, "bool")
        self.assertEqual(b.to_list(), [True, False])


class ArithmeticTest(unittest.TestCase):
    """逐元素运算."""

    def test_binary(self):
        """**用例**: 数组与数组、数组与标量
        """
        a = NDArray([1.0, 2.0, 3.0])
        b = NDArray([10.0, 20.0, 30.0])
        self.assertEqual(a.add(b).to_list(), [11.0, 22.0, 33.0])
        self.assertEqual(a.sub(b).to_list(), [-9.0, -18.0, -27.0])
        self.assertEqual(a.mul(b).to_list(), [10.0, 40.0, 90.0])
        self.assertEqual(b.div(a).to_list(), [10.0, 10.0, 10.0])
        self.assertEqual((a + 1).to_list(), [2.0, 3.0, 4.0])
        self.assertEqual((1 - a).to_list(), [0.0, -1.0, -2.0])

    def test_unary(self):
        """**用例**: 绝对值、平方根、对数、幂
        """
        self.assertEqual(NDArray([-1.0, 2.0]).abs().to_list(), [1.0, 2.0])
        self.assertEqual(NDArray([4.0, 9.0]).sqrt().to_list(), [2.0, 3.0])
        self.assertEqual(NDArray([1.0, math.e]).log().to_list(), [0.0, 1.0])
        self.assertEqual(NDArray([2.0]).pow(3).to_list(), [8.0])


class CompareLogicTest(unittest.TestCase):
    """比较与逻辑."""

    def test_compare(self):
        """**用例**: 比较返回布尔数组
        """
        a = NDArray([1.0, 2.0, 3.0])
        self.assertEqual(a.gt(2).to_list(), [False, False, True])
        self.assertEqual(a.gte(2).to_list(), [False, True, True])
        self.assertEqual(a.lt(2).to_list(), [True, False, False])
        self.assertEqual(a.equal(2).to_list(), [False, True, False])
        self.assertEqual(a.not_equal(2).to_list(), [True, False, True])
        self.assertEqual((a > 2).to_list(), [False, False, True])

    def test_logic(self):
        """**用例**: 逻辑与/或/非
        """
        a = NDArray([True, True, False, False])
        b = NDArray([True, False, True, False])
        self.assertEqual(a.logic_and(b).to_list(), [True, False, False, False])
        self.assertEqual(a.logic_or(b).to_list(), [True, True, True, False])
        self.assertEqual(a.logic_not().to_list(), [False, False, True, True])
        self.assertEqual((a & b).to_list(), [True, False, False, False])


class ReduceTest(unittest.TestCase):
    """归约."""

    def test_basic(self):
        """**用例**: 求和、均值、中位数、极值、方差、标准差
        """
        a = NDArray([1.0, 2.0, 3.0, 4.0, 5.0])
        self.assertEqual(a.sum(), 15.0)
        self.assertEqual(a.mean(), 3.0)
        self.assertEqual(a.median(), 3.0)
        self.assertEqual(a.min(), 1.0)
        self.assertEqual(a.max(), 5.0)
        self.assertEqual(a.argmin(), 0)
        self.assertEqual(a.argmax(), 4)
        self.assertEqual(a.var(), 2.0)
        self.assertAlmostEqual(a.std(), math.sqrt(2.0), places=15)

    def test_cumsum(self):
        """**用例**: 累计和
        """
        self.assertEqual(NDArray([1.0, 2.0, 3.0]).cumsum().to_list(), [1.0, 3.0, 6.0])


class WindowTest(unittest.TestCase):
    """移位、差分、滚动、EWM."""

    def test_shift_diff(self):
        """**用例**: 移位越界 NaN, 差分
        """
        a = NDArray([1.0, 2.0, 3.0])
        shifted = a.shift(1)
        self.assertTrue(math.isnan(shifted[0]))
        self.assertEqual(shifted.to_list()[1:], [1.0, 2.0])
        diff = a.diff(1)
        self.assertTrue(math.isnan(diff[0]))
        self.assertEqual(diff.to_list()[1:], [1.0, 1.0])

    def test_rolling(self):
        """**用例**: 滚动窗口完整才计算
        """
        a = NDArray([1.0, 2.0, 3.0, 4.0])
        mean = a.rolling_mean(2)
        self.assertTrue(math.isnan(mean[0]))
        self.assertEqual(mean.to_list()[1:], [1.5, 2.5, 3.5])
        self.assertEqual(a.rolling_sum(2).to_list()[1:], [3.0, 5.0, 7.0])

    def test_ewm(self):
        """**用例**: 指数加权首值作种子
        """
        a = NDArray([1.0, 2.0, 3.0])
        values = a.ewm_mean(0.3).to_list()
        self.assertEqual(values[0], 1.0)
        self.assertAlmostEqual(values[1], 1.3, places=15)


class MiscTest(unittest.TestCase):
    """切片、填充、类型转换."""

    def test_slice_and_fill(self):
        """**用例**: 切片返回新数组, NaN 填充
        """
        a = NDArray([1.0, math.nan, 3.0])
        self.assertEqual(a.slice(0, 1).to_list(), [1.0])
        filled = a.fill_na(0.0)
        self.assertEqual(filled.to_list(), [1.0, 0.0, 3.0])

    def test_astype(self):
        """**用例**: float64 与 bool 互转
        """
        a = NDArray([1.0, 0.0, 2.0])
        mask = a.astype("bool")
        self.assertEqual(mask.dtype, "bool")
        self.assertEqual(mask.to_list(), [True, False, True])
        restored = mask.astype("float64")
        self.assertEqual(restored.to_list(), [1.0, 0.0, 1.0])


if __name__ == "__main__":
    unittest.main()
