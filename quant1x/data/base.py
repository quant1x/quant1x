# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

"""
基础数据类型定义
"""

from .adapter import PLUGIN_MASK_BASEDATA_DATA
from .meta.timestamp import Timestamp

# baseKind is the local offset for base data kinds (mirrors C++ baseKind)
BASEDATA_KIND = 0

# 基础数据类型常量
BASEDATA_XDXR                 = PLUGIN_MASK_BASEDATA_DATA | (BASEDATA_KIND +  1)  # 基础数据-除权除息
BASEDATA_RAW_DAILY_KLINE      = PLUGIN_MASK_BASEDATA_DATA | (BASEDATA_KIND +  2)  # 基础数据-未复权K线
BASEDATA_KLINE                = PLUGIN_MASK_BASEDATA_DATA | (BASEDATA_KIND +  3)  # 基础数据-前复权K线
BASEDATA_TRANSACTION          = PLUGIN_MASK_BASEDATA_DATA | (BASEDATA_KIND +  4)  # 基础数据-历史成交
BASEDATA_MINUTES              = PLUGIN_MASK_BASEDATA_DATA | (BASEDATA_KIND +  5)  # 基础数据-分时数据
BASEDATA_QUARTERLY_REPORTS    = PLUGIN_MASK_BASEDATA_DATA | (BASEDATA_KIND +  6)  # 基础数据-季报
BASEDATA_SAFETY_SCORE         = PLUGIN_MASK_BASEDATA_DATA | (BASEDATA_KIND +  7)  # 基础数据-安全分
BASEDATA_WIDE_KLINE           = PLUGIN_MASK_BASEDATA_DATA | (BASEDATA_KIND +  8)  # 基础数据-宽表
BASEDATA_PERFORMANCE_FORECAST = PLUGIN_MASK_BASEDATA_DATA | (BASEDATA_KIND +  9)  # 基础数据-业绩预告
BASEDATA_CHIP_DISTRIBUTION    = PLUGIN_MASK_BASEDATA_DATA | (BASEDATA_KIND + 10)  # 基础数据-筹码分布
BASEDATA_MINUTE_KLINE         = PLUGIN_MASK_BASEDATA_DATA | (BASEDATA_KIND + 11)  # 基础数据-分钟级别K线

# Market first list time
MarketCnFirstListTime = "1990-12-19"
GLOBAL_DEFAULT_START_DATE = "1900-01-01"

# MarketFirstDate is the market first-listing date as a pre-market Timestamp
market_first_date: Timestamp

# 初始化市场首日日期
# 与 C++ 严格保持一致：解析常量并取盘前时间。
try:
    ts = Timestamp.parse(MarketCnFirstListTime)
    market_first_date = ts.get_pre_market_time()
except Exception as e:
    raise RuntimeError(f"datasets: failed to parse MarketCnFirstListTime: {e}")