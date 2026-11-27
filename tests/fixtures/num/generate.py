# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

"""生成 num 规格层的跨语言对拍样本（只在 Python 侧运行，其它语言只读产物）

输出 ``cases.csv``：一行一个用例，输入序列与期望序列都用 ``|`` 分隔。
期望值只由 ``quant1x.num`` 的 Python spec 生成，其它语言只能被比对。
重跑本脚本必须得到同样字节。

列含义：
    function  算子名（对应 quant1x/num/*.py 的函数名）
    case      用例标签
    a, b, c   输入序列（float64，Nan 写作 nan；bool 写作 0/1）
    w         逐位置窗口序列（int64）
    n         标量参数（alpha / periods / digits / length ...）
    expected  期望输出序列；标量输出只有一项
"""

from __future__ import annotations

import csv
import math
from pathlib import Path

from quant1x import num

ROOT = Path(__file__).resolve().parent

A = [1.0, 2.0, 3.0, 4.0, 5.0, 4.0, 3.0, 2.0, 1.0, 2.0]
B = [2.0, 2.0, 3.0, 3.0, 4.0, 4.0, 5.0, 5.0, 6.0, 6.0]
AN = [1.0, math.nan, 3.0, 4.0, math.nan, 4.0, 3.0, 2.0, 1.0, 2.0]
W = [3, 3, 3, 3, 3, 0, -1, 2, 2, 2]
COND = [1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0]


def enc(values) -> str:
    """浮点/布尔/整型序列编码为 ``|`` 分隔字符串。"""
    return "|".join(repr(float(value)) for value in values)


def scalar(value) -> str:
    return repr(float(value))


ROWS: list[list[str]] = []


def emit(function: str, case: str, a=(), b=(), c=(), w=(), n="", expected=()) -> None:
    ROWS.append([
        function,
        case,
        enc(a),
        enc(b),
        enc(c),
        "|".join(str(int(value)) for value in w),
        n,
        enc(expected),
    ])


def emit_bool(function: str, case: str, a=(), b=(), c=(), w=(), n="", expected=()) -> None:
    ROWS.append([
        function,
        case,
        enc(a),
        enc(b),
        enc(c),
        "|".join(str(int(value)) for value in w),
        n,
        "|".join("1" if value else "0" for value in expected),
    ])


def emit_scalar(function: str, case: str, a=(), b=(), c=(), w=(), n="", expected=()) -> None:
    ROWS.append([
        function,
        case,
        enc(a),
        enc(b),
        enc(c),
        "|".join(str(int(value)) for value in w),
        n,
        "|".join(scalar(value) for value in expected),
    ])


# dtype / to_scalar
emit_scalar("to_scalar", "series", a=A, expected=[num.to_scalar(A)])
emit_scalar("to_scalar", "empty", a=[], expected=[num.to_scalar([])])
emit("default_like", "series", a=A, expected=num.default_like(A))

# elementwise: 一元
for function in ("abs", "sqrt", "log"):
    emit(function, "series", a=A, expected=getattr(num, function)(A))
    emit(function, "nan", a=AN, expected=getattr(num, function)(AN))
emit("abs", "negative", a=[-1.0, -0.0, 2.0], expected=num.abs([-1.0, -0.0, 2.0]))
emit("sqrt", "negative", a=[-1.0, 0.0, 4.0], expected=num.sqrt([-1.0, 0.0, 4.0]))
emit("log", "nonpositive", a=[-1.0, 0.0, 2.0], expected=num.log([-1.0, 0.0, 2.0]))
emit("pow", "const", a=A, n="2", expected=num.pow(A, 2))
emit("pow", "fractional", a=[1.0, 4.0, 9.0], n="0.5", expected=num.pow([1.0, 4.0, 9.0], 0.5))

# elementwise: 二元
for function in ("add", "sub", "mul", "div", "maximum", "minimum"):
    emit(function, "series", a=A, b=B, expected=getattr(num, function)(A, B))
    emit(function, "nan", a=AN, b=B, expected=getattr(num, function)(AN, B))
