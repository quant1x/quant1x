# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""参数本体: 取值动作、取值形态、类型转换器与单个参数的定义。

动作名沿用 `argparse` 的写法, 语义与 Rust `clap` 的 `ArgAction` 一一对应
(`store` ↔ `Set`、`store_true` ↔ `SetTrue`、`count` ↔ `Count`), 跨语言对照见 `argparser.md`。
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import Any, Callable, Mapping, Sequence

__all__ = [
    "REMAINDER",
    "SUPPRESS",
    "Action",
    "Argument",
    "Namespace",
    "ValueParser",
    "parse_bool",
    "parse_float",
    "parse_int",
]


class _Sentinel:
    """用作哨兵的单例标记, `repr` 即其名字。"""

    __slots__ = ("_name",)

    def __init__(self, name: str) -> None:
        """构造: 记录哨兵的名字

        :param name: 名字, 供 `repr` 输出
        """
        self._name = name

    def __repr__(self) -> str:
        """可读形式: 直接给出哨兵名字

        :return: 哨兵名字
        """
        return self._name


#: 默认值取本标记表示"不写进命名空间", 对应 `argparse.SUPPRESS`。
SUPPRESS = _Sentinel("SUPPRESS")

#: 内部哨兵: 没有显式给出默认值, 与"默认值是 `None`"区分。
MISSING = _Sentinel("MISSING")

#: 位置参数的取值形态: 其余未匹配的 token 全部收进来, 对应 `argparse.REMAINDER`。
REMAINDER = "..."


class Action(Enum):
    """参数出现时怎么取值、怎么落进命名空间。"""

    STORE = "store"
    STORE_TRUE = "store_true"
    STORE_FALSE = "store_false"
    STORE_CONST = "store_const"
    APPEND = "append"
    APPEND_CONST = "append_const"
    EXTEND = "extend"
    COUNT = "count"
    HELP = "help"
    VERSION = "version"

    def takes_values(self) -> bool:
        """判取值: 该动作是否从命令行吃掉取值

        :return: `store` / `append` / `extend` 为 `True`
        """
        return self in (Action.STORE, Action.APPEND, Action.EXTEND)


@dataclass(frozen=True, slots=True)
class ValueParser:
    """取值的类型转换器: 把命令行上的字符串翻成目标值, 非法取值抛 `ValueError`。

    `type_name` 只用于文案 (帮助里的取值说明), 不参与转换。
    """

    func: Callable[[str], Any]
    type_name: str = ""

    def __call__(self, text: str) -> Any:
        """转值: 按登记的转换函数把文本转成目标值

        :param text: 命令行上的原文
        :return: 转换后的取值
        """
        return self.func(text)

    @classmethod
    def of(cls, func: Callable[[str], Any], type_name: str = "") -> ValueParser:
        """包转换器: 把任意 `str -> Any` 的可调用包成本类

        :param func: 转换函数
        :param type_name: 帮助里显示的取值类型名; 空串表示取函数名
        :return: 包装好的转换器
        """
        return cls(func, type_name or getattr(func, "__name__", ""))


def _range_text(minimum: int | float | None, maximum: int | float | None) -> str:
    """写区间: 把上下界写成 clap 风格的开区间记法

    :param minimum: 下界; `None` 表示不限
    :param maximum: 上界; `None` 表示不限
    :return: 区间文本; 上下界都不限时为空串
    """
    if minimum is None and maximum is None:
        return ""
    if minimum is None:
        return f"..={maximum}"
    if maximum is None:
        return f"{minimum}.."
    return f"{minimum}..={maximum}"


def _check_range(value: int | float, minimum: int | float | None, maximum: int | float | None) -> None:
    """查越界: 取值落在闭区间外即报错

    文案与 clap 的 ranged parser 同形。

    :param value: 待检查取值
    :param minimum: 下界; `None` 表示不限
    :param maximum: 上界; `None` 表示不限
    :raises ValueError: 取值越界
    """
    if minimum is not None and value < minimum:
        raise ValueError(f"{value} is not in {_range_text(minimum, maximum)}")
    if maximum is not None and value > maximum:
        raise ValueError(f"{value} is not in {_range_text(minimum, maximum)}")


def parse_int(minimum: int | None = None, maximum: int | None = None) -> ValueParser:
    """造整数转换器: 十进制整数, 可带闭区间上下界

    :param minimum: 下界; `None` 表示不限
    :param maximum: 上界; `None` 表示不限
    :return: 整数转换器
    """

    def convert(text: str) -> int:
        """转整数: 按十进制解析并检查上下界

        :param text: 命令行上的原文
        :return: 解析得到的整数
        :raises ValueError: 不是合法的十进制整数或越界
        """
        try:
            value = int(text, 10)
        except ValueError:
            raise ValueError("invalid digit found in string") from None
        _check_range(value, minimum, maximum)
        return value

    return ValueParser(convert, "INTEGER")


