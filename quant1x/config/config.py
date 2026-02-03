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
from typing import Tuple
import yaml
import dotenv
import logging
from yarg import get
from quant1x import system
from quant1x.std import filesystem
from datetime import datetime

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

def get_quant1x_config_filename() -> Tuple[str, str]:
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

    yaml_filename = filesystem.expand_user(yaml_filename)
    return yaml_filename, quant1x_work


# 安全加载YAML配置
def load_config(file_path: str) -> dict:
    """安全加载YAML配置"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            config = yaml.safe_load(f) or {}
            return config
    except FileNotFoundError:
        logging.warning(f"配置文件 {file_path} 不存在，使用默认配置")
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
        """初始化配置"""
        self.debug = False
        self.__home_path = system.homedir()
        config_filename, quant1x_work = get_quant1x_config_filename()
        self.__config_filename = config_filename
        self.__config = load_config(self.__config_filename)
        self.__work_keyword = quant1x_work

        # 初始化路径
        # 如果 work_keyword 为空（表示未通过环境变量指定），
        # 在构建默认主路径时应回退到默认关键词，避免生成类似 '~/.\' 的路径。
        effective_work = self.__work_keyword if self.__work_keyword else default_quant1x_work
        self.__default_path = f'~/.{effective_work}'
        self.__default_main_path = os.path.join(self.__home_path, f'.{effective_work}')

        self.meta_path = os.path.join(self.__default_main_path, 'meta')
        """str: 元数据路径"""

        # 解析 debug，兼容 bool/int/str 等类型
        raw_debug = self.__config.get('debug', False)
        if isinstance(raw_debug, bool):
            self.debug = raw_debug
        else:
            try:
                dbg_str = str(raw_debug).strip()
                self.debug = dbg_str.lower() in ['true', '1', 'yes', 'on']
            except Exception:
                self.debug = False

        # 解析 basedir，保证为字符串并去除空白
        data_path_raw = self.__config.get('basedir', '') or ''
        self.data_path = str(data_path_raw).strip()
        """str: 数据目录 """

        if not self.data_path:
            self.data_path = self.__default_main_path
        self.data_path = filesystem.expand_user(self.data_path)

        # 解析 logdir，保证为字符串并去除空白
        log_path_raw = self.__config.get('logdir', '') or ''
        self.log_path = str(log_path_raw).strip()
        """str: 日志目录 """

        if not self.log_path:
            self.log_path = os.path.join(self.__default_main_path, 'logs')
        self.log_path = filesystem.expand_user(self.log_path)
        
        # 数据路径
        self.kline_path = os.path.join(self.data_path, 'day')
        """str: K线路径 """
        
        # 如果配置文件不存在，则回写一个默认的 yaml 配置文件，并添加注释
        try:
            if not os.path.isfile(self.__config_filename):
                # 使用已加载的配置作为默认，否则使用基于主路径的默认 basedir
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
                        # 如果包含 debug，则按用户期望的格式写入一行，并附带注释
                        wf.write(f"debug: {'true' if self.debug else 'false'} # 是否启用调试模式\n")
                        # 如果包含 basedir，则按用户期望的格式写入一行，并附带注释
                        wf.write(f"basedir: {self.__default_path} # 数据路径\n")
                        wf.write(f"logdir: {self.__default_path}/logs # 日志路径\n")
                        
                    logging.info(f"配置文件 {self.__config_filename} 不存在，已创建默认配置文件")
                except Exception as e:
                    logging.warning(f"创建默认配置文件失败: {e}")
        except Exception:
            # 不要阻塞初始化流程，记录错误继续
            logging.exception("检查或创建配置文件时发生错误")

        


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

