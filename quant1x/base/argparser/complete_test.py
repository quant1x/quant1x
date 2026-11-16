# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""`complete` 的用例: 三种 shell 的脚本骨架、候选词与转义。

运行: python -m quant1x.base.argparser.complete_test
"""

from quant1x.base.argparser import Parser
from quant1x.base.argparser.complete import complete

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


def tool() -> Parser:
    """建示例命令: 顶层三个选项加两条子命令, 子命令各带一个选项

    :return: 建好的解析器
    """
    parser = Parser("demo", about="演示")
    parser.add_argument("-v", "--verbose", action="store_true", help="verbose output", global_=True)
    parser.add_argument("--mode", help="mode")
    parser.add_argument("--secret", hide=True)
    parser.add_argument("kind", choices=["x", "y"])
    subs = parser.add_subparsers(title="Commands")
    alpha = subs.add_parser("alpha", about="alpha command")
    alpha.add_argument("--file", help="input file")
    beta = subs.add_parser("beta", about="beta command")
    beta.add_argument("--count", type=int)
    return parser


bash = complete(tool(), "bash")
zsh = complete(tool(), "zsh")
fish = complete(tool(), "fish")

check("bash 脚本以单个换行收尾", bash.endswith("\n") and not bash.endswith("\n\n"))
check("bash 注册补全函数", "complete -F _argparser_demo demo" in bash, bash.splitlines()[-1])
check("bash 函数名与 prog 同名", bash.startswith("_argparser_demo() {\n"), bash.splitlines()[0])
check("bash 取当前词", 'cur="${COMP_WORDS[COMP_CWORD]}"' in bash)
check("bash 顶层分支含选项与子命令", 'compgen -W "-h --help -v --verbose --mode alpha beta x y"' in bash, bash)
check("bash 子命令分支", '"alpha") COMPREPLY=' in bash and "--file" in bash, bash)
check("bash 状态机按子命令下沉", '"|alpha") cmd="alpha" ;;' in bash and '"|beta") cmd="beta" ;;' in bash, bash)
check("bash 隐藏参数不进候选", "--secret" not in bash)
check("bash 全局参数在子命令候选里", '"alpha") COMPREPLY=( $(compgen -W "-h --help --file -v --verbose"' in bash, bash)

check("zsh 头带 compdef", zsh.startswith("#compdef demo\n"), zsh.splitlines()[0])
check("zsh 用 compadd 交候选", 'compadd -- "${candidates[@]}"' in zsh)
check("zsh 末尾挂上函数", zsh.endswith('_argparser_demo "$@"\n'), zsh.splitlines()[-1])
check("zsh 顶层候选", '"") candidates=(-h --help -v --verbose --mode alpha beta x y) ;;' in zsh, zsh)
check("zsh 状态机", 'for word in ${words[2,CURRENT-1]}; do' in zsh)
check("zsh 子命令候选", '"alpha") candidates=(-h --help --file -v --verbose) ;;' in zsh, zsh)

check("fish 短长选项合并成一行", "complete -c demo -s v -l verbose -d 'verbose output'" in fish, fish)
check("fish 无说明时不写 -d", "complete -c demo -n '__fish_seen_subcommand_from beta' -l count\n" in fish, fish)
check("fish 子命令带条件", "complete -c demo -f -a alpha -d 'alpha command'" in fish)
check(
    "fish 子命令的选项挂在条件里",
    "complete -c demo -n '__fish_seen_subcommand_from alpha' -l file -d 'input file'" in fish,
    fish,
)
check("fish 位置参数 choices", "complete -c demo -f -a 'x y'" in fish)
check("fish 隐藏参数不进候选", "--secret" not in fish)

bare = complete(Parser("solo"), "bash")
check("无子命令时不生成状态机", "$cmd|$word" not in bare, bare)
check("无子命令时仍有顶层分支", '        "") COMPREPLY=' in bare, bare)

check("prog 可被覆盖", "complete -F _argparser_tool tool" in complete(tool(), "bash", "tool"))
check("prog 里的非标识符换成下划线", "_argparser_my_tool" in complete(Parser("my-tool"), "bash"))

quoted = Parser("quote")
quoted.add_argument("--note", help="it's a 'note'")
check("fish 转义单引号", r"-d 'it\'s a \'note\''" in complete(quoted, "fish"), complete(quoted, "fish"))

try:
    complete(tool(), "powershell")
except ValueError as exc:
    check("未知 shell 报 ValueError", "powershell" in str(exc), exc)
else:
    check("未知 shell 报 ValueError", False)

print()
if _failures:
    print(f"FAILED: {', '.join(_failures)}")
    raise SystemExit(1)
print("all checks passed")
