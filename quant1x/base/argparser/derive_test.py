# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""`derive` 的用例: 注解到转换器 / 数量形态 / 必填的映射。

运行: python -m quant1x.base.argparser.derive_test
"""

from dataclasses import dataclass, field
from enum import Enum
from typing import Literal

from quant1x.base.argparser import Parser, UsageError
from quant1x.base.argparser.derive import add_arguments, from_dataclass

_failures: list[str] = []


def check(name: str, condition: bool, detail: object = "") -> None:
    """记一条断言: 失败时不中断, 跑完统一以非零退出码收尾

    :param name: 断言名, 输出与失败汇总都用它
    :param condition: 断言结果
    :param detail: 失败时附带的现场信息
    """
    if condition:
        print(f"ok   {name}")
    else:
        _failures.append(name)
        print(f"FAIL {name}" + (f" | {detail}" if detail != "" else ""))


def failure(parser: Parser, argv: list[str]) -> str:
    """跑一次解析并取用法错误的文本

    :param parser: 待测命令
    :param argv: 待解析的命令行
    :return: 错误文本; 没报错时为空串
    """
    try:
        parser.parse_args(argv)
    except UsageError as exc:
        return str(exc)
    return ""


class Level(Enum):
    DEBUG = "debug"
    INFO = "info"


@dataclass
class Options:
    output: str = field(metadata={"help": "输出文件"})
    jobs: int = 4
    ratio: float = 1.0
    verbose: bool = False
    keep: bool = True
    level: Literal["debug", "info"] = "info"
    mode: Level = Level.INFO
    tags: list[str] = field(default_factory=list)
    sizes: list[int] = field(default_factory=list)
    note: str | None = None
    files: str = field(default="", metadata={"positional": True})


parser = from_dataclass(Options, "demo", color="never")

namespace = parser.parse_args(["--output", "a.txt"])
check("无默认值的字段是必填", "output" in failure(parser, []), failure(parser, []))
check("选项名由字段名去下划线", "--output" in parser.format_usage(), parser.format_usage())
check("字符型字段原样落值", namespace.output == "a.txt", namespace.output)
check("整数默认值", namespace.jobs == 4, namespace.jobs)
check("浮点默认值", namespace.ratio == 1.0, namespace.ratio)
check("布尔默认假", namespace.verbose is False)
check("布尔默认真", namespace.keep is True)
check("可选值默认取字面量", namespace.level == "info", namespace.level)
check("枚举默认取成员取值", namespace.mode == "info", namespace.mode)
check("多值字段默认空列表", namespace.tags == [] and namespace.sizes == [], namespace)
check("可空字段默认 None", namespace.note is None, namespace.note)

namespace = parser.parse_args(
    ["--output", "a.txt", "--jobs", "8", "--ratio", "0.5", "--verbose", "--keep", "--mode", "debug"]
)
check("整数转换", namespace.jobs == 8 and isinstance(namespace.jobs, int), namespace.jobs)
check("浮点转换", namespace.ratio == 0.5 and isinstance(namespace.ratio, float), namespace.ratio)
check("开关置真", namespace.verbose is True)
check("默认真的开关置假", namespace.keep is False, namespace.keep)
check("枚举取值进命名空间", namespace.mode == "debug", namespace.mode)

namespace = parser.parse_args(["--output", "a.txt", "--tags", "x", "y", "--sizes", "1", "2"])
check("多值字段收成列表", namespace.tags == ["x", "y"], namespace.tags)
check("多值字段的元素转换", namespace.sizes == [1, 2], namespace.sizes)

namespace = parser.parse_args(["--output", "a.txt", "--note"])
check("可空字段裸写落 None", namespace.note is None, namespace.note)
namespace = parser.parse_args(["--output", "a.txt", "--note", "hi"])
check("可空字段带值", namespace.note == "hi", namespace.note)

text = failure(parser, ["--output", "a.txt", "--jobs", "many"])
check("非法整数报取值非法", "invalid value 'many'" in text, text)
text = failure(parser, ["--output", "a.txt", "--level", "warn"])
check("可选值非法时报候选", "invalid value 'warn'" in text, text)
text = failure(parser, ["--output", "a.txt", "--mode", "trace"])
check("枚举取值同样受限", "invalid value 'trace'" in text, text)

namespace = parser.parse_args(["--output", "a.txt", "a.txt.bak"])
check("位置参数字段按顺序取值", namespace.files == "a.txt.bak", namespace.files)
namespace = parser.parse_args(["--output", "a.txt"])
check("位置参数有默认值时可省", namespace.files == "", namespace.files)

check("说明文字进帮助", "输出文件" in parser.format_help(), parser.format_help())
order = [argument.dest for argument in parser.arguments if argument.dest != "help"]
check("选项按字段顺序登记", order == list(Options.__dataclass_fields__), order)

solo = Parser("solo", color="never")
added = add_arguments(solo, Options)
check("可挂到已有命令上", len(added) == len(Options.__dataclass_fields__), len(added))
check("登记结果的首个参数是必填", added[0].required, added[0])


def _unsupported() -> None:
    """建一个含不支持注解的 dataclass 并派生参数, 应在定义期报错"""
    @dataclass
    class Bad:
        payload: dict[str, str] = field(default_factory=dict)

    from_dataclass(Bad, "bad", color="never")


def _optional_bool() -> None:
    """建一个布尔可空字段的 dataclass 并派生参数, 应在定义期报错"""
    @dataclass
    class Bad:
        flag: bool | None = None

    from_dataclass(Bad, "bad", color="never")


def _mixed_literal() -> None:
    """建一个可选值类型不一致的 dataclass 并派生参数, 应在定义期报错"""
    @dataclass
    class Bad:
        mixed: Literal[1, "two"] = 1

    from_dataclass(Bad, "bad", color="never")


for label, builder in (
    ("不支持的注解在定义期报错", _unsupported),
    ("布尔不能是可空形态", _optional_bool),
    ("可选值类型必须一致", _mixed_literal),
):
    try:
        builder()
    except ValueError:
        check(label, True)
    else:
        check(label, False)

try:
    from_dataclass(str, "bad", color="never")
except TypeError:
    check("非 dataclass 报 TypeError", True)
else:
    check("非 dataclass 报 TypeError", False)

print()
if _failures:
    print(f"{len(_failures)} 处断言失败")
    raise SystemExit(1)
print("derive: 全部断言通过")
