# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""`Parser` 的用例: 选项 / 位置参数取值、语法形态、校验与子命令。

运行: python -m quant1x.base.argparser.parser_test
"""

import contextlib
import io
import os
import tempfile

from quant1x.base.argparser import (
    REMAINDER,
    SUPPRESS,
    Argument,
    ErrorKind,
    HelpRequested,
    Namespace,
    Parser,
    UsageError,
    ValueParser,
    parse_bool,
    parse_int,
)

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
    :return: 错误文本, 含 usage 与提示
    """
    try:
        parser.parse_args(argv)
    except UsageError as exc:
        return str(exc)
    raise AssertionError(f"应当报错: argv={argv}")


def _base() -> Parser:
    """建本节复用的命令: 计数 / 必填取值 / 可选值 / 位置参数

    :return: 建好的解析器
    """
    parser = Parser("demo", about="演示命令行", color="never")
    parser.add_argument("-v", "--verbose", action="count", help="输出更详细")
    parser.add_argument("-o", "--output", required=True, help="输出文件")
    parser.add_argument("--level", choices=["debug", "info", "warn"], default="info", help="日志级别")
    parser.add_argument("files", nargs="+", help="输入文件")
    return parser


def _tool() -> Parser:
    """建两级子命令: `tool` → `build` / `run`, `build` → `release` / `debug`

    :return: 建好的解析器
    """
    parser = Parser("tool", about="工具集", version="9.9.9", color="never")
    parser.add_argument("-q", "--quiet", action="store_true", global_=True, help="安静模式")
    commands = parser.add_subparsers(dest="command", required=True)
    build = commands.add_parser("build", about="构建", visible_aliases=["b"])
    build.add_argument("-j", "--jobs", type=int, default=4, help="并发数")
    targets = build.add_subparsers(dest="target", title="Targets")
    targets.add_parser("release", about="发布版")
    targets.add_parser("debug", about="调试版")
    run = commands.add_parser("run", about="运行", aliases=["r"])
    run.add_argument("script", help="脚本")
    return parser


# ---- 选项与取值 --------------------------------------------------------

namespace = _base().parse_args(["-o", "out.txt", "-vv", "--level=warn", "a.txt", "b.txt"])
check(
    "长短选项与 = 取值",
    namespace.output == "out.txt" and namespace.verbose == 2 and namespace.level == "warn",
    namespace,
)
check("位置参数收成列表", namespace.files == ["a.txt", "b.txt"], namespace.files)
check("下标读取与 contains", namespace["level"] == "warn" and "files" in namespace)
check("as_dict", namespace.as_dict()["output"] == "out.txt")

namespace = _base().parse_args(["-oout.txt", "a.txt"])
check("短选项粘值", namespace.output == "out.txt" and namespace.level == "info", namespace)
check("未出现的开关落隐式缺省", namespace.verbose == 0, namespace)

namespace = _base().parse_args(["--output", "out.txt", "a.txt"])
check("长选项带空格取值", namespace.output == "out.txt")

text = failure(_base(), ["a.txt"])
check("缺必填选项", "the following required arguments were not provided" in text and "--output <OUTPUT>" in text, text)
check("错误里带 usage", "Usage: demo --output <OUTPUT> [OPTIONS] <FILES>..." in text, text)
check("错误里带帮助提示", "For more information, try '--help'." in text, text)

text = failure(_base(), ["-o"])
check("选项缺值", "a value is required for '--output <OUTPUT>' but none was supplied" in text, text)

text = failure(_base(), ["-o", "out.txt", "--level", "warnn", "a.txt"])
check("可选值非法", "invalid value 'warnn' for '--level <LEVEL>'" in text, text)
check("可选值清单", "[possible values: debug, info, warn]" in text, text)
check("可选值给建议", "a similar value exists: 'warn'" in text, text)

text = failure(_base(), ["-o", "out.txt", "--verbsoe"])
check("未知选项 (拼错且非前缀)", "unexpected argument '--verbsoe' found" in text, text)
check("未知选项给建议", "a similar argument exists: '--verbose'" in text, text)
check("长选项前缀缩写照常解析", _base().parse_args(["-o", "x", "--verbos", "a.txt"]).verbose == 1)

text = failure(_base(), ["-o", "out.txt", "--zzz"])
check("无从建议的长选项", "to pass '--zzz' as a value, use '-- --zzz'" in text, text)

