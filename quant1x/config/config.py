# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
from typing import Tuple
import yaml
import dotenv
import logging
from datetime import datetime

from quant1x.std import filesystem as fs, system as sys, strings
# 加载环境变量
dotenv.load_dotenv()

logger = logging.getLogger(__name__)

default_workspace_path = 'q1x'
"""默认工作目录关键词"""

def get_quant1x_env(key: str) -> str:
    key = key.strip()
    # 1. 尝试从开发环境变量文件.env中读取
    try:
        val = sys.read_dotenv(key)
        if val and len(val) > 0:
            return val
    except Exception:
        pass
    
    # 2. 尝试从系统环境变量中读取
    val = sys.env(key)
    if val and len(val) > 0:
        return val

    # 3. 回退到默认值

    return ''

def get_quant1x_work_keyword() -> str:
    """
    获取quant1x工作目录的关键词
    :return:
    """
    quant1x_work_env = sys.env('QUANT1X_WORK')
    if quant1x_work_env and len(quant1x_work_env) > 0:
        return quant1x_work_env

    # fallback: read project .env in a read-only way via system helper (does not mutate os.environ)
    try:
        val = sys.read_dotenv('QUANT1X_WORK')
        if val and len(val) > 0:
            return val
    except Exception:
        pass

    return ''

def get_quant1x_config_filename() -> Tuple[str, str]:
    """
    获取quant1x.yaml文件路径
    优先级: QUANT1X_WORK指定的目录 > ~/runtime/etc > 默认~/.quant1x
    :return: 配置文件路径
    """
    default_config_filename = 'quant1x.yaml'
    user_home = fs.homedir()
    quant1x_work = get_quant1x_work_keyword()

    if quant1x_work:
        # 使用环境变量指定的工作目录
        quant1x_root = os.path.join(user_home, f'.{quant1x_work}')
        yaml_filename = os.path.join(quant1x_root, default_config_filename)
    else:
        # 检查 ~/runtime/etc/quant1x.yaml
        yaml_filename = os.path.join(user_home, 'runtime', 'etc', default_config_filename)
        if not os.path.isfile(yaml_filename):
            # 回退到默认 ~/.quant1x/quant1x.yaml
            quant1x_root = os.path.join(user_home, f'.{default_workspace_path}')
            yaml_filename = os.path.join(quant1x_root, default_config_filename)

    yaml_filename = fs.expand_user(yaml_filename)
    return yaml_filename, quant1x_work


