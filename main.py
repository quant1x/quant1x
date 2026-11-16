# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

from __future__ import annotations

import os
import signal
import sys
from typing import List, Optional
from concurrent.futures import ThreadPoolExecutor, as_completed, Future

from tqdm import tqdm

from quant1x.base.argparser import Namespace, Parser
from quant1x.data.adapter import DataAdapter
from quant1x.data import adapter
from quant1x.data.meta.instrument import Instrument
from quant1x.contrib.data.tdx import *  # 导入TDX数据源
from quant1x.contrib.data.tdx.datasource import TdxDataSource

# 全局中断标志
_interrupted = False

# ---------------------------------------------------------------------------
# 并发控制
# ---------------------------------------------------------------------------
MAX_WORKERS = 4  # 单 plugin 内并发下载线程数

# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------
def build_parser() -> Parser:
    """构建命令行解析器: 根命令 + update 子命令"""
    parser = Parser(os.path.basename(sys.argv[0]))
    subparsers = parser.add_subparsers(dest="command")
    update_parser = subparsers.add_parser("update", about="Update cached data (base / features)")
    update_parser.add_argument(
        "--all",
        action="store_true",
        dest="update_all",
        help="Update all base and feature components.",
    )
    update_parser.add_argument(
        "--base",
        default=None,
        help="Comma-separated base names to update (e.g., 'xdxr,bar').",
    )
    update_parser.add_argument(
        "--feature",
        default=None,
        help="Comma-separated feature names to update (e.g., 'ma,macd').",
    )
    return parser

def parse_comma_separated(value):
    """将逗号分隔字符串转为 list[str], 空值返回 None"""
    if not value:
        return None
    parts = [v.strip() for v in value.split(",") if v.strip()]
    return parts if parts else None

# ---------------------------------------------------------------------------
# 辅助
# ---------------------------------------------------------------------------
def _resolve_plugins(
    update_all: bool,
    base: Optional[List[str]],
    feature: Optional[List[str]],
) -> List[DataAdapter]:
    """解析用户选项, 返回需要运行的 plugin 列表. """
    if update_all is None and base is None and feature is None:
        return []

    if update_all:
        return adapter.plugins(0)

    plugins: List[DataAdapter] = []
    if base:
        plugins.extend(adapter.plugins_with_name(adapter.PLUGIN_MASK_BASEDATA, base))
    if feature:
        plugins.extend(adapter.plugins_with_name(adapter.PLUGIN_MASK_FEATURE, feature))
    return plugins

def _load_instruments() -> List[Instrument]:
    """加载全市场股票列表(A股). """
    ds = TdxDataSource()
    return ds.list_instruments("all")

# ---------------------------------------------------------------------------
# 命令: update
# ---------------------------------------------------------------------------
def _signal_handler(signum, frame):
    """处理 SIGINT (CTRL+C), 设置中断标志. """
    global _interrupted
    _interrupted = True

def run_update(namespace: Namespace) -> None:
    """执行 update 子命令"""
    update_all = namespace.update_all
    base = parse_comma_separated(namespace.base)
    feature = parse_comma_separated(namespace.feature)
    global _interrupted
    _interrupted = False

    # 注册 CTRL+C 处理器
    original_handler = signal.signal(signal.SIGINT, _signal_handler)

    try:
        plugins = _resolve_plugins(update_all, base, feature)
        if not plugins:
            print("Nothing to update.")
            return

        instruments = _load_instruments()
        total_instruments = len(instruments)
        print(f"Loaded {total_instruments} instruments.")
        print(
            f"Running {len(plugins)} task(s): "
            + ", ".join(f"{p.key()}:{p.name()}" for p in plugins)
        )

        # 外层进度条: plugin 级别; 内层进度条: 单 plugin 内的股票级别
        plugin_bar = tqdm(plugins, desc="Overall Progress", unit="plugin")
        for plugin in plugin_bar:
            if _interrupted:
                break

            kind = plugin.key()
            name = plugin.name()
            desc = f"{kind} ({name})"

            def _update_one(inst: Instrument) -> bool:
                """更新单只股票, 成功返回 True. """
                if _interrupted:
                    return False
                try:
                    # plugin.update() 签名有两类: Instrument 或 str(code)
                    try:
                        plugin.update(inst)
                    except TypeError:
                        plugin.update(inst.ticker)  # type: ignore[arg-type]
                    return True
                except Exception:
                    return False

            success = 0
            failed = 0
            stock_bar = tqdm(total=total_instruments, desc=desc, unit="stock", leave=False)
            with ThreadPoolExecutor(max_workers=MAX_WORKERS) as executor:
                futures: dict[Future, Instrument] = {
                    executor.submit(_update_one, inst): inst for inst in instruments
                }
                try:
                    for future in as_completed(futures):
                        if _interrupted:
                            # 取消所有未完成的 future
                            for f in futures:
                                f.cancel()
                            break
                        try:
                            if future.result():
                                success += 1
                            else:
                                failed += 1
                        except Exception:
                            failed += 1
                        stock_bar.update(1)
                except KeyboardInterrupt:
                    _interrupted = True
                    for f in futures:
                        f.cancel()
            stock_bar.close()

            if _interrupted:
                plugin_bar.set_description(f"Overall Progress (interrupted at {kind})")
                break

            plugin_bar.set_description(
                f"Overall Progress ({success}/{total_instruments} {kind})"
            )
        plugin_bar.close()

        if _interrupted:
            print("Update interrupted by user.")
        else:
            print("All plugins completed!")
    finally:
        # 恢复原始信号处理器
        signal.signal(signal.SIGINT, original_handler)

def main(argv=None) -> int:
    """入口: 解析命令行并分发子命令"""
    parser = build_parser()
    namespace = parser.parse_or_exit(argv)
    command = getattr(namespace, "command", None)
    if command == "update":
        run_update(namespace)
    else:
        parser.print_help()
    return 0


if __name__ == "__main__":
    sys.exit(main())