text = failure(_base(), ["-o", "out.txt"])
check("缺必填位置参数", "<FILES>" in text, text)

# ---- 位置参数与数量形态 ------------------------------------------------

parser = Parser("pos", color="never")
parser.add_argument("src", nargs="*", help="源")
parser.add_argument("dst", help="目标")
namespace = parser.parse_args(["a", "b", "c"])
check("变长位置参数给后面的固定项留够", namespace.src == ["a", "b"] and namespace.dst == "c", namespace)
namespace = parser.parse_args(["c"])
check("变长位置参数为空", namespace.src == [] and namespace.dst == "c", namespace)

parser = Parser("rem", color="never")
parser.add_argument("--profile", default="dev", help="档案")
parser.add_argument("argv", nargs=REMAINDER, help="余下原样")
namespace = parser.parse_args(["--profile", "prod", "run", "--x", "1"])
check("REMAINDER 吃下余下全部", namespace.argv == ["run", "--x", "1"] and namespace.profile == "prod", namespace)

parser = Parser("pair", color="never")
parser.add_argument("--pair", nargs=2, type=int, help="两个整数")
namespace = parser.parse_args(["--pair", "1", "2"])
check("nargs=2 转列表", namespace.pair == [1, 2], namespace)
text = failure(parser, ["--pair", "1"])
check("nargs=2 不足", "2 values required for '--pair <PAIR>' but 1 was provided" in text, text)

parser = Parser("maybe", color="never")
parser.add_argument("--opt", nargs="?", const="CONST", default="DEFAULT", help="可选")
check("nargs=? 缺省", parser.parse_args([]).opt == "DEFAULT")
check("nargs=? 裸写取 const", parser.parse_args(["--opt"]).opt == "CONST")
check("nargs=? 给值", parser.parse_args(["--opt", "v"]).opt == "v")

parser = Parser("many", color="never")
parser.add_argument("--many", nargs="+", type=int, help="多个")
namespace = parser.parse_args(["--many", "1", "2", "3"])
check("nargs=+ 收到选项为止", namespace.many == [1, 2, 3], namespace)
text = failure(parser, ["--many"])
check("nargs=+ 至少一个", "a value is required for '--many <MANY>'" in text, text)

# ---- 取值动作 ----------------------------------------------------------

parser = Parser("actions", color="never")
parser.add_argument("-v", action="count", help="计数")
parser.add_argument("--on", action="store_true", help="开关")
parser.add_argument("--off", action="store_false", help="反开关")
parser.add_argument("--mode", action="store_const", const="fast", default="slow", help="常量")
parser.add_argument("-D", action="append", help="追加")
parser.add_argument("--tag", action="append_const", const="T", help="追加常量")
parser.add_argument("-I", action="extend", nargs="+", help="展开")
namespace = parser.parse_args(["-vvv", "--on", "--off", "--mode", "-D", "A", "-D", "B", "--tag", "-I", "x", "y"])
check("count", namespace.v == 3, namespace.v)
check("store_true", namespace.on is True)
check("store_false", namespace.off is False)
check("store_const", namespace.mode == "fast", namespace.mode)
check("append", namespace.D == ["A", "B"], namespace.D)
check("append_const", namespace.tag == ["T"], namespace.tag)
check("extend", namespace.I == ["x", "y"], namespace.I)
defaults = parser.parse_args([])
check("隐式缺省: count / store_true / store_false", defaults.v == 0 and defaults.on is False and defaults.off is True)

parser = Parser("delim", color="never")
parser.add_argument("--ids", value_delimiter=",", type=int, help="逗号分隔")
parser.add_argument("--raw", action="extend", nargs="+", value_delimiter=",", help="展开并分隔")
namespace = parser.parse_args(["--ids", "1,2,3", "--raw", "a,b", "--raw", "c"])
check("value_delimiter 拆成列表", namespace.ids == [1, 2, 3], namespace.ids)
check("value_delimiter 与 extend", namespace.raw == ["a", "b", "c"], namespace.raw)

# ---- 语法形态 ----------------------------------------------------------

