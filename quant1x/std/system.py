# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
import sys
from typing import Tuple

from matplotlib.pylab import f
from . import filesystem

def application() -> Tuple[str, str, str]:
    """
    获取当前应用程序的路径信息。

    Returns:
        Tuple[str, str, str]: 返回包含目录路径、文件名(不含扩展名)和扩展名的元组
    """
    # 处理 python -m 方式运行的情况
    if sys.argv[0] == '-m':
        # 找到 __main__ 模块
        main_module = sys.modules.get('__main__')
        if main_module:
            # 检查 __main__ 的 __package__ 属性
            package_name = getattr(main_module, '__package__', '')
            if package_name:
                # __package__ 会包含实际的模块路径，如 quant1x.log.logger
                # 从包名提取文件名（最后一部分）
                filename = package_name.split('.')[-1]
                dir_path = os.getcwd()
                return dir_path, filename, '.py'

        # 回退：从 sys.modules 中查找最匹配的模块
        # 查找包含项目包名且最深的模块
        target_module = None
        max_depth = 0
        for module_name, module in sys.modules.items():
            if module_name.startswith('quant1x') and hasattr(module, '__file__') and module.__file__:
                depth = module_name.count('.')
                if depth > max_depth:
                    max_depth = depth
                    target_module = (module_name, module)

        if target_module:
            module_name, module = target_module
            app_path = os.path.abspath(module.__file__)
            dir_path, full_filename = os.path.split(app_path)
            filename, ext = os.path.splitext(full_filename)
            # 如果是 __init__.py，使用模块名的最后一部分
            if filename == '__init__':
                filename = module_name.split('.')[-1]
            return dir_path, filename, ext

        # 如果找不到，使用默认值
        dir_path = os.getcwd()
        filename = 'app'
        return dir_path, filename, '.py'

    # 正常情况
    app_path = os.path.abspath(sys.argv[0])

    dir_path, full_filename = os.path.split(app_path)
    filename, ext = os.path.splitext(full_filename)

    # 如果 filename 是 __main__，尝试从运行命令中提取模块名
    if filename == '__main__' and len(sys.argv) > 1:
        # python -m module.name.submodule
        module_name = sys.argv[1]
        # 取最后一部分作为文件名
        filename = module_name.split('.')[-1]
        ext = '.py'

    return dir_path, filename, ext


def getuser() -> str:
    """
    获取当前用户名。

    Returns:
        str: 当前用户名
    """
    home_dir = filesystem.homedir()
    _, username = os.path.split(home_dir)
    return username

def env(key: str) -> str:
    """
    获取环境变量
    """
    value = os.getenv(key, '')
    return value.strip()

def read_dotenv(key: str) -> str:
    """
    只读地从项目附近的 .env 文件读取指定的环境变量 `key`（不写入 os.environ）。
    搜索顺序：从当前工作目录开始，逐级向上查找每一级目录下的 `.env`，直到根目录；
    如果都找不到，再使用 dotenv.find_dotenv() 作为最后的回退。
    返回值：如果找不到或解析失败，返回空字符串。
    """
    if not key:
        return ''

    def find_env_upwards(start_path) -> 'str | None':
        """从 start_path 开始，向上逐级查找 `.env` 文件，找到则返回该文件的绝对路径字符串；找不到返回 None。"""
        try:
            from pathlib import Path
            p = Path(start_path)
            for d in [p] + list(p.parents):
                env_file = d / '.env'
                #print('checking for .env at', env_file)
                if env_file.is_file():
                    return str(env_file)
        except Exception:
            pass
        return None

    try:
        import dotenv
        from pathlib import Path

        # 按用户要求：先用 cmd（运行时的 cwd），然后用 python 后面的脚本文件的绝对路径（sys.argv[0]）
        starts = [Path.cwd()]
        try:
            entry = Path(sys.argv[0]).resolve()
            if entry.is_file():
                starts.append(entry.parent)
            else:
                # 回退到本模块文件夹
                starts.append(Path(__file__).absolute().parent)
        except Exception:
            starts.append(Path(__file__).absolute().parent)

        for start in starts:
            env_path = find_env_upwards(start)
            if env_path:
                try:
                    vals = dotenv.dotenv_values(env_path)
                    raw = vals.get(key)
                    if raw:
                        return str(raw).strip().strip('"\'')
                except Exception:
                    # 解析失败则继续到下一个起点或回退策略
                    continue

        # 最后回退到 dotenv.find_dotenv()
        found = dotenv.find_dotenv()
        if found:
            try:
                vals = dotenv.dotenv_values(found)
                raw = vals.get(key)
                if raw:
                    return str(raw).strip().strip('"\'')
            except Exception:
                pass
    except Exception:
        pass
    return ''
