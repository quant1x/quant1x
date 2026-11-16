# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""文件热加载: 候选文件动过就重新取数, 没动就回缓存（`Reloader`）。

判定只看候选文件的存在性与修改时间(`st_mtime_ns`), 不读内容也不做哈希 —— 每次取值只多几次
`stat`, 换来"改完文件立刻生效"。路径每次取值时现问, 故运行期新建的文件同样能被发现。

本模块与具体读取对象无关: `.env` 只是其一处用法, 任何"若干文件 -> 一个值"的过程都可套用。
"""

import os
from pathlib import Path
from typing import Callable, Generic, Iterable, TypeVar, cast

T = TypeVar('T')


def _snapshot(paths: Iterable[str | Path]) -> tuple[tuple[str, int | None], ...]:
    """拍快照: 按路径排序后逐个取修改时间

    排序只为让快照可比, 与优先级无关; 文件不存在时 mtime 记 `None`。

    :param paths: 候选文件路径
    :return: `(路径, mtime_ns)` 组成的元组
    """
    items = []
    for path in paths:
        text = str(path)
        try:
            items.append((text, os.stat(text).st_mtime_ns))
        except OSError:
            items.append((text, None))
    return tuple(sorted(items))


class Reloader(Generic[T]):
    """热加载容器: 包住"读这些文件算出一个值"的过程

    :param paths: 候选文件路径; 无参可调用, 每次取值时调用一次(可随时改主意)
    :param load: 取数函数, 无参; 仅在快照变化时被调用
    """

    def __init__(self, paths: Callable[[], Iterable[str | Path]], load: Callable[[], T]) -> None:
        self._paths = paths
        self._load = load
        self._stamp: tuple[tuple[str, int | None], ...] | None = None
        self._value: T | None = None

    def get(self) -> T:
        """取值: 快照与上次一致就回缓存, 否则重新取数并记下新快照

        :return: 取数函数的结果
        """
        stamp = _snapshot(self._paths())
        if stamp != self._stamp:
            self._value = self._load()
            self._stamp = stamp
        return cast(T, self._value)

    def reset(self) -> None:
        """作废缓存: 下一次 `get` 必定重新取数"""
        self._stamp = None
        self._value = None
