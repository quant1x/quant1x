# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""变量展开: 把文本里的 `$NAME` / `${NAME}` 换成对应取值（`expand`）。

取值先查调用方给的键值表, 表里没有的再查兜底表(通常是系统环境变量), 两处都没有就原样保留 ——
留痕好过静默吃成空串。被引用到的值本身带引用时继续展开; 引用成环时整段原样返回, 不做半展开。
`$$` 是字面 `$`, 其后紧跟的名字不再当引用。

键名只认 `[A-Za-z_][A-Za-z0-9_]*`, 不认 `${NAME:-默认值}` 这类带修饰的写法 —— 需要默认值由调用方
自己兜。不做引号区分: 传进来的文本是剥完引号的值, 一律展开。
"""

import re
from typing import Mapping

# 展开目标: `$$`(转义) / `${NAME}` / `$NAME`
PATTERN = re.compile(r'\$(\$)|\$\{([A-Za-z_][A-Za-z0-9_]*)\}|\$([A-Za-z_][A-Za-z0-9_]*)')


class _Cycle(Exception):
    """展开链成环: 内部信号, 不在 `expand` 之外露出"""


def _lookup(name: str, values: Mapping[str, str], fallback: Mapping[str, str] | None) -> str | None:
    """查值: 先查键值表, 再查兜底表

    :param name: 变量名
    :param values: 键值表
    :param fallback: 兜底表; 传 `None` 表示不兜底
    :return: 命中值; 两处都没有时为 `None`
    """
    if name in values:
        return values[name]
    if fallback is not None and name in fallback:
        return fallback[name]
    return None


def _replace(text: str, values: Mapping[str, str], fallback: Mapping[str, str] | None, seen: frozenset[str]) -> str:
    """逐处替换: 递归展开被引用到的值, `seen` 是当前展开链上的键, 用于断环

    :param text: 待展开的文本
    :param values: 键值表
    :param fallback: 兜底表; 传 `None` 表示不兜底
    :param seen: 展开链上已引用过的键
    :return: 展开后的文本
    :raises _Cycle: 引用成环
    """
    def hit(match: re.Match[str]) -> str:
        if match.group(1):
            return '$'
        name = match.group(2) or match.group(3)
        if name in seen:
            raise _Cycle
        referenced = _lookup(name, values, fallback)
        if referenced is None:
            return match.group(0)
        return _replace(referenced, values, fallback, seen | {name})

    return PATTERN.sub(hit, text)


def expand(text: str, values: Mapping[str, str], fallback: Mapping[str, str] | None = None) -> str:
    """展开文本: 把 `$NAME` / `${NAME}` 换成对应取值

    引用成环时整段原样返回, 不做半展开 —— 环上的键互相指向, 半截结果只会让人误读。

    :param text: 待展开的文本
    :param values: 键值表, 优先查
    :param fallback: 兜底表(如系统环境变量), 键值表没命中时查; 传 `None` 表示不兜底
    :return: 展开后的文本
    """
    try:
        return _replace(text, values, fallback, frozenset())
    except _Cycle:
        return text