def parse_float(minimum: float | None = None, maximum: float | None = None) -> ValueParser:
    """造浮点转换器: 可带闭区间上下界

    :param minimum: 下界; `None` 表示不限
    :param maximum: 上界; `None` 表示不限
    :return: 浮点转换器
    """

    def convert(text: str) -> float:
        """转浮点: 解析浮点字面量并检查上下界

        :param text: 命令行上的原文
        :return: 解析得到的浮点数
        :raises ValueError: 不是合法的浮点字面量或越界
        """
        try:
            value = float(text)
        except ValueError:
            raise ValueError("invalid float literal") from None
        _check_range(value, minimum, maximum)
        return value

    return ValueParser(convert, "FLOAT")


#: 布尔取值的可识别写法, 大小写不敏感。
_BOOL_WORDS = {
    "true": True,
    "yes": True,
    "on": True,
    "1": True,
    "false": False,
    "no": False,
    "off": False,
    "0": False,
}


def parse_bool() -> ValueParser:
    """造布尔转换器: 认 `true/false`、`yes/no`、`on/off`、`1/0`, 大小写不敏感

    :return: 布尔转换器
    """

    def convert(text: str) -> bool:
        """转布尔: 在可识别写法里查表

        :param text: 命令行上的原文, 前后空白会被去掉
        :return: 对应的布尔值
        :raises ValueError: 不在可识别写法里
        """
        try:
            return _BOOL_WORDS[text.strip().lower()]
        except KeyError:
            raise ValueError("provided string was not one of true/false/yes/no/on/off/1/0") from None

    return ValueParser(convert, "BOOL")


@dataclass(kw_only=True)
class Argument:
    """单个命令行参数的定义, 由 `Parser.add_argument` 构造并登记。

    位置参数没有 `names`; 选项至少有一个以 `-` 开头的名字。字段一律关键字传参,
    避免把字符串当成名字序列塞进 `names`。
    """

    names: tuple[str, ...] = ()
    dest: str = ""
    action: Action = Action.STORE
    nargs: int | str | None = None
    type: ValueParser | None = None
    choices: Sequence[Any] | Mapping[Any, str] | None = None
    default: Any = MISSING
    const: Any = MISSING
    required: bool = False
    metavar: str | None = None
    help: str | None = None
    long_help: str | None = None
    env: str | None = None
    alias: tuple[str, ...] = ()
    visible_alias: tuple[str, ...] = ()
    conflicts_with: tuple[str, ...] = ()
    requires: tuple[str, ...] = ()
    overrides_with: tuple[str, ...] = ()
    required_unless_present: tuple[str, ...] = ()
    value_delimiter: str | None = None
    hide: bool = False
    hide_default: bool = False
    allow_hyphen_values: bool = False
    global_: bool = False
    group: str | None = None

    def is_positional(self) -> bool:
        """判位置参数: 没有选项名, 按命令行顺序吃取值

        :return: 没有名字即为 `True`
        """
        return not self.names

    def takes_values(self) -> bool:
        """判取值: 该参数是否吃取值

        :return: `store_true` / `count` 这类开关为 `False`
        """
        return self.action.takes_values()

    def collects(self) -> bool:
        """判成列表: 取值是否恒以列表形态落进命名空间

        `nargs` 为整数、`*` / `+` / REMAINDER, 或配了 `value_delimiter` 时成立; `nargs` 缺省与 `?` 是单值。

        :return: 落进命名空间时为列表即 `True`
        """
        if self.value_delimiter is not None:
            return True
        if self.nargs in ("*", "+", REMAINDER):
            return True
        return isinstance(self.nargs, int)

    def min_count(self) -> int:
        """算下限: 最少吃几个取值

        :return: 最少取值个数
        """
        nargs = self.nargs
        if nargs in ("?", "*", REMAINDER):
            return 0
        if nargs == "+":
            return 1
        if isinstance(nargs, int):
            return nargs
        return 1

    def max_count(self) -> int | None:
        """算上限: 最多吃几个取值

        :return: 最多取值个数; `None` 表示无上限
        """
        nargs = self.nargs
        if nargs in ("*", "+", REMAINDER):
            return None
        if isinstance(nargs, int):
            return nargs
        return 1

    def value_name(self) -> str:
        """取显示名: 取值的显示名

        :return: `metavar`; 没给时为大写化的 `dest`
        """
        return self.metavar or self.dest.upper()

    def value_spec(self) -> str:
        """写占位: 取值的占位写法

        :return: 形如 `<FILE>` 的文本
        """
        return f"<{self.value_name()}>"

    def primary_name(self) -> str:
        """取主名: 指代该参数时优先用长选项

        :return: 长选项; 只有短选项时退回第一个名字, 位置参数退回 `dest`
        """
        for name in self.names:
            if name.startswith("--"):
                return name
        return self.names[0] if self.names else self.dest

    def error_name(self) -> str:
        """写错误指代: 错误文案里怎么指代这个参数

        :return: 形如 `--output <FILE>` / `<FILE>` / `-v` 的文本
        """
        if self.is_positional():
            return self.value_spec()
        name = self.primary_name()
        return f"{name} {self.value_spec()}" if self.takes_values() else name

    def help_names(self) -> str:
        """写帮助左列: 帮助里该参数怎么列

        :return: 形如 `-o, --output <FILE>`; 位置参数按取值形态写成 `<FILE>` / `[FILE]...`
        """
        if self.is_positional():
            return self.usage_spec()
        head = ", ".join((*self.names, *self.visible_alias))
        if self.action is Action.COUNT:
            head += "..."
        return f"{head} {self.value_spec()}" if self.takes_values() else head

    def usage_spec(self) -> str:
        """写用法占位: usage 行里的位置参数占位

        :return: `<FILE>` / `[FILE]` / `<FILE>...` / `[FILE]...` 之一
        """
        name = self.value_name()
        nargs = self.nargs
        if nargs == "?":
            return f"[{name}]"
        if nargs in ("*", REMAINDER):
            return f"[{name}]..."
        if nargs == "+" or isinstance(nargs, int):
            return f"<{name}>..."
        return f"<{name}>"

    def choice_values(self) -> list[Any] | None:
        """列可选值: 给出可选值集合

        :return: 可选值列表; 字典形态取其键, 没有可选值时为 `None`
        """
        if self.choices is None:
            return None
        return list(self.choices.keys()) if isinstance(self.choices, Mapping) else list(self.choices)

    def choice_help(self, value: Any) -> str | None:
        """取选项说明: 给出字典形态可选值的说明文字

        :param value: 可选值
        :return: 说明文字; 非字典形态或该值没有说明时为 `None`
        """
        if isinstance(self.choices, Mapping):
            return self.choices.get(value)
        return None


