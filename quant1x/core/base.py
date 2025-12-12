# -*- coding: utf-8 -*-
import os

LANGUAGE = "python"
DEFAULT_BASE_PATH = f"~/.q1x-{LANGUAGE}"

_QUANT1X_BASE_PATH = None

def get_base_path() -> str:
    """
    返回默认的基础路径，如果无法展开用户目录则返回默认路径
    """
    global _QUANT1X_BASE_PATH
    if _QUANT1X_BASE_PATH is None:
        _QUANT1X_BASE_PATH = os.path.expanduser(DEFAULT_BASE_PATH)
    return _QUANT1X_BASE_PATH

def get_meta_path() -> str:
    """
    返回元数据存储的基础路径
    meta目录位于基础路径下的meta子目录中
    """
    return os.path.join(get_base_path(), "meta")