# 安全加载YAML配置
def load_config(file_path: str) -> dict:
    """
    安全加载YAML配置文件
    
    Args:
        file_path (str): YAML配置文件的路径
        
    Returns:
        dict: 解析后的配置字典, 如果文件不存在则返回空字典
        
    Raises:
        ValueError: 当YAML格式错误或其他加载错误时抛出
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            config = yaml.safe_load(f) or {}
            return config
    except FileNotFoundError:
        logger.warning(f"配置文件 {file_path} 不存在, 使用默认配置")
        return {}
    except yaml.YAMLError as e:
        raise ValueError(f"YAML格式错误: {str(e)}")
    except Exception as e:
        raise ValueError(f"加载配置失败: {str(e)}")


class Quant1XConfig:
    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance._initialize()
        return cls._instance

    def _initialize(self):
        """
        初始化配置, 包括加载配置文件, 设置路径和调试模式
        
        主要功能: 
        - 加载或创建默认配置文件
        - 设置工作路径, 数据路径, 日志路径等
        - 解析调试模式设置
        - 确保所有路径存在并已展开用户路径
        
        属性初始化: 
        - debug (bool): 是否启用调试模式
        - __home_path (str): 用户主目录路径
        - __config_filename (str): 配置文件路径
        - __config (dict): 加载的配置字典
        - __work_keyword (str): 工作目录关键字
        - __default_path (str): 默认路径(带~符号)
        - __default_main_path (str): 默认主路径(完整路径)
        - meta_path (str): 元数据存储路径
        - data_path (str): 数据存储路径
        - log_path (str): 日志存储路径
        - kline_path (str): K线数据存储路径
        
        处理逻辑: 
        1. 加载配置文件, 不存在则创建默认配置
        2. 解析调试模式设置, 支持多种格式
        3. 设置并验证各路径
        4. 确保配置文件目录存在
        """
        self.debug = False
        self.__home_path = fs.homedir()
        config_filename, quant1x_work = get_quant1x_config_filename()
        self.__config_filename = config_filename
        self.__config = load_config(self.__config_filename)
        self.__work_keyword = quant1x_work

        # 初始化路径
        # 如果 work_keyword 为空(表示未通过环境变量指定), 
        # 在构建默认主路径时应回退到默认关键词, 避免生成类似 '~/.\' 的路径. 
        effective_work = self.__work_keyword if self.__work_keyword else default_workspace_path
        self.__default_path = f'~/.{effective_work}'
        self.__default_main_path = os.path.join(self.__home_path, f'.{effective_work}')

        self.meta_path = os.path.join(self.__default_main_path, 'meta')
        """str: 元数据路径"""

        # 解析 debug, 兼容 bool/int/str 等类型
        # 开发环境.env优先
        # fallback: read project .env in a read-only way via system helper (does not mutate os.environ)
        dbg_str = get_quant1x_env('QUANT1X_DEBUG')
        dbg_str = dbg_str.strip()
        try:
            if len(dbg_str) > 0:
                self.debug = strings.str_to_bool(dbg_str)
            else:
                raw_debug = self.__config.get('debug', False)
                if isinstance(raw_debug, bool):
                    self.debug = raw_debug
                else:
                    dbg_str = str(raw_debug).strip()
                    self.debug = strings.str_to_bool(dbg_str)
        except Exception:
            self.debug = False

        # 解析 basedir, 保证为字符串并去除空白
        data_path_raw = self.__config.get('basedir', '') or ''
        self.data_path = str(data_path_raw).strip()
        """str: 数据目录 """

        if not self.data_path:
            self.data_path = self.__default_main_path
        self.data_path = fs.expand_user(self.data_path)

        # 解析 logdir, 保证为字符串并去除空白
        log_path_raw = self.__config.get('logdir', '') or ''
        self.log_path = str(log_path_raw).strip()
        """str: 日志目录 """

        if not self.log_path:
            self.log_path = os.path.join(self.data_path, 'logs')
        self.log_path = fs.expand_user(self.log_path)
        
        # 数据路径
        self.kline_path = os.path.join(self.data_path, 'day')
        """str: K线路径 """
        
        # 如果配置文件不存在, 则回写一个默认的 yaml 配置文件, 并添加注释
        try:
            if not os.path.isfile(self.__config_filename):
                # 使用已加载的配置作为默认, 否则使用基于主路径的默认 basedir
                cfg_dir = os.path.dirname(self.__config_filename)
                if cfg_dir and not os.path.isdir(cfg_dir):
                    os.makedirs(cfg_dir, exist_ok=True)

                # include creation timestamp in header
                created_ts = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                header = (
                    "# quant1x configuration file\n"
                    "# Generated by Quant1XConfig. Edit with care.\n"
                    f"# Created: {created_ts}\n"
                )
                try:
                    with open(self.__config_filename, 'w', encoding='utf-8') as wf:
                        wf.write(header)
                        wf.write("\n")
                        # 如果包含 debug, 则按用户期望的格式写入一行, 并附带注释
                        wf.write(f"debug: {'true' if self.debug else 'false'} # 是否启用调试模式\n")
                        # 如果包含 basedir, 则按用户期望的格式写入一行, 并附带注释
                        wf.write(f"basedir: {self.__default_path} # 数据路径\n")
                        wf.write(f"logdir: {self.__default_path}/logs # 日志路径\n")
                        
                    logger.info(f"配置文件 {self.__config_filename} 不存在, 已创建默认配置文件")
                except Exception as e:
                    logger.warning(f"创建默认配置文件失败: {e}")
        except Exception:
            # 不要阻塞初始化流程, 记录错误继续
            logger.exception("检查或创建配置文件时发生错误")

# 创建配置单例
base_config = Quant1XConfig()


# ===== 缓存路径工具函数 (对齐 C++ config/cache.cpp 和 Rust config.rs) =====

def _cache_id(code: str) -> str:
    """构建缓存ID: 市场缩写 + 纯代码, 如 'sh600000'"""
    from quant1x.data.market import detect_symbol
    inst = detect_symbol(code)
    market_code = inst.exchange.identifier.lower()
    return market_code + inst.ticker


def _cache_id_path(code: str) -> str:
    """code从后保留3位, 市场缩写+从头到倒数第3的代码, 确保每个目录只有000~999个代码"""
    N = 3
    cache_id = _cache_id(code)
    if len(cache_id) <= N:
        return cache_id
    prefix = cache_id[:len(cache_id) - N]
    return f"{prefix}/{cache_id}"


def top10_holders_filename(code: str, date: str) -> str:
    """前十大流通股股东缓存文件名 (模块级函数, 供 contrib 层使用)"""
    from quant1x.std.time import get_quarter_by_date
    id_path = _cache_id_path(code)
    quarter_str, _, _ = get_quarter_by_date(date)
    holding_path = os.path.join(base_config.data_path, "holding")
    full_path = os.path.join(holding_path, quarter_str, f"{id_path}.csv")
    os.makedirs(os.path.dirname(full_path), exist_ok=True)
    return full_path


def reports_filename(date: str) -> str:
    """季报缓存文件名 (模块级函数, 供 contrib 层使用)"""
    from quant1x.std.time import get_quarter_by_date
    quarter_str, _, _ = get_quarter_by_date(date)
    path = os.path.join(base_config.data_path, "infoq", quarter_str)
    os.makedirs(path, exist_ok=True)
    return os.path.join(path, "reports.csv")
