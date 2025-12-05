#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
@Project : quant1x
@Package : quant1x.config
@File    : config.py
@Author  : wangfeng
@Date    : 2025/9/15 16:37
@Desc    : 加载配置文件, 支持.env指定工作目录
"""
import os
import yaml
import dotenv
from yarg import get
from quant1x import system

# 加载环境变量
dotenv.load_dotenv()

default_quant1x_work = 'quant1x' # 默认工作目录关键词

def get_quant1x_work_keyword() -> str:
    """
    获取quant1x工作目录的关键词
    :return:
    """
    quant1x_work_env = system.env('QUANT1X_WORK')
    if quant1x_work_env and len(quant1x_work_env) > 0:
        return quant1x_work_env

    # fallback: read project .env in a read-only way via system helper (does not mutate os.environ)
    try:
        val = system.read_dotenv('QUANT1X_WORK')
        if val and len(val) > 0:
            return val
    except Exception:
        pass

    return ''

def get_quant1x_config_filename() -> str:
    """
    获取quant1x.yaml文件路径
    优先级: QUANT1X_WORK指定的目录 > ~/runtime/etc > 默认~/.quant1x
    :return: 配置文件路径
    """
    default_config_filename = 'quant1x.yaml'
    user_home = system.homedir()
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
            quant1x_root = os.path.join(user_home, f'.{default_quant1x_work}')
            yaml_filename = os.path.join(quant1x_root, default_config_filename)

    yaml_filename = os.path.expanduser(yaml_filename)
    return yaml_filename


# 安全加载YAML配置
def load_config(file_path: str) -> dict:
    """安全加载YAML配置"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            config = yaml.safe_load(f) or {}
            return config
    except FileNotFoundError:
        raise ValueError(f"配置文件 {file_path} 不存在")
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
        """初始化配置"""
        self.__home_path = system.homedir()
        self.__config_filename = get_quant1x_config_filename()
        self.__config = load_config(self.__config_filename)
        self.__work_keyword = get_quant1x_work_keyword()

        # 初始化路径
        # 如果 work_keyword 为空（表示未通过环境变量指定），
        # 在构建默认主路径时应回退到默认关键词，避免生成类似 '~/.\' 的路径。
        effective_work = self.__work_keyword if self.__work_keyword else default_quant1x_work
        self.__default_main_path = os.path.join(self.__home_path, f'.{effective_work}')

        self.meta_path = os.path.join(self.__default_main_path, 'meta')
        """str: 元数据路径"""

        self.data_path = self.__config.get('basedir', '').strip()
        """str: 数据目录 """

        if not self.data_path:
            self.data_path = os.path.join(self.__default_main_path, 'data')
        self.data_path = os.path.expanduser(self.data_path)
        # 数据路径
        self.kline_path = os.path.join(self.data_path, 'day')
        """str: K线路径 """


# 创建配置单例
quant1x_config = Quant1XConfig()

def get_historical_trade_filename(code: str, date: str) -> str:
    """
    获取历史成交记录文件路径
    目录结构: ${trans}/${YYYY}/${YYYYMMDD}/${SecurityCode}.csv
    """
    date_str = date.replace('-', '').replace('/', '')
    year = date_str[:4]
    base_path = os.path.join(quant1x_config.data_path, 'trans')
    return os.path.join(base_path, year, date_str, f"{code}.csv")

def get_xdxr_path() -> str:
    """获取除权除息文件路径"""
    return os.path.join(quant1x_config.data_path, 'xdxr')

def get_xdxr_filename(code: str) -> str:
    """
    获取除权除息文件路径
    目录结构: ${xdxr}/${subpath}/${SecurityCode}.csv
    subpath: code[:-3]
    """
    if len(code) <= 3:
        sub = ""
    else:
        sub = code[:-3]
        
    base_path = get_xdxr_path()
    return os.path.join(base_path, sub, f"{code}.csv")

def get_day_path() -> str:
    """获取日K线文件路径"""
    return quant1x_config.kline_path

def get_kline_filename(code: str, forward: bool = True) -> str:
    """
    获取日K线文件路径
    目录结构: ${day}/${subpath}/${SecurityCode}.${ext}
    subpath: code[:-3]
    ext: csv if forward else raw
    """
    if len(code) <= 3:
        sub = ""
    else:
        sub = code[:-3]
        
    base_path = get_day_path()
    ext = "csv" if forward else "raw"
    return os.path.join(base_path, sub, f"{code}.{ext}")

from quant1x import std

def get_holding_path() -> str:
    return os.path.join(quant1x_config.data_path, "holding")

def cache_id_path(code: str) -> str:
    """
    Generate cache ID path.
    SH600000 -> SH600/SH600000
    """
    if len(code) <= 3:
        return code
    
    prefix = code[:-3]
    return os.path.join(prefix, code)

def top10_holders_filename(code: str, date_str: str) -> str:
    quarter, _, _ = std.get_quarter_by_date(date_str)
    id_path = cache_id_path(code)
    return os.path.join(get_holding_path(), quarter, f"{id_path}.csv")

def quarterly_cache_path(date: str) -> str:
    quarter, _, _ = std.get_quarter_by_date(date)
    return os.path.join(quant1x_config.data_path, "infoq", quarter)

def quarterly_filename(date: str, keyword: str) -> str:
    return os.path.join(quarterly_cache_path(date), f"{keyword}.csv")

def reports_filename(date: str) -> str:
    return quarterly_filename(date, "reports")

