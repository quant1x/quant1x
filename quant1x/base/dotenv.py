# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""`.env` 与系统环境变量的**唯一读取口**（`env` / `read_dotenv`）。

**为什么单独成模块**: 读 `.env` 的优先级要被 base 下任意模块复用（数据根、缓存目录都从环境变量取），
而把这段逻辑塞进文件操作模块会让"读配置"与"操作文件"耦在一起。这里只依赖标准库与同目录的
`expandvars` / `reload`；项目根定位复用 `filesystem` 的向上查找，`filesystem` 不引用本模块，不会成环。

**优先级**（与 spec 一致，别在别处另起一套）：

1. 项目根的 `.env`；
2. 用户主目录的 `.env`；
3. 系统环境变量；
4. 调用方给的默认值。

`.env` 只存在于两处 —— 用户主目录 `~/.env` 与项目根 `.env`（项目根按 `filesystem.find_project_root` 的标记定位）。
两份都读、逐键合并：同一个键两处都有时取项目根，只有主目录那份有时就用主目录的值，不存在
"先找到哪份就停"。合并结果在进程内缓存成一张键值表，热加载由 `base/reload.py` 负责 —— 每次取值
比对两处 `.env` 的存在性与修改时间，任一份动过才重新读盘。只读，不写 `os.environ`。

**变量展开**：值的展开交给 `base/expandvars.py`，键值表是那份合并结果（另一份 `.env` 里的键同样
可用），兜底表是系统环境变量。
"""

import os
import sys
from pathlib import Path

from . import filesystem
from .expandvars import expand
from .reload import Reloader


def get_os_env(key: str) -> str:
    """取系统变量: 读系统环境变量并去首尾空白

    :param key: 变量名
    :return: 变量值; 未设置时为空串
    """
    return os.getenv(key, '').strip()


def _value(raw: str) -> str:
    """洗值: 取 `KEY=` 右边的内容

    先剥成对引号再 trim; 未加引号时截到空白后的 `#` 注释处。

    :param raw: 等号右边的原始文本
    :return: 去引号、去注释、去首尾空白后的取值
    """
    raw = raw.strip()
    if raw[:1] in ("'", '"') and raw[0] in raw[1:]:
        return raw[1:raw.index(raw[0], 1)].strip()
    for index, char in enumerate(raw):
        if char == '#' and (index == 0 or raw[index - 1].isspace()):
            return raw[:index].rstrip()
    return raw


def _parse(path: Path) -> dict[str, str]:
    """解析 `.env`: 把整份文件读成键值对

    只做最简解析: `KEY=VALUE`、`#` 注释、成对引号, 容忍 BOM 与 `export ` 前缀。键名不做替换, 值的展开在合并之后由 `_expand` 统一做。

    :param path: `.env` 文件路径
    :return: 键值对; 文件读不到时为空字典
    """
    try:
        lines = path.read_text(encoding='utf-8-sig').splitlines()
    except OSError:
        return {}
    values = {}
    for line in lines:
        line = line.strip()
        if not line or line.startswith('#') or '=' not in line:
            continue
        key, _, raw = line.partition('=')
        values[key.strip().removeprefix('export ').strip()] = _value(raw)
    return values


def _starts() -> list[Path]:
    """给查找起点: 按优先级去重后给出定位项目根的各个起点目录

    顺序为工作目录、入口脚本所在目录、本模块所在目录。

    :return: 起点目录列表
    """
    starts = [Path.cwd()]
    entry = Path(sys.argv[0]).resolve() if sys.argv[0] else None
    if entry is None or not entry.is_file():
        entry = Path(__file__).resolve()
    for start in (entry.parent, Path(__file__).resolve().parent):
        if start not in starts:
            starts.append(start)
    return starts


def _project_root() -> Path | None:
    """定位项目根: 按 `filesystem.find_project_root` 的标记向上查找

    依次用各起点去找, 第一个能定位到项目根的起点即有效。

    :return: 项目根目录; 一处都定位不到时为 `None`
    """
    for start in _starts():
        root = filesystem.find_project_root(str(start))
        if root:
            return Path(root)
    return None


def _paths() -> list[Path]:
    """列出 `.env` 候选: 用户主目录在前、项目根在后, 后者覆盖前者

    主目录一律用 `Path.home()`, 不走 `filesystem.homedir()` —— 后者本身靠 `.env` 定值, 会绕回来。

    :return: 候选 `.env` 路径列表
    """
    home = Path.home()
    paths = [home / '.env']
    root = _project_root()
    if root is not None and root != home:
        paths.append(root / '.env')
    return paths


def _parse_all() -> dict[str, str]:
    """读全量: 两份 `.env` 合并后逐键展开

    各候选 `.env` 依次覆盖前一份, 故项目根的同名键盖住主目录的、缺的键由主目录那份补上。
    展开在合并之后做, 值里的引用因此能取到另一份 `.env` 的键。

    :return: 合并并展开后的键值表
    """
    values: dict[str, str] = {}
    for path in _paths():
        values.update(_parse(path))
    environ = {name: value.strip() for name, value in os.environ.items()}
    return {key: expand(raw, values, environ) for key, raw in values.items()}


# 热加载容器: 两处 `.env` 的存在性/修改时间一动就重新读盘, 没动就用上一次的键值表
_loader = Reloader(_paths, _parse_all)


def _load() -> dict[str, str]:
    """取键值表: 两处 `.env` 没动就回缓存, 动过就重新读盘

    判定与重取在 `base/reload.Reloader`, 每次取值只比对文件的存在性与修改时间。

    :return: 合并并展开后的键值表
    """
    return _loader.get()


def read_dotenv(key: str) -> str:
    """只读 `.env`: 从主目录与项目根两份 `.env` 的合并结果里取指定键

    两处都没有该键时返回空串; 换系统环境变量兜底是 `env` 的事。不写入 `os.environ`。

    :param key: 环境变量名, 前后空白会被去掉
    :return: 取值; 找不到、键为空或解析失败时为空串
    """
    key = key.strip()
    if not key:
        return ''
    return _load().get(key, '')


def env(key: str, default: str = '') -> str:
    """取环境变量: 按 `.env` > 系统环境变量 > 默认值的优先级取一个值

    :param key: 环境变量名, 前后空白会被去掉
    :param default: 前两级都取不到时的兜底值
    :return: 命中的那一级取值
    """
    key = key.strip()
    return read_dotenv(key) or get_os_env(key) or default
