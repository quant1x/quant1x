# -*- coding: UTF-8 -*-
import os.path
import sys

import file


def application() -> tuple[str, str, str]:
    """
    获取应用程序的名字
    :return: 目录, 文件名, 扩展名
    """
    app = sys.argv[0]
    # print(app)
    # print(os.path.abspath(app))
    abspath = os.path.abspath(app)
    # print(os.path.realpath(app))
    dirname, filename = os.path.split(abspath)
    (filename, ext) = os.path.splitext(filename)
    # print(filename, ext)
    return dirname, filename, ext


def getuser():
    """
    获取用户名
    :return:
    """
    home = file.homedir()
    _, username = os.path.split(home)
    return username


if __name__ == '__main__':
    name = application()
    print(name)
