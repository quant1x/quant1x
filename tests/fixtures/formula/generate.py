"""生成公式与指标的对拍样本（只在 Python 侧运行，其它语言只读产物）

输出：

- `cases.csv`：公式函数的逐位置输入与期望值，长表，一行一个位置；
- `indicators/bars.csv`：120 根合成 K 线；
- `indicators/<指标>.csv`：指标输出列，一行一根 K 线。

口径：期望值只由 `quant1x.formula` / `quant1x.indicators` 生成，其它语言只能被比对。
重跑本脚本必须得到同样字节（依赖固定随机种子与固定输入序列）。
"""

from __future__ import annotations

import argparse
import csv
import io
import math
from pathlib import Path

import numpy as np
import pandas as pd

from quant1x import formula
from quant1x.indicators.cdtd import cdtd
from quant1x.indicators.f89k import f89k
from quant1x.indicators.hjk import hjk
from quant1x.indicators.ma1x import ma1x
from quant1x.indicators.p2fb import p2fb

ROOT = Path(__file__).resolve().parent

# 固定输入：0 级与 1 级函数的逐位置样本
A = [1.0, 2.0, 3.0, 4.0, 5.0, 4.0, 3.0, 2.0, 1.0, 2.0,
     3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 7.0, 6.0, 5.0, 4.0,
     3.0, 2.0, 1.0, 2.0, 3.0, 4.0, 5.0, 4.0, 3.0, 2.0,
     1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0]
B = [2.0, 2.0, 2.0, 3.0, 4.0, 5.0, 6.0, 5.0, 4.0, 3.0,
     2.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0,
     8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 2.0, 3.0,
     4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 8.0, 7.0, 6.0, 5.0]
A_NAN = list(A)
A_NAN[3] = math.nan
A_NAN[10] = math.nan
WINDOW = [3] * 10 + [5] * 10 + [0, -1] + [3] * 18
WEIGHT = [0.5] * 20 + [math.nan] + [0.3] * 19

CASES = [
    ("abs", "series", {"a": A}),
    ("abs", "nan", {"a": A_NAN}),
    ("sqrt", "series", {"a": A}),
    ("ln", "series", {"a": A}),
    ("pow", "const", {"a": A, "n": 2}),
    ("max", "series", {"a": A, "b": B}),
    ("min", "series", {"a": A, "b": B}),
    ("if", "series", {"a": [1.0 if value > 0.0 else 0.0 for value in A], "b": B, "c": A_NAN}),
    ("ref", "const", {"a": A, "n": 1}),
    ("ref", "variable", {"a": A, "w": WINDOW}),
    ("diff", "const", {"a": A, "n": 2}),
    ("sum", "const", {"a": A, "n": 5}),
    ("sum", "all", {"a": A, "n": 0}),
    ("sum", "variable", {"a": A, "w": WINDOW}),
    ("const", "series", {"a": A}),
    ("hhv", "const", {"a": A, "n": 5}),
    ("hhv", "variable", {"a": A, "w": WINDOW}),
    ("llv", "const", {"a": A, "n": 5}),
    ("llv", "variable", {"a": A, "w": WINDOW}),
    ("hhvbars", "const", {"a": A, "n": 5}),
    ("llvbars", "const", {"a": A, "n": 5}),
    ("ma", "const", {"a": A, "n": 5}),
    ("ma", "variable", {"a": A, "w": WINDOW}),
    ("ema", "const", {"a": A, "n": 5}),
    ("sma", "const", {"a": A, "n": 5, "m": 1}),
    ("wma", "const", {"a": A, "n": 5}),
    ("std", "const", {"a": A, "n": 5}),
    ("avedev", "const", {"a": A, "n": 5}),
    ("slope", "const", {"a": A, "n": 5}),
    ("forcast", "const", {"a": A, "n": 5}),
    ("dma", "const", {"a": A, "n": 0.3}),
    ("dma", "variable", {"a": A, "b": WEIGHT}),
    ("last", "series", {"a": [1.0 if value > 2.0 else 0.0 for value in A], "n": 4, "m": 1}),
    ("count", "const", {"a": [1.0 if value > 2.0 else 0.0 for value in A], "n": 5}),
    ("every", "const", {"a": [1.0 if value > 2.0 else 0.0 for value in A], "n": 5}),
    ("exist", "const", {"a": [1.0 if value > 2.0 else 0.0 for value in A], "n": 5}),
    ("filter", "const", {"a": [1.0 if value > 4.0 else 0.0 for value in A], "n": 3}),
    ("barslast", "series", {"a": [1.0 if value > 4.0 else 0.0 for value in A]}),
    ("barslastcount", "series", {"a": [1.0 if value > 2.0 else 0.0 for value in A]}),
    ("barssincen", "const", {"a": [1.0 if value > 4.0 else 0.0 for value in A], "n": 5}),
    ("cross", "series", {"a": A, "b": B}),
    ("longcross", "const", {"a": A, "b": B, "n": 3}),
    ("valuewhen", "series", {"a": [1.0 if value > 4.0 else 0.0 for value in A], "b": A}),
    ("between", "const", {"a": A, "n": 2.0, "m": 5.0}),
    ("toprange", "series", {"a": A}),
    ("lowrange", "series", {"a": A}),
    ("macd", "default", {"a": A}),
    ("kdj", "default", {"a": A, "b": B, "c": A}),
    ("rsi", "series", {"a": A}),
    ("boll", "default", {"a": A}),
    ("ztprice", "const", {"a": [10.0, 9.99, 3.33, 1.005, 2.675], "n": 0.1}),
]

