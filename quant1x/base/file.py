# -*- coding: UTF-8 -*-
import os


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
