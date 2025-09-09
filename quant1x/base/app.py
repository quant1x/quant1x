# -*- coding: UTF-8 -*-
"""
应用程序工具模块，提供获取应用信息和用户信息的函数。
"""

import os.path
import sys
from typing import Tuple

from quant1x.base import file


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
    home_dir = file.homedir()
    _, username = os.path.split(home_dir)
    return username


if __name__ == '__main__':
    app_info = application()
    print(app_info)