LABELS = {
    "macd": ("dif", "dea", "macd"),
    "kdj": ("k", "d", "j"),
    "boll": ("upper", "mid", "lower"),
}


def _call(name: str, args: dict):
    a = args.get("a", [])
    b = args.get("b", [])
    c = args.get("c", [])
    w = args.get("w", [])
    n = args.get("n", 0)
    m = args.get("m", 0)

    table = {
        "abs": lambda: formula.ABS(a),
        "sqrt": lambda: formula.SQRT(a),
        "ln": lambda: formula.LN(a),
        "pow": lambda: formula.POW(a, n),
        "max": lambda: formula.MAX(a, b),
        "min": lambda: formula.MIN(a, b),
        "if": lambda: formula.IF(a, b, c),
        "ref": lambda: formula.REF(a, n) if w == [] else formula.REF(a, w),
        "diff": lambda: formula.DIFF(a, n),
        "sum": lambda: formula.SUM(a, n) if w == [] else formula.SUM(a, w),
        "const": lambda: formula.CONST(a),
        "hhv": lambda: formula.HHV(a, n) if w == [] else formula.HHV(a, w),
        "llv": lambda: formula.LLV(a, n) if w == [] else formula.LLV(a, w),
        "hhvbars": lambda: formula.HHVBARS(a, n),
        "llvbars": lambda: formula.LLVBARS(a, n),
        "ma": lambda: formula.MA(a, n) if w == [] else formula.MA(a, w),
        "ema": lambda: formula.EMA(a, n),
        "sma": lambda: formula.SMA(a, n, m),
        "wma": lambda: formula.WMA(a, n),
        "std": lambda: formula.STD(a, n),
        "avedev": lambda: formula.AVEDEV(a, n),
        "slope": lambda: formula.SLOPE(a, n),
        "forcast": lambda: formula.FORCAST(a, n),
        "dma": lambda: formula.DMA(a, n) if "b" not in args else formula.DMA(a, b),
        "last": lambda: formula.LAST(a, n, m),
        "count": lambda: formula.COUNT(a, n),
        "every": lambda: formula.EVERY(a, n),
        "exist": lambda: formula.EXIST(a, n),
        "filter": lambda: formula.FILTER(a, n),
        "barslast": lambda: formula.BARSLAST(a),
        "barslastcount": lambda: formula.BARSLASTCOUNT(a),
        "barssincen": lambda: formula.BARSSINCEN(a, n),
        "cross": lambda: formula.CROSS(a, b),
        "longcross": lambda: formula.LONGCROSS(a, b, n),
        "valuewhen": lambda: formula.VALUEWHEN(a, b),
        "between": lambda: formula.BETWEEN(a, n, m),
        "toprange": lambda: formula.TOPRANGE(a),
        "lowrange": lambda: formula.LOWRANGE(a),
        "rsi": lambda: formula.RSI(a),
        "ztprice": lambda: formula.ZTPRICE(np.asarray(a, dtype=float), float(n)),
    }
    if name == "macd":
        return formula.MACD(a)
    if name == "kdj":
        return formula.KDJ(a, b, c)
    if name == "boll":
        return formula.BOLL(a)
    return table[name]()


