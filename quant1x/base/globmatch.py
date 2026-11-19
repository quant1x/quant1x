# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

"""锚定 glob 匹配 (globstar).

整条路径从根起逐段比对: ``**`` 匹配零到多段, 其余段交给
:func:`fnmatch.fnmatchcase` (`*` / `?` / ``[...]``, 不跨段、大小写敏感、点开头的
名字照常匹配). 模式以 ``/**`` 结尾时要求该目录下至少还有一个条目 (等价
``/**/*``), 与 :meth:`pathlib.PurePath.full_match` 一致. 也即与 Python 3.13 的
``PurePath.full_match()``、bash 的 ``globstar`` 同义.

路径与模式都可写成**项目根绝对形式** (前导 ``/``), 与相对形式等价: 匹配一律以项目根为锚,
``/third_party/a.h`` 与 ``third_party/a.h`` 是同一个位置, 两侧可混写.

只做匹配, 不碰文件系统; 配置段怎么定义由调用方负责。
"""

from __future__ import annotations

import fnmatch
from typing import Sequence


def match(path: str, pattern: str) -> bool:
    """单条模式是否匹配整条路径 (锚定)

    :param path: 正斜杠分隔的路径 (通常相对某个根)
    :param pattern: 正斜杠分隔的模式
    :return: 整条匹配为真; `path` 或 `pattern` 为空时为假
    """
    if not path or not pattern:
        return False
    parts = [p for p in path.split("/") if p]
    pats = [p for p in pattern.split("/") if p]
    if not pats:
        return False
    if pats[-1] == "**":
        # 尾段 `/**` 表示「目录下至少一个条目」, 等价 `/**/*` (与 full_match 一致)
        pats.append("*")
    n, m = len(parts), len(pats)
    # dp[i][j] = parts[i:] 与 pats[j:] 是否匹配
    dp = [[False] * (m + 1) for _ in range(n + 1)]
    dp[n][m] = True
    for j in range(m - 1, -1, -1):
        if pats[j] == "**":
            dp[n][j] = dp[n][j + 1]
    for i in range(n - 1, -1, -1):
        for j in range(m - 1, -1, -1):
            if pats[j] == "**":
                dp[i][j] = dp[i][j + 1] or dp[i + 1][j]
            else:
                dp[i][j] = dp[i + 1][j + 1] and fnmatch.fnmatchcase(parts[i], pats[j])
    return dp[0][0]


def match_any(path: str, patterns: Sequence[str]) -> bool:
    """任一条模式匹配整条路径即为真

    :param path: 正斜杠分隔的路径
    :param patterns: 模式序列
    :return: 命中任一模式为真
    """
    return any(match(path, pattern) for pattern in patterns)


__all__ = ["match", "match_any"]
