# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

"""num 规格层用例: 窗口、NaN、回归与舍入口径."""

import math
import unittest

import numpy as np

from quant1x import num


class ReduceTest(unittest.TestCase):
    """归约: 顺序累加与总体口径."""

    def test_sum_and_mean(self):
        """**用例**: 顺序累加与均值
        """
        self.assertEqual(num.sum_value([1.0, 2.0, 3.0, 4.0]), 10.0)
        self.assertEqual(num.mean([1.0, 2.0, 3.0, 4.0]), 2.5)

    def test_std_is_population(self):
        """**用例**: 标准差用总体口径(分母 n)
        """
        self.assertAlmostEqual(num.std([1.0, 2.0, 3.0, 4.0]), math.sqrt(1.25), places=15)

    def test_empty_conventions(self):
        """**用例**: 空输入时 sum 为 0、mean 为 NaN
        """
        self.assertEqual(num.sum_value([]), 0.0)
        self.assertTrue(math.isnan(num.mean([])))
        self.assertEqual(num.variance([]), 0.0)

    def test_nan_propagates(self):
        """**用例**: NaN 参与归约即 NaN
        """
        self.assertTrue(math.isnan(num.mean([1.0, math.nan])))


class RollingTest(unittest.TestCase):
    """滚动窗口: 完整窗口才计算, 含 NaN 即 NaN."""

    def test_incomplete_window(self):
        """**用例**: 窗口不足的位置为 NaN
        """
        result = num.rolling_mean([1.0, 2.0, 3.0, 4.0], 2)
        self.assertTrue(math.isnan(result[0]))
        self.assertEqual(result[1:].tolist(), [1.5, 2.5, 3.5])

    def test_nan_in_window(self):
        """**用例**: 窗口内含 NaN 的位置为 NaN
        """
        result = num.rolling_sum([1.0, math.nan, 3.0, 4.0], 2)
        self.assertTrue(math.isnan(result[1]))
        self.assertTrue(math.isnan(result[2]))
        self.assertEqual(result[3], 7.0)

    def test_variable_window(self):
        """**用例**: 逐位置窗口按位生效, 0 与负数视为无窗口
        """
        result = num.rolling_max([1.0, 3.0, 2.0, 5.0], [2, 2, 0, -1])
        self.assertTrue(math.isnan(result[0]))
        self.assertEqual(result[1], 3.0)
        self.assertTrue(math.isnan(result[2]))
        self.assertTrue(math.isnan(result[3]))

    def test_hhvbars_tie_takes_latest(self):
        """**用例**: 并列极值取最近的一个
        """
        self.assertEqual(num.rolling_hhvbars([1.0, 3.0, 3.0, 2.0], 4)[3], 1.0)


class EwmTest(unittest.TestCase):
    """指数加权: 首值作种子, NaN 跳过."""

    def test_seed_and_recursion(self):
        """**用例**: 首值等于种子, 后续按 alpha 递推
        """
        result = num.ewm_mean([1.0, 2.0, 3.0], 0.5)
        self.assertEqual(result.tolist(), [1.0, 1.5, 2.25])

    def test_leading_nan_skipped(self):
        """**用例**: 首个有效值之前为 NaN, 之后沿用前值
        """
        result = num.ewm_mean([math.nan, math.nan, 1.0, math.nan, 3.0], 0.5)
        self.assertTrue(math.isnan(result[0]))
        self.assertTrue(math.isnan(result[1]))
        self.assertEqual(result[2], 1.0)
        self.assertEqual(result[3], 1.0)
        self.assertEqual(result[4], 2.0)


class RegressionTest(unittest.TestCase):
    """回归: 均值中心化闭式解."""

    def test_slope_and_forcast(self):
        """**用例**: 线性序列的斜率与末端预测值
        """
        self.assertEqual(num.slope([1.0, 2.0, 3.0, 4.0]), 1.0)
        self.assertEqual(num.forcast([1.0, 2.0, 3.0, 4.0]), 4.0)

    def test_degenerate_window(self):
        """**用例**: 不足两点为 NaN
        """
        self.assertTrue(math.isnan(num.slope([1.0])))
        self.assertTrue(math.isnan(num.forcast([])))


class DecimalTest(unittest.TestCase):
    """小数位: 半程远离零."""

    def test_half_away_from_zero(self):
        """**用例**: 正负 0.5 都向远离零的方向进位
        """
        self.assertEqual(num.decimal(0.5, 0), 1.0)
        self.assertEqual(num.decimal(-0.5, 0), -1.0)
        self.assertEqual(num.decimal(10.005, 2), 10.01)


class AlignTest(unittest.TestCase):
    """对齐与条件选择."""

    def test_align_pads_nan(self):
        """**用例**: 不足补 NaN, 超长截断
        """
        self.assertEqual(num.align([1.0, 2.0], 4).tolist()[:2], [1.0, 2.0])
        self.assertTrue(math.isnan(num.align([1.0, 2.0], 4)[3]))
        self.assertEqual(num.align([1.0, 2.0, 3.0], 2).tolist(), [1.0, 2.0])

    def test_where(self):
        """**用例**: 条件选择按最长对齐
        """
        result = num.where([True, False], [1.0, 2.0], [3.0, 4.0])
        self.assertEqual(result.tolist(), [1.0, 4.0])

    def test_shift_fills_nan(self):
        """**用例**: 移位越界为 NaN
        """
        result = num.shift([1.0, 2.0, 3.0], 1)
        self.assertTrue(np.isnan(result[0]))
        self.assertEqual(result[1:].tolist(), [1.0, 2.0])


if __name__ == "__main__":
    unittest.main()
