# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

import logging

# 包初始化阶段只用标准库日志: quant1x.log 会反向依赖 config, 此处引入会成环
_logger = logging.getLogger(__name__)


def _get_version() -> str:
    # 1. 优先读取 setuptools_scm 写入的 _version.py
    try:
        from ._version import version as _v
        return _v
    except (ImportError, ModuleNotFoundError) as exc:
        _logger.debug("读取 _version.py 失败, 回退到下一种取版本方式: %s", exc)

    # 2. 已安装时从包元数据获取
    try:
        from importlib.metadata import version
        return version("quant1x")
    except Exception as exc:
        _logger.debug("从包元数据取版本失败, 回退到 setuptools_scm: %s", exc)

    # 3. 开发环境 fallback: setuptools_scm 直接查询 git
    try:
        from setuptools_scm import get_version
        return get_version(root="..", relative_to=__file__)
    except Exception as exc:
        _logger.debug("setuptools_scm 查询 git 失败, 使用开发版占位: %s", exc)

    return "0.0.0-dev"

__version__ = _get_version()
