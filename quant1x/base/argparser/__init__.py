# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""命令行解析: Python `argparse` 的语义 + Rust `clap` 的交互。"""

from .complete import SHELLS, complete
from .derive import add_arguments, from_dataclass
from .error import ArgParserError, ErrorKind, HelpRequested, UsageError, VersionRequested
from .parser import ArgumentGroup, MutuallyExclusiveGroup, Parser, SubParsers, ValueGroup
from .spec import REMAINDER, SUPPRESS, Action, Argument, Namespace, ValueParser, parse_bool, parse_float, parse_int

__all__ = [
    "REMAINDER",
    "SHELLS",
    "SUPPRESS",
    "Action",
    "ArgParserError",
    "Argument",
    "ArgumentGroup",
    "ErrorKind",
    "HelpRequested",
    "MutuallyExclusiveGroup",
    "Namespace",
    "Parser",
    "SubParsers",
    "UsageError",
    "ValueGroup",
    "ValueParser",
    "VersionRequested",
    "add_arguments",
    "complete",
    "from_dataclass",
    "parse_bool",
    "parse_float",
    "parse_int",
]
