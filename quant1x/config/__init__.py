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
    quant1x_config
)

__all__ = [
    "quant1x_config",
    "get_quant1x_config_filename",
]
