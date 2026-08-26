# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

from dataclasses import dataclass
from functools import total_ordering


_MASK64 = (1 << 64) - 1
_MASK128 = (1 << 128) - 1


@total_ordering
@dataclass(frozen=True)
class Uint128:
    hi: int = 0
    lo: int = 0

    def __post_init__(self) -> None:
        object.__setattr__(self, "hi", self.hi & _MASK64)
        object.__setattr__(self, "lo", self.lo & _MASK64)

    @classmethod
    def from64(cls, value: int) -> "Uint128":
        return cls(0, value)

    @classmethod
    def from_bytes(cls, raw: bytes) -> "Uint128":
        if len(raw) != 16:
            raise ValueError("Uint128 expects exactly 16 bytes")
        return cls(
            int.from_bytes(raw[0:8], "big"),
            int.from_bytes(raw[8:16], "big"),
        )

    @classmethod
    def from_int(cls, value: int) -> "Uint128":
        value &= _MASK128
        return cls((value >> 64) & _MASK64, value & _MASK64)

    def to_int(self) -> int:
        return (self.hi << 64) | self.lo

    def to_bytes(self) -> bytes:
        return self.hi.to_bytes(8, "big") + self.lo.to_bytes(8, "big")

    def compare(self, other: "Uint128") -> int:
        if self.hi < other.hi:
            return -1
        if self.hi > other.hi:
            return 1
        if self.lo < other.lo:
            return -1
        if self.lo > other.lo:
            return 1
        return 0

    def __lt__(self, other: object) -> bool:
        if not isinstance(other, Uint128):
            return NotImplemented
        return self.compare(other) < 0

    def add(self, other: "Uint128") -> "Uint128":
        return Uint128.from_int(self.to_int() + other.to_int())

    def sub(self, other: "Uint128") -> "Uint128":
        return Uint128.from_int(self.to_int() - other.to_int())

    def inc(self) -> "Uint128":
        return self.add(UINT128_ONE)

    def dec(self) -> "Uint128":
        return self.sub(UINT128_ONE)

    def lsh(self, shift: int) -> "Uint128":
        if shift >= 128:
            return UINT128_ZERO
        return Uint128.from_int((self.to_int() << shift) & _MASK128)

    def rsh(self, shift: int) -> "Uint128":
        if shift >= 128:
            return UINT128_ZERO
        return Uint128.from_int(self.to_int() >> shift)

    def or_(self, other: "Uint128") -> "Uint128":
        return Uint128(self.hi | other.hi, self.lo | other.lo)

    def and_(self, other: "Uint128") -> "Uint128":
        return Uint128(self.hi & other.hi, self.lo & other.lo)

    def xor(self, other: "Uint128") -> "Uint128":
        return Uint128(self.hi ^ other.hi, self.lo ^ other.lo)

    def not_(self) -> "Uint128":
        return Uint128(~self.hi & _MASK64, ~self.lo & _MASK64)

    def is_zero(self) -> bool:
        return self.hi == 0 and self.lo == 0


UINT128_ZERO = Uint128()
UINT128_ONE = Uint128(lo=1)
UINT128_MAX = Uint128(hi=_MASK64, lo=_MASK64)