parser = Parser("syntax", color="never")
parser.add_argument("-a", action="store_true", help="甲")
parser.add_argument("-b", action="store_true", help="乙")
parser.add_argument("-c", help="丙")
parser.add_argument("-n", "--number", type=int, help="整数")
parser.add_argument("rest", nargs="*", help="余下")
namespace = parser.parse_args(["-abc", "v"])
check("短选项聚合 (末位带值)", namespace.a and namespace.b and namespace.c == "v", namespace)
namespace = parser.parse_args(["-ab", "-cv"])
check("短选项聚合 (粘值)", namespace.a and namespace.b and namespace.c == "v", namespace)
text = failure(parser, ["-abz"])
check("聚合里未认出的短选项单独报", "unexpected argument '-z' found" in text, text)
namespace = parser.parse_args(["-n", "-5"])
check("签名数字当取值", namespace.number == -5, namespace)
namespace = parser.parse_args(["-n", "3", "-7"])
check("签名数字当位置参数", namespace.number == 3 and namespace.rest == ["-7"], namespace)
namespace = parser.parse_args(["-1", "+1", "-1.5", "+.5"])
check("签名数字整体当一个取值", namespace.rest == ["-1", "+1", "-1.5", "+.5"], namespace)
namespace = parser.parse_args(["--", "-a", "-b"])
check("-- 之后全是位置参数", namespace.a is False and namespace.rest == ["-a", "-b"], namespace)
namespace = parser.parse_args(["-"])
check("单个 - 是位置参数", namespace.rest == ["-"], namespace)
namespace = parser.parse_args(["-c="])
check("等号后可以是空串", namespace.c == "", namespace)

parser = Parser("hyphen", color="never")
parser.add_argument("--cmd", allow_hyphen_values=True, help="命令")
namespace = parser.parse_args(["--cmd", "-x"])
check("allow_hyphen_values", namespace.cmd == "-x", namespace)

parser = Parser("abbr", color="never")
parser.add_argument("--output", help="输出")
parser.add_argument("--outdir", help="目录")
namespace = parser.parse_args(["--outp", "x"])
check("长选项唯一前缀", namespace.output == "x", namespace)
text = failure(parser, ["--out", "x"])
check("长选项前缀歧义", "ambiguous argument '--out'" in text and "--outdir" in text, text)

parser = Parser("noabbr", allow_abbrev=False, color="never")
parser.add_argument("--output", help="输出")
text = failure(parser, ["--outp", "x"])
check("关掉前缀匹配", "unexpected argument '--outp'" in text, text)

# ---- 校验: 必填 / 互斥 / 冲突 / 依赖 -----------------------------------

parser = Parser("rules", color="never")
group = parser.add_mutually_exclusive_group(required=True)
group.add_argument("--json", action="store_true", help="JSON 输出")
group.add_argument("--yaml", action="store_true", help="YAML 输出")
parser.add_argument("--fast", action="store_true", conflicts_with=["json"], help="快")
parser.add_argument("--cache", action="store_true", requires=["fast"], help="缓存")
check("互斥组二选一", parser.parse_args(["--json"]).json is True)
text = failure(parser, ["--json", "--yaml"])
check("互斥组只准一个", "the argument '--json' cannot be used with '--yaml'" in text, text)
text = failure(parser, [])
check("互斥组必选其一", "one of the following arguments is required" in text and "--yaml" in text, text)
text = failure(parser, ["--json", "--fast"])
check("conflicts_with", "the argument '--fast' cannot be used with '--json'" in text, text)
text = failure(parser, ["--yaml", "--cache"])
check("requires", "the argument '--cache' requires '--fast'" in text, text)

parser = Parser("badref", color="never")
parser.add_argument("--a", action="store_true", conflicts_with=["missing"], help="甲")
try:
    parser.parse_args(["--a"])
except ValueError as exc:
    check("冲突引用未登记参数报 ValueError", "missing" in str(exc), exc)
else:
    check("冲突引用未登记参数报 ValueError", False)

# ---- 覆盖: overrides_with ----------------------------------------------

over = Parser("over", color="never")
over.add_argument("--a", default="A", help="甲")
over.add_argument("--b", default="B", overrides_with=["a"], help="乙")
check("后出现的留下", over.parse_args(["--a", "1", "--b", "2"]).as_dict()["b"] == "2")
check("被覆盖方退回默认值", over.parse_args(["--a", "1", "--b", "2"]).as_dict()["a"] == "A")
check("覆盖是双向的", over.parse_args(["--b", "2", "--a", "1"]).as_dict()["a"] == "1")
check("反向时先出现的退回默认值", over.parse_args(["--b", "2", "--a", "1"]).as_dict()["b"] == "B")
check("重复出现仍是最后那个赢", over.parse_args(["--a", "1", "--b", "2", "--a", "3"]).as_dict()["a"] == "3")

