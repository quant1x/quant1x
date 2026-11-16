# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""命令定义与解析: 选项匹配、位置参数分配、子命令分发、缺省值与校验。

解析是单趟左到右的扫描: 遇到选项就地取走它的取值, 其余 token 攒起来交给位置参数;
遇到子命令名则把剩余 token 整段交给子命令, 此前攒下的 token 仍归本命令的位置参数。

失败一律抛异常, 不打印也不退出; `parse_or_exit` 才负责输出与退出码。
"""

from __future__ import annotations

import os
import re
import sys
from typing import Any, Callable, Iterable, Sequence, TextIO

from .error import ErrorKind, HelpRequested, UsageError, VersionRequested, best_match, format_items
from .spec import (
    MISSING,
    REMAINDER,
    SUPPRESS,
    Action,
    Argument,
    Namespace,
    ValueParser,
    parse_bool,
    parse_float,
    parse_int,
)

__all__ = ["ArgumentGroup", "MutuallyExclusiveGroup", "Parser", "SubParsers"]

#: 签名数字: 可选的 `+` / `-` 号加整数或小数, 整个 token 当一个取值参数 (`-1`、`-.5`、`+3`、`-1.5`)。
_SIGNED_NUMBER = re.compile(r"^[+-]?\d+$|^[+-]?\d*\.\d+$")

#: 动作名的可识别写法, 键做过小写与去下划线归一; 兼容 clap 的 `SetTrue` 一类拼法。
_ACTION_ALIASES = {
    "store": Action.STORE,
    "set": Action.STORE,
    "storetrue": Action.STORE_TRUE,
    "settrue": Action.STORE_TRUE,
    "storefalse": Action.STORE_FALSE,
    "setfalse": Action.STORE_FALSE,
    "storeconst": Action.STORE_CONST,
    "append": Action.APPEND,
    "appendconst": Action.APPEND_CONST,
    "extend": Action.EXTEND,
    "count": Action.COUNT,
    "help": Action.HELP,
    "version": Action.VERSION,
}

#: `nargs` 的合法字符串形态。
_NARGS_STRINGS = ("?", "*", "+", REMAINDER)

#: `@file` 展开的嵌套层数上限; 只用来挡住自引用, 正常用法远到不了。
_MAX_INCLUDE_DEPTH = 16


def _coerce_action(value: Any) -> Action:
    """归一动作: 把动作名换成 `Action`

    :param value: `Action` 成员或动作名字符串, 大小写与下划线不敏感
    :return: 对应的 `Action`
    :raises TypeError: 无法识别的动作名
    """
    if isinstance(value, Action):
        return value
    if isinstance(value, str):
        key = value.strip().lower().replace("_", "").replace("-", "")
        if key in _ACTION_ALIASES:
            return _ACTION_ALIASES[key]
    raise TypeError(f"无法识别的 action: {value!r}")


def _normalize_nargs(value: Any) -> int | str | None:
    """校验 nargs: 只认 `None`、非负整数与 `?` / `*` / `+` / REMAINDER

    :param value: 待校验的 `nargs`
    :return: 校验后的取值
    :raises ValueError: 形态不合法或为负
    """
    if value is None:
        return None
    if isinstance(value, int) and not isinstance(value, bool):
        if value < 0:
            raise ValueError(f"nargs 不能为负: {value}")
        return value
    if value in _NARGS_STRINGS:
        return value
    raise ValueError(f"无法识别的 nargs: {value!r}")


def _dest_of(names: Sequence[str]) -> str:
    """推 dest: 由选项名推命名空间里的键名

    优先取长选项, 中划线转下划线。

    :param names: 选项名序列
    :return: 推导出的 `dest`
    """
    for name in names:
        if name.startswith("--"):
            return name[2:].replace("-", "_")
    return names[0].lstrip("-").replace("-", "_")


def _split_option(token: str) -> tuple[str, str | None]:
    """拆内联值: 把 `--key=value` / `-k=value` 拆成名字与取值

    :param token: 选项 token
    :return: `(名字, 内联取值)`; 没有 `=` 时取值为 `None`
    """
    if "=" in token:
        name, _, value = token.partition("=")
        return name, value
    return token, None


def _unique(items: Iterable[str]) -> list[str]:
    """去重保序: 保持首次出现顺序去重

    :param items: 待去重的序列
    :return: 去重后的列表
    """
    seen: dict[str, None] = {}
    for item in items:
        seen.setdefault(item, None)
    return list(seen)


def _slots(tokens: Sequence[str], start: int) -> list[tuple[int, str]]:
    """配序号: 给一段 token 配上扫描序号

    序号只用于把 `extras` 还原成命令行顺序。

    :param tokens: 待编号的 token
    :param start: 首个 token 的序号
    :return: `(序号, token)` 列表
    """
    return [(start + offset, token) for offset, token in enumerate(tokens)]


def _implicit_default(arg: Argument) -> Any:
    """隐式默认值: 没显式给默认值时的落值

    `store_true` 落假、`store_false` 落真、`count` 落 0, 位置参数的 `*` / REMAINDER 落空列表, 其余落 `None`。

    :param arg: 参数定义
    :return: 隐式默认值
    """
    if arg.action is Action.STORE_TRUE:
        return False
    if arg.action is Action.STORE_FALSE:
        return True
    if arg.action is Action.COUNT:
        return 0
    if arg.is_positional() and arg.nargs in ("*", REMAINDER):
        return []
    return None


def _infer_parser(default: Any) -> ValueParser | None:
    """推断转换器: 按默认值的 Python 类型推取值转换器

    只认布尔、整数、浮点三类, 其余类型不推断。

    :param default: 默认值
    :return: 对应的转换器; 类型不在此列时为 `None`
    """
    if isinstance(default, bool):
        return parse_bool()
    if isinstance(default, int):
        return parse_int()
    if isinstance(default, float):
        return parse_float()
    return None


class _Run:
    """一次解析的共享状态: 命名空间、出现记录、参与的命令链与待报缺口。

    `extras` 连同扫描序号一起收: 未匹配的 token 来自扫描途中的未知选项与收尾时的多余
    位置 token 两处, 存序号才能在返回前还原成命令行顺序。`tolerant` 为真时未知选项
    也进 `extras` 而不报错 (`parse_known_args`)。
    """

    def __init__(self, tolerant: bool = False) -> None:
        """构造: 起一份解析状态

        :param tolerant: 为真时未知选项也收进 `extras` 而不报错
        """
        self.tolerant = tolerant
        self.namespace = Namespace()
        self.present: set[str] = set()
        self.seen: list[str] = []
        self.parsers: list[Parser] = []
        self.missing: list[Argument] = []
        self.extras: list[tuple[int, str]] = []
        self.subcommand: str | None = None


class ArgumentGroup:
    """帮助里的一个参数分组, 只影响渲染不影响解析。"""

    def __init__(self, parser: Parser, title: str, description: str | None = None) -> None:
        """构造: 记录所属命令与分组标题

        :param parser: 所属命令
        :param title: 分组标题, 也是帮助里的分节名
        :param description: 分组说明, 列在标题之下
        """
        self._parser = parser
        self.title = title
        self.description = description

    def add_argument(self, *flags: str, **kwargs: Any) -> Argument:
        """登记参数: 把参数登记到所属命令并归到本组

        :param flags: 参数名; 首个以 `-` 开头是选项, 否则是位置参数
        :param kwargs: 透传给 `Parser.add_argument` 的关键字参数
        :return: 登记到的参数定义
        """
        arg = self._parser.add_argument(*flags, **kwargs)
        arg.group = self.title
        return arg


class MutuallyExclusiveGroup:
    """互斥组: 组内参数最多出现一个; `required` 为真时至少出现一个。"""

    def __init__(self, parser: Parser, required: bool = False) -> None:
        """构造: 记录所属命令与是否必选

        :param parser: 所属命令
        :param required: 为真时组内至少出现一个
        """
        self._parser = parser
        self.required = required
        self.arguments: list[Argument] = []

    def add_argument(self, *flags: str, **kwargs: Any) -> Argument:
        """登记参数: 把参数登记到所属命令并纳入本互斥组

        :param flags: 参数名
        :param kwargs: 透传给 `Parser.add_argument` 的关键字参数
        :return: 登记到的参数定义
        """
        arg = self._parser.add_argument(*flags, **kwargs)
        self.arguments.append(arg)
        return arg


class ValueGroup:
    """多值组: 组内参数可以同时出现; `required` 为真时至少要出现一个。"""

    def __init__(self, parser: Parser, name: str, required: bool = False) -> None:
        """构造: 记录所属命令、组名与是否必选

        :param parser: 所属命令
        :param name: 组名, 报必填缺口时按 `<组名>` 显示
        :param required: 为真时组内至少出现一个
        """
        self._parser = parser
        self.name = name
        self.required = required
        self.arguments: list[Argument] = []

    def add_argument(self, *flags: str, **kwargs: Any) -> Argument:
        """登记参数: 把参数登记到所属命令并纳入本组

        :param flags: 参数名
        :param kwargs: 透传给 `Parser.add_argument` 的关键字参数
        :return: 登记到的参数定义
        """
        arg = self._parser.add_argument(*flags, **kwargs)
        self.arguments.append(arg)
        return arg


class SubParsers:
    """子命令集合: 名字 (含别名) 到命令的映射, 并记录帮助里的展示顺序。"""

    def __init__(
        self,
        parser: Parser,
        *,
        dest: str | None = None,
        title: str = "Commands",
        required: bool = False,
        help: str | None = None,
    ) -> None:
        """构造: 起一份空的子命令集合

        :param parser: 所属命令
        :param dest: 命中子命令时写进命名空间的键名; `None` 表示不写
        :param title: 帮助里的分节标题
        :param required: 为真时至少要给一个子命令
        :param help: 分节说明
        """
        self._parser = parser
        self.dest = dest
        self.title = title
        self.required = required
        self.help = help
        self._commands: dict[str, Parser] = {}
        self._order: list[Parser] = []
        self._visible: dict[str, tuple[str, ...]] = {}

    def add_parser(
        self,
        name: str,
        *,
        about: str | None = None,
        aliases: Sequence[str] = (),
        visible_aliases: Sequence[str] = (),
        **kwargs: Any,
    ) -> Parser:
        """登记子命令: 建一条子命令并登记名字与别名

        `aliases` 不出现在帮助里, `visible_aliases` 出现; 版本号、缩写、着色与宽度缺省沿用父命令。

        :param name: 子命令名
        :param about: 一句话说明, 列在命令表里
        :param aliases: 不在帮助里展示的别名
        :param visible_aliases: 在帮助里展示的别名
        :param kwargs: 透传给 `Parser` 的关键字参数
        :return: 建好的子命令
        :raises ValueError: 名字或别名与已登记的子命令重复
        """
        parent = self._parser
        kwargs.setdefault("version", parent.version)
        kwargs.setdefault("allow_abbrev", parent.allow_abbrev)
        kwargs.setdefault("color", parent.color)
        kwargs.setdefault("max_width", parent.max_width)
        child = Parser(f"{parent.prog} {name}", about=about, **kwargs)
        child._parent = parent
        child._name = name
        for key in (name, *aliases, *visible_aliases):
            if key in self._commands:
                raise ValueError(f"子命令名重复登记: {key!r}")
            self._commands[key] = child
        self._order.append(child)
        self._visible[name] = tuple(visible_aliases)
        return child

    def lookup(self, name: str) -> Parser | None:
        """找子命令: 按名字或别名找

        :param name: 子命令名或别名
        :return: 子命令; 没登记时为 `None`
        """
        return self._commands.get(name)

    def has_command(self, name: str) -> bool:
        """判已登记: 该名字 (含别名) 是否已被登记

        :param name: 子命令名或别名
        :return: 已登记为 `True`
        """
        return name in self._commands

    def canonical(self, name: str) -> str:
        """归正名: 把别名解析回注册时的正式名

        :param name: 子命令名或别名
        :return: 正式名
        """
        return self._commands[name]._name or name

    def commands(self) -> list[tuple[str, tuple[str, ...], Parser]]:
        """列命令: 按注册顺序给出全部子命令

        :return: (正式名, 展示用别名, 命令) 列表
        """
        return [(command._name or "", self._visible.get(command._name or "", ()), command) for command in self._order]


class Parser:
    """一条命令: 参数定义、解析入口与帮助渲染。子命令本身也是 `Parser`。"""

    def __init__(
        self,
        prog: str | None = None,
        *,
        about: str | None = None,
        long_about: str | None = None,
        version: str | None = None,
        epilog: str | None = None,
        usage: str | None = None,
        add_help: bool = True,
        allow_abbrev: bool = True,
        argument_default: Any = MISSING,
        color: str = "auto",
        max_width: int = 100,
        auto_width: bool = False,
        subcommand_negates_reqs: bool = False,
        fromfile_prefix_chars: str | None = None,
    ) -> None:
        """构造: 建一条命令, 并按需登记 `-h` / `-V`

        :param prog: 命令名; `None` 表示取 `sys.argv[0]` 的文件名
        :param about: 一句话说明
        :param long_about: 长帮助里的说明
        :param version: 版本号; 给了才登记 `-V` / `--version`
        :param epilog: 帮助末尾的补充段落
        :param usage: 自定义 usage 行; `None` 表示按参数自动生成
        :param add_help: 是否自动登记 `-h` / `--help`
        :param allow_abbrev: 是否允许长选项唯一前缀匹配
        :param argument_default: 参数级默认值未给时的兜底默认值
        :param color: 着色策略, 取 `auto` / `always` / `never`
        :param max_width: 帮助的最大宽度
        :param auto_width: 为真时读 `COLUMNS` 取宽度
        :param subcommand_negates_reqs: 为真时给了子命令就不再要求本层必填项
        :param fromfile_prefix_chars: `@file` 展开的前缀字符集; `None` 表示不展开
        """
        self.prog = prog or os.path.basename(sys.argv[0]) or "prog"
        self.about = about
        self.long_about = long_about
        self._version = version
        self.epilog = epilog
        self.usage = usage
        self.allow_abbrev = allow_abbrev
        self.argument_default = argument_default
        self.color = color
        self.max_width = max_width
        self.auto_width = auto_width
        self.subcommand_negates_reqs = subcommand_negates_reqs
        self.fromfile_prefix_chars = fromfile_prefix_chars
        self._arguments: list[Argument] = []
        self._by_name: dict[str, Argument] = {}
        self._positionals: list[Argument] = []
        self._dests: dict[str, Argument] = {}
        self._groups: dict[str, str | None] = {}
        self._mutex: list[MutuallyExclusiveGroup] = []
        self._value_groups: list[ValueGroup] = []
        self._subparsers: SubParsers | None = None
        self._parent: Parser | None = None
        self._name: str | None = None
        if add_help:
            self.add_argument("-h", "--help", action=Action.HELP, dest="help", default=SUPPRESS, help="Print help")
        if version:
            self.add_argument(
                "-V", "--version", action=Action.VERSION, dest="version", default=SUPPRESS, help="Print version"
            )

    # ---- 定义 ----------------------------------------------------------

    @property
    def version(self) -> str | None:
        """取版本号: 子命令没自带时沿用父命令的

        :return: 版本号; 都没有时为 `None`
        """
        if self._version is not None:
            return self._version
        return self._parent.version if self._parent is not None else None

    @property
    def parent(self) -> Parser | None:
        """取父命令: 顶层命令为 `None`

        :return: 父命令
        """
        return self._parent

    @property
    def name(self) -> str | None:
        """取子命令名: 在父命令里的子命令名

        :return: 子命令名; 顶层命令为 `None`
        """
        return self._name

    @property
    def arguments(self) -> tuple[Argument, ...]:
        """取参数: 本命令登记的参数 (含位置参数)

        :return: 参数元组, 按注册顺序
        """
        return tuple(self._arguments)

    @property
    def positionals(self) -> tuple[Argument, ...]:
        """取位置参数: 本命令的位置参数

        :return: 位置参数元组
        """
        return tuple(self._positionals)

    @property
    def subparsers(self) -> SubParsers | None:
        """取子命令集合: 没登记则为 `None`

        :return: 子命令集合
        """
        return self._subparsers

    @property
    def groups(self) -> dict[str, str | None]:
        """取分组: 帮助分组的标题到描述

        :return: 标题到描述的字典, 按登记顺序
        """
        return dict(self._groups)

    @property
    def mutex_groups(self) -> tuple[MutuallyExclusiveGroup, ...]:
        """取互斥组: 按登记顺序

        :return: 互斥组元组
        """
        return tuple(self._mutex)

    @property
    def value_groups(self) -> tuple[ValueGroup, ...]:
        """取多值组: 按登记顺序

        :return: 多值组元组
        """
        return tuple(self._value_groups)

    def global_arguments(self) -> list[Argument]:
        """取全局参数: 祖先命令上标了 `global_` 的参数

        子命令里同样可用, 也出现在子命令帮助里; 按由远及近的登记顺序排列。

        :return: 全局参数列表
        """
        collected: list[Argument] = []
        parent = self._parent
        while parent is not None:
            collected.extend(arg for arg in parent._arguments if arg.global_)
            parent = parent._parent
        collected.reverse()
        return collected

    def add_argument(self, *flags: str, **kwargs: Any) -> Argument:
        """登记参数: 首个名字以 `-` 开头是选项, 否则是位置参数

        选项名不能重复、不能是签名数字; 位置参数只能有一个名字, 必填性由 `nargs` 决定。

        :param flags: 参数名; 位置参数只取第一个且不以 `-` 开头
        :param kwargs: 取值动作、形态、转换器、默认值与校验等关键字参数
        :return: 登记到的参数定义
        :raises TypeError: 没有给参数名, 或给了不认识的关键字参数
        :raises ValueError: 名字 / `dest` 重复, 位置参数给了多个名字或 `required`, 或 `nargs` 与动作不匹配
        """
        if not flags:
            raise TypeError("至少需要一个参数名")
        positional = not flags[0].startswith("-")
        position_name = flags[0].replace("-", "_") if positional else ""
        if positional:
            if len(flags) > 1:
                raise ValueError(f"位置参数只能有一个名字: {flags!r}")
            if any(arg.nargs == REMAINDER for arg in self._positionals):
                raise ValueError("REMAINDER 位置参数必须是最后一个, 其后不能再加位置参数")
            if kwargs.get("required"):
                raise ValueError("位置参数的必填性由 nargs 决定, 不接受 required")
        names = () if positional else tuple(flags)
        alias = tuple(kwargs.pop("alias", ()))
        visible_alias = tuple(kwargs.pop("visible_alias", ()))
        for name in (*names, *alias, *visible_alias):
            if not name.startswith("-"):
                raise ValueError(f"选项名必须以 '-' 开头: {name!r}")
            if _SIGNED_NUMBER.match(name):
                raise ValueError(f"签名数字不能作选项名, 会被整体当成取值: {name!r}")

        dest = kwargs.pop("dest", None) or (position_name if positional else _dest_of(names))
        action = _coerce_action(kwargs.pop("action", Action.STORE))
        nargs = _normalize_nargs(kwargs.pop("nargs", None))
        type_ = kwargs.pop("type", None)
        const = kwargs.pop("const", MISSING)
        default = kwargs.pop("default", MISSING)
        required = bool(kwargs.pop("required", False))
        metavar = kwargs.pop("metavar", None)
        help_text = kwargs.pop("help", None)
        long_help = kwargs.pop("long_help", None)
        env = kwargs.pop("env", None)
        conflicts_with = tuple(kwargs.pop("conflicts_with", ()))
        requires = tuple(kwargs.pop("requires", ()))
        overrides_with = tuple(kwargs.pop("overrides_with", ()))
        required_unless_present = tuple(kwargs.pop("required_unless_present", ()))
        value_delimiter = kwargs.pop("value_delimiter", None)
        hide = bool(kwargs.pop("hide", False))
        hide_default = bool(kwargs.pop("hide_default", False))
        allow_hyphen_values = bool(kwargs.pop("allow_hyphen_values", False))
        global_ = bool(kwargs.pop("global_", False))
        choices = kwargs.pop("choices", None)
        if kwargs:
            raise TypeError(f"add_argument 不认识的参数: {', '.join(sorted(kwargs))}")

        if not action.takes_values():
            if nargs not in (None, 0):
                raise ValueError(f"action={action.value} 不吃取值, 不能配 nargs={nargs!r}")
            nargs = 0
            if action in (Action.STORE_TRUE, Action.STORE_FALSE):
                if const is MISSING:
                    const = action is Action.STORE_TRUE
                if default is MISSING:
                    default = not const
            elif action is Action.COUNT:
                if default is MISSING:
                    default = 0
            elif action in (Action.STORE_CONST, Action.APPEND_CONST) and const is MISSING:
                raise ValueError(f"action={action.value} 必须给 const")
        elif value_delimiter is not None and nargs is None:
            # 配了分隔符就是「一条 token 里塞多个值」, 数量按一个 token 算, 落值仍是列表
            nargs = 1

        if type_ is None and action.takes_values() and default is not MISSING:
            type_ = _infer_parser(default)
        if type_ is not None and not isinstance(type_, ValueParser):
            type_ = ValueParser.of(type_)

        for name in (*names, *alias, *visible_alias):
            if name in self._by_name:
                raise ValueError(f"选项名重复登记: {name!r}")
        if dest in self._dests:
            raise ValueError(f"dest 重复登记: {dest!r}")

        argument = Argument(
            names=names,
            dest=dest,
            action=action,
            nargs=nargs,
            type=type_,
            choices=choices,
            default=default,
            const=const,
            required=required,
            metavar=metavar,
            help=help_text,
            long_help=long_help,
            env=env,
            alias=alias,
            visible_alias=visible_alias,
            conflicts_with=conflicts_with,
            requires=requires,
            overrides_with=overrides_with,
            required_unless_present=required_unless_present,
            value_delimiter=value_delimiter,
            hide=hide,
            hide_default=hide_default,
            allow_hyphen_values=allow_hyphen_values,
            global_=global_,
        )
        self._arguments.append(argument)
        self._dests[dest] = argument
        for name in (*names, *alias, *visible_alias):
            self._by_name[name] = argument
        if positional:
            self._positionals.append(argument)
        return argument

    def add_group(self, title: str, description: str | None = None) -> ArgumentGroup:
        """登记分组: 该组的参数在帮助里单独成节

        :param title: 分组标题
        :param description: 分组说明
        :return: 建好的分组
        """
        self._groups[title] = description
        return ArgumentGroup(self, title, description)

    def add_mutually_exclusive_group(self, required: bool = False) -> MutuallyExclusiveGroup:
        """登记互斥组: 组内参数最多出现一个

        :param required: 为真时组内至少出现一个
        :return: 建好的互斥组
        """
        group = MutuallyExclusiveGroup(self, required)
        self._mutex.append(group)
        return group

    def add_value_group(self, name: str, *, required: bool = False) -> ValueGroup:
        """登记多值组: 组内参数可同时出现

        :param name: 组名
        :param required: 为真时组内至少出现一个
        :return: 建好的多值组
        """
        group = ValueGroup(self, name, required)
        self._value_groups.append(group)
        return group

    def add_subparsers(
        self,
        *,
        dest: str | None = None,
        title: str = "Commands",
        required: bool = False,
        help: str | None = None,
    ) -> SubParsers:
        """登记子命令集合: 只能登记一次

        :param dest: 命中子命令时把名字原样写进命名空间的键名
        :param title: 帮助里的分节标题
        :param required: 为真时至少要给一个子命令
        :param help: 分节说明
        :return: 建好的子命令集合
        :raises ValueError: 已经登记过子命令集合
        """
        if self._subparsers is not None:
            raise ValueError("子命令集合只能登记一次")
        self._subparsers = SubParsers(self, dest=dest, title=title, required=required, help=help)
        return self._subparsers

    # ---- 解析 ----------------------------------------------------------

    def parse_args(self, argv: Sequence[str] | None = None) -> Namespace:
        """解析命令行: 输入非法抛 `UsageError`, 请求帮助 / 版本抛对应异常

        :param argv: 待解析的 token; `None` 表示取 `sys.argv[1:]`
        :return: 解析结果
        """
        namespace, extras = self._parse_tokens(argv, tolerant=False)
        if extras:
            raise self._fail(f"unexpected argument '{extras[0]}' found", kind=ErrorKind.UNKNOWN_ARGUMENT)
        return namespace

    def parse_known_args(self, argv: Sequence[str] | None = None) -> tuple[Namespace, list[str]]:
        """解析命令行(容错): 同 `parse_args`, 但认不出的 token 收进返回值而不报错

        未知的选项只把该 token 本身收进 `extras`, 不连带吞掉它后面的 token —— 那些照常按位置参数分配 (与 `argparse` 一致); 返回值按命令行顺序排列。

        :param argv: 待解析的 token; `None` 表示取 `sys.argv[1:]`
        :return: (解析结果, 未认出的 token 列表)
        """
        return self._parse_tokens(argv, tolerant=True)

    def _expand_files(self, tokens: Sequence[str], depth: int = 0) -> list[str]:
        """展开 `@file`: 把 `@file` 形态的 token 展开成文件里的行

        与 `argparse` 同口径: 每行原样作一个 token (不拆词、不去空白、不认注释), 展开结果里再出现的前缀继续展开; 文件按 UTF-8 读。

        :param tokens: 待展开的 token
        :param depth: 已展开的层数, 用于挡住自引用
        :return: 展开后的 token 列表
        :raises UsageError: 文件打不开或嵌套过深
        """
        prefixes = self.fromfile_prefix_chars or ""
        expanded: list[str] = []
        for token in tokens:
            if not token or token[0] not in prefixes:
                expanded.append(token)
                continue
            path = token[1:]
            if depth >= _MAX_INCLUDE_DEPTH:
                raise self._fail(f"@file nesting too deep: '{path}'", kind=ErrorKind.OTHER)
            try:
                with open(path, encoding="utf-8") as handle:
                    body = handle.read()
            except OSError:
                raise self._fail(f"unable to open file '{path}'", kind=ErrorKind.OTHER) from None
            lines = [line[:-1] if line.endswith("\r") else line for line in body.split("\n")]
            if lines and lines[-1] == "":
                lines.pop()
            expanded.extend(self._expand_files(lines, depth + 1))
        return expanded

    def _parse_tokens(self, argv: Sequence[str] | None, *, tolerant: bool) -> tuple[Namespace, list[str]]:
        """解析骨架: 两个解析入口共用的流程

        :param argv: 待解析的 token; `None` 表示取 `sys.argv[1:]`
        :param tolerant: 为真时未知选项收进 `extras` 而不报错
        :return: (解析结果, 未认出的 token 列表)
        """
        tokens = self._expand_files(list(sys.argv[1:] if argv is None else argv))
        run = _Run(tolerant)
        self._parse(tokens, run, ())
        self._finish(run)
        return run.namespace, [token for _, token in sorted(run.extras, key=lambda item: item[0])]

    def parse_or_exit(self, argv: Sequence[str] | None = None, *, stderr: TextIO | None = None) -> Namespace:
        """解析并出口: 帮助 / 版本退出码 0, 用法错误退出码 2

        :param argv: 待解析的 token; `None` 表示取 `sys.argv[1:]`
        :param stderr: 写错误信息的流; `None` 表示 `sys.stderr`
        :return: 解析结果
        :raises SystemExit: 请求帮助 / 版本, 或输入不合法
        """
        try:
            return self.parse_args(argv)
        except HelpRequested as exc:
            target = exc.parser or self
            sys.stdout.write(target.format_help(long=exc.long) + "\n")
            raise SystemExit(exc.exit_code) from None
        except VersionRequested as exc:
            target = exc.parser or self
            sys.stdout.write((target.version_text() or "") + "\n")
            raise SystemExit(exc.exit_code) from None
        except UsageError as exc:
            stream = sys.stderr if stderr is None else stderr
            stream.write(str(exc) + "\n")
            raise SystemExit(exc.exit_code) from None

    def version_text(self) -> str:
        """渲染版本行: 版本行的正文

        :return: `<prog> <version>` 形态的文本
        """
        return f"{self.prog} {self.version or ''}".strip()

    def _parse(self, tokens: list[str], run: _Run, chain: tuple[Parser, ...], base: int = 0) -> None:
        """扫描: 逐 token 处理一段命令行

        命中子命令时把剩余 token 整段交给子命令; `base` 是本段首个 token 在整条命令行里的位置, 子命令续着它往下排, 这样跨层收下的 `extras` 才能按命令行顺序还原。

        :param tokens: 本段待扫描的 token
        :param run: 本次解析的共享状态
        :param chain: 从上到下的命令链
        :param base: 本段首个 token 在整条命令行里的位置
        """
        run.parsers.append(self)
        pending: list[tuple[int, str]] = []
        index = 0
        while index < len(tokens):
            token = tokens[index]
            if token == "--":
                pending.extend(_slots(tokens[index + 1:], base + index + 1))
                break
            if self._is_option(token):
                expanded = self._expand_cluster(token, chain)
                if expanded is not None:
                    tokens[index:index + 1] = expanded
                    continue
                index = self._consume_option(tokens, index, run, chain, base)
                continue
            if self._remainder_ready(pending):
                pending.extend(_slots(tokens[index:], base + index))
                break
            if self._help_subcommand_of(token, pending):
                raise HelpRequested(long=True, parser=self._help_target(tokens[index + 1:]))
            child = self._subcommand_of(token, pending)
            if child is not None:
                run.subcommand = token
                if self._subparsers is not None and self._subparsers.dest:
                    run.namespace[self._subparsers.dest] = self._subparsers.canonical(token)
                child._parse(tokens[index + 1:], run, (*chain, self), base + index + 1)
                index = len(tokens)
                break
            pending.append((base + index, token))
            index += 1
        self._assign_positionals(pending, run)

    @staticmethod
    def _is_option(token: str) -> bool:
        """判选项: `-` / `--` 与签名数字以外的 `-` 开头 token 是选项

        :param token: 待判 token
        :return: 是选项为 `True`
        """
        if token == "--":
            return True
        if not token.startswith("-") or token == "-":
            return False
        return _SIGNED_NUMBER.match(token) is None

    def _expand_cluster(self, token: str, chain: tuple[Parser, ...]) -> list[str] | None:
        """拆短选项聚合: 把 `-abc` 拆成 `-a -b -c`

        :param token: 待拆的 token
        :param chain: 从上到下的命令链, 供查祖先命令的全局参数
        :return: 拆开后的 token 列表; 拆不动时为 `None`
        """
        if token.startswith("--") or len(token) <= 2 or "=" in token:
            return None
        first = self._lookup("-" + token[1], chain)
        if first is None or first.takes_values():
            return None
        expanded: list[str] = []
        index = 1
        while index < len(token):
            piece = "-" + token[index]
            arg = self._lookup(piece, chain)
            if arg is None:
                # 未认出的尾巴连短横线一起留下, 由后续识别失败报出具体的短选项
                expanded.append("-" + token[index:])
                break
            if arg.takes_values():
                rest = token[index + 1:]
                expanded.append(f"{piece}={rest[1:] if rest.startswith('=') else rest}" if rest else piece)
                break
            expanded.append(piece)
            index += 1
        return expanded

    def _positional_minimum(self) -> int:
        """算位置参数下限: 本命令位置参数至少要几个 token 才够填

        :return: 最少 token 数
        """
        return sum(arg.min_count() for arg in self._positionals)

    def _subcommand_of(self, token: str, pending: Sequence[tuple[int, str]]) -> Parser | None:
        """认子命令: `token` 是子命令名且位置参数已够填

        :param token: 待判 token
        :param pending: 已攒下的非选项 token
        :return: 命中的子命令; 不命中或位置参数还没填够时为 `None`
        """
        if self._subparsers is None:
            return None
        child = self._subparsers.lookup(token)
        if child is None:
            return None
        return child if len(pending) >= self._positional_minimum() else None

    def _help_subcommand_of(self, token: str, pending: Sequence[tuple[int, str]]) -> bool:
        """认 help 子命令: `token` 是否命中自动生成的 `help`

        本命令位置参数尚未够填时 `help` 仍是普通位置 token; 用户自己登记了 `help` 子命令时以用户的为准, 不生成。

        :param token: 待判 token
        :param pending: 已攒下的非选项 token
        :return: 命中自动生成的 `help` 为 `True`
        """
        if token != "help" or self._subparsers is None or self._subparsers.has_command("help"):
            return False
        return len(pending) >= self._positional_minimum()

    def _help_target(self, rest: Sequence[str]) -> Parser:
        """定 help 目标: `help a b` 的下沉目标

        按子命令名逐级走, 走到认不出的名字即报错。

        :param rest: `help` 之后的 token
        :return: 要展示帮助的那条命令
        """
        target: Parser = self
        for token in rest:
            subparsers = target._subparsers
            child = subparsers.lookup(token) if subparsers is not None else None
            if child is None:
                raise target._fail(f"unrecognized subcommand '{token}'", kind=ErrorKind.UNKNOWN_ARGUMENT)
            target = child
        return target

    def _remainder_ready(self, pending: Sequence[tuple[int, str]]) -> bool:
        """判 REMAINDER 就绪: 前序位置参数是否已填够

        :param pending: 已攒下的非选项 token
        :return: 后续 token 全归 REMAINDER 时为 `True`
        """
        for index, arg in enumerate(self._positionals):
            if arg.nargs != REMAINDER:
                continue
            needed = sum(item.min_count() for item in self._positionals[:index])
            return len(pending) >= needed
        return False

    def _lookup(self, name: str, chain: Sequence[Parser]) -> Argument | None:
        """查参数: 先本命令 (含长选项前缀匹配), 再祖先命令上的 `global_` 参数

        :param name: 选项名
        :param chain: 从上到下的命令链
        :return: 命中的参数; 都没有时为 `None`
        """
        arg = self._by_name.get(name)
        if arg is not None:
            return arg
        if self.allow_abbrev and name.startswith("--") and len(name) > 2:
            arg = self._abbreviate(name)
            if arg is not None:
                return arg
        for parent in reversed(chain):
            found = parent._by_name.get(name)
            if found is not None and found.global_:
                return found
        return None

    def _abbreviate(self, name: str) -> Argument | None:
        """前缀匹配: 长选项唯一前缀匹配

        :param name: 用户输入的长选项
        :return: 唯一命中的参数; 没有命中时为 `None`
        :raises UsageError: 命中多个, 报成歧义
        """
        matched: dict[str, Argument] = {}
        for option, arg in self._by_name.items():
            if option.startswith("--") and option.startswith(name):
                matched.setdefault(arg.dest, arg)
        if not matched:
            return None
        if len(matched) > 1:
            options = sorted({option for option in self._by_name if option.startswith(name)})
            raise self._fail(
                f"ambiguous argument '{name}'",
                kind=ErrorKind.AMBIGUOUS_ARGUMENT,
                lines=[f"could be one of: {format_items(options)}"],
            )
        return next(iter(matched.values()))

    def _unknown_argument(self, name: str, chain: Sequence[Parser]) -> UsageError:
        """报未知选项: 给出相似项建议, 长选项再补一句怎么当取值传

        :param name: 用户输入的选项名
        :param chain: 从上到下的命令链
        :return: 构造好的用法错误
        """
        candidates = list(self._by_name)
        for parent in chain:
            candidates.extend(option for option, arg in parent._by_name.items() if arg.global_)
        tips: list[str] = []
        suggestion = best_match(name, candidates)
        if suggestion is not None:
            tips.append(f"a similar argument exists: '{suggestion}'")
        elif name.startswith("--"):
            tips.append(f"to pass '{name}' as a value, use '-- {name}'")
        return self._fail(f"unexpected argument '{name}' found", kind=ErrorKind.UNKNOWN_ARGUMENT, tips=tips)

    def _consume_option(self, tokens: list[str], index: int, run: _Run, chain: tuple[Parser, ...], base: int) -> int:
        """吃选项: 处理一个选项 token

        :param tokens: 待扫描的 token
        :param index: 该选项在 `tokens` 里的下标
        :param run: 本次解析的共享状态
        :param chain: 从上到下的命令链
        :param base: 本段首个 token 在整条命令行里的位置
        :return: 下一个待处理下标
        """
        token = tokens[index]
        name, inline = _split_option(token)
        arg = self._lookup(name, chain)
        if arg is None:
            attached = self._attached_short(name, chain)
            if attached is None:
                if run.tolerant:
                    run.extras.append((base + index, token))
                    return index + 1
                raise self._unknown_argument(name, chain)
            arg, inline = attached
        if not arg.takes_values():
            if inline is not None:
                raise self._fail(
                    f"unexpected value '{inline}' for '{arg.error_name()}' found; no more were expected",
                    kind=ErrorKind.INVALID_VALUE,
                )
            # 长短帮助靠命中的名字区分 (`-h` 走短帮助, `--help` 走长帮助)
            if arg.action is Action.HELP:
                raise HelpRequested(long=name.startswith("--"), parser=self)
            if arg.action is Action.VERSION:
                raise VersionRequested(parser=self)
            self._apply(arg, [], run)
            return index + 1
        if inline is not None:
            self._apply(arg, self._convert_texts(arg, [inline]), run)
            return index + 1
        stop, raw = self._take_values(arg, tokens, index + 1)
        self._apply(arg, self._convert_texts(arg, raw), run)
        return stop

    def _attached_short(self, name: str, chain: tuple[Parser, ...]) -> tuple[Argument, str] | None:
        """拆紧贴取值: `-ovalue` / `-o=value` 里的 `value` 是短选项的取值

        :param name: 待拆的 token
        :param chain: 从上到下的命令链
        :return: (命中的参数, 取值); 不成立时为 `None`
        """
        if name.startswith("--") or len(name) <= 2:
            return None
        arg = self._lookup(name[:2], chain)
        if arg is None or not arg.takes_values():
            return None
        value = name[2:]
        return arg, (value[1:] if value.startswith("=") else value)

    def _take_values(self, arg: Argument, tokens: list[str], start: int) -> tuple[int, list[str]]:
        """取取值: 按 `nargs` 取走取值

        :param arg: 参数定义
        :param tokens: 待扫描的 token
        :param start: 取值起点下标
        :return: (下一个待处理下标, 原始取值列表)
        :raises UsageError: 取值个数不足
        """
        if arg.nargs == REMAINDER:
            return len(tokens), list(tokens[start:])
        minimum, maximum = arg.min_count(), arg.max_count()
        taken: list[str] = []
        index = start
        while index < len(tokens):
            if maximum is not None and len(taken) >= maximum:
                break
            if self._is_option(tokens[index]) and not arg.allow_hyphen_values:
                break
            taken.append(tokens[index])
            index += 1
        if len(taken) < minimum:
            raise self._fail_missing_value(arg, minimum, len(taken))
        return index, taken

    def _fail_missing_value(self, arg: Argument, needed: int, given: int) -> UsageError:
        """报取值不足: 单个与多个的文案分开, 与 clap 同形

        :param arg: 参数定义
        :param needed: 需要的取值个数
        :param given: 实际给出的取值个数
        :return: 构造好的用法错误
        """
        name = arg.error_name()
        if needed <= 1:
            return self._fail(
                f"a value is required for '{name}' but none was supplied", kind=ErrorKind.MISSING_VALUE
            )
        verb = "was" if given == 1 else "were"
        return self._fail(
            f"{needed} values required for '{name}' but {given} {verb} provided", kind=ErrorKind.MISSING_VALUE
        )

    def _convert_texts(self, arg: Argument, raw: Sequence[str]) -> list[Any]:
        """批量转换: 按 `value_delimiter` 拆分后逐个转换

        :param arg: 参数定义
        :param raw: 原始取值 token
        :return: 转换后的取值列表
        """
        texts: list[str] = []
        for text in raw:
            if arg.value_delimiter is not None:
                texts.extend(text.split(arg.value_delimiter))
            else:
                texts.append(text)
        return [self._convert(arg, text) for text in texts]

    def _convert(self, arg: Argument, text: str) -> Any:
        """转换取值: 转换单个取值并校验可选值

        :param arg: 参数定义
        :param text: 原始取值 token
        :return: 转换后的取值
        :raises UsageError: 转换失败或不在可选值里
        """
        value: Any = text
        if arg.type is not None:
            try:
                value = arg.type(text)
            except (ValueError, TypeError) as exc:
                raise self._fail(
                    f"invalid value '{text}' for '{arg.error_name()}'",
                    kind=ErrorKind.INVALID_VALUE,
                    reason=str(exc),
                ) from None
        choices = arg.choice_values()
        if choices is not None and value not in choices:
            tips: list[str] = []
            suggestion = best_match(text, [str(item) for item in choices])
            if suggestion is not None:
                tips.append(f"a similar value exists: '{suggestion}'")
            raise self._fail(
                f"invalid value '{text}' for '{arg.error_name()}'",
                kind=ErrorKind.INVALID_VALUE,
                lines=[f"[possible values: {format_items(choices)}]"],
                tips=tips,
            )
        return value

    def _apply(self, arg: Argument, values: list[Any], run: _Run) -> None:
        """落值: 把一次出现落进命名空间, 按动作决定覆盖 / 追加 / 计数

        :param arg: 参数定义
        :param values: 本次出现转换后的取值
        :param run: 本次解析的共享状态
        """
        action = arg.action
        if action is Action.HELP:
            raise HelpRequested(long=arg.primary_name() == "--help", parser=self)
        if action is Action.VERSION:
            raise VersionRequested(parser=self)
        dest = arg.dest
        run.present.add(dest)
        run.seen.append(dest)
        if action is Action.COUNT:
            base = run.namespace.get(dest)
            run.namespace[dest] = (base if isinstance(base, int) else _implicit_default(arg)) + 1
            return
        if action in (Action.STORE_TRUE, Action.STORE_FALSE, Action.STORE_CONST):
            run.namespace[dest] = arg.const
            return
        if action is Action.APPEND_CONST:
            self._append(run, dest, arg.const)
            return
        if action is Action.EXTEND:
            base = run.namespace.get(dest)
            if not isinstance(base, list):
                base = list(arg.default) if isinstance(arg.default, (list, tuple)) else []
            base.extend(values)
            run.namespace[dest] = base
            return
        if action is Action.APPEND:
            self._append(run, dest, values if arg.collects() else (values[0] if values else None))
            return
        if not values:
            run.namespace[dest] = arg.const if arg.const is not MISSING else None
            return
        run.namespace[dest] = values if arg.collects() else values[0]

    @staticmethod
    def _append(run: _Run, dest: str, item: Any) -> None:
        """追加: 往命名空间里的列表追加一项, 首次出现时新开列表

        :param run: 本次解析的共享状态
        :param dest: 命名空间里的键名
        :param item: 要追加的取值
        """
        current = run.namespace.get(dest)
        items = list(current) if isinstance(current, list) else []
        items.append(item)
        run.namespace[dest] = items

    def _assign_positionals(self, pending: Sequence[tuple[int, str]], run: _Run) -> None:
        """分位置参数: 把攒下的非选项 token 按定义顺序分给位置参数

        变长形态 (`*` / `+` / `?`) 先给后面的位置参数留够最小值, 剩下的归自己, 因此 `src*` 后接 `dst` 时 `dst` 一定拿得到最后一个 token。

        :param pending: 已攒下的非选项 token 及序号
        :param run: 本次解析的共享状态
        """
        tokens = [token for _, token in pending]
        positionals = self._positionals
        cursor = 0
        total = len(tokens)
        for index, arg in enumerate(positionals):
            tail = sum(item.min_count() for item in positionals[index + 1:])
            available = max(0, total - cursor - tail)
            if arg.nargs == REMAINDER:
                take = total - cursor
            elif arg.max_count() is None:
                take = available
            else:
                take = min(arg.max_count() or 0, available)
            if take < arg.min_count():
                run.missing.append(arg)
                continue
            chunk = tokens[cursor:cursor + take]
            cursor += take
            self._store_positional(arg, chunk, run)
        if cursor < total:
            run.extras.extend(pending[cursor:])

    def _store_positional(self, arg: Argument, chunk: Sequence[str], run: _Run) -> None:
        """落位置参数: 空片段按 const / default / 空列表依次退让

        :param arg: 参数定义
        :param chunk: 分给该参数的那一段 token
        :param run: 本次解析的共享状态
        """
        if not chunk:
            if arg.const is not MISSING:
                run.namespace[arg.dest] = arg.const
            elif arg.default is not MISSING:
                run.namespace[arg.dest] = arg.default
            elif arg.nargs in ("*", REMAINDER):
                run.namespace[arg.dest] = []
            else:
                run.namespace[arg.dest] = None
            return
        run.present.add(arg.dest)
        run.seen.append(arg.dest)
        values = self._convert_texts(arg, chunk)
        run.namespace[arg.dest] = values if arg.collects() else values[0]

    def _finish(self, run: _Run) -> None:
        """收尾: 覆盖 → 环境变量兜底 → 校验 → 填默认值

        :param run: 本次解析的共享状态
        """
        self._apply_overrides(run)
        self._apply_env(run)
        self._validate(run)
        self._apply_defaults(run)

    def _registry(self, run: _Run) -> tuple[dict[str, Argument], Callable[[str], str]]:
        """建索引: 命令链上的 dest → 参数, 以及引用解析器

        :param run: 本次解析的共享状态
        :return: (dest 到参数的字典, 名字或 dest 到 dest 的解析函数)
        :raises ValueError: 父子命令共用同一个 dest
        """
        registry: dict[str, Argument] = {}
        by_name: dict[str, Argument] = {}
        for parser in run.parsers:
            for arg in parser._arguments:
                if arg.action in (Action.HELP, Action.VERSION):
                    # 帮助 / 版本只在解析途中抛异常, 从不落进命名空间, 各层同名不冲突
                    continue
                if arg.dest in registry:
                    raise ValueError(f"命令链上的 dest 重复: {arg.dest!r} (父命令与子命令不能共用 dest)")
                registry[arg.dest] = arg
            by_name.update(parser._by_name)

        def resolve(ref: str) -> str:
            """解析引用: 把名字或 dest 换成 dest

            :param ref: 参数名或 dest
            :return: dest
            :raises ValueError: 指向了未登记的参数
            """
            if ref in registry:
                return ref
            found = by_name.get(ref)
            if found is None:
                raise ValueError(f"overrides_with / conflicts_with / requires 指向了未登记的参数: {ref!r}")
            return found.dest

        return registry, resolve

    def _apply_overrides(self, run: _Run) -> None:
        """应用覆盖: `overrides_with` 双向生效

        一对里命令行上后出现的留下, 先出现的退回默认值。

        :param run: 本次解析的共享状态
        """
        registry, resolve = self._registry(run)
        pairs: dict[str, set[str]] = {}
        for arg in registry.values():
            for ref in arg.overrides_with:
                other = resolve(ref)
                pairs.setdefault(arg.dest, set()).add(other)
                pairs.setdefault(other, set()).add(arg.dest)
        if not pairs:
            return
        kept: set[str] = set()
        for dest in run.seen:
            for other in pairs.get(dest, ()):
                kept.discard(other)
            kept.add(dest)
        for dest in {item for item in run.seen if item not in kept}:
            run.present.discard(dest)
            del run.namespace[dest]

    def _apply_env(self, run: _Run) -> None:
        """读环境变量: 命令行没出现时从环境变量取值, 命中即视为"出现过"

        :param run: 本次解析的共享状态
        """
        for parser in run.parsers:
            for arg in parser._arguments:
                if arg.env is None or arg.dest in run.present or arg.env not in os.environ:
                    continue
                text = os.environ[arg.env]
                if arg.takes_values():
                    parser._apply(arg, parser._convert_texts(arg, [text]), run)
                    continue
                try:
                    enabled = parse_bool()(text)
                except ValueError as exc:
                    raise parser._fail(
                        f"invalid value '{text}' for '{arg.error_name()}'",
                        kind=ErrorKind.INVALID_VALUE,
                        reason=str(exc),
                    ) from None
                fallback = arg.default if arg.default is not MISSING and arg.default is not SUPPRESS else False
                run.namespace[arg.dest] = arg.const if enabled else fallback
                run.present.add(arg.dest)

    @staticmethod
    def _needs_argument(arg: Argument, run: _Run, resolve: Callable[[str], str]) -> bool:
        """判必填: `required`, 或 `required_unless_present` 列出的一个都没出现

        :param arg: 参数定义
        :param run: 本次解析的共享状态
        :param resolve: 名字或 dest 到 dest 的解析函数
        :return: 这一刻算必填为 `True`
        """
        if arg.required:
            return True
        if not arg.required_unless_present:
            return False
        return not any(resolve(ref) in run.present for ref in arg.required_unless_present)

    def _validate(self, run: _Run) -> None:
        """校验: `conflicts_with` → 互斥组 → 多值组 → `requires` → 必填

        报出第一处问题。

        :param run: 本次解析的共享状态
        """
        parsed = run.parsers
        leaf = parsed[-1]
        registry, resolve = self._registry(run)

        def label(dest: str) -> str:
            """取指代: 把 dest 换成错误文案里的写法

            :param dest: 命名空间里的键名
            :return: 错误文案里的指代
            """
            return registry[dest].error_name()

        for arg in registry.values():
            if arg.dest not in run.present:
                continue
            for ref in arg.conflicts_with:
                dest = resolve(ref)
                if dest in run.present:
                    raise leaf._fail(
                        f"the argument '{arg.error_name()}' cannot be used with '{label(dest)}'",
                        kind=ErrorKind.CONFLICT,
                    )

        for parser in parsed:
            for group in parser._mutex:
                hits = [arg for arg in group.arguments if arg.dest in run.present]
                if len(hits) > 1:
                    raise leaf._fail(
                        f"the argument '{hits[0].error_name()}' cannot be used with '{hits[1].error_name()}'",
                        kind=ErrorKind.CONFLICT,
                    )
                if not hits and group.required:
                    raise leaf._fail(
                        "one of the following arguments is required:",
                        kind=ErrorKind.MISSING_REQUIRED,
                        lines=[arg.error_name() for arg in group.arguments],
                    )

        for parser in parsed:
            for group in parser._value_groups:
                if not group.required:
                    continue
                if any(arg.dest in run.present for arg in group.arguments):
                    continue
                raise leaf._fail(
                    "the following required arguments were not provided:",
                    kind=ErrorKind.MISSING_REQUIRED,
                    lines=[f"<{group.name}>"],
                )

        for arg in registry.values():
            if arg.dest not in run.present:
                continue
            for ref in arg.requires:
                dest = resolve(ref)
                if dest not in run.present:
                    raise leaf._fail(
                        f"the argument '{arg.error_name()}' requires '{label(dest)}'",
                        kind=ErrorKind.MISSING_REQUIRED,
                    )

        missing: list[Argument] = list(run.missing)
        for parser in parsed:
            if parser.subcommand_negates_reqs and run.subcommand is not None:
                continue
            for arg in parser._arguments:
                if arg.dest in run.present or not self._needs_argument(arg, run, resolve):
                    continue
                missing.append(arg)
            if parser is leaf and parser._subparsers is not None and parser._subparsers.required:
                raise leaf._fail(
                    "the following required arguments were not provided:",
                    kind=ErrorKind.MISSING_REQUIRED,
                    lines=["<COMMAND>"],
                )
        if missing:
            raise leaf._fail(
                "the following required arguments were not provided:",
                kind=ErrorKind.MISSING_REQUIRED,
                lines=_unique(arg.error_name() for arg in missing),
            )

    def _apply_defaults(self, run: _Run) -> None:
        """填默认值: 没出现在命令行上的参数填默认值

        `SUPPRESS` 表示不写进命名空间。

        :param run: 本次解析的共享状态
        """
        for parser in run.parsers:
            for arg in parser._arguments:
                if arg.dest in run.namespace:
                    continue
                if arg.default is not MISSING:
                    value = arg.default
                elif parser.argument_default is not MISSING:
                    value = parser.argument_default
                else:
                    value = _implicit_default(arg)
                if value is SUPPRESS:
                    continue
                run.namespace[arg.dest] = value

    # ---- 渲染 ----------------------------------------------------------

    def usage_spec(self) -> str:
        """渲染用法: usage 行 (`Usage:` 之后) 的字符串形态

        :return: 用法行文本; 自定义 `usage` 优先
        """
        if self.usage is not None:
            return self.usage
        parts = [self.prog]
        for arg in self._arguments:
            if arg.required and not arg.hide:
                parts.append(arg.error_name())
        if any(not arg.hide and not arg.required and arg.action not in (Action.HELP, Action.VERSION)
               for arg in self._arguments if not arg.is_positional()):
            parts.append("[OPTIONS]")
        parts.extend(arg.usage_spec() for arg in self._positionals if not arg.hide)
        if self._subparsers is not None:
            parts.append("<COMMAND>" if self._subparsers.required else "[COMMAND]")
        return " ".join(parts)

    def width(self) -> int:
        """取宽度: 开了 `auto_width` 时读 `COLUMNS`, 取不到或非法就用 `max_width`

        :return: 帮助的生效宽度
        """
        if self.auto_width:
            raw = os.environ.get("COLUMNS", "").strip()
            if raw.isdigit() and int(raw) > 0:
                return int(raw)
        return self.max_width

    def format_help(self, long: bool = False) -> str:
        """渲染帮助: 整段帮助文本 (不含末尾换行)

        :param long: 为真时取长帮助
        :return: 帮助文本
        """
        from .help import render_help

        return render_help(self, long=long)

    def format_usage(self) -> str:
        """渲染用法: 单行用法文本

        :return: 用法行
        """
        from .help import render_usage

        return render_usage(self)

    def print_help(self, file: TextIO | None = None, long: bool = False) -> None:
        """打印帮助: 把帮助写到流 (默认 stdout)

        :param file: 目标流; `None` 表示 `sys.stdout`
        :param long: 为真时取长帮助
        """
        stream = sys.stdout if file is None else file
        stream.write(self.format_help(long=long) + "\n")

    def _fail(self, message: str, *, kind: ErrorKind, **kwargs: Any) -> UsageError:
        """造错误: 构造一个带类别、本命令 usage 与帮助提示的 `UsageError`

        :param message: 错误消息, 落在首行冒号之前
        :param kind: 类别判据
        :param kwargs: 透传给 `UsageError` 的其余字段
        :return: 构造好的用法错误
        """
        return UsageError(
            message,
            kind=kind,
            usage=kwargs.pop("usage", self.usage_spec()),
            hint=kwargs.pop("hint", "For more information, try '--help'." if "help" in self._dests else None),
            **kwargs,
        )

    def _color_enabled(self) -> bool:
        """判着色: `always` / `never` 直接定, `auto` 看 stdout 与 `NO_COLOR`

        :return: 需要着色为 `True`
        """
        if self.color == "always":
            return True
        if self.color == "never":
            return False
        if os.environ.get("NO_COLOR"):
            return False
        try:
            return bool(sys.stdout.isatty())
        except (AttributeError, ValueError):
            return False
