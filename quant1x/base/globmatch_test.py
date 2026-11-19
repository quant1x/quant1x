# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

"""globmatch 的用例: 锚定 globstar 的语义边界."""

from __future__ import annotations

import unittest
from pathlib import PurePosixPath

from .globmatch import match, match_any

# 3.13+ 才有 full_match; 经 getattr 取, 3.12 的 typeshed 里没有该属性
_FULL_MATCH = getattr(PurePosixPath, "full_match", None)


class MatchTest(unittest.TestCase):
    """锚定 glob 匹配 (globstar)."""

    def test_globstar_zero_or_more_segments(self):
        """**用例**: `**` 匹配零到多段
        """
        self.assertTrue(match("stb.h", "**/*.h"))
        self.assertTrue(match("a/b/c.h", "**/*.h"))
        self.assertTrue(match("a/c.h", "a/**/c.h"))
        self.assertTrue(match("a/x/y/c.h", "a/**/c.h"))
        self.assertTrue(match("third_party/stb.h", "third_party/**"))
        self.assertTrue(match("third_party/a/b.h", "third_party/**"))

    def test_anchored(self):
        """**用例**: 整条路径从根起匹配, 不从中间开始
        """
        self.assertTrue(match("libs/stb.h", "libs/*.h"))
        self.assertFalse(match("libs/sub/stb.h", "libs/*.h"))
        self.assertFalse(match("x/libs/stb.h", "libs/*.h"))
        self.assertFalse(match("x/third_party/a.h", "third_party/**"))
        self.assertFalse(match("third_partyish/x.h", "third_party/**"))
        self.assertFalse(match("b/a/c.h", "a/**/c.h"))

    def test_single_segment_wildcards(self):
        """**用例**: `*` 不跨段, `?` 与 `[...]` 按段解释
        """
        self.assertTrue(match("a.h", "*.h"))
        self.assertFalse(match("a.c", "*.h"))
        self.assertTrue(match("a1.h", "a?.h"))
        self.assertTrue(match("a1.h", "a[0-9].h"))
        self.assertFalse(match("a.h", "a[0-9].h"))
        self.assertTrue(match("a-.h", "a[a-].h"))
        self.assertFalse(match("az.h", "a[a-].h"))
        self.assertTrue(match("a[b.h", "a[b.h"))

    def test_root_absolute_form(self):
        """**用例**: 项目根绝对写法（前导 `/`）与相对写法等价, 两侧可混写
        """
        self.assertTrue(match("/third_party/a.h", "third_party/**"))
        self.assertTrue(match("third_party/a.h", "/third_party/**"))
        self.assertTrue(match("/third_party/a.h", "/third_party/**"))
        self.assertFalse(match("/x/third_party/a.h", "/third_party/**"))
        self.assertTrue(match("/a/b.h", "/**/*.h"))

    def test_dotfile_and_case(self):
        """**用例**: 点开头的名字照常匹配 (fnmatch 语义), 大小写敏感
        """
        self.assertTrue(match(".hidden.h", "*.h"))
        self.assertFalse(match("A.H", "*.h"))

    def test_trailing_globstar_needs_one_segment(self):
        """**用例**: 模式以 `/**` 结尾时要求目录下至少一个条目 (等价 `/**/*`)
        """
        self.assertFalse(match("a", "a/**"))
        self.assertTrue(match("a/b", "a/**"))
        self.assertTrue(match("a/b/c", "a/**"))
        self.assertTrue(match("a", "**"))

    def test_empty_inputs(self):
        """**用例**: 空路径或空模式一律不匹配
        """
        self.assertFalse(match("", "**"))
        self.assertFalse(match("a.h", ""))
        self.assertFalse(match_any("a.h", []))

    def test_match_any(self):
        """**用例**: 任一条模式命中即为真
        """
        self.assertTrue(match_any("libs/a.h", ["third_party/**", "libs/*.h"]))
        self.assertFalse(match_any("libs/a.h", ["third_party/**", "**.c"]))

    @unittest.skipUnless(_FULL_MATCH is not None, "需要 Python 3.13+ 的 full_match")
    def test_parity_with_full_match(self):
        """**用例**: 与 `PurePosixPath.full_match()` 逐例一致 (规则名以此为准)
        """
        assert _FULL_MATCH is not None
        paths = ["a", "a/b", "a/b/c.h", "a/x/y/c.h", "b/a/c.h", "stb.h", "a/.h",
                 "libs/a.h", "libs/sub/a.h", "x/libs/a.h", "third_party/a.h"]
        patterns = ["**", "*.h", "**/*.h", "a/**", "a/**/c.h", "libs/*.h",
                    "third_party/**", "a/b", "*/b", "a/*/c.h"]
        for path in paths:
            for pattern in patterns:
                self.assertEqual(
                    match(path, pattern),
                    _FULL_MATCH(PurePosixPath(path), pattern),
                    f"{path} vs {pattern}",
                )


if __name__ == "__main__":
    unittest.main()
