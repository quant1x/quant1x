# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""帮助与错误的版式: usage 行、分节帮助、错误块与 ANSI 着色。

版式固定, 不随内容猜测: 章节顺序是 `Usage` → `Commands` → `Arguments` → `Options` → 自定义分组
→ `epilog`; 左列超过半幅时说明文字换到下一行并对齐缩进。颜色只加在标题与选项字面量上,
`[default: ...]` / `[env: ...]` / `[possible values: ...]` 一律保留原样, 便于用例按文本断言。
"""

from __future__ import annotations

import os
import sys
import textwrap
from typing import TYPE_CHECKING, Any, Sequence

from .error import format_items
from .spec import MISSING, Action, Argument

if TYPE_CHECKING:
    from .error import UsageError
    from .parser import Parser

__all__ = ["Style", "render_error", "render_help", "render_usage"]

_RESET = "\x1b[0m"
_BOLD = "\x1b[1m"
_ITALIC_UNDERLINE = "\x1b[1m\x1b[4m"
_GREEN = "\x1b[32m"
_YELLOW = "\x1b[33m"
_RED = "\x1b[31m"


class Style:
    """ANSI 着色开关; 关闭时所有方法原样返回文本, 因此版式与宽度计算只看可见字符。"""

    def __init__(self, enabled: bool) -> None:
        """定开关: 记录着色是否启用

        :param enabled: 为真时各方法给文本套 ANSI 转义
        """
        self.enabled = enabled

    def _wrap(self, code: str, text: str) -> str:
        """套转义: 给文本包上转义码与复位码

        :param code: ANSI 转义码
        :param text: 待着色文本
        :return: 开关关闭或文本为空时原样返回
        """
        return f"{code}{text}{_RESET}" if self.enabled and text else text

    def heading(self, text: str) -> str:
        """标题着色: 章节标题与 `Usage:` / `error:` 前缀

        :param text: 待着色文本
        :return: 着色后的文本
        """
        return self._wrap(_ITALIC_UNDERLINE, text)

    def bold(self, text: str) -> str:
        """加粗着色: 需要强调的文本

        :param text: 待着色文本
        :return: 着色后的文本
        """
        return self._wrap(_BOLD, text)

    def literal(self, text: str) -> str:
        """字面量着色: 选项与命令字面量

        :param text: 待着色文本
        :return: 着色后的文本
        """
        return self._wrap(_GREEN, text)

    def error(self, text: str) -> str:
        """错误着色: `error:` 前缀

        :param text: 待着色文本
        :return: 着色后的文本
        """
        return self._wrap(_RED, text)

    def warning(self, text: str) -> str:
        """提示着色: `tip:` 前缀

        :param text: 待着色文本
        :return: 着色后的文本
        """
        return self._wrap(_YELLOW, text)


def _auto_color(stream: Any) -> bool:
    """自动判色: 按目标流是否为终端决定着色

    设置 `NO_COLOR` 环境变量时一律关闭。

    :param stream: 目标输出流
    :return: 需要着色为 `True`
    """
    if os.environ.get("NO_COLOR"):
        return False
    try:
        return bool(stream.isatty())
    except (AttributeError, ValueError):
        return False


def _wrap_paragraphs(text: str, width: int, indent: str = "", first_indent: str | None = None) -> list[str]:
    """折行: 按段落把文本折成多行

    :param text: 待折行文本, 换行符分段
    :param width: 整行宽度(含缩进)
    :param indent: 续行缩进
    :param first_indent: 首行缩进; `None` 表示与 `indent` 相同
    :return: 折好的各行; 段间保留一个空行位
    """
    head = indent if first_indent is None else first_indent
    lines: list[str] = []
    for index, paragraph in enumerate(text.split("\n")):
        wrapper = textwrap.TextWrapper(
            width=max(width, 20),
            initial_indent=head if index == 0 else indent,
            subsequent_indent=indent,
            break_long_words=False,
            break_on_hyphens=False,
        )
        wrapped = wrapper.wrap(paragraph)
        lines.extend(wrapped if wrapped else [""])
    return lines


def _format_value(value: Any) -> str:
    """写默认值: 默认值在帮助里的写法

    布尔用 `true` / `false`, 列表与元组逐个递归后逗号分隔, 其余用 `str`。

    :param value: 默认值
    :return: 帮助文本里的写法
    """
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, (list, tuple)):
        return ", ".join(_format_value(item) for item in value)
    return str(value)


def _argument_text(arg: Argument, long: bool) -> str:
    """拼说明: 一行说明文字, 由帮助正文与默认值 / 环境变量 / 可选值拼成

    :param arg: 参数定义
    :param long: 为真时取长帮助, 并把各可选值的说明逐条续在后面
    :return: 说明文字
    """
    text = (arg.long_help if long and arg.long_help else arg.help) or ""
    extras: list[str] = []
    shows_default = arg.action in (Action.STORE, Action.APPEND, Action.EXTEND)
    if shows_default and arg.default is not MISSING and arg.default is not None and not arg.hide_default:
        extras.append(f"[default: {_format_value(arg.default)}]")
    if arg.env:
        extras.append(f"[env: {arg.env}]")
    choices = arg.choice_values()
    if choices is not None:
        extras.append(f"[possible values: {format_items(choices)}]")
    if extras:
        text = f"{text} {' '.join(extras)}".strip()
    described = [(value, arg.choice_help(value)) for value in choices or [] if arg.choice_help(value)]
    if long and described:
        notes = "\n".join(f"- {value}: {note}" for value, note in described)
        text = f"{text}\n{notes}" if text else notes
    return text


def _visible_options(parser: Parser) -> list[Argument]:
    """挑可见选项: 取本命令的选项与祖先下沉的全局参数

    隐藏项剔除, 帮助 / 版本排最后(与 clap 一致)。

    :param parser: 解析器
    :return: 选项列表
    """
    options = [arg for arg in parser.arguments if not arg.is_positional() and not arg.hide]
    options.extend(arg for arg in parser.global_arguments() if not arg.hide)
    tail = [arg for arg in options if arg.action in (Action.HELP, Action.VERSION)]
    return [arg for arg in options if arg.action not in (Action.HELP, Action.VERSION)] + tail


def _table(
    title: str,
    rows: Sequence[tuple[str, str]],
    width: int,
    style: Style,
    description: str | None = None,
) -> str:
    """排表格: 输出标题加两列表格

    左列对齐, 右列折行并与右列首字符对齐; 左列超宽时说明文字换到下一行并加深缩进。

    :param title: 章节标题
    :param rows: 左列与右列文本对
    :param width: 整行宽度(含缩进)
    :param style: 着色开关
    :param description: 标题下的分组说明
    :return: 表格文本; 无行时为空串
    """
    if not rows:
        return ""
    left_width = min(max(len(left) for left, _ in rows), max(16, width // 3))
    gutter = 2
    base = 2 + left_width + gutter
    lines = [title]
    if description:
        lines.extend(_wrap_paragraphs(description, width, "  ", "  "))
        lines.append("")
    for left, text in rows:
        head = style.literal("  " + left)
        if not text:
            lines.append(head)
            continue
        if len(left) > left_width:
            lines.append(head)
            lines.extend(_wrap_paragraphs(text, width, " " * 4))
            continue
        indent = " " * base
        chunks = _wrap_paragraphs(text, width, indent, indent)
        lines.append(head + " " * (base - 2 - len(left)) + chunks[0].lstrip())
        lines.extend(chunks[1:])
    return "\n".join(lines)


def _command_rows(parser: Parser) -> list[tuple[str, str]]:
    """排子命令行: 给出子命令章节的行

    内容为名字加展示别名, 自动生成的 `help` 排在最后。

    :param parser: 解析器
    :return: 左列与右列文本对; 没有子命令时为空列表
    """
    subparsers = parser.subparsers
    if subparsers is None:
        return []
    rows: list[tuple[str, str]] = []
    for name, aliases, command in subparsers.commands():
        head = f"{name}, {', '.join(aliases)}" if aliases else name
        rows.append((head, command.about or ""))
    if not subparsers.has_command("help"):
        rows.append(("help", "Print this message or the help of the given subcommand(s)"))
    return rows


def render_usage(parser: Parser) -> str:
    """渲染用法: 输出单行用法文本

    :param parser: 解析器
    :return: 用法行
    """
    style = Style(parser._color_enabled())
    return f"{style.heading('Usage:')} {parser.usage_spec()}"


def render_help(parser: Parser, long: bool = False) -> str:
    """渲染帮助: 输出整段帮助文本, 段间空一行

    章节顺序为 `Usage` → `Commands` → `Arguments` → `Options` → 自定义分组 → `epilog`。

    :param parser: 解析器
    :param long: 为真时取长帮助与长说明
    :return: 帮助文本
    """
    style = Style(parser._color_enabled())
    width = parser.width()
    blocks: list[str] = []
    about = (parser.long_about if long else None) or parser.about
    if about:
        blocks.append("\n".join(_wrap_paragraphs(about, width)))
    blocks.append(render_usage(parser))
    if parser.subparsers is not None:
        commands = _table(style.heading(f"{parser.subparsers.title}:"), _command_rows(parser), width, style)
        if commands:
            blocks.append(commands)
    positionals = [arg for arg in parser.positionals if not arg.hide]
    if positionals:
        rows = [(arg.help_names(), _argument_text(arg, long)) for arg in positionals]
        blocks.append(_table(style.heading("Arguments:"), rows, width, style))
    options = _visible_options(parser)
    # 祖先命令下沉的全局参数带着父命令的分组标题, 本层没有该分组时退回默认的 Options 节
    default_options = [arg for arg in options if arg.group is None or arg.group not in parser.groups]
    if default_options:
        rows = [(arg.help_names(), _argument_text(arg, long)) for arg in default_options]
        blocks.append(_table(style.heading("Options:"), rows, width, style))
    for title, description in parser.groups.items():
        grouped = [arg for arg in options if arg.group == title]
        if not grouped:
            continue
        rows = [(arg.help_names(), _argument_text(arg, long)) for arg in grouped]
        blocks.append(_table(style.heading(f"{title}:"), rows, width, style, description))
    if parser.epilog:
        blocks.append("\n".join(_wrap_paragraphs(parser.epilog, width)))
    return "\n\n".join(blocks)


def render_error(error: UsageError, color: bool | None = None) -> str:
    """渲染错误: 输出错误块

    首行是 `error:` 加消息, 之后依次是缩进的细节、`tip:` 建议、usage 行与帮助提示。

    :param error: 用法错误
    :param color: 是否着色; `None` 表示按 `sys.stderr` 是否为终端自动判定
    :return: 错误块文本
    """
    enabled = _auto_color(sys.stderr) if color is None else color
    style = Style(enabled)
    head = f"{style.error('error:')} {error.message}"
    if error.reason:
        head = f"{head}: {error.reason}"
    lines = [head]
    lines.extend(f"  {line}" for line in error.lines)
    if error.tips:
        lines.append("")
        lines.extend(f"  {style.warning('tip:')} {tip}" for tip in error.tips)
    if error.usage:
        lines.append("")
        lines.append(f"{style.heading('Usage:')} {error.usage}")
    if error.hint:
        lines.append("")
        lines.append(error.hint)
    return "\n".join(lines)
