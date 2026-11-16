# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""`base/expandvars` 的离线回归: `$NAME` / `${NAME}` 的展开与各类边界。"""

import unittest

from . import expandvars


class ExpandvarsTest(unittest.TestCase):
    def test_both_forms(self) -> None:
        """`${NAME}` 与 `$NAME` 两种写法都换成取值"""
        values = {"BASE": "/srv/data"}
        self.assertEqual("/srv/data/a", expandvars.expand("${BASE}/a", values))
        self.assertEqual("/srv/data/b", expandvars.expand("$BASE/b", values))

    def test_recursive(self) -> None:
        """引用套引用: 逐层展开到底"""
        values = {"C": "leaf", "B": "${C}", "A": "${B}"}
        self.assertEqual("leaf", expandvars.expand("${A}", values))

    def test_undefined_is_kept(self) -> None:
        """查不到引用时原样保留, 不吃成空串"""
        self.assertEqual("${MISSING}/x", expandvars.expand("${MISSING}/x", {"OTHER": "x"}))

    def test_circular_is_kept(self) -> None:
        """引用成环时整段原样返回, 不做半展开也不抛错"""
        values = {"A": "${B}", "B": "${A}"}
        self.assertEqual("${B}", expandvars.expand(values["A"], values))
        self.assertEqual("${A}", expandvars.expand(values["B"], values))

    def test_double_dollar_is_literal(self) -> None:
        """`$$` 展开成字面 `$`, 后面的名字不再当引用"""
        self.assertEqual("${OTHER}", expandvars.expand("$${OTHER}", {"OTHER": "x"}))
        self.assertEqual("$", expandvars.expand("$$", {}))

    def test_fallback_used_when_values_miss(self) -> None:
        """键值表没命中时查兜底表"""
        self.assertEqual("/os/x", expandvars.expand("${PROBE}/x", {}, {"PROBE": "/os"}))

    def test_values_beat_fallback(self) -> None:
        """两边都有同一个键时以键值表为准"""
        self.assertEqual("/own", expandvars.expand("${PROBE}", {"PROBE": "/own"}, {"PROBE": "/os"}))

    def test_no_fallback_keeps_literal(self) -> None:
        """没给兜底表时, 键值表里没有的引用原样保留"""
        self.assertEqual("${PROBE}", expandvars.expand("${PROBE}", {}))

    def test_unrecognized_shapes_are_kept(self) -> None:
        """不认的写法原样保留: `$1` 不是合法键名, `${A:-x}` 是带修饰的形式"""
        self.assertEqual("$1 / ${A:-x}", expandvars.expand("$1 / ${A:-x}", {"A": "x"}))


if __name__ == "__main__":
    unittest.main()
