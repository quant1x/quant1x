# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

"""跨语言对拍样本的守门用例: 已签入的 fixture 必须与 Python spec 逐字节一致."""

import importlib.util
import tempfile
import unittest
from pathlib import Path

_FIXTURE_ROOT = Path(__file__).resolve().parent / "fixtures" / "formula"
_GENERATOR_PATH = _FIXTURE_ROOT / "generate.py"


def _load_generator():
    """**加载样本生成器**: 按文件路径加载，避免把 fixture 目录做成包

    :return: 生成器模块
    """
    spec = importlib.util.spec_from_file_location("formula_fixture_generator", _GENERATOR_PATH)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class FixtureFreshnessTest(unittest.TestCase):
    """fixture 与 Python spec 的一致性."""

    @classmethod
    def setUpClass(cls):
        """**准备生成器**
        """
        cls.generator = _load_generator()

    def test_formula_cases_are_reproducible(self):
        """**用例**: 重新生成的 cases.csv 与签入文件逐字节一致
        """
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.generator.generate_cases(root)
            produced = (root / "cases.csv").read_text(encoding="utf-8")
        committed = (_FIXTURE_ROOT / "cases.csv").read_text(encoding="utf-8")
        self.assertEqual(produced, committed)

    def test_indicator_fixtures_are_reproducible(self):
        """**用例**: 重新生成的指标样本与签入文件逐字节一致
        """
        names = ["bars", "cdtd", "f89k", "hjk", "ma1x", "p2fb"]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.generator.generate_indicators(root)
            produced = {
                name: (root / "indicators" / f"{name}.csv").read_text(encoding="utf-8")
                for name in names
            }
        for name in names:
            with self.subTest(name=name):
                committed = (_FIXTURE_ROOT / "indicators" / f"{name}.csv").read_text(encoding="utf-8")
                self.assertEqual(produced[name], committed)


if __name__ == "__main__":
    unittest.main()
