# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

from quant1x.log import logger


def _get_version() -> str:
    # 1. 优先读取 setuptools_scm 写入的 _version.py
    try:
        from ._version import version as _v
        return _v
    except (ImportError, ModuleNotFoundError) as exc:
        logger.debug("读取 _version.py 失败, 回退到下一种取版本方式: {}", exc)

    # 2. 已安装时从包元数据获取
    try:
        from importlib.metadata import version
        return version("quant1x")
    except Exception as exc:
        logger.debug("从包元数据取版本失败, 回退到 setuptools_scm: {}", exc)

    # 3. 开发环境 fallback: setuptools_scm 直接查询 git
    try:
        from setuptools_scm import get_version
        return get_version(root="..", relative_to=__file__)
    except Exception as exc:
        logger.debug("setuptools_scm 查询 git 失败, 使用开发版占位: {}", exc)

    return "0.0.0-dev"

__version__ = _get_version()
