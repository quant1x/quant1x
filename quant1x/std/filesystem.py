# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

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
    with open(filename, 'w') as done_file:
        pass

def update_file_mtime(filename: str, timestamp: float = 0):
    """
    更新文件的修改时间(mtime)，如果文件不存在则创建空文件
    
    Args:
        filename (str): 需要更新时间的文件路径
        timestamp (float): 指定的时间戳，默认为0表示使用当前时间
    
    Raises:
        Exception: 文件操作失败时静默处理（不抛出异常）
    """
    if timestamp == 0:
        timestamp = time.time()
    try:
        if not os.path.exists(filename):
            touch(filename)
        os.utime(filename, (int(timestamp), int(timestamp)))
    except Exception:
        pass