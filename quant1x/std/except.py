# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

"""
异常处理模块 - Python implementation converted from Go
提供统一的异常处理机制
"""

from abc import ABC, abstractmethod
from typing import Any


class Throwable(ABC):
    """异常接口"""
    @abstractmethod
    def code(self) -> int:
        """获取错误码"""
        pass


class Exception(Exception, Throwable):
    """自定义异常类"""

    def __init__(self, code: int, message: str, *args: Any):
        """
        创建一个新的异常

        Args:
            code: 错误码
            message: 错误信息模板
            *args: 格式化参数
        """
        self._code = code
        if args:
            self._message = message % args
        else:
            self._message = message

        # 调用父类 Exception 的构造函数
        super().__init__(self._message)

    def code(self) -> int:
        """获取错误码"""
        return self._code

    def __str__(self) -> str:
        """格式化输出错误信息"""
        return f"#{self._code}, message={self._message}"

    @property
    def message(self) -> str:
        """获取错误信息"""
        return self._message

    def success(self) -> bool:
        """
        检查异常是否表示成功状态

        Returns:
            bool: 如果 code == 0 则返回 True, 表示成功
        """
        return self._code == 0


def new_exception(code: int, message: str, *args: Any) -> Exception:
    """
    创建一个新的异常

    Args:
        code: 错误码
        message: 错误信息模板
        *args: 格式化参数

    Returns:
        Exception: 异常实例
    """
    return Exception(code, message, *args)