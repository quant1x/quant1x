# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from .config import base_config as config
from .cache import MaxCachedDaysToDropOnIncrementalUpdate
from .market import detect_symbol, detect_instrument_type_by_rule
from .datasource import PlateCategory, DataHandler
__all__ = [
    "config",
    "MaxCachedDaysToDropOnIncrementalUpdate",
    "detect_symbol", "detect_instrument_type_by_rule",
    "PlateCategory", "DataHandler",
]
# from .security import securities, stock_name
# from .market import klines, get_period_name, convert_klines_trading, get_minutes_data, get_tick_transaction, date_format
# from .sector import block_list, sector_filename, get_sector_list, get_sector_constituents
# from .f10 import cache_f10, get_f10
from . import adapter

# from .xdxr import (
#     load_xdxr,
#     save_xdxr,
#     DataXdxr
# )
trains_begin_date = "2024-10-01"