class Namespace:
    """解析结果的属性式容器, 对应 `argparse.Namespace`, 另支持下标读取。"""

    def __init__(self, **kwargs: Any) -> None:
        """构造: 把关键字参数直接挂成属性

        :param kwargs: 要挂上的键值对
        """
        self.__dict__.update(kwargs)

    def __getattr__(self, name: str) -> Any:
        """取属性: 命名空间里没有该名字时的兜底

        :param name: 属性名
        :raises AttributeError: 恒抛; 类型检查据此认可任意属性名
        """
        raise AttributeError(name)

    def __getitem__(self, key: str) -> Any:
        """按下标取: 取命名空间里的键值

        :param key: 键名
        :return: 对应的取值
        :raises KeyError: 没有该键
        """
        try:
            return self.__dict__[key]
        except KeyError:
            raise KeyError(key) from None

    def __setitem__(self, key: str, value: Any) -> None:
        """按下标写: 写入或覆盖一个键

        :param key: 键名
        :param value: 取值
        """
        self.__dict__[key] = value

    def __delitem__(self, key: str) -> None:
        """按下标删: 删掉一个键

        :param key: 键名
        :raises KeyError: 没有该键
        """
        try:
            del self.__dict__[key]
        except KeyError:
            raise KeyError(key) from None

    def __contains__(self, key: object) -> bool:
        """判包含: 键是否在命名空间里

        :param key: 键名
        :return: 存在为 `True`
        """
        return key in self.__dict__

    def __iter__(self):
        """迭代键: 按写入顺序逐个给出键名

        :return: 键名的迭代器
        """
        return iter(self.__dict__)

    def __len__(self) -> int:
        """取键数: 命名空间里有多少个键

        :return: 键的个数
        """
        return len(self.__dict__)

    def __eq__(self, other: object) -> bool:
        """判相等: 与另一个命名空间的键值完全相同即为相等

        :param other: 比较对象
        :return: 双方都是 `Namespace` 且键值一致为 `True`
        """
        return isinstance(other, Namespace) and self.__dict__ == other.__dict__

    def __repr__(self) -> str:
        """可读形式: `Namespace(k=v, ...)`

        :return: 带键值的文本
        """
        body = ", ".join(f"{key}={value!r}" for key, value in self.__dict__.items())
        return f"Namespace({body})"

    def keys(self):
        """列键: 给出全部键名

        :return: 键名视图
        """
        return self.__dict__.keys()

    def values(self):
        """列取值: 给出全部取值

        :return: 取值视图
        """
        return self.__dict__.values()

    def items(self):
        """列键值对: 给出全部键值对

        :return: 键值对视图
        """
        return self.__dict__.items()

    def get(self, key: str, default: Any = None) -> Any:
        """取值(带兜底): 键存在则给取值, 否则给兜底

        :param key: 键名
        :param default: 没有该键时的兜底值
        :return: 取值或兜底值
        """
        return self.__dict__.get(key, default)

    def as_dict(self) -> dict[str, Any]:
        """转字典: 浅拷贝成普通字典

        :return: 新的字典
        """
        return dict(self.__dict__)
