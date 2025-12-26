# -*- coding: UTF-8 -*-
"""
缓存适配器接口定义
"""
from abc import ABC, abstractmethod
from typing import List, Optional, Dict, Any
import threading
from ..exchange import Timestamp


# Kind 表示插件类型标识
Kind = int

# 插件类型掩码
PLUGIN_MASK_BASE_DATA = 0x1000000000000000  # 基础数据
PLUGIN_MASK_FEATURE = 0x2000000000000000   # 特征数据
PLUGIN_MASK_STRATEGY = 0x3000000000000000  # 策略

# 默认数据提供者
DEFAULT_DATA_PROVIDER = "engine"


class Schema(ABC):
    """缓存的概要信息接口"""

    @abstractmethod
    def kind(self) -> Kind:
        """数据类型"""
        pass

    @abstractmethod
    def owner(self) -> str:
        """提供者"""
        pass

    @abstractmethod
    def key(self) -> str:
        """数据关键词，key与cache落地强关联"""
        pass

    @abstractmethod
    def name(self) -> str:
        """特性名称"""
        pass

    @abstractmethod
    def usage(self) -> str:
        """控制台参数提示信息，数据描述"""
        pass


class DataAdapter(Schema):
    """数据适配器接口"""

    @abstractmethod
    def print(self, code: str, dates: Optional[List[Timestamp]] = None) -> None:
        """控制台打印"""
        pass

    @abstractmethod
    def update(self, code: str, date: Optional[Timestamp] = None) -> None:
        """更新数据"""
        pass


class FeatureAdapter(DataAdapter):
    """特征数据适配器接口"""

    @abstractmethod
    def filename(self, timestamp: Optional[Timestamp] = None) -> str:
        """返回对应的聚合文件路径"""
        pass

    @abstractmethod
    def init(self, timestamp: Timestamp) -> None:
        """初始化"""
        pass

    @abstractmethod
    def clone(self) -> 'FeatureAdapter':
        """克隆"""
        pass

    @abstractmethod
    def headers(self) -> List[str]:
        """表头"""
        pass

    @abstractmethod
    def values(self) -> List[str]:
        """值"""
        pass


# 插件注册管理
_plugin_mutex = threading.Lock()
_plugin_map: Dict[Kind, DataAdapter] = {}


class PluginAlreadyExistsError(Exception):
    """插件已存在异常"""
    pass


def get_data_adapter(kind: Kind) -> Optional[DataAdapter]:
    """按 Kind 获取已注册的适配器"""
    with _plugin_mutex:
        return _plugin_map.get(kind)


def register_plugin(plugin: DataAdapter) -> None:
    """注册一个 DataAdapter"""
    with _plugin_mutex:
        kind = plugin.kind()
        if kind in _plugin_map:
            raise PluginAlreadyExistsError("the plugin already exists")
        _plugin_map[kind] = plugin


def plugins_with_name(plugin_type: Kind, keywords: List[str]) -> List[DataAdapter]:
    """根据类型掩码和关键字列表返回匹配的适配器"""
    if not keywords:
        return []

    with _plugin_mutex:
        kw_set = set(keywords)

        candidates = []
        for plugin in _plugin_map.values():
            if (plugin.kind() & plugin_type) == plugin_type:
                if plugin.key() in kw_set:
                    candidates.append((plugin.kind(), plugin))

        if not candidates:
            return []

        # 按 kind 排序
        candidates.sort(key=lambda x: x[0])

        return [plugin for _, plugin in candidates]


def plugins(mask: Kind = 0) -> List[DataAdapter]:
    """返回按 kind 排序的适配器列表。mask 为 0 返回全部。"""
    with _plugin_mutex:
        plugin_list = []
        for plugin in _plugin_map.values():
            if mask == 0 or ((plugin.kind() & mask) == mask):
                plugin_list.append(plugin)

        # 按 kind 排序
        plugin_list.sort(key=lambda p: p.kind())
        return plugin_list


# 插件注册器基类
class register:
    """插件注册器"""

    def __init__(self, plugin_class):
        # 创建插件实例并注册
        try:
            plugin = plugin_class()
            register_plugin(plugin)
        except PluginAlreadyExistsError:
            # 插件已存在，忽略
            pass