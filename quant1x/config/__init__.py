#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
@Project : quant1x
@Package : quant1x.config
@File    : __init__.py
@Author  : wangfeng
@Date    : 2025/9/15 16:28
@Desc    : 配置信息
"""
from .config import (
    get_quant1x_config_filename,
    get_historical_trade_filename,
    quant1x_config
)

# 导出常用配置路径
data_path = quant1x_config.data_path
meta_path = quant1x_config.meta_path
kline_path = quant1x_config.kline_path

__all__ = [
    "get_quant1x_config_filename",
    "get_historical_trade_filename",
    "data_path",
    "meta_path",
    "kline_path",
]