emit("add", "short_right", a=A, b=[1.0, 2.0], expected=num.add(A, [1.0, 2.0]))
emit("add", "long_right", a=A, b=B + [7.0, 8.0], expected=num.add(A, B + [7.0, 8.0]))

# elementwise: 标量
for function in ("add_scalar", "sub_scalar", "mul_scalar", "div_scalar",
                 "maximum_scalar", "minimum_scalar"):
    emit(function, "series", a=A, n="1.5", expected=getattr(num, function)(A, 1.5))
emit("maximum_scalar", "nan", a=AN, n="3", expected=num.maximum_scalar(AN, 3.0))
emit("minimum_scalar", "nan", a=AN, n="3", expected=num.minimum_scalar(AN, 3.0))

# elementwise: 条件选择
emit("where", "series", a=COND, b=A, c=B, expected=num.where(COND, A, B))
emit("where", "short", a=[1.0, 0.0], b=A, c=B, expected=num.where([1.0, 0.0], A, B))

# logic: 比较
for function in ("gt", "gte", "lt", "lte", "equal", "not_equal"):
    emit_bool(function, "series", a=A, b=B, expected=getattr(num, function)(A, B))
    emit_bool(function, "nan", a=AN, b=B, expected=getattr(num, function)(AN, B))
emit_bool("gt", "short_right", a=A, b=[1.0, 2.0], expected=num.gt(A, [1.0, 2.0]))

# logic: 布尔
emit_bool("to_bools", "series", a=[1.0, 0.0, -1.0, math.nan], expected=num.to_bools([1.0, 0.0, -1.0, math.nan]))
emit_bool("logic_and", "series", a=COND, b=[1.0] * 10, expected=num.logic_and(COND, [1.0] * 10))
emit_bool("logic_or", "series", a=COND, b=[0.0] * 10, expected=num.logic_or(COND, [0.0] * 10))
emit_bool("logic_not", "series", a=COND, expected=num.logic_not(COND))
emit_bool("logic_and", "short", a=[1.0, 0.0], b=[1.0, 1.0, 1.0], expected=num.logic_and([1.0, 0.0], [1.0, 1.0, 1.0]))
emit_scalar("all_of", "true", a=[1.0, 1.0], expected=[num.all_of([1.0, 1.0])])
emit_scalar("all_of", "false", a=[1.0, 0.0], expected=[num.all_of([1.0, 0.0])])
emit_scalar("any_of", "series", a=[0.0, 0.0, 1.0], expected=[num.any_of([0.0, 0.0, 1.0])])
emit_scalar("count_true", "series", a=COND, expected=[num.count_true(COND)])

# reduce
for function in ("sum_value", "mean", "median", "min_value", "max_value",
                 "argmin", "argmax", "variance", "std"):
    emit_scalar(function, "series", a=A, expected=[getattr(num, function)(A)])
    emit_scalar(function, "empty", a=[], expected=[getattr(num, function)([])])
    emit_scalar(function, "nan", a=AN, expected=[getattr(num, function)(AN)])
emit_scalar("covariance", "series", a=A, b=B, expected=[num.covariance(A, B)])
emit_scalar("covariance", "mismatch", a=A, b=[1.0], expected=[num.covariance(A, [1.0])])

# rolling: 常量窗口
ROLL = ("rolling_sum", "rolling_mean", "rolling_std", "rolling_min", "rolling_max",
        "rolling_avedev", "rolling_hhvbars", "rolling_llvbars")
for function in ROLL:
    emit(function, "const", a=A, n="3", expected=getattr(num, function)(A, 3))
    emit(function, "nan", a=AN, n="3", expected=getattr(num, function)(AN, 3))
    emit(function, "variable", a=A, w=W, expected=getattr(num, function)(A, W))