def _text(value) -> str:
    if value is None:
        return ""
    if isinstance(value, (bool, np.bool_)):
        return "1" if value else "0"
    number = float(value)
    if math.isnan(number):
        return ""
    return repr(number)


def render_cases() -> str:
    """**渲染公式样本**: 返回 `cases.csv` 的完整文本

    :return: CSV 文本（LF 行尾）
    """
    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(["function", "case", "index", "a", "b", "c", "d", "w", "n", "m", "expected"])
    for name, case, args in CASES:
        result = _call(name, args)
        outputs = result if isinstance(result, tuple) else (result,)
        for output_index, series in enumerate(outputs):
            if len(outputs) == 1:
                label = name
            else:
                label = f"{name}.{LABELS[name][output_index]}"
            values = np.asarray(series).tolist()
            for index, expected in enumerate(values):
                writer.writerow([
                    label,
                    case,
                    index,
                    _text(args.get("a", [])[index]) if "a" in args and index < len(args["a"]) else "",
                    _text(args.get("b", [])[index]) if "b" in args and index < len(args["b"]) else "",
                    _text(args.get("c", [])[index]) if "c" in args and index < len(args["c"]) else "",
                    _text(args.get("d", [])[index]) if "d" in args and index < len(args["d"]) else "",
                    _text(args.get("w", [])[index]) if "w" in args and index < len(args["w"]) else "",
                    _text(args.get("n")) if "n" in args else "",
                    _text(args.get("m")) if "m" in args else "",
                    _text(expected),
                ])
    return buffer.getvalue()


def generate_cases(root: Path = ROOT) -> None:
    """**写出公式样本**: 覆盖 `root/cases.csv`

    :param root: 输出目录
    """
    (root / "cases.csv").write_text(render_cases(), encoding="utf-8", newline="\n")


def bars() -> pd.DataFrame:
    rows = 120
    rng = np.random.default_rng(7)
    close = 10 + np.cumsum(rng.normal(0, 0.1, rows))
    return pd.DataFrame(
        {
            "date": pd.date_range("2024-01-01", periods=rows, freq="D").strftime("%Y-%m-%d"),
            "open": np.concatenate(([close[0]], close[:-1])),
            "close": close,
            "high": close + 0.2,
            "low": close - 0.2,
            "volume": rng.integers(1000, 5000, rows).astype(float),
            "amount": rng.integers(10000, 50000, rows).astype(float),
        }
    )


def generate_indicators(root: Path = ROOT) -> None:
    """**写出指标样本**: 覆盖 `root/indicators/`

    :param root: 输出目录
    """
    directory = root / "indicators"
    directory.mkdir(parents=True, exist_ok=True)
    frame = bars()
    frame.to_csv(directory / "bars.csv", index=False, lineterminator="\n")

    results = {
        "cdtd": (cdtd(frame), ["cdtd.wen", "cdtd.S1", "cdtd.B", "cdtd.S2", "cdtd.S3"]),
        "f89k": (f89k(frame), ["f89k.zs", "f89k.b", "f89k.DN"]),
        "hjk": (hjk("SH600000", frame), ["hjk.b"]),
        "ma1x": (ma1x(frame), ["DN", "ZS", "YL", "ma1x.hc", "ma1x.tp"]),
        "p2fb": (p2fb("SH600000", frame), ["p2fb.hgzx"]),
    }
    for name, (output, columns) in results.items():
        table = pd.DataFrame({"date": frame["date"]})
        for column in columns:
            values = output[column]
            if values.dtype == bool:
                table[column] = values.astype(int)
            else:
                table[column] = values
        table.to_csv(directory / f"{name}.csv", index=False, lineterminator="\n")


def main() -> None:
    parser = argparse.ArgumentParser(description="生成 formula / indicators 对拍样本")
    parser.parse_args()
    generate_cases()
    generate_indicators()


if __name__ == "__main__":
    main()
