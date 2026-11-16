# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""由命令定义生成 bash / zsh / fish 的补全脚本。

候选词只看「已敲定的子命令链」: 选项名、子命令名、位置参数的 `choices`;
不区分「正在写取值」的语境, 也不补文件路径。
"""

from collections.abc import Iterator

from .parser import Parser

SHELLS = ("bash", "zsh", "fish")


def complete(parser: Parser, shell: str, prog: str | None = None) -> str:
    """生成补全: 按 `shell` 生成补全脚本

    :param parser: 顶层命令
    :param shell: shell 名, 取 `bash` / `zsh` / `fish`
    :param prog: 脚本里的命令名; 缺省取 `parser.prog`
    :return: 补全脚本全文
    :raises ValueError: `shell` 不在支持范围内
    """
    if shell not in SHELLS:
        raise ValueError(f"不支持的 shell: {shell!r}")
    name = prog or parser.prog
    nodes = list(_walk(parser))
    if shell == "bash":
        return _bash(name, nodes)
    if shell == "zsh":
        return _zsh(name, nodes)
    return _fish(name, nodes)


def _walk(parser: Parser, chain: tuple[str, ...] = ()) -> Iterator[tuple[tuple[str, ...], Parser]]:
    """遍历命令: 按前序给出 (子命令链, 命令)

    :param parser: 当前命令
    :param chain: 到达当前命令的子命令链, 顶层为空
    :return: (子命令链, 命令) 的迭代器
    """
    yield chain, parser
    subs = parser.subparsers
    if subs is None:
        return
    for command, _, child in subs.commands():
        yield from _walk(child, (*chain, command))


def _words(parser: Parser) -> list[str]:
    """列候选词: 给出一个命令的候选词

    选项名、子命令名、位置参数的 `choices` 按登记顺序去重; 含空白的词剔除。

    :param parser: 命令
    :return: 候选词列表
    """
    collected: list[str] = []
    for argument in (*parser.arguments, *parser.global_arguments()):
        if not argument.hide:
            collected.extend(argument.names)
    subs = parser.subparsers
    if subs is not None:
        collected.extend(command for command, _, _ in subs.commands())
    for positional in parser.positionals:
        values = positional.choice_values()
        if values:
            collected.extend(str(value) for value in values)
    words = [word for word in collected if word and not any(char.isspace() for char in word)]
    return list(dict.fromkeys(words))


def _function(name: str) -> str:
    """拼函数名: 给出脚本里的函数名

    非标识符字符一律换成下划线。

    :param name: 命令名
    :return: 函数名
    """
    return f"_argparser_{''.join(char if char.isalnum() or char == '_' else '_' for char in name)}"


def _transitions(nodes: list[tuple[tuple[str, ...], Parser]]) -> list[tuple[str, str]]:
    """列状态边: 给出状态机里每条边

    键形如 `|alpha`、`alpha|beta`, 取值是命中后的命令链文本。

    :param nodes: `_walk` 给出的 (子命令链, 命令) 列表
    :return: (键, 命令链文本) 列表
    """
    return [
        (f"{' '.join(chain[:-1])}|{chain[-1]}", " ".join(chain)) for chain, _ in nodes if chain
    ]


def _bash(name: str, nodes: list[tuple[tuple[str, ...], Parser]]) -> str:
    """生成 bash 脚本: 按命令树生成补全函数与注册行

    :param name: 命令名
    :param nodes: `_walk` 给出的 (子命令链, 命令) 列表
    :return: bash 补全脚本
    """
    function = _function(name)
    lines = [f"{function}() {{", "    local cur cmd word", '    cur="${COMP_WORDS[COMP_CWORD]}"', '    cmd=""']
    edges = _transitions(nodes)
    if edges:
        lines.append('    for word in "${COMP_WORDS[@]:1:COMP_CWORD-1}"; do')
        lines.append('        case "$cmd|$word" in')
        lines.extend(f'            "{key}") cmd="{chain}" ;;' for key, chain in edges)
        lines.append("        esac")
        lines.append("    done")
    lines.append('    case "$cmd" in')
    for chain, parser in nodes:
        words = " ".join(_words(parser))
        lines.append(f'        "{" ".join(chain)}") COMPREPLY=( $(compgen -W "{words}" -- "$cur") ) ;;')
    lines.append("    esac")
    lines.append("}")
    lines.append(f"complete -F {function} {name}")
    return "\n".join(lines) + "\n"


def _zsh(name: str, nodes: list[tuple[tuple[str, ...], Parser]]) -> str:
    """生成 zsh 脚本: 按命令树生成 `#compdef` 补全函数

    :param name: 命令名
    :param nodes: `_walk` 给出的 (子命令链, 命令) 列表
    :return: zsh 补全脚本
    """
    function = _function(name)
    lines = [f"#compdef {name}", "", f"{function}() {{", "    local -a candidates", '    local cmd="" word']
    edges = _transitions(nodes)
    if edges:
        lines.append("    for word in ${words[2,CURRENT-1]}; do")
        lines.append('        case "$cmd|$word" in')
        lines.extend(f'            "{key}") cmd="{chain}" ;;' for key, chain in edges)
        lines.append("        esac")
        lines.append("    done")
    lines.append('    case "$cmd" in')
    for chain, parser in nodes:
        words = " ".join(_words(parser))
        lines.append(f'        "{" ".join(chain)}") candidates=({words}) ;;')
    lines.append("    esac")
    lines.append('    compadd -- "${candidates[@]}"')
    lines.append("}")
    lines.append("")
    lines.append(f'{function} "$@"')
    return "\n".join(lines) + "\n"


def _fish(name: str, nodes: list[tuple[tuple[str, ...], Parser]]) -> str:
    """生成 fish 脚本: 按命令树逐条生成 `complete` 行

    :param name: 命令名
    :param nodes: `_walk` 给出的 (子命令链, 命令) 列表
    :return: fish 补全脚本
    """
    lines: list[str] = []
    for chain, parser in nodes:
        condition = f" -n '__fish_seen_subcommand_from {chain[-1]}'" if chain else ""
        for argument in parser.arguments:
            if argument.hide or argument.is_positional():
                continue
            spelled = [word for flag in argument.names if (word := _fish_flag(flag)) is not None]
            if not spelled:
                continue
            detail = f" -d '{_quote(argument.help)}'" if argument.help else ""
            lines.append(f"complete -c {name}{condition} {' '.join(spelled)}{detail}")
        subs = parser.subparsers
        if subs is not None:
            for command, _, child in subs.commands():
                detail = f" -d '{_quote(child.about)}'" if child.about else ""
                lines.append(f"complete -c {name}{condition} -f -a {command}{detail}")
        for positional in parser.positionals:
            values = positional.choice_values()
            if values:
                spelled = " ".join(str(value) for value in values if not any(c.isspace() for c in str(value)))
                if spelled:
                    lines.append(f"complete -c {name}{condition} -f -a '{spelled}'")
    return "\n".join(lines) + "\n"


def _fish_flag(flag: str) -> str | None:
    """翻 fish 选项: 把选项名翻成 fish 的拼法

    `-h` 转 `-s h`, `--help` 转 `-l help`, 其余形态不补。

    :param flag: 选项名
    :return: fish 拼法; 形态不支持时为 `None`
    """
    if flag.startswith("--") and len(flag) > 2:
        return f"-l {flag[2:]}"
    if len(flag) == 2 and flag.startswith("-"):
        return f"-s {flag[1]}"
    return None


def _quote(text: str) -> str:
    """转义引号: 把说明文字放进单引号

    反斜杠与单引号各转义一层。

    :param text: 原说明文字
    :return: 可放进单引号的文本
    """
    return text.replace("\\", "\\\\").replace("'", "\\'")
