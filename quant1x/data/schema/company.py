# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

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

    