# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""由 dataclass 的类型注解派生参数定义。

一条 dataclass 就是一条命令: 字段名去下划线加 `--` 成选项名, 类型注解决定转换器与数量形态;
无默认值的字段是必填。`field(metadata={"positional": True})` 把字段登记成位置参数,
`metadata={"help": "..."}` 给说明文字。认不出的注解在定义期报错, 不静默退回字符串。
"""
from __future__ import annotations

import dataclasses
import typing
from enum import Enum
from typing import Any

try:
    from types import UnionType  # Python 3.10+
except ImportError:  # 3.8 / 3.9: 联合类型只由 typing.Union 表示
    UnionType = typing.Union

from .parser import Parser
from .spec import Argument, parse_float, parse_int

__all__ = ["add_arguments", "from_dataclass"]

#: 字段既没有 `default` 也没有 `default_factory`。
_NO_DEFAULT = dataclasses.MISSING

#: 多值形态的容器类型。
_COLLECTIONS = (list, set, frozenset, tuple)


def from_dataclass(cls: type | object, prog: str | None = None, **parser_kwargs: Any) -> Parser:
    """建命令: 按 `cls` 的字段顺序与类型注解建一条命令

    :param cls: dataclass 类型或实例
    :param prog: 命令名; `None` 表示交给 `Parser` 自行推断
    :param parser_kwargs: 透传给 `Parser` 的关键字参数
    :return: 登记好全部参数的解析器
    """
    parser = Parser(prog, **parser_kwargs)
    add_arguments(parser, cls)
    return parser


def add_arguments(parser: Parser, cls: type | object) -> list[Argument]:
    """登记参数: 把 `cls` 的字段逐个登记到 `parser`

    :param parser: 目标解析器
    :param cls: dataclass 类型或实例
    :return: 登记到的参数定义, 顺序与字段一致
    :raises TypeError: `cls` 不是 dataclass
    """
    kind = cls if isinstance(cls, type) else type(cls)
    if not dataclasses.is_dataclass(kind):
        raise TypeError(f"需要 dataclass 类型: {kind!r}")
    hints = typing.get_type_hints(kind)
    added: list[Argument] = []
    for item in dataclasses.fields(kind):
        annotation = hints.get(item.name, item.type)
        added.append(parser.add_argument(*_names(item), **_kwargs(item, annotation)))
    return added


def _names(item: dataclasses.Field[Any]) -> tuple[str, ...]:
    """定参数名: 给出字段对应的参数名

    位置参数用字段名, 选项用 `--` 加去下划线的字段名。

    :param item: dataclass 字段
    :return: 参数名元组
    """
    if _is_positional(item):
        return (item.name,)
    return ("--" + item.name.replace("_", "-"),)


def _is_positional(item: dataclasses.Field[Any]) -> bool:
    """判位置参数: 字段是否登记成位置参数

    :param item: dataclass 字段
    :return: `metadata` 里 `positional` 为真即为 `True`
    """
    return bool(item.metadata.get("positional", False))


def _default_of(item: dataclasses.Field[Any]) -> Any:
    """取默认值: 取字段的默认取值

    :param item: dataclass 字段
    :return: 默认取值; 既无 `default` 也无 `default_factory` 时为 `_NO_DEFAULT`
    """
    if item.default is not _NO_DEFAULT:
        return item.default
    if item.default_factory is not _NO_DEFAULT:
        return item.default_factory()
    return _NO_DEFAULT


def _kwargs(item: dataclasses.Field[Any], annotation: Any) -> dict[str, Any]:
    """翻关键字: 把一个字段翻成 `add_argument` 的关键字

    布尔字段是开关: 默认假写 `store_true`, 默认真写 `store_false`; 其余按注解给数量形态与转换器, 无默认值的字段标为必填。

    :param item: dataclass 字段
    :param annotation: 字段的类型注解
    :return: 关键字参数字典
    """
    kwargs: dict[str, Any] = {}
    help_text = item.metadata.get("help")
    if help_text:
        kwargs["help"] = help_text
    default = _default_of(item)
    if isinstance(default, Enum):
        default = default.value
    if annotation is bool:
        # 布尔字段是开关: 默认假写 store_true, 默认真写 store_false
        kwargs["action"] = "store_true" if default is _NO_DEFAULT or not default else "store_false"
        if default is not _NO_DEFAULT:
            kwargs["default"] = default
        elif not _is_positional(item):
            kwargs["required"] = True
        return kwargs
    _shape(kwargs, annotation)
    if default is not _NO_DEFAULT:
        kwargs["default"] = default
        if _is_positional(item) and "nargs" not in kwargs:
            kwargs["nargs"] = "?"
    elif not _is_positional(item):
        kwargs["required"] = True
    return kwargs


def _shape(kwargs: dict[str, Any], annotation: Any) -> None:
    """定数量形态: 按注解写入 `nargs` 与转换器

    可空形态用 `?`, 多值形态用 `*`, 其余按标量处理。

    :param kwargs: 待填充的关键字参数
    :param annotation: 字段的类型注解
    """
    origin = typing.get_origin(annotation)
    if origin is typing.Union or origin is UnionType:
        inner = _optional_inner(annotation)
        if inner is bool:
            raise ValueError("布尔字段不能是可空形态, 请直接写 bool")
        kwargs["nargs"] = "?"
        _scalar(kwargs, inner)
        return
    if origin in _COLLECTIONS:
        args = typing.get_args(annotation)
        element = args[0] if args else str
        if element is Ellipsis or element is bool:
            raise ValueError(f"多值形态的元素类型不支持: {annotation!r}")
        kwargs["nargs"] = "*"
        _scalar(kwargs, element)
        return
    _scalar(kwargs, annotation)


def _optional_inner(annotation: Any) -> Any:
    """取可空内层: 取 `X | None` 里的 `X`

    :param annotation: 字段的类型注解
    :return: 内层类型
    :raises ValueError: 注解不是两元的 `X | None`
    """
    args = typing.get_args(annotation)
    if type(None) not in args or len(args) != 2:
        raise ValueError(f"可空形态只能写成 X | None: {annotation!r}")
    return next(arg for arg in args if arg is not type(None))


def _scalar(kwargs: dict[str, Any], annotation: Any) -> None:
    """定标量形态: 按标量注解写入转换器与可选值

    `Literal` 与 `Enum` 给出 `choices`, `int` / `float` 给出对应转换器, 其余非 `str` 注解在定义期报错。

    :param kwargs: 待填充的关键字参数
    :param annotation: 标量类型注解
    """
    if annotation is bool:
        raise ValueError("布尔字段只能单独出现, 不能作为可空或多值的元素")
    origin = typing.get_origin(annotation)
    if origin is typing.Literal:
        values = list(typing.get_args(annotation))
        kwargs["choices"] = values
        _scalar(kwargs, _common_kind(values))
        return
    if isinstance(annotation, type) and issubclass(annotation, Enum):
        values = [member.value for member in annotation]
        kwargs["choices"] = values
        if values:
            _scalar(kwargs, _common_kind(values))
        return
    if annotation is int:
        kwargs["type"] = parse_int()
    elif annotation is float:
        kwargs["type"] = parse_float()
    elif annotation is not str:
        raise ValueError(f"不支持的注解: {annotation!r}")


def _common_kind(values: list[Any]) -> type:
    """取共同类型: 取一组可选值的共同类型

    :param values: 可选值列表
    :return: 共同类型; 空列表时为 `str`
    :raises ValueError: 可选值类型不一致
    """
    if not values:
        return str
    kinds = {type(value) for value in values}
    if len(kinds) != 1:
        raise ValueError(f"可选值的类型必须一致: {values!r}")
    return kinds.pop()
