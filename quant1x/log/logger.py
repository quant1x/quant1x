# -*- coding: UTF-8 -*-
"""
日志配置模块，提供预配置的logger实例。
日志文件存储在用户主目录的.quant1x/logs目录下，按天轮转，保留10天。
"""

import atexit
import sys
from pathlib import Path

from loguru import logger as __logger
from quant1x import system

# 配置日志路径
__USER_HOME = Path(system.homedir()).expanduser()
__LOG_DIR = __USER_HOME / ".quant1x" / "logs"
__LOG_DIR.mkdir(parents=True, exist_ok=True)  # 确保日志目录存在

# 获取应用名称作为日志文件名
_, filename, _ = system.application()
_LOG_NAME = "quant1x" if filename == "pythonservice" else filename
_LOG_FILE = __LOG_DIR / f"{_LOG_NAME}.log"

# 定义日志格式
_LOG_FORMAT = (
    "<green>{time:YYYY-MM-DD HH:mm:ss.SSS}</green> | "
    "<level>{level: <8}</level> | "
    "<cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - <level>{message}</level>"
)

# 移除默认的handler
__logger.remove()

# 添加控制台handler
__logger.add(
    sys.stderr,
    format=_LOG_FORMAT,
    level="INFO",
    enqueue=True,
)

# 添加文件handler
__logger.add(
    _LOG_FILE,
    rotation="00:00",  # 每天轮转
    retention="10 days",  # 保留10天
    compression="zip",  # 压缩旧日志
    encoding="utf-8",  # 明确指定编码
    enqueue=True,  # 异步写入
    backtrace=True,  # 启用错误回溯
    diagnose=True,  # 显示诊断信息
    format=_LOG_FORMAT,
    level="DEBUG",
)

# 注册退出时的清理函数，确保异步日志被刷新
atexit.register(__logger.remove)

#logger = __logger

if __name__ == "__main__":
    __logger.warning("日志配置测试")
    __logger.info(f"日志文件路径: {_LOG_FILE}")
