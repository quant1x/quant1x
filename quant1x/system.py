# -*- coding: UTF-8 -*-
import os
import sys
from typing import Tuple

from matplotlib.pylab import f


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
    搜索顺序：从当前工作目录开始，逐级向上查找每一级目录下的 `.env`，直到根目录；
    如果都找不到，再使用 dotenv.find_dotenv() 作为最后的回退。
    返回值：如果找不到或解析失败，返回空字符串。
    """
    if not key:
        return ''

    def find_env_upwards(start_path) -> 'str | None':
        """从 start_path 开始，向上逐级查找 `.env` 文件，找到则返回该文件的绝对路径字符串；找不到返回 None。"""
        try:
            from pathlib import Path
            p = Path(start_path)
            for d in [p] + list(p.parents):
                env_file = d / '.env'
                #print('checking for .env at', env_file)
                if env_file.is_file():
                    return str(env_file)
        except Exception:
            pass
        return None

    try:
        import dotenv
        from pathlib import Path

        # 按用户要求：先用 cmd（运行时的 cwd），然后用 python 后面的脚本文件的绝对路径（sys.argv[0]）
        starts = [Path.cwd()]
        try:
            entry = Path(sys.argv[0]).resolve()
            if entry.is_file():
                starts.append(entry.parent)
            else:
                # 回退到本模块文件夹
                starts.append(Path(__file__).absolute().parent)
        except Exception:
            starts.append(Path(__file__).absolute().parent)

        for start in starts:
            env_path = find_env_upwards(start)
            if env_path:
                try:
                    vals = dotenv.dotenv_values(env_path)
                    raw = vals.get(key)
                    if raw:
                        return str(raw).strip().strip('"\'')
                except Exception:
                    # 解析失败则继续到下一个起点或回退策略
                    continue

        # 最后回退到 dotenv.find_dotenv()
        found = dotenv.find_dotenv()
        if found:
            try:
                vals = dotenv.dotenv_values(found)
                raw = vals.get(key)
                if raw:
                    return str(raw).strip().strip('"\'')
            except Exception:
                pass
    except Exception:
        pass
    return ''
