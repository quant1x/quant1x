# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

def _get_version() -> str:
    # 1. 优先读取 setuptools_scm 写入的 _version.py
    try:
        from ._version import version as _v
        return _v
    except (ImportError, ModuleNotFoundError):
        pass

    # 2. 已安装时从包元数据获取
    try:
        from importlib.metadata import version
        return version("quant1x")
    except Exception:
        pass

    # 3. 开发环境 fallback: setuptools_scm 直接查询 git
    try:
        from setuptools_scm import get_version
        return get_version(root="..", relative_to=__file__)
    except Exception:
        pass

    return "0.0.0-dev"

__version__ = _get_version()