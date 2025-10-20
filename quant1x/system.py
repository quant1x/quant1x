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


def read_dotenv(key: str) -> str:
    """
    只读地从项目附近的 .env 文件读取指定的环境变量 `key`（不写入 os.environ）。
    搜索顺序：当前工作目录 -> 本文件目录及若干父目录 -> dotenv.find_dotenv()
    返回值：如果找不到或解析失败，返回空字符串。
    """
    if not key:
        return ''
    try:
        from pathlib import Path
        import dotenv

        candidates = []
        candidates.append(Path.cwd())
        this_dir = Path(__file__).resolve().parent
        candidates.append(this_dir)
        for p in this_dir.parents[:4]:
            candidates.append(p)

        for start in candidates:
            env_path = start.joinpath('.env')
            if env_path.is_file():
                try:
                    vals = dotenv.dotenv_values(str(env_path))
                    val = vals.get(key)
                    if val:
                        return str(val).strip().strip('"\'')
                except Exception:
                    continue

        found = dotenv.find_dotenv()
        if found:
            vals = dotenv.dotenv_values(found)
            val = vals.get(key)
            if val:
                return str(val).strip().strip('"\'')
    except Exception:
        pass
    return ''
