# -*- coding: UTF-8 -*-
import os
import sys
from typing import Tuple


def application() -> Tuple[str, str, str]:
    """
    获取当前应用程序的路径信息。

    Returns:
        Tuple[str, str, str]: 返回包含目录路径、文件名(不含扩展名)和扩展名的元组
    """
    app_path = os.path.abspath(sys.argv[0])
    dir_path, full_filename = os.path.split(app_path)
    filename, ext = os.path.splitext(full_filename)
    return dir_path, filename, ext


def getuser() -> str:
    """
    获取当前用户名。

    Returns:
        str: 当前用户名
    """
    home_dir = homedir()
    _, username = os.path.split(home_dir)
    return username

def env(key: str) -> str:
    """
    获取环境变量
    """
    value = os.getenv(key, '')
    return value.strip()


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


def homedir() -> str:
    """
    获取宿主目录
    首先会依次获取环境变量QUANT1X_HOME, GOX_HOME, 如果不存在则用~
    :return:
    """
    user_home = env('QUANT1X_HOME')
    if len(user_home) == 0:
        user_home = env("GOX_HOME")
    if len(user_home) == 0:
        user_home = os.path.expanduser('~')
    return user_home
