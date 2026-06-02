# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import click
from typing import List, Optional
from concurrent.futures import ThreadPoolExecutor, as_completed
from rich.console import Console
from rich.progress import (
    Progress,
    SpinnerColumn,
    TextColumn,
    BarColumn,
    TaskProgressColumn,
    TimeElapsedColumn,
)
from quant1x.data.adapter import DataAdapter
from quant1x.data import adapter
from quant1x.data.meta.instrument import Instrument
from quant1x.contrib.data.tdx import *  # 导入TDX数据源
from quant1x.contrib.data.tdx.datasource import TdxDataSource

console = Console()

# ---------------------------------------------------------------------------
# 并发控制
# ---------------------------------------------------------------------------
MAX_WORKERS = 4  # 单 plugin 内并发下载线程数

# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------
@click.group()
def cli():
    pass

def parse_comma_separated(ctx, param, value):
    """将逗号分隔字符串转为 list[str]，空值返回 None"""
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
    """解析用户选项，返回需要运行的 plugin 列表。"""
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
    """加载全市场股票列表（A股）。"""
    ds = TdxDataSource()
    return ds.list_instruments("all")

# ---------------------------------------------------------------------------
# 命令: update
# ---------------------------------------------------------------------------
@click.command()
@click.option(
    "--all",
    "update_all",
    is_flag=True,
    default=False,
    help="Update all base and feature components.",
)
@click.option(
    "--base",
    default=None,
    callback=parse_comma_separated,
    help="Comma-separated base names to update (e.g., 'xdxr,kline').",
)
@click.option(
    "--feature",
    default=None,
    callback=parse_comma_separated,
    help="Comma-separated feature names to update (e.g., 'ma,macd').",
)
def update(update_all: bool, base: Optional[List[str]], feature: Optional[List[str]]):
    plugins = _resolve_plugins(update_all, base, feature)
    if not plugins:
        console.print("[yellow]Nothing to update.[/yellow]")
        return

    instruments = _load_instruments()
    total_instruments = len(instruments)
    console.print(f"[cyan]Loaded {total_instruments} instruments.[/cyan]")
    console.print(
        f"[cyan]Running {len(plugins)} task(s):[/cyan] "
        + ", ".join(f"{p.key()}:{p.name()}" for p in plugins)
    )

    # ---- 单 Progress 实例内管理两级进度条 ----
    with Progress(
        SpinnerColumn(),
        TextColumn("[progress.description]{task.description}"),
        BarColumn(bar_width=30),
        TaskProgressColumn(),
        TextColumn("•"),
        TimeElapsedColumn(),
        console=console,
        refresh_per_second=8,  # Rich 内置渲染节流，无需手动 throttle
    ) as progress:
        # 外层任务：plugin 级别
        plugin_task = progress.add_task(
            "[bold]Overall Progress", total=len(plugins)
        )

        for plugin in plugins:
            kind = plugin.key()
            name = plugin.name()
            desc = f"{kind} ({name})"

            # 内层任务：股票级别
            stock_task = progress.add_task(
                f"  [cyan]{desc}[/cyan]", total=total_instruments
            )

            def _update_one(inst: Instrument) -> bool:
                """更新单只股票，成功返回 True。"""
                try:
                    # plugin.update() 签名有两类：Instrument 或 str(code)
                    try:
                        plugin.update(inst)
                    except TypeError:
                        plugin.update(inst.ticker)  # type: ignore[arg-type]
                    progress.update(stock_task, advance=1)
                    return True
                except Exception:
                    progress.update(stock_task, advance=1)
                    return False

            success = 0
            failed = 0
            with ThreadPoolExecutor(max_workers=MAX_WORKERS) as executor:
                futures = {executor.submit(_update_one, inst): inst for inst in instruments}
                for future in as_completed(futures):
                    if future.result():
                        success += 1
                    else:
                        failed += 1

            # 股票级别任务完成，隐藏
            progress.update(stock_task, visible=False)

            # 推进外层 plugin 进度
            progress.update(plugin_task, advance=1, description=f"[bold]Overall Progress[/bold] ({success}/{total_instruments} {kind})")

    console.print("[green]✅ All plugins completed![/green]")

cli.add_command(update)

if __name__ == "__main__":
    cli()