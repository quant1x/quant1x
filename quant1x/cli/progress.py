# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import time
from typing import Dict, Optional
from rich.progress import Progress, TaskID

class ThrottledTask:
    def __init__(
        self,
        progress: Progress,
        task_id: TaskID,
        min_interval: float = 0.01,
        min_batch: int = 1000,
    ):
        self._progress = progress
        self._task_id = task_id
        self._min_interval = min_interval
        self._min_batch = min_batch
        self._accumulated = 0
        self._last_flush = time.monotonic()

    def update(self, advance: int = 1) -> None:
        self._accumulated += advance
        now = time.monotonic()
        if (
            self._accumulated >= self._min_batch
            or (now - self._last_flush) >= self._min_interval
        ):
            if self._accumulated > 0:
                self._progress.update(self._task_id, advance=self._accumulated)
                self._accumulated = 0
                self._last_flush = now

    def flush(self) -> None:
        if self._accumulated > 0:
            self._progress.update(self._task_id, advance=self._accumulated)
            self._accumulated = 0


class ThrottledMultiProgress:
    def __init__(self, progress: Progress):
        self._progress = progress
        self._tasks: Dict[TaskID, ThrottledTask] = {}

    def add_task(self, description: str, total: int, **kwargs) -> TaskID:
        task_id = self._progress.add_task(description, total=total, **kwargs)
        self._tasks[task_id] = ThrottledTask(self._progress, task_id)
        return task_id

    def update(self, task_id: TaskID, advance: int = 1) -> None:
        if task_id not in self._tasks:
            raise KeyError(f"Task {task_id} not managed by ThrottledMultiProgress")
        self._tasks[task_id].update(advance)

    def get_task_proxy(self, task_id: TaskID):
        """返回一个可直接调用 .update() 的代理对象（可选语法糖）"""
        class _Proxy:
            def __init__(self, parent, tid):
                self._parent = parent
                self._tid = tid
            def update(self, advance=1):
                self._parent.update(self._tid, advance)
        return _Proxy(self, task_id)

    def close(self):
        for task in self._tasks.values():
            task.flush()
        self._progress.refresh()