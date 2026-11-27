# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

from __future__ import annotations
import atexit
import sys

from loguru import logger as _logger

from quant1x.base import filesystem as fs, system
from quant1x.base.singleton import SingletonInitPolicy, ThreadSafeSingletonABC
from quant1x.config.config import base_config as config


class LoggingSetup(ThreadSafeSingletonABC):
    """loguru 配置单例: 进程内只初始化一次.

    quant1x/__init__.py 与各业务模块都从 quant1x.log 取同一个 logger,
    把配置收进单例, 避免包初始化与模块导入各自执行一遍 sink 注册.
    """

    _init_policy = SingletonInitPolicy.LAZY

    def initialize(self) -> None:
        log_dir = config.log_path
        fs.mkdirs(log_dir)

        # 获取应用名称
        _, filename, _ = system.application()
        log_name = f"{filename}_service" if filename == "pythonservice" else filename
        log_file = f"{log_dir}/{log_name}.log"

        level = "DEBUG" if config.debug else "INFO"

        fmt = (
            "<green>{time:YYYY-MM-DD HH:mm:ss.SSS}</green> | "
            "<level>{level: <8}</level> | "
            "<cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - <level>{message}</level>"
        )

        # 移除默认 handler
        _logger.remove()

        # 控制台输出(非服务环境)
        if filename != "pythonservice":
            _logger.add(
                sys.stderr,
                format=fmt,
                level=level,
                enqueue=True,
                catch=True,
            )

        # 文件输出
        _logger.add(
            log_file,
            rotation="00:00",
            retention="10 days",
            compression="zip",
            encoding="utf-8",
            enqueue=True,
            backtrace=True,
            diagnose=True,
            format=fmt,
            level=level,
            catch=True,
        )

        # 注册退出清理
        atexit.register(_logger.complete)

        _logger.info(f"日志文件路径: {log_file}")

    def cleanup(self) -> None:
        _logger.complete()


# ========== 统一导出 ==========
LoggingSetup.get_instance()
logger = _logger

if __name__ == "__main__":
    logger.warning("日志配置测试")
    logger.error("日志测试错误")