# ewm
emit("ewm_mean", "const", a=A, n="0.3", expected=num.ewm_mean(A, 0.3))
emit("ewm_mean", "nan", a=AN, n="0.3", expected=num.ewm_mean(AN, 0.3))

# shift / diff / cumsum
for periods in (0, 1, 2, -1, -2):
    emit("shift", f"p{periods}", a=A, n=str(periods), expected=num.shift(A, periods))
    emit("ref", f"p{periods}", a=A, n=str(periods), expected=num.ref(A, periods))
    emit("diff", f"p{periods}", a=A, n=str(periods), expected=num.diff(A, periods))
emit("cumsum", "series", a=A, expected=num.cumsum(A))
emit("cumsum", "nan", a=AN, expected=num.cumsum(AN))

# fillna
emit("fill", "series", a=AN, n="0", expected=num.fill(AN, 0.0))
emit("fill_na", "series", a=AN, n="-1", expected=num.fill_na(AN, -1.0))
emit_bool("is_na", "series", a=AN, expected=num.is_na(AN))

# align / repeat
emit("align", "truncate", a=A, n="4", expected=num.align(A, 4))
emit("align", "extend", a=A, n="12", expected=num.align(A, 12))
emit("pad_to", "extend", a=A, n="12", expected=num.pad_to(A, 12))
emit("repeat", "series", a=[1.0, 2.0], n="5", expected=num.repeat([1.0, 2.0], 5))
emit("repeat", "empty", a=[], n="3", expected=num.repeat([], 3))

# regression
emit_scalar("linear_regression", "series", a=A, expected=num.linear_regression(A))
emit_scalar("linear_regression", "degenerate", a=[1.0], expected=num.linear_regression([1.0]))
emit_scalar("linear_regression", "nan", a=AN, expected=num.linear_regression(AN))
emit_scalar("slope", "series", a=A, expected=[num.slope(A)])
emit_scalar("forcast", "series", a=A, expected=[num.forcast(A)])

# decimals
emit_scalar("decimal", "half_up", a=[0.5], n="0", expected=[num.decimal(0.5, 0)])
emit_scalar("decimal", "half_down", a=[-0.5], n="0", expected=[num.decimal(-0.5, 0)])
emit_scalar("decimal", "nan", a=[math.nan], n="0", expected=[num.decimal(math.nan, 0)])
emit("decimal_series", "series", a=A, n="1", expected=num.decimal(A, 1))
emit_scalar("change_rate", "series", a=[2.0], b=[3.0], expected=[num.change_rate(2.0, 3.0)])
emit_scalar("net_change_rate", "series", a=[2.0], b=[3.0], expected=[num.net_change_rate(2.0, 3.0)])

# matrix
emit_scalar("shape", "series", a=A, expected=num.shape(A))
emit("concat1d", "series", a=A, b=B, expected=num.concat1d(A, B))
MATRIX = [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]
for column in num.transpose2d(MATRIX):
    emit("transpose2d", f"c{len(ROWS)}", a=list(column), expected=list(column))
emit("zeros", "series", n="3", expected=num.zeros(3))
emit("ones", "series", n="3", expected=num.ones(3))
emit("arange", "positive", n="5", expected=num.arange(5.0))
emit("arange", "step", a=[5.0], b=[0.0], c=[-1.0], expected=num.arange(5.0, 0.0, -1.0))
emit("linspace", "series", n="5", expected=num.linspace(0.0, 1.0, 5))
emit("linspace", "one", n="1", expected=num.linspace(0.0, 1.0, 1))
emit("linspace", "empty", n="0", expected=num.linspace(0.0, 1.0, 0))


def main() -> None:
    """把用例写成 CSV。"""
    path = ROOT / "cases.csv"
    with path.open("w", newline="", encoding="utf-8", errors="strict") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(["function", "case", "a", "b", "c", "w", "n", "expected"])
        writer.writerows(ROWS)
    print(f"{path}: {len(ROWS)} cases")


if __name__ == "__main__":
    main()
