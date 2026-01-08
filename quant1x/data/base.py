# -*- coding: UTF-8 -*-
"""
基础数据类型定义
"""

from .adapter import PLUGIN_MASK_BASE_DATA
from quant1x.exchange import Timestamp
from quant1x.exchange.code import MarketCnFirstListTime

# baseKind is the local offset for base data kinds (mirrors C++ baseKind)
BASE_KIND = 0

# 基础数据类型常量
BASE_XDXR                 = PLUGIN_MASK_BASE_DATA | (BASE_KIND +  1)  # 基础数据-除权除息
BASE_RAW_DAILY_KLINE      = PLUGIN_MASK_BASE_DATA | (BASE_KIND +  2)  # 基础数据-未复权K线
BASE_KLINE                = PLUGIN_MASK_BASE_DATA | (BASE_KIND +  3)  # 基础数据-前复权K线
BASE_TRANSACTION          = PLUGIN_MASK_BASE_DATA | (BASE_KIND +  4)  # 基础数据-历史成交
BASE_MINUTES              = PLUGIN_MASK_BASE_DATA | (BASE_KIND +  5)  # 基础数据-分时数据
BASE_QUARTERLY_REPORTS    = PLUGIN_MASK_BASE_DATA | (BASE_KIND +  6)  # 基础数据-季报
BASE_SAFETY_SCORE         = PLUGIN_MASK_BASE_DATA | (BASE_KIND +  7)  # 基础数据-安全分
BASE_WIDE_KLINE           = PLUGIN_MASK_BASE_DATA | (BASE_KIND +  8)  # 基础数据-宽表
BASE_PERFORMANCE_FORECAST = PLUGIN_MASK_BASE_DATA | (BASE_KIND +  9)  # 基础数据-业绩预告
BASE_CHIP_DISTRIBUTION    = PLUGIN_MASK_BASE_DATA | (BASE_KIND + 10)  # 基础数据-筹码分布
BASE_MINUTE_KLINE         = PLUGIN_MASK_BASE_DATA | (BASE_KIND + 11)  # 基础数据-分钟级别K线

# MarketFirstDate is the market first-listing date as a pre-market Timestamp
market_first_date: Timestamp

# 初始化市场首日日期
# 与 C++ 严格保持一致：解析常量并取盘前时间。
try:
    ts = Timestamp.parse(MarketCnFirstListTime)
    market_first_date = ts.get_pre_market_time()
except Exception as e:
    raise RuntimeError(f"datasets: failed to parse MarketCnFirstListTime: {e}")