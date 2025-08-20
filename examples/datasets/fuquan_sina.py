#!/usr/bin/env python
# -*- coding:utf-8 -*-
"""
复权因子测试
"""

import akshare as ak
qfq_factor_df = ak.stock_zh_a_daily(symbol="sh600018", adjust="qfq-factor")
print(qfq_factor_df)