# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""帮助与错误版式的用例: usage 组成、章节顺序、标注、折行与着色。

运行: python -m quant1x.base.argparser.help_test
"""
from __future__ import annotations

import re

from quant1x.base.argparser import HelpRequested, Parser, UsageError

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
    """跑一次注定失败的解析, 取错误文本

    :param parser: 待测命令
    :param argv: 待解析的命令行
    :return: 错误文本
    """
    try:
        parser.parse_args(argv)
    except UsageError as exc:
        return str(exc)
    raise AssertionError(f"应当报错: argv={argv}")


def section(text: str, title: str) -> list[str]:
    """取某个章节标题之后、下一个空行之前的行

    :param text: 帮助全文
    :param title: 章节标题
    :return: 该章节正文各行; 没有该章节时为空列表
    """
    lines = text.splitlines()
    try:
        start = lines.index(title) + 1
    except ValueError:
        return []
    body: list[str] = []
    for line in lines[start:]:
        if not line.strip():
            break
        body.append(line)
    return body


# ---- usage 的组成 ------------------------------------------------------

parser = Parser("bare", color="never")
check("只有帮助选项时不写 [OPTIONS]", parser.usage_spec() == "bare", parser.usage_spec())

parser = Parser("opt", color="never")
parser.add_argument("-a", "--all", action="store_true", help="全部")
parser.add_argument("--out", required=True, help="输出")
parser.add_argument("files", nargs="+", help="文件")
check(
    "必填选项写进 usage, 其余收成 [OPTIONS]",
    parser.usage_spec() == "opt --out <OUT> [OPTIONS] <FILES>...",
    parser.usage_spec(),
)

parser = Parser("cmd", color="never")
parser.add_subparsers(required=True)
check("必填子命令写 <COMMAND>", parser.usage_spec() == "cmd <COMMAND>", parser.usage_spec())
parser = Parser("cmd2", color="never")
parser.add_subparsers()
check("非必填子命令写 [COMMAND]", parser.usage_spec() == "cmd2 [COMMAND]", parser.usage_spec())

parser = Parser("own", usage="own <自定义>", color="never")
parser.add_argument("--x", help="忽略")
check("usage 覆盖", parser.usage_spec() == "own <自定义>", parser.usage_spec())
check("format_usage 带 Usage: 前缀", parser.format_usage().startswith("Usage: own <自定义>"), parser.format_usage())

# ---- 章节顺序与内容 ----------------------------------------------------

parser = Parser(
    "tool",
    about="工具集",
    long_about="工具集的完整说明, 只在 --help 里出现。",
    epilog="示例: tool build -j 8",
    version="1.0.0",
    color="never",
)
parser.add_argument("-v", "--verbose", action="count", help="输出更详细")
parser.add_argument("--level", choices=["debug", "info", "warn"], default="info", help="日志级别")
parser.add_argument("--jobs", type=int, default=4, env="TOOL_JOBS", help="并发数")
parser.add_argument("--secret", hide=True, help="不展示")
parser.add_argument("--quiet", action="store_true", hide_default=True, help="安静")
parser.add_argument("src", nargs="*", help="源文件")
commands = parser.add_subparsers(title="Commands", dest="command")
commands.add_parser("build", about="构建", visible_aliases=["b"])
commands.add_parser("run", about="运行")

text = parser.format_help()
check("短帮助用 about", text.startswith("工具集\n"), text[:40])
check("章节顺序", [text.index(item) for item in ("Usage:", "Commands:", "Arguments:", "Options:")] ==
      sorted(text.index(item) for item in ("Usage:", "Commands:", "Arguments:", "Options:")), text)
check("epilog 在最后", text.rstrip().endswith("示例: tool build -j 8"), text[-60:])
check("隐藏参数不出现", "--secret" not in text, text)
check("count 动作带省略号", "-v, --verbose..." in text, text)
check("默认值标注", "[default: info]" in text, text)
check("env 标注", "[env: TOOL_JOBS]" in text, text)
check("可选值标注", "[possible values: debug, info, warn]" in text, text)
check("开关不标默认值", "安静" in text and "[default: false]" not in text, text)
check("版本选项在帮助里", "-V, --version" in text, text)
check("帮助选项在帮助里", "-h, --help" in text, text)

long_text = parser.format_help(long=True)
check("长帮助用 long_about", long_text.startswith("工具集的完整说明"), long_text[:40])
check("长帮助不重复短 about", long_text.count("工具集") == 1, long_text[:80])

rows = section(text, "Options:")
check("选项行数 (help/version 排最后)", len(rows) == 6, rows)
check("help/version 是最后两行", "Print help" in rows[-2] and "Print version" in rows[-1], rows)
check("隐藏默认值", "[default: true]" not in text, text)

rows = section(text, "Commands:")
check("子命令展示别名", rows[0].strip().startswith("build, b") and rows[1].strip().startswith("run"), rows)
check("Commands 末尾自动列 help", rows[-1].strip().startswith("help") and "Print this message" in rows[-1], rows)
rows = section(text, "Arguments:")
check("位置参数章节", bool(rows) and rows[0].strip().startswith("[SRC]..."), rows)

# ---- 长帮助里的可选值说明 ----------------------------------------------

parser = Parser("level", color="never")
parser.add_argument("--level", choices={"debug": "逐条打印", "info": "常规"}, default="info", help="级别",
                    long_help="日志级别, 影响输出量")
short_text = parser.format_help()
long_text = parser.format_help(long=True)
check("短帮助用 help", "影响输出量" not in short_text and "级别" in short_text, short_text)
check("长帮助用 long_help", "影响输出量" in long_text, long_text)
check("长帮助列可选值说明", "- debug: 逐条打印" in long_text, long_text)

# ---- 分组 --------------------------------------------------------------

parser = Parser("grouped", color="never")
group = parser.add_group("Database", "数据库相关")
group.add_argument("--host", default="localhost", help="主机")
group.add_argument("--port", type=int, default=5432, help="端口")
parser.add_argument("--debug", action="store_true", help="调试")
text = parser.format_help()
check("分组标题出现", "Database:" in text, text)
check("分组描述出现", "数据库相关" in text, text)
check("分组内参数归组", "Database:" in text and text.index("--host") > text.index("Database:"), text)
check("未分组参数留在 Options", text.index("--debug") < text.index("Database:"), text)

# ---- 对齐与折行 --------------------------------------------------------

parser = Parser("align", color="never")
parser.add_argument("-a", "--all", help="全部")
parser.add_argument("--beta", metavar="BETA", help="乙")
text = parser.format_help()
rows = section(text, "Options:")
first, second = rows[0], rows[1]
check("两行说明同列对齐", first.index("全部") == second.index("乙"), rows)
check("左列两空格缩进", first.startswith("  -a, --all"), rows)

parser = Parser("wrap", color="never", max_width=44)
parser.add_argument("--long", metavar="VALUE", help="这是一段足够长的说明文字, 必须在窄幅下折行并保持对齐缩进")
text = parser.format_help()
option_rows = section(text, "Options:")
check("窄幅折行出多行", len(option_rows) >= 2, option_rows)
continuation = [line for line in option_rows[1:] if line.strip()]
check("折行按说明列缩进 (2+14+2)", bool(continuation) and continuation[0].startswith(" " * 18), option_rows)
check("折行不超过 max_width", all(len(line) <= 44 for line in text.splitlines()), text)

# ---- 子命令帮助继承全局参数 --------------------------------------------

parser = Parser("tool", color="never")
parser.add_argument("-q", "--quiet", action="store_true", global_=True, help="安静模式")
commands = parser.add_subparsers(dest="command")
build = commands.add_parser("build", about="构建")
build.add_argument("-j", "--jobs", type=int, default=4, help="并发数")
child = build.format_help()
check("子命令 prog 带父命令", "Usage: tool build [OPTIONS]" in child, child)
check("全局参数下沉到子命令", "-q, --quiet" in child, child)
check("子命令自己的参数在", "-j, --jobs" in child, child)

nested = Parser("nested", color="never")
nested_commands = nested.add_subparsers(dest="command")
nested_build = nested_commands.add_parser("build", about="构建")
nested_build.add_subparsers(dest="target").add_parser("release", about="发布版")
nested_text = nested_build.format_help()
check("子命令帮助也列 help", "Print this message" in nested_text, nested_text)

try:
    parser.parse_args(["build", "--help"])
except HelpRequested as exc:
    check("子命令 --help 走长帮助", exc.long is True and exc.parser is not None and exc.parser.prog == "tool build")
else:
    check("子命令 --help 走长帮助", False)

try:
    parser.parse_args(["-h"])
except HelpRequested as exc:
    check("-h 走短帮助", exc.long is False and exc.parser is parser)
else:
    check("-h 走短帮助", False)

# ---- 错误版式 ----------------------------------------------------------

parser = Parser("demo", about="演示", color="never")
parser.add_argument("--level", choices=["debug", "info", "warn"], help="级别")
text = failure(parser, ["--level", "warnn"])
check("错误首行带 error:", text.startswith("error: invalid value 'warnn' for '--level <LEVEL>'"), text)
check("细节行缩进两格", "\n  [possible values: debug, info, warn]" in text, text)
check("建议行带 tip:", "\n  tip: a similar value exists: 'warn'" in text, text)
check("错误里有 usage", "\nUsage: demo [OPTIONS]" in text, text)
check("错误末尾给帮助提示", text.endswith("For more information, try '--help'."), text)

parser = Parser("nohelp", add_help=False, color="never")
parser.add_argument("--x", help="甲")
text = failure(parser, ["--y"])
check("无帮助选项时不提 --help", "For more information" not in text, text)

error = UsageError("自定义", reason="原因", lines=["明细"], tips=["建议"], usage="demo")
check("UsageError 可直接构造", str(error).startswith("error: 自定义: 原因"), str(error))

# ---- 着色 --------------------------------------------------------------

parser = Parser("color", color="always")
parser.add_argument("--x", help="甲")
check("color=always 有着色", "\x1b[" in parser.format_help(), parser.format_help())
parser = Parser("color", color="never")
parser.add_argument("--x", help="甲")
check("color=never 无着色", "\x1b[" not in parser.format_help())
check("错误着色可关闭", "\x1b[" not in failure(parser, ["--y"]))
ansi = re.compile(r"\x1b\[[0-9;]*m")


def plain(text: str) -> str:
    """去掉 ANSI 转义, 便于比较可见宽度

    :param text: 可能带转义的文本
    :return: 剥掉转义后的文本
    """
    return ansi.sub("", text)


colored = Parser("color", color="always")
colored.add_argument("--x", help="甲")
mono = Parser("color", color="never")
mono.add_argument("--x", help="甲")
check("着色不影响版式", plain(colored.format_help()) == mono.format_help())

print()
if _failures:
    print(f"FAILED: {', '.join(_failures)}")
    raise SystemExit(1)
print("all checks passed")
