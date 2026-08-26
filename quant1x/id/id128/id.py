# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import base64
from dataclasses import dataclass

from .uint128 import Uint128


@dataclass(frozen=True)
class ID:
    raw: bytes

    def __post_init__(self) -> None:
        if len(self.raw) != 16:
            raise ValueError("ID expects exactly 16 bytes")

    @classmethod
    def from_uint128(cls, value: Uint128) -> "ID":
        return cls(value.to_bytes())

    def bytes(self) -> bytes:
        return self.raw

    def __str__(self) -> str:
        return base64.urlsafe_b64encode(self.raw).rstrip(b"=").decode("ascii")

    def node_id(self) -> int:
        return int.from_bytes(self.raw[8:12], "big")

    def seq(self) -> int:
        return int.from_bytes(self.raw[12:16], "big")

    def hlc(self) -> int:
        return int.from_bytes(self.raw[0:8], "big")