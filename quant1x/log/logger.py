# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations
import atexit
import sys
from loguru import logger as __logger

from quant1x.config.config import base_config as config
from quant1x.std import filesystem as fs, system

# ========== 防护：仅当未配置时执行 ==========
if not getattr(__logger, "_quant1x_configured", False):
    # 配置日志路径
    __LOG_DIR = config.log_path
    fs.mkdirs(__LOG_DIR)

    # 获取应用名称
    _, filename, _ = system.application()
    _LOG_NAME = f"{filename}_service" if filename == "pythonservice" else filename
    _LOG_FILE = f"{__LOG_DIR}/{_LOG_NAME}.log"
    
    _LOG_LEVEL = "DEBUG"# if config.debug else "INFO"

    # 日志格式
    _LOG_FORMAT = (
        "<green>{time:YYYY-MM-DD HH:mm:ss.SSS}</green> | "
        "<level>{level: <8}</level> | "
        "<cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - <level>{message}</level>"
    )

    # 移除默认 handler
    __logger.remove()

    # 控制台输出（非服务环境）
    if filename != "pythonservice":
        __logger.add(
            sys.stderr,
            format=_LOG_FORMAT,
            level=_LOG_LEVEL,
            enqueue=True,
            catch=True,
        )

    # 文件输出
    __logger.add(
        _LOG_FILE,
        rotation="00:00",
        retention="10 days",
        compression="zip",
        encoding="utf-8",
        enqueue=True,
        backtrace=True,
        diagnose=True,
        format=_LOG_FORMAT,
        level=_LOG_LEVEL,
        catch=True,
    )
    

    # 标记已配置
    __logger._quant1x_configured = True
    
    __logger.info(f"日志文件路径: {_LOG_FILE}")

    # 注册退出清理
    atexit.register(__logger.complete)

# ========== 统一导出 ==========
logger = __logger

if __name__ == "__main__":
    logger.warning("日志配置测试")
    logger.error("日志测试错误")