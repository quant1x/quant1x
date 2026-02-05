# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

# from setuptools_scm import get_version

# try:
#     from ._version import version as __version__
# except ImportError:
#     __version__ = get_version(root="..", relative_to=__file__)

try:
    from importlib.metadata import version, PackageNotFoundError
except ImportError:
    # 兼容 Python < 3.8（如仍需支持）
    from importlib_metadata import version, PackageNotFoundError
    
try:
    __version__ = version("quant1x")
except (ImportError, PackageNotFoundError):
    try:
        from setuptools_scm import get_version
        __version__ = get_version(root="..", relative_to=__file__)
    except Exception:
        __version__ = "0.0.0-dev"