one_way = Parser("one", color="never")
one_way.add_argument("--x", default="X", help="甲")
one_way.add_argument("--y", default="Y", overrides_with=["x"], help="乙")
check("单侧声明双向生效", one_way.parse_args(["--x", "1", "--y", "2"]).as_dict()["y"] == "2")
check("单侧声明的反向也生效", one_way.parse_args(["--y", "2", "--x", "1"]).as_dict()["x"] == "1")

bad = Parser("bad", color="never")
bad.add_argument("--a", overrides_with=["nope"], help="甲")
try:
    bad.parse_args(["--a", "1"])
except ValueError as exc:
    check("overrides_with 指向未登记参数报定义期错误", "未登记的参数" in str(exc), str(exc))
else:
    check("overrides_with 指向未登记参数报定义期错误", False)

# ---- 环境变量兜底 ------------------------------------------------------

os.environ["ARGPARSER_LEVEL"] = "warn"
os.environ["ARGPARSER_DRY"] = "yes"
try:
    parser = Parser("env", color="never")
    parser.add_argument("--level", choices=["debug", "info", "warn"], env="ARGPARSER_LEVEL", help="级别")
    parser.add_argument("--dry-run", action="store_true", env="ARGPARSER_DRY", help="演练")
    namespace = parser.parse_args([])
    check("env 兜底取值", namespace.level == "warn", namespace)
    check("env 布尔开关", namespace.dry_run is True, namespace)
    namespace = parser.parse_args(["--level", "info"])
    check("命令行优先于 env", namespace.level == "info", namespace)

    parser = Parser("envbad", color="never")
    parser.add_argument("--level", choices=["debug", "info", "warn"], env="ARGPARSER_LEVEL", help="级别")
    os.environ["ARGPARSER_LEVEL"] = "verbose"
    text = failure(parser, [])
    check("env 取值非法也报错", "invalid value 'verbose' for '--level <LEVEL>'" in text, text)
finally:
    os.environ.pop("ARGPARSER_LEVEL", None)
    os.environ.pop("ARGPARSER_DRY", None)

# ---- 子命令 ------------------------------------------------------------

parser = _tool()
namespace = parser.parse_args(["-q", "build", "-j", "8", "release"])
check(
    "子命令与全局参数",
    namespace.quiet is True and namespace.command == "build" and namespace.jobs == 8 and namespace.target == "release",
    namespace,
)
namespace = parser.parse_args(["run", "s.py", "-q"])
check("全局参数可写在子命令之后", namespace.quiet is True and namespace.script == "s.py", namespace)
namespace = parser.parse_args(["b"])
check("展示别名照常解析", namespace.command == "build" and namespace.jobs == 4, namespace)
namespace = parser.parse_args(["r", "x.py"])
check("隐藏别名照常解析", namespace.command == "run" and namespace.script == "x.py", namespace)
text = failure(parser, [])
check("子命令必填", "<COMMAND>" in text, text)
text = failure(parser, ["build", "--job-x", "8"])
check("子命令里未知选项", "unexpected argument '--job-x' found" in text, text)

parser = Parser("negates", subcommand_negates_reqs=True, color="never")
parser.add_argument("--out", required=True, help="输出")
negated = parser.add_subparsers(dest="cmd", required=True)
negated.add_parser("run")
check("subcommand_negates_reqs 放行父命令必填项", parser.parse_args(["run"]).cmd == "run")

commands = _tool().subparsers
assert commands is not None
check("子命令按注册顺序", [name for name, _, _ in commands.commands()] == ["build", "run"])
check("展示别名只在 visible_aliases 里", commands.commands()[0][1] == ("b",) and commands.commands()[1][1] == ())
check("别名解析回正名", commands.canonical("r") == "run")
build_parser = commands.lookup("build")
check("子命令的 prog 带父命令", build_parser is not None and build_parser.prog == "tool build")
check("子命令沿用父命令版本", build_parser is not None and build_parser.version == "9.9.9")

# ---- 自动生成的 help 子命令 --------------------------------------------


def help_target(parser: Parser, argv: list[str]) -> tuple[bool, str | None]:
    """跑一次 `help ...`, 取是否走长帮助与目标命令的 prog

    :param parser: 待测命令
    :param argv: 待解析的命令行
    :return: (是否走长帮助, 目标命令的 prog); 没抛请求时为 `(False, None)`
    """
    try:
        parser.parse_args(argv)
    except HelpRequested as exc:
        return exc.long, exc.parser.prog if exc.parser is not None else None
    return False, None


