# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
import sys

def _find_package_name(start_dir: str) -> str:
    """从 start_dir 向上查找 LICENSE 文件, 返回其父目录名作为 package_name"""
    current = os.path.abspath(start_dir)
    for _ in range(10):  # 最多向上查找10层
        license_path = os.path.join(current, "LICENSE")
        if os.path.isfile(license_path):
            return os.path.basename(current)
        parent = os.path.dirname(current)
        if parent == current:  # 已到根目录
            break
        current = parent
    raise FileNotFoundError(
        "无法找到 LICENSE 文件, 请确保在项目根目录或其子目录下运行此脚本. "
        f"当前起始目录: {os.path.abspath(start_dir)}"
    )


def get_module_path(file_path: str) -> str:
    """将文件路径转换为模块路径"""
    # 获取工作目录(项目根目录)
    workspace_root = os.getcwd()
    package_name = _find_package_name(workspace_root)
    # 标准化路径
    file_path = os.path.normpath(file_path)
    
    print(f"\033[90m[run_module] 工作目录: {workspace_root}\033[0m")
    print(f"\033[90m[run_module] 包名: {package_name}\033[0m")
    print(f"\033[90m[run_module] 文件路径: {file_path}\033[0m")
    top_level_package = os.path.basename(workspace_root)
    print(f"\033[90m[run_module] 顶级包名: {top_level_package}\033[0m")
    if top_level_package != package_name:
        # 如果顶级包名和包名不同, 则用上级目录作为顶级包名的目录
        workspace_root = os.path.dirname(workspace_root)
    print(f"\033[90m[run_module] 确定路径: {workspace_root}\033[0m")
    
    # 计算相对路径
    try:
        rel_path = os.path.relpath(file_path, workspace_root)
        print(f"\033[90m[run_module] 相对路径: {rel_path}\033[0m")
    except ValueError:
        # 如果不在同一个驱动器
        print("\033[90m[run_module] ⚠️ 警告: 文件与工作目录不在同一驱动器\033[0m")
        # 使用绝对路径
        rel_path = file_path
    
    # 转换为模块路径
    if rel_path.endswith('.py'):
        rel_path = rel_path[:-3]
    
    # 处理路径分隔符
    module_path = rel_path.replace('\\', '.').replace('/', '.')
    
    # 处理 __init__.py
    if module_path.endswith('.__init__'):
        module_path = module_path[:-9]
    
    # 清理开头的点
    while module_path.startswith('.'):
        module_path = module_path[1:]
    
    print(f"\033[90m[run_module] 模块路径: {module_path}\033[0m")
    return module_path

def main():
    if len(sys.argv) < 2:
        print("\033[90m[run_module] 使用方法: python run_module.py <python文件> [参数...]\033[0m")
        sys.exit(1)
    
    file_path = sys.argv[1]
    extra_args = sys.argv[2:]
    
    # 获取模块路径
    module_path = get_module_path(file_path)
    
    if not module_path:
        print("\033[90m[run_module] ❌ 无法确定模块路径\033[0m")
        sys.exit(1)
    
    # 执行命令
    cmd = [sys.executable, "-m", module_path] + extra_args
    print(f"\033[90m[run_module] 执行命令: {' '.join(cmd)}\033[0m")
    print("\033[90m" + "-" * 50 + "\033[0m")
    
    # 运行
    result = os.system(' '.join(cmd))
    sys.exit(result >> 8 if result else 0)

if __name__ == "__main__":
    main()