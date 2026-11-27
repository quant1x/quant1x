# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

"""pandas 门面用例: 位置对齐、运算符与滚动/EWM 转发."""

import math
import unittest

from quant1x.pandas import DataFrame, Series


class SeriesTest(unittest.TestCase):
    """Series 的容器与运算符."""

    def test_arithmetic_is_elementwise(self):
        """**用例**: 算术运算符逐元素且返回新序列
        """
        left = Series([1.0, 2.0, 3.0])
        right = Series([10.0, 20.0, 30.0])
        self.assertEqual((left + right).values.tolist(), [11.0, 22.0, 33.0])
        self.assertEqual((left * 2).values.tolist(), [2.0, 4.0, 6.0])
        self.assertEqual(left.values.tolist(), [1.0, 2.0, 3.0])

    def test_comparison_returns_bool(self):
        """**用例**: 比较返回布尔序列
        """
        result = Series([1.0, 2.0, 3.0]) > 1.5
        self.assertEqual(result.values.dtype, bool)
        self.assertEqual(result.values.tolist(), [False, True, True])

    def test_rolling_forwards_to_num(self):
        """**用例**: rolling 与 quant1x.num 同口径
        """
        result = Series([1.0, 2.0, 3.0, 4.0]).rolling(2).mean()
        self.assertTrue(math.isnan(result.values[0]))
        self.assertEqual(result.values[1:].tolist(), [1.5, 2.5, 3.5])

    def test_ewm_span_and_alpha(self):
        """**用例**: span 换算成 alpha, 首值作种子
        """
        by_span = Series([1.0, 2.0, 3.0]).ewm(span=3).mean()
        by_alpha = Series([1.0, 2.0, 3.0]).ewm(alpha=0.5).mean()
        self.assertAlmostEqual(by_span.values[1], 1.5, places=15)
        self.assertEqual(by_alpha.values.tolist(), [1.0, 1.5, 2.25])

    def test_ewm_requires_one_parameter(self):
        """**用例**: span 与 alpha 必须二选一
        """
        with self.assertRaises(ValueError):
            Series([1.0]).ewm()
        with self.assertRaises(ValueError):
            Series([1.0]).ewm(span=3, alpha=0.5)


class DataFrameTest(unittest.TestCase):
    """DataFrame 的列读写."""

    def test_get_and_set(self):
        """**用例**: 取列得到 Series, 写列按位置对齐
        """
        frame = DataFrame({"a": [1.0, 2.0], "b": [3.0, 4.0]})
        frame["c"] = frame["a"] + frame["b"]
        self.assertEqual(frame.columns, ["a", "b", "c"])
        self.assertEqual(frame["c"].values.tolist(), [4.0, 6.0])

    def test_copy_is_independent(self):
        """**用例**: copy 后改副本不影响原表
        """
        frame = DataFrame({"a": [1.0, 2.0]})
        clone = frame.copy()
        clone["a"] = [9.0, 9.0]
        self.assertEqual(frame["a"].values.tolist(), [1.0, 2.0])


if __name__ == "__main__":
    unittest.main()
