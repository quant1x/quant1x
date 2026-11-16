# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""解析失败与控制流异常, 以及"你是不是想输入"的相似项建议。

三类异常的分工:

- `UsageError`: 命令行输入不合法 (未知参数、缺值、取值非法、校验未通过), 进程退出码 2;
- `HelpRequested` / `VersionRequested`: 请求打印帮助 / 版本, 不算错误, 退出码 0。

异常只承载结构化字段 (`message` / `reason` / `lines` / `tips` / `usage`), 具体版式由
`help.render_error` 负责, 因此 `str(error)` 是非着色文本, 也是用例的断言对象。
"""

from __future__ import annotations

from enum import Enum
from typing import TYPE_CHECKING, Any, Iterable, Sequence

if TYPE_CHECKING:
    from .parser import Parser

__all__ = [
    "ArgParserError",
    "ErrorKind",
    "HelpRequested",
    "UsageError",
    "VersionRequested",
    "best_match",
]


class ArgParserError(Exception):
    """本模块所有异常的基类。"""


class ErrorKind(Enum):
    """用法错误的类别, 对应 clap 的 `ErrorKind`。

    调用方按类别分支, 不必匹配文案; 文案改了不影响判据。
    """

    UNKNOWN_ARGUMENT = "unknown_argument"
    AMBIGUOUS_ARGUMENT = "ambiguous_argument"
    MISSING_VALUE = "missing_value"
    INVALID_VALUE = "invalid_value"
    MISSING_REQUIRED = "missing_required"
    CONFLICT = "conflict"
    OTHER = "other"


class UsageError(ArgParserError):
    """命令行输入不合法。

    `kind` 是类别判据; `reason` 接在首行冒号之后 (如类型转换失败的原因); `lines` 逐条缩进
    列在首行之下 (如可选值清单、缺失的必填项); `tips` 是 `tip:` 开头的建议行。
    """

    exit_code = 2

    def __init__(
        self,
        message: str,
        *,
        kind: ErrorKind = ErrorKind.OTHER,
        reason: str | None = None,
        lines: Iterable[str] = (),
        tips: Iterable[str] = (),
        usage: str | None = None,
        hint: str | None = "For more information, try '--help'.",
    ) -> None:
        """构造: 登记错误的结构化字段, 版式留给 `help.render_error`

        :param message: 首行冒号前的错误消息
        :param kind: 类别判据
        :param reason: 接在首行冒号之后的原因
        :param lines: 逐条缩进列在首行之下
        :param tips: `tip:` 开头的建议行
        :param usage: usage 行文本
        :param hint: 末尾的帮助提示; `None` 表示不打印
        """
        super().__init__(message)
        self.message = message
        self.kind = kind
        self.reason = reason
        self.lines = tuple(lines)
        self.tips = tuple(tips)
        self.usage = usage
        self.hint = hint

    def __str__(self) -> str:
        # 延迟导入: 版式属于 help 模块, 放在模块顶部会与 parser 形成循环
        """转文本: 渲染成不着色的错误块

        :return: 错误块文本
        """
        from .help import render_error

        return render_error(self)


class HelpRequested(ArgParserError):
    """`-h` / `--help`: 请求帮助; `long` 为真表示走长帮助 (`--help`)。"""

    exit_code = 0

    def __init__(self, long: bool = False, parser: Parser | None = None) -> None:
        """构造: 记录请求的是长帮助还是短帮助

        :param long: 为真表示走长帮助 (`--help`)
        :param parser: 发起请求的解析器, 供调用方渲染
        """
        super().__init__("--help" if long else "-h")
        self.long = long
        self.parser = parser


class VersionRequested(ArgParserError):
    """`-V` / `--version`: 请求打印版本。"""

    exit_code = 0

    def __init__(self, parser: Parser | None = None) -> None:
        """构造: 记录发起版本请求的解析器

        :param parser: 发起请求的解析器, 供调用方渲染
        """
        super().__init__("--version")
        self.parser = parser


def _jaro(left: str, right: str) -> float:
    """算相似度: 计算 Jaro 相似度

    匹配窗口为 `max(长度) // 2 - 1`, 匹配后再按错位数折半修正。

    :param left: 待比较字符串
    :param right: 待比较字符串
    :return: 相似度, 落在 `[0, 1]`
    """
    if left == right:
        return 1.0
    left_len, right_len = len(left), len(right)
    if not left_len or not right_len:
        return 0.0
    window = max(0, max(left_len, right_len) // 2 - 1)
    left_hit = [False] * left_len
    right_hit = [False] * right_len
    matches = 0
    for index, char in enumerate(left):
        start = max(0, index - window)
        stop = min(index + window + 1, right_len)
        for position in range(start, stop):
            if not right_hit[position] and right[position] == char:
                left_hit[index] = right_hit[position] = True
                matches += 1
                break
    if not matches:
        return 0.0
    transpositions = 0
    cursor = 0
    for index in range(left_len):
        if not left_hit[index]:
            continue
        while not right_hit[cursor]:
            cursor += 1
        if left[index] != right[cursor]:
            transpositions += 1
        cursor += 1
    transpositions //= 2
    return (matches / left_len + matches / right_len + (matches - transpositions) / matches) / 3


def jaro_winkler(left: str, right: str) -> float:
    """算相似度: 在 Jaro 之上给相同前缀(最多 4 字符)加权

    :param left: 待比较字符串
    :param right: 待比较字符串
    :return: 相似度, 落在 `[0, 1]`
    """
    score = _jaro(left, right)
    prefix = 0
    for a, b in zip(left, right):
        if a != b or prefix == 4:
            break
        prefix += 1
    return score + prefix * 0.1 * (1.0 - score)


def best_match(word: str, candidates: Sequence[str], threshold: float = 0.8) -> str | None:
    """挑最像的: 在候选里挑与 `word` 最相似的一个

    阈值与 clap 的 `did_you_mean` 一致 (Jaro-Winkler > 0.8)。前缀相同的长选项容易越过阈值: `--verbose` 与 `--version` 的相似度为 0.8825, 输入错一处时二者会互相推荐。

    :param word: 用户输入的原文
    :param candidates: 候选集合
    :param threshold: 相似度下限
    :return: 最相似的候选; 没人越过阈值时为 `None`
    """
    best: str | None = None
    best_score = threshold
    for candidate in candidates:
        if not candidate or candidate == word:
            continue
        score = jaro_winkler(word, candidate)
        if score > best_score:
            best, best_score = candidate, score
    return best


def format_items(values: Sequence[Any]) -> str:
    """拼清单: 把可选值拼成 `a, b, c` 形态

    :param values: 取值序列
    :return: 逗号加空格分隔的文本
    """
    return ", ".join(str(value) for value in values)
