# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from dataclasses import dataclass

@dataclass
class CompanyInfoChunk:
    title: str = ""
    """标题"""
    filename: str = ""
    """文件名"""
    offset: int = 0
    """偏移量"""
    size: int = 0
    """大小"""

    