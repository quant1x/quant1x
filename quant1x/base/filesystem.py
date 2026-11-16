# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

import functools
import os
import time
import tempfile


def homedir() -> str:
    """
    获取当前用户的主目录
    优先级:
    1. QUANT1X_HOME 环境变量
    2. GOX_HOME 环境变量
    3. HOME 环境变量
    4. USERPROFILE 环境变量 (Windows only)
    5. 系统临时目录 (fallback)
    """
    home = os.getenv("QUANT1X_HOME")
    if home:
        return home
    
    home = os.getenv("GOX_HOME")
    if home:
        return home
        
    home = os.getenv("HOME")
    if home:
        return home
        
    if os.name == 'nt':
        home = os.getenv("USERPROFILE")
        if home:
            return home
            
    return tempfile.gettempdir()

def expand_user(path: str) -> str:
    """
    展开用户主目录 (例如 "~/data" -> "/home/user/data")
    """
    if not path:
        return path
        
    path = path.strip()
    if not path.startswith('~'):
        return path
        
    home = homedir()
    if len(path) == 1:
        return home
        
    if path[1] == '/' or path[1] == '\\':
        return os.path.join(home, path[2:])
        
    return path

def mkdirs(path: str):
    """
    创建目录
    :param path:
    :return:
    """
    if not os.path.exists(path):
        os.makedirs(path)


def touch(filename: str):
    """
    创建一个空文件
    :param filename:
    :return:
    """
    directory = os.path.dirname(filename)
    mkdirs(directory)
    with open(filename, 'w'):
        pass

def update_file_mtime(filename: str, timestamp: float = 0):
    """
    更新文件的修改时间(mtime), 如果文件不存在则创建空文件
    
    Args:
        filename (str): 需要更新时间的文件路径
        timestamp (float): 指定的时间戳, 默认为0表示使用当前时间
    
    Raises:
        Exception: 文件操作失败时静默处理(不抛出异常)
    """
    if timestamp == 0:
        timestamp = time.time()
    try:
        if not os.path.exists(filename):
            touch(filename)
        os.utime(filename, (int(timestamp), int(timestamp)))
    except Exception:  # noqa: S110 - 尽力而为: 设置文件时间失败不影响调用方
        pass


# 项目根标记（按此顺序定位，第一个命中的目录即项目根）
PROJECT_MARKERS = ('pyproject.toml', 'LICENSE', '.git')
# 向上查找项目根标记的最大层数（防起点在深层目录时一路扫到文件系统根）
MAX_UP_LEVELS = 10


@functools.lru_cache(maxsize=None)
def find_upward(start_dir: str, marker: str) -> str:
    """
    向上定位: 从 start_dir 起向上找 marker(文件或目录)所在目录

    最多向上 MAX_UP_LEVELS 层, 防深层起点一路扫到文件系统根。

    :param start_dir: 查找起点目录
    :param marker: 标记名, 文件或目录均可
    :return: 命中目录的绝对路径; 未找到时为空串
    """
    current = os.path.abspath(start_dir)
    for _ in range(MAX_UP_LEVELS):
        if os.path.exists(os.path.join(current, marker)):
            return current
        parent = os.path.dirname(current)
        if parent == current:  # 已到文件系统根
            break
        current = parent
    return ''


@functools.lru_cache(maxsize=None)
def find_project_root(start_dir: str) -> str:
    """
    定位项目根: 从 start_dir 向上按标记定位项目根

    标记优先级为 pyproject.toml -> LICENSE -> .git。

    :param start_dir: 查找起点目录
    :return: 项目根目录; 三个标记全未命中时为空串
    """
    for marker in PROJECT_MARKERS:
        root = find_upward(start_dir, marker)
        if root:
            return root
    return ''
