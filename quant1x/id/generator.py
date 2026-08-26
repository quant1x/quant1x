# -*- coding: utf-8 -*-
# Copyright (c) 2026 Quant1X. All rights reserved.
# Author: wangfeng <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

from __future__ import annotations

from .hlc import HLC
from .uint128 import Uint128


class Generator:
    def __init__(self, node_id: int, hlc: HLC) -> None:
        if hlc is None:
            raise ValueError("id: nil HLC")
        self.hlc = hlc
        self.node_id = node_id & 0xFFFFFFFF

    def next(self) -> Uint128:
        hlc_value, seq = self.hlc.now()
        return Uint128(
            hi=hlc_value,
            lo=((self.node_id & 0xFFFFFFFF) << 32) | (seq & 0xFFFFFFFF),
        )