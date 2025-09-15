# -*- coding: UTF-8 -*-
import os
import sys


def project_path(package: str = "quant1x", filename: str = '') -> str:
    filename = filename.strip()
    if len(filename) == 0:
        # filename = __file__
        raise Exception('filename is empty')
    pos = filename.rfind(package)
    project_path = filename[:pos - 1]
    root_path = os.path.abspath(project_path)
    return root_path


def redirect(package: str = "quant1x", filename: str = ''):
    """
    重定向于当前工程目录
    :return:
    """
    root_path = project_path(package, filename)
    sys.path.insert(0, root_path)


if __name__ == '__main__':
    sys.exit(redirect())
