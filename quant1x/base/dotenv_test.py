# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

"""`base/dotenv` 的离线回归: 项目根 `.env` > 主目录 `.env` > 系统环境变量 > 默认值。

这套优先级属于模块契约, 所以这里不只验"读得到", 还逐条钉住"谁盖谁"、两份 `.env` 的逐键合并、
变量展开以及热加载。
"""

import os
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from . import dotenv


class DotenvTest(unittest.TestCase):
    def setUp(self) -> None:
        """起临时目录当项目根, 主目录放它下面, 并作废热加载缓存
        """
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        # 项目根标记: `_project_root` 靠它把临时目录认成项目根
        (self.root / "pyproject.toml").write_text("", encoding="utf-8")
        self.home = self.root / "home"
        self.home.mkdir()
        # 模块级热加载容器跨用例存活: 逐个用例前后各作废一次, 免得上一个用例的值漏过来
        dotenv._loader.reset()
        self.addCleanup(dotenv._loader.reset)
        for patcher in (
            patch.object(dotenv, "_starts", return_value=[self.root]),
            patch.object(Path, "home", return_value=self.home),
        ):
            patcher.start()
            self.addCleanup(patcher.stop)

    def _env_file(self, text: str) -> None:
        """在项目根写一份 `.env`

        :param text: `.env` 全文
        """
        (self.root / ".env").write_text(text, encoding="utf-8")

    def _home_env_file(self, text: str) -> None:
        """在用户主目录写一份 `.env`

        :param text: `.env` 全文
        """
        (self.home / ".env").write_text(text, encoding="utf-8")

    def test_dotenv_beats_os_env(self) -> None:
        """`.env` 与系统环境变量同名时以前者为准
        """
        self._env_file("QUANT1X_PROBE=from-dotenv\n")
        with patch.dict(os.environ, {"QUANT1X_PROBE": "from-os"}, clear=False):
            self.assertEqual("from-dotenv", dotenv.env("QUANT1X_PROBE"))

    def test_os_env_used_when_dotenv_has_no_such_key(self) -> None:
        """两处 `.env` 都没有该键时退到系统环境变量
        """
        self._env_file("QUANT1X_OTHER=x\n")
        self._home_env_file("QUANT1X_ANOTHER=y\n")
        with patch.dict(os.environ, {"QUANT1X_PROBE": "from-os"}, clear=False):
            self.assertEqual("from-os", dotenv.env("QUANT1X_PROBE"))

    def test_default_when_neither_has_it(self) -> None:
        """两处都没有该键时给默认值, 没给默认值则为空串
        """
        self._env_file("QUANT1X_OTHER=x\n")
        with patch.dict(os.environ, {"QUANT1X_PROBE": ""}, clear=False):
            self.assertEqual("fallback", dotenv.env("QUANT1X_PROBE", "fallback"))
            self.assertEqual("", dotenv.env("QUANT1X_PROBE"))

    def test_quotes_and_padding_are_stripped(self) -> None:
        """成对引号剥掉, 引号内的空白一并 trim
        """
        self._env_file('QUANT1X_PROBE="  quoted  "\nQUANT1X_SINGLE=\'single\'\n')
        self.assertEqual("quoted", dotenv.env("QUANT1X_PROBE"))
        self.assertEqual("single", dotenv.env("QUANT1X_SINGLE"))

    def test_inline_comment_export_prefix_and_bom(self) -> None:
        """BOM、`export ` 前缀与行内 `# 注释` 都要正确去掉
        """
        self._env_file("\ufeffexport QUANT1X_PROBE=token  # 备注\nQUANT1X_HASH=\"a#b/c\"\n")
        self.assertEqual("token", dotenv.env("QUANT1X_PROBE"))
        self.assertEqual("a#b/c", dotenv.env("QUANT1X_HASH"))

    def test_values_expand_both_forms(self) -> None:
        """`${NAME}` 与 `$NAME` 两种写法都换成引用到的取值"""
        self._env_file("QUANT1X_BASE=/srv/data\nQUANT1X_BRACED=${QUANT1X_BASE}/a\nQUANT1X_BARE=$QUANT1X_BASE/b\n")
        self.assertEqual("/srv/data/a", dotenv.env("QUANT1X_BRACED"))
        self.assertEqual("/srv/data/b", dotenv.env("QUANT1X_BARE"))

    def test_expansion_uses_the_merged_table(self) -> None:
        """展开取合并表: 主目录那份的值引用到的键被项目根盖过时, 用的是项目根的值
        """
        self._env_file("QUANT1X_BASE=/project\n")
        self._home_env_file("QUANT1X_BASE=/home\nQUANT1X_PROBE=${QUANT1X_BASE}/data\n")
        self.assertEqual("/project/data", dotenv.env("QUANT1X_PROBE"))

    def test_expansion_falls_back_to_os_env(self) -> None:
        """合并表里没有的引用退到系统环境变量
        """
        self._env_file("QUANT1X_PROBE=${QUANT1X_OS_PROBE}/x\n")
        with patch.dict(os.environ, {"QUANT1X_OS_PROBE": "/from-os"}, clear=False):
            self.assertEqual("/from-os/x", dotenv.env("QUANT1X_PROBE"))

    def test_project_root_wins_per_key(self) -> None:
        """两份 `.env` 逐键合并: 同名键取项目根, 只有主目录有的键取主目录
        """
        self._env_file("QUANT1X_PROBE=from-root\nQUANT1X_ONLY_ROOT=root\n")
        self._home_env_file("QUANT1X_PROBE=from-home\nQUANT1X_ONLY_HOME=home\n")
        self.assertEqual("from-root", dotenv.env("QUANT1X_PROBE"))
        self.assertEqual("root", dotenv.env("QUANT1X_ONLY_ROOT"))
        self.assertEqual("home", dotenv.env("QUANT1X_ONLY_HOME"))

    def test_home_dotenv_used_when_project_root_has_none(self) -> None:
        """项目根没有 `.env` 时, 主目录那份就是全部依据
        """
        self._home_env_file("QUANT1X_PROBE=from-home\n")
        self.assertEqual("from-home", dotenv.env("QUANT1X_PROBE"))

    def test_home_only_when_project_root_is_unknown(self) -> None:
        """定位不到项目根时只读主目录, 不拿工作目录附近的 `.env` 顶替
        """
        self._home_env_file("QUANT1X_PROBE=from-home\n")
        with patch.object(dotenv, "_project_root", return_value=None):
            self.assertEqual("from-home", dotenv.env("QUANT1X_PROBE"))

    def test_project_root_is_located_upwards_from_cwd(self) -> None:
        """起点在深目录时, 按项目根标记向上定位, 取项目根那份 `.env`
        """
        self._env_file("QUANT1X_PROBE=root\n")
        nested = self.root / "a" / "b"
        nested.mkdir(parents=True)
        self._home_env_file("QUANT1X_PROBE=home\n")
        with patch.object(dotenv, "_starts", return_value=[nested]):
            self.assertEqual("root", dotenv.env("QUANT1X_PROBE"))

    def test_untouched_dotenv_is_not_reread(self) -> None:
        """`.env` 没动过时只查缓存, 不再重新读盘"""
        self._env_file("QUANT1X_PROBE=first\n")
        self.assertEqual("first", dotenv.env("QUANT1X_PROBE"))
        with patch.object(dotenv, "_parse", wraps=dotenv._parse) as parsed:
            self.assertEqual("first", dotenv.env("QUANT1X_PROBE"))
            self.assertEqual(0, parsed.call_count)

    def test_changed_dotenv_is_reloaded(self) -> None:
        """`.env` 改过之后不必重启, 下一次取值即读到新值"""
        path = self.root / ".env"
        self._env_file("QUANT1X_PROBE=first\n")
        self.assertEqual("first", dotenv.env("QUANT1X_PROBE"))
        self._env_file("QUANT1X_PROBE=second\n")
        # 显式把 mtime 往前推, 免得同一次测试内的两次写盘落在同一个时间刻度上
        os.utime(path, ns=(os.stat(path).st_mtime_ns + 10 ** 9,) * 2)
        self.assertEqual("second", dotenv.env("QUANT1X_PROBE"))

    def test_appearing_dotenv_is_noticed(self) -> None:
        """起初没有 `.env`, 运行期新建后也能被发现"""
        self.assertEqual("", dotenv.read_dotenv("QUANT1X_PROBE"))
        self._env_file("QUANT1X_PROBE=late\n")
        self.assertEqual("late", dotenv.env("QUANT1X_PROBE"))

    def test_blank_key_is_empty(self) -> None:
        """空键与全空白键一律给空串
        """
        self.assertEqual("", dotenv.read_dotenv(""))
        self.assertEqual("", dotenv.read_dotenv("   "))


if __name__ == "__main__":
    unittest.main()