parser = _tool()
check("help 子命令打印本命令帮助", help_target(parser, ["help"]) == (True, "tool"), help_target(parser, ["help"]))
check(
    "help 子命令下沉一级",
    help_target(parser, ["help", "build"]) == (True, "tool build"),
    help_target(parser, ["help", "build"]),
)
check(
    "help 子命令下沉两级",
    help_target(parser, ["help", "build", "release"]) == (True, "tool build release"),
    help_target(parser, ["help", "build", "release"]),
)
text = failure(parser, ["help", "nosuch"])
check("help 指向未知子命令", "unrecognized subcommand 'nosuch'" in text, text)

out = io.StringIO()
code = 0
with contextlib.redirect_stdout(out):
    try:
        parser.parse_or_exit(["help", "build"])
    except SystemExit as exc:
        code = exc.code if isinstance(exc.code, int) else 1
check("help 子命令退 0 并打印目标帮助", code == 0 and "Usage: tool build" in out.getvalue(), out.getvalue())

parser = Parser("ownhelp", color="never")
own = parser.add_subparsers(dest="cmd")
own.add_parser("help", about="自定义帮助")
check("自定义 help 子命令不被顶掉", parser.parse_args(["help"]).cmd == "help")

parser = Parser("poshelp", color="never")
parser.add_argument("first", help="第一个")
pos = parser.add_subparsers(dest="cmd")
pos.add_parser("build")
namespace = parser.parse_args(["help", "build"])
check("位置参数没填够时 help 当位置参数", namespace.first == "help" and namespace.cmd == "build", namespace)

# ---- 已知参数与多余参数 ------------------------------------------------

parser = Parser("known", color="never")
parser.add_argument("--flag", action="store_true", help="开关")
parser.add_argument("files", nargs="*", help="文件")
namespace, extras = parser.parse_known_args(["--flag", "a", "b"])
check("parse_known_args 无多余", namespace.files == ["a", "b"] and extras == [], extras)

parser = Parser("surplus", color="never")
parser.add_argument("first", help="第一个")
namespace, extras = parser.parse_known_args(["a", "b", "c"])
check("多余位置参数进 extras", namespace.first == "a" and extras == ["b", "c"], extras)
text = failure(parser, ["a", "b"])
check("parse_args 拒绝多余位置参数", "unexpected argument 'b' found" in text, text)

parser = Parser("tolerant", color="never")
parser.add_argument("--flag", action="store_true", help="开关")
parser.add_argument("first", help="第一个")
namespace, extras = parser.parse_known_args(["--nope", "a", "--flag"])
check(
    "未知选项只收自身不吞取值",
    extras == ["--nope"] and namespace.first == "a" and namespace.flag is True,
    (namespace, extras),
)
namespace, extras = parser.parse_known_args(["a", "b", "--nope"])
check("extras 按命令行顺序", extras == ["b", "--nope"], extras)
namespace, extras = parser.parse_known_args(["-xval", "a"])
check("未知短选项粘值整体收下", extras == ["-xval"], extras)
namespace, extras = parser.parse_known_args(["--nope=1", "a"])
check("未知长选项带等号整体收下", extras == ["--nope=1"], extras)
namespace, extras = Parser("nofiles", color="never").parse_known_args(["--nope", "value"])
check("没有位置参数可接时取值也不丢", extras == ["--nope", "value"], extras)
text = failure(parser, ["a", "--nope"])
check("parse_args 仍严格报未知选项", "unexpected argument '--nope' found" in text, text)
check("严格模式保留相似项建议", "a similar argument exists: '--flag'" in failure(parser, ["a", "--flah"]), "无建议")

parser = Parser("cross", color="never")
parser.add_argument("first", help="第一个")
parser.add_subparsers(dest="cmd").add_parser("build")
namespace, extras = parser.parse_known_args(["a", "b", "build", "c"])
check("跨层 extras 仍按命令行顺序", namespace.first == "a" and extras == ["b", "c"], extras)

# ---- SUPPRESS 与 argument_default -------------------------------------

parser = Parser("suppress", color="never")
parser.add_argument("--gone", default=SUPPRESS, help="不出现")
parser.add_argument("--kept", default=None, help="出现")
namespace = parser.parse_args([])
check("SUPPRESS 不写命名空间", "gone" not in namespace and "kept" in namespace, namespace)

