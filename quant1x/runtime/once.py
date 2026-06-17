# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
import threading
from typing import Callable, Optional
from .scheduler import Scheduler
from quant1x.log import logger

class RollingOnce:
    def __init__(
        self,
        name: str,
        cron: str,
        timezone: str = 'Asia/Shanghai'
    ):
        """
        :param name: 唯一标识
        :param cron: Cron 表达式(如 "0 30 8 * * *")
        :param timezone: 时区(仅在首次创建全局调度器时生效)
        """
        self.name = name
        self._done = False
        self._lock = threading.Lock()
        self._job_id = f"rolling_once_reset_{name}"
        self._has_cron = cron is not None

        if cron is not None:
            # 获取全局调度器(自动创建)
            scheduler = Scheduler.get_instance(timezone=timezone)

            # 先移除可能存在的旧任务, 避免冲突
            scheduler.remove_job(self._job_id, force=True)

            # 添加 reset 任务
            job_id = scheduler.add_job(
                func=self.reset,
                cron_expression=cron,
                id=self._job_id,
                replace_existing=True
            )
            if not job_id:
                raise RuntimeError(f"Failed to register reset task for {name}")

    def do(self, func: Callable, *args, **kwargs):
        if not self._done:
            with self._lock:
                if not self._done:
                    logger.debug(f"[{self.name}] Executing callback")
                    func(*args, **kwargs)
                    self._done = True

    def reset(self):
        with self._lock:
            was_done = self._done
            self._done = False
            logger.debug(f"[{self.name}] Reset triggered (was {'active' if was_done else 'idle'})")

    def close(self):
        """手动清理(通常不需要, 由全局调度器统一管理)"""
        if self._has_cron:
            scheduler = Scheduler.get_instance()
            scheduler.remove_job(self._job_id, force=True)


if __name__ == "__main__":
    import time

    def my_task():
        logger.warning("Task executed")

    rolling_once = RollingOnce(name="test_once", cron="*/1 * * * * *")  # 每秒重置一次

    for i in range(5):
        print(f"Attempt {i+1} to execute task")
        rolling_once.do(my_task)
        time.sleep(10)  # 每10秒尝试执行一次任务