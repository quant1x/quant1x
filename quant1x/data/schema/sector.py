# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

from dataclasses import dataclass, field
from typing import List

@dataclass
class Sector:
    """板块信息结构体"""
    name: str = ""
    code: str = ""
    type: int = 0
    count: int = 0
    block: str = ""
    constituent_stocks: List[str] = field(default_factory=List)