parser = Parser("defaults", argument_default=SUPPRESS, color="never")
parser.add_argument("--x", help="不出现")
namespace = parser.parse_args([])
check("argument_default=SUPPRESS", "x" not in namespace, namespace)

# ---- 类型转换 ----------------------------------------------------------

parser = Parser("types", color="never")
parser.add_argument("--count", type=parse_int(1, 10), help="1..10")
parser.add_argument("--ratio", type=ValueParser.of(float, "FLOAT"), help="比例")
parser.add_argument("--flag", type=parse_bool(), help="布尔")
check("闭区间内", parser.parse_args(["--count", "5"]).count == 5)
text = failure(parser, ["--count", "11"])
check("越界报区间", "11 is not in 1..=10" in text, text)
text = failure(parser, ["--count", "abc"])
check("非法整数文案", "invalid digit found in string" in text, text)
check("浮点转换", parser.parse_args(["--ratio", "0.5"]).ratio == 0.5)
check("布尔转换 (大小写不敏感)", parser.parse_args(["--flag", "YES"]).flag is True)

parser = Parser("infer", color="never")
parser.add_argument("--port", type=int, default=8080, help="端口")
check("type 可传内置类型", parser.parse_args(["--port", "9000"]).port == 9000)

parser = Parser("deduce", color="never")
parser.add_argument("--port", default=8080, help="端口")
check("按默认值类型推断转换器", parser.parse_args(["--port", "9000"]).port == 9000)

# ---- 错误类别 ----------------------------------------------------------


def kind_of(parser: Parser, argv: list[str]) -> ErrorKind | None:
    """取一次失败解析的错误类别

    :param parser: 待测命令
    :param argv: 待解析的命令行
    :return: 错误类别; 没报错时为 `None`
    """
    try:
        parser.parse_args(argv)
    except UsageError as exc:
        return exc.kind
    return None


parser = Parser("kinds", color="never")
parser.add_argument("--output", required=True, help="输出")
parser.add_argument("--outdir", help="目录")
parser.add_argument("--level", choices=["debug", "info"], help="级别")
check("类别: 未知参数", kind_of(parser, ["--nope"]) is ErrorKind.UNKNOWN_ARGUMENT)
check("类别: 前缀歧义", kind_of(parser, ["--out"]) is ErrorKind.AMBIGUOUS_ARGUMENT)
check("类别: 缺必填", kind_of(parser, []) is ErrorKind.MISSING_REQUIRED)
check("类别: 取值非法", kind_of(parser, ["--output", "x", "--level", "vv"]) is ErrorKind.INVALID_VALUE)
check("类别: 缺取值", kind_of(parser, ["--output"]) is ErrorKind.MISSING_VALUE)
parser = Parser("kindc", color="never")
parser.add_argument("--a", action="store_true", help="甲")
parser.add_argument("--b", action="store_true", conflicts_with=["a"], help="乙")
check("类别: 冲突", kind_of(parser, ["--a", "--b"]) is ErrorKind.CONFLICT)
check("类别: 手搓错误默认 OTHER", UsageError("自定义").kind is ErrorKind.OTHER)
check("动作名兼容 clap 拼法", Parser("c", color="never").add_argument("--x", action="SetTrue").action.takes_values()
      is False)

# ---- 退出码与输出 ------------------------------------------------------

parser = Parser("exit", about="演示", version="1.2.3", color="never")
parser.add_argument("--n", type=int, default=1, help="数量")
for argv in (["--version"], ["-h"], ["--n", "x"]):
    expected_code = 2 if "x" in argv else 0
    out, err = io.StringIO(), io.StringIO()
    code = 0
    with contextlib.redirect_stdout(out):
        try:
            parser.parse_or_exit(argv, stderr=err)
        except SystemExit as exc:
            code = exc.code if isinstance(exc.code, int) else 1
    check(f"退出码 {argv}", code == expected_code, code)
check("版本行是 prog + version", parser.version_text() == "exit 1.2.3", parser.version_text())
out = io.StringIO()
with contextlib.redirect_stdout(out):
    try:
        parser.parse_or_exit(["-h"])
    except SystemExit:
        pass
check("parse_or_exit 打印帮助", "Usage: exit [OPTIONS]" in out.getvalue(), out.getvalue())
err = io.StringIO()
try:
    parser.parse_or_exit(["--n", "x"], stderr=err)
except SystemExit:
    pass
