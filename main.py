# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import click
from rich.console import Console
from rich.progress import Progress, SpinnerColumn, TimeElapsedColumn
from quant1x.cli.progress import ThrottledTask, ThrottledMultiProgress
from quant1x.data.adapter import DataAdapter, FeatureAdapter
from quant1x.data import adapter
from quant1x.contrib.data.tdx import * # 导入TDX数据源

console = Console()

@click.group()
def cli():
    pass

def parse_comma_separated(ctx, param, value):
    """将逗号分隔字符串转为 list[str]，空值返回 None"""
    if not value:
        return None
    # 去除空格，过滤空项
    parts = [v.strip() for v in value.split(",") if v.strip()]
    return parts if parts else None

@click.command()
@click.option(
    "--all",
    "update_all",
    is_flag=True,
    default=False,
    help="Update all base and feature components (default if no options given)."
)
@click.option(
    "--base",
    default=None,
    callback=parse_comma_separated,
    help="Comma-separated base names to update (e.g., 'xdxr,kline'). Ignored if --all is used."
)
@click.option(
    "--feature",
    default=None,
    callback=parse_comma_separated,
    help="Comma-separated feature names to update (e.g., 'ma,macd'). Ignored if --all is used."
)
def update(update_all: bool, base: list[str] | None, feature: list[str] | None):
    plugins = []
    if update_all is None and base is None and feature is None:
        console.print("[red]Error: No options given.[/red]")
        return
    if update_all:
        plugins = adapter.plugins(0)
    else:
        if base is not None:
            plugins.extend(adapter.plugins_with_name(adapter.PLUGIN_MASK_BASEDATA, base))
        if feature is not None:
            plugins.extend(adapter.plugins_with_name(adapter.PLUGIN_MASK_FEATURE, feature))
    if not plugins:
        console.print("[yellow]Nothing to update.[/yellow]")
        return
    console.print(f"[cyan]Running {len(plugins)} tasks:[/cyan] {[f'{p.key()}:{p.name()}' for p in plugins]}")
    # for p in plugins:
    #     p.update()
    

    with Progress(
        SpinnerColumn(),
        *Progress.get_default_columns(),
        TimeElapsedColumn(),
        console=console,
    ) as progress:
        # TODO: 这里只是一个演示, 并不会真正的更新数据
        throttled = ThrottledMultiProgress(progress)
        total = 1000000
        task_refs = {}
        for p in plugins:
            kind = p.key()
            name = p.name()
            tid = throttled.add_task(f"Updating {kind}: {name}", total=total)
            task_refs[(kind, name)] = tid

        for _ in range(total):
            for key in task_refs:
                throttled.update(task_refs[key], advance=1)
            #import time
            #time.sleep(0.01)
        throttled.close()

    console.print("[green]✅ Done![/green]")
cli.add_command(update)

if __name__ == "__main__":
    cli()