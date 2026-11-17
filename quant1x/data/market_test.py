# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""`data/market` 的涨停比例与 `formula.ZTPRICE` 的离线回归。

覆盖沪深主板、创业板、科创板、北交所、ST 统一 5% 与无涨跌幅市场,
以及涨停价四舍五入到分的边界(9.55 * 1.10 = 10.505 -> 10.51)。
"""

import unittest

import numpy as np
import pandas as pd

from ..formula.formula import ZTPRICE
from .market import limit_up_ratio


class LimitUpRatioTest(unittest.TestCase):
    def test_main_board(self):
        for code in ("SH600000", "600000", "000001", "002415"):
            with self.subTest(code=code):
                self.assertAlmostEqual(limit_up_ratio(code), 0.10)

    def test_chinext(self):
        for code in ("300750", "301269"):
            with self.subTest(code=code):
                self.assertAlmostEqual(limit_up_ratio(code), 0.20)

    def test_star(self):
        for code in ("688981", "689009"):
            with self.subTest(code=code):
                self.assertAlmostEqual(limit_up_ratio(code), 0.30)

    def test_bse(self):
        for code in ("920077", "830799", "430047", "870508"):
            with self.subTest(code=code):
                self.assertAlmostEqual(limit_up_ratio(code), 0.30)

    def test_st_applies_to_all_boards(self):
        for code in ("600000", "000001", "300750", "688981", "920077"):
            with self.subTest(code=code):
                self.assertAlmostEqual(limit_up_ratio(code, "*ST某某"), 0.05)

    def test_market_without_limit(self):
        self.assertAlmostEqual(limit_up_ratio("AAPL.US"), 0.0)
        self.assertAlmostEqual(limit_up_ratio("00700.HK"), 0.0)


class ZTPriceTest(unittest.TestCase):
    def test_scalar(self):
        self.assertAlmostEqual(ZTPRICE(10.00, 0.10), 11.00)
        self.assertAlmostEqual(ZTPRICE(9.99, 0.10), 10.99)
        self.assertAlmostEqual(ZTPRICE(10.00, 0.20), 12.00)
        self.assertAlmostEqual(ZTPRICE(10.00, 0.30), 13.00)

    def test_half_up_rounding(self):
        # 9.55 * 1.10 = 10.505; 四舍五入到分为 10.51, 银行家舍入会得到 10.50
        self.assertAlmostEqual(ZTPRICE(9.55, 0.10), 10.51)

    def test_zero_ratio(self):
        self.assertAlmostEqual(ZTPRICE(10.00, 0.0), 10.00)

    def test_series(self):
        prices = pd.Series([10.00, np.nan, 9.99])
        result = ZTPRICE(prices, 0.10)
        self.assertIsInstance(result, pd.Series)
        self.assertAlmostEqual(result.iloc[0], 11.00)
        self.assertTrue(np.isnan(result.iloc[1]))
        self.assertAlmostEqual(result.iloc[2], 10.99)

    def test_ndarray(self):
        prices = np.array([10.00, np.nan, 9.99])
        result = ZTPRICE(prices, 0.10)
        self.assertIsInstance(result, np.ndarray)
        self.assertAlmostEqual(result[0], 11.00)
        self.assertTrue(np.isnan(result[1]))
        self.assertAlmostEqual(result[2], 10.99)


if __name__ == "__main__":
    unittest.main()
