#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
@Project : quant1x
@Package : quant1x.config
@File    : config.py
@Author  : wangfeng
@Date    : 2025/9/15 16:37
@Desc    :
"""
import os
import yaml
import dotenv
from yarg import get
from quant1x import system

# 加载环境变量
dotenv.load_dotenv()

def get_quant1x_work_keyword() -> str:
    """
    获取quant1x工作目录的关键词
    :return:
    """
    quant1x_work = 'quant1x'
    quant1x_work_env = system.env('QUANT1X_WORK')
    if len(quant1x_work_env) > 0:
        quant1x_work = quant1x_work_env
    return quant1x_work

def get_quant1x_config_filename() -> str:
    """
    获取quant1x.yaml文件路径
    :return:
    """
    # 默认配置文件名
    default_config_filename = 'quant1x.yaml'
    yaml_filename = os.path.join('~', 'runtime', 'etc', default_config_filename)
    yaml_filename = os.path.expanduser(yaml_filename)
    user_home = system.homedir()
    quant1x_work = get_quant1x_work_keyword()
    if not os.path.isfile(yaml_filename):
        quant1x_root = os.path.join(user_home, f'.{quant1x_work}')
        yaml_filename = os.path.join(quant1x_root, default_config_filename)
        yaml_filename = os.path.expanduser(yaml_filename)
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
        self.__default_main_path = os.path.join(self.__home_path, f'.{self.__work_keyword}')

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
