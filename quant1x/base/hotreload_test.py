# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""`base/hotreload` 的离线回归: 候选文件动过才重新取数, 没动就回缓存。"""

import os
import tempfile
import unittest
from pathlib import Path

from . import hotreload


class ReloaderTest(unittest.TestCase):
    def setUp(self) -> None:
        """起临时目录并清零调用计数, 取数函数据此判断有没有被重新调用
        """
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.calls = 0

    def _loader(self, paths) -> hotreload.Reloader:
        """造一个容器: 取数函数每调用一次计数加一, 返回 `v1` / `v2` ...

        :param paths: 候选路径提供者
        :return: 热加载容器
        """
        def load() -> str:
            self.calls += 1
            return f"v{self.calls}"

        return hotreload.Reloader(paths, load)

    def _touch(self, path: Path) -> None:
        """把文件 mtime 往前推一秒: 同一次测试内的两次写盘可能落在同一个时间刻度上

        :param path: 目标文件
        """
        os.utime(path, ns=(os.stat(path).st_mtime_ns + 10 ** 9,) * 2)

    def test_loads_once_when_files_are_stable(self) -> None:
        """文件没动就只取一次数"""
        path = self.root / "a.txt"
        path.write_text("x", encoding="utf-8")
        loader = self._loader(lambda: [path])
        self.assertEqual("v1", loader.get())
        self.assertEqual("v1", loader.get())
        self.assertEqual(1, self.calls)

    def test_changed_file_reloads(self) -> None:
        """文件 mtime 变了就重新取数"""
        path = self.root / "a.txt"
        path.write_text("x", encoding="utf-8")
        loader = self._loader(lambda: [path])
        self.assertEqual("v1", loader.get())
        path.write_text("y", encoding="utf-8")
        self._touch(path)
        self.assertEqual("v2", loader.get())

    def test_appearing_file_reloads(self) -> None:
        """候选文件从无到有也算变化"""
        path = self.root / "a.txt"
        loader = self._loader(lambda: [path])
        self.assertEqual("v1", loader.get())
        path.write_text("x", encoding="utf-8")
        self.assertEqual("v2", loader.get())

    def test_vanished_file_reloads(self) -> None:
        """候选文件被删掉同样算变化"""
        path = self.root / "a.txt"
        path.write_text("x", encoding="utf-8")
        loader = self._loader(lambda: [path])
        self.assertEqual("v1", loader.get())
        path.unlink()
        self.assertEqual("v2", loader.get())

    def test_path_order_does_not_matter(self) -> None:
        """快照按路径排序, 路径提供者换个顺序不等于文件变了"""
        first = self.root / "a.txt"
        second = self.root / "b.txt"
        first.write_text("x", encoding="utf-8")
        second.write_text("y", encoding="utf-8")
        paths = [[first, second], [second, first]]
        loader = self._loader(lambda: paths.pop(0))
        self.assertEqual("v1", loader.get())
        self.assertEqual("v1", loader.get())

    def test_reset_forces_reload(self) -> None:
        """作废缓存后即使文件没动也要重新取数"""
        path = self.root / "a.txt"
        path.write_text("x", encoding="utf-8")
        loader = self._loader(lambda: [path])
        self.assertEqual("v1", loader.get())
        loader.reset()
        self.assertEqual("v2", loader.get())

    def test_paths_are_asked_every_time(self) -> None:
        """路径每次取值时现问, 免得运行期新增的候选文件永远看不到"""
        asked = []

        def paths():
            asked.append(True)
            return [self.root / "a.txt"]

        loader = self._loader(paths)
        loader.get()
        loader.get()
        self.assertEqual(2, len(asked))


if __name__ == "__main__":
    unittest.main()