check("parse_or_exit 把错误写到 stderr", str(err.getvalue()).startswith("error: invalid value"), err.getvalue())

# ---- @file 展开 --------------------------------------------------------


def _files_parser(prefix: str | None = "@") -> Parser:
    """建吃 `@file` 的命令: 一个选项加一组位置参数

    :param prefix: `@file` 展开的前缀字符集; `None` 表示不展开
    :return: 建好的解析器
    """
    parser = Parser("files", color="never", fromfile_prefix_chars=prefix)
    parser.add_argument("-o", "--output", help="输出文件")
    parser.add_argument("files", nargs="*", help="输入文件")
    return parser


def _write(path: str, body: str) -> str:
    """按 UTF-8 写一段文本

    :param path: 目标文件路径
    :param body: 要写入的内容
    :return: 写好的路径
    """
    with open(path, "w", encoding="utf-8") as handle:
        handle.write(body)
    return path


with tempfile.TemporaryDirectory() as tmp:
    no_prefix = _files_parser(None)
    check("@file 默认不展开", no_prefix.parse_args(["@a.txt"]).as_dict()["files"] == ["@a.txt"])

    flat = _files_parser()
    flat_path = _write(os.path.join(tmp, "flat.txt"), "--output\nout.txt\nx.txt\n")
    namespace = flat.parse_args([f"@{flat_path}"]).as_dict()
    check("@file 每行一个 token", namespace["output"] == "out.txt" and namespace["files"] == ["x.txt"], namespace)

    spaced_path = _write(os.path.join(tmp, "spaced.txt"), "join me\n")
    check("@file 行内不拆词", flat.parse_args([f"@{spaced_path}"]).as_dict()["files"] == ["join me"])

    inner_path = _write(os.path.join(tmp, "inner.txt"), "--output\ninner.txt\nb.txt\n")
    outer_path = _write(os.path.join(tmp, "outer.txt"), f"@{inner_path}\n")
    nested = flat.parse_args([f"@{outer_path}"]).as_dict()
    check("@file 嵌套继续展开", nested["output"] == "inner.txt" and nested["files"] == ["b.txt"], nested)

    known, extras = flat.parse_known_args([f"@{inner_path}", "--nope"])
    check("parse_known_args 同样展开 @file", known.as_dict()["output"] == "inner.txt" and extras == ["--nope"], extras)

    blank_path = _write(os.path.join(tmp, "blank.txt"), "a.txt\n\nb.txt\n")
    check("@file 空行是一个空 token", flat.parse_args([f"@{blank_path}"]).as_dict()["files"] == ["a.txt", "", "b.txt"])

    missing = os.path.join(tmp, "nope.txt")
    check("@file 打不开按用法错误报", failure(flat, [f"@{missing}"]).startswith(f"error: unable to open file '{missing}'"))
    check("@file 打不开的类别是 OTHER", kind_of(flat, [f"@{missing}"]) is ErrorKind.OTHER)

    loop_path = os.path.join(tmp, "loop.txt")
    _write(loop_path, f"@{loop_path}\n")
    check("@file 自引用按嵌套过深报", "nesting too deep" in failure(flat, [f"@{loop_path}"]))

# ---- 宽度: auto_width / COLUMNS ----------------------------------------

narrow = Parser("demo", color="never", max_width=44)
narrow.add_argument("--name", help="名字")
check("默认不读 COLUMNS", narrow.width() == 44)
os.environ["COLUMNS"] = "60"
check("未开 auto_width 时 COLUMNS 不生效", narrow.width() == 44)

auto = Parser("demo", color="never", max_width=44, auto_width=True)
auto.add_argument("--name", help="名字")
check("开了 auto_width 后读 COLUMNS", auto.width() == 60)
os.environ["COLUMNS"] = "abc"
check("COLUMNS 非法时退回 max_width", auto.width() == 44)
os.environ["COLUMNS"] = "0"
check("COLUMNS 非正时退回 max_width", auto.width() == 44)
os.environ.pop("COLUMNS", None)
check("COLUMNS 缺省时退回 max_width", auto.width() == 44)

os.environ["COLUMNS"] = "44"
check("auto_width 影响帮助折行", auto.format_help(False) == narrow.format_help(False))
os.environ.pop("COLUMNS", None)

# ---- 必选组与 required_unless_present ----------------------------------

grouped = Parser("grouped", color="never")
inputs = grouped.add_value_group("input", required=True)
inputs.add_argument("--file", help="文件")
inputs.add_argument("--dir", help="目录")
check("多值组: 出现一个即可", grouped.parse_args(["--file", "x"]).as_dict()["file"] == "x")
check("多值组: 成员可以同时出现", grouped.parse_args(["--file", "x", "--dir", "y"]).as_dict()["dir"] == "y")
check("多值组: 一个都不出现报缺", "<input>" in failure(grouped, []) and
      "required arguments were not provided" in failure(grouped, []))

loose = Parser("loose", color="never")
optional_group = loose.add_value_group("maybe")
optional_group.add_argument("--a")
check("多值组: 非必选时空着也不报错", loose.parse_args([]).as_dict() == {"a": None})

gated = Parser("gated", color="never")
gated.add_argument("--token", required_unless_present=("user",))
gated.add_argument("--user")
gated.add_argument("--key", overrides_with=("user",))
check("required_unless_present: 条件满足则不报缺", gated.parse_args(["--user", "u"]).as_dict()["user"] == "u")
check("required_unless_present: 条件不满足报缺",
      "required arguments were not provided" in failure(gated, []))
check("覆盖先于必填判定: 被覆盖掉的不再算出现过",
      "required arguments were not provided" in failure(gated, ["--user", "u", "--key", "k"]))

# ---- 定义期校验 --------------------------------------------------------


def _dup_option() -> None:
    """建一个重复登记同一选项名的命令, 应在定义期报错
    """
    parser = Parser("x", color="never")
    parser.add_argument("--a", help="甲")
    parser.add_argument("--a", help="乙")


def _dup_dest() -> None:
    """建一个两个选项共用同一 dest 的命令, 应在定义期报错
    """
    parser = Parser("x", color="never")
    parser.add_argument("--a", help="甲")
    parser.add_argument("--b", dest="a", help="乙")


def _remainder_then_more() -> None:
    """建一个 REMAINDER 位置参数后还接位置参数的命令, 应在定义期报错
    """
    parser = Parser("x", color="never")
    parser.add_argument("rest", nargs=REMAINDER)
    parser.add_argument("tail", help="尾")


def _two_subparsers() -> None:
    """建一个登记两次子命令集合的命令, 应在定义期报错
    """
    parser = Parser("x", color="never")
    parser.add_subparsers(title="Commands")
    parser.add_subparsers(title="More")


def _dest_in_chain() -> None:
    """建一个父子命令共用 dest 的命令, 应在解析期报错
    """
    parser = Parser("x", color="never")
    parser.add_argument("-v", action="store_true", global_=True, help="甲")
    commands = parser.add_subparsers()
    child = commands.add_parser("c")
    child.add_argument("-v", action="store_true", help="乙")
    parser.parse_args(["c"])


for label, builder in (
    ("位置参数不接受 required", lambda: Parser("x", color="never").add_argument("f", required=True)),
    ("选项名重复", _dup_option),
    ("dest 重复", _dup_dest),
    ("签名数字的名字", lambda: Parser("x", color="never").add_argument("-1", help="坏")),
    ("未知关键字", lambda: Parser("x", color="never").add_argument("--a", nargs_=1)),
    ("REMAINDER 之后不能加位置参数", _remainder_then_more),
    ("未知动作名", lambda: Parser("x", color="never").add_argument("--a", action="explode")),
    ("重复登记子命令集合", _two_subparsers),
    ("命令链上 dest 重复", _dest_in_chain),
):
    try:
        builder()
    except (ValueError, TypeError) as exc:
        check(label, True, exc)
    else:
        check(label, False)

# ---- 辅助类型 ----------------------------------------------------------

arg = Argument(names=(), dest="files", nargs="*")
check("位置参数帮助名", arg.help_names() == "[FILES]..." and arg.usage_spec() == "[FILES]...", arg.help_names())
arg = Argument(names=("-o", "--output"), dest="output", metavar="FILE")
check("选项错误指代", arg.error_name() == "--output <FILE>", arg.error_name())
check("选项帮助左列", arg.help_names() == "-o, --output <FILE>", arg.help_names())
check("Namespace 相等", Namespace(a=1) == Namespace(a=1) and Namespace(a=1) != Namespace(a=2))
check("Namespace 缺键", "b" not in Namespace(a=1))

print()
if _failures:
    print(f"FAILED: {', '.join(_failures)}")
    raise SystemExit(1)
print("all checks passed")
