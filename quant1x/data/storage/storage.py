# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from abc import ABC, abstractmethod
from typing import Any, List, Type, TypeVar, Generic, overload

from .csv import csv_to_slice, slice_to_csv
from quant1x.types import T
from quant1x.data import Timestamp
from quant1x.data import Instrument

class FileStorage(ABC, Generic[T]):
    """文件存储接口（专用于单一 dataclass 类型）"""
    
    def __init__(self, data_type: Type[T]) -> None:
        self._data_type = data_type
        self._file_name = self.file_name()
    
    @abstractmethod
    def file_name(self) -> str:
        """返回文件名"""
        raise NotImplementedError("Subclass must implement `file_name`")
    
    @abstractmethod
    def should_initialize(self, timestamp: Timestamp = Timestamp.now()) -> bool:
        raise NotImplementedError("Subclass must implement `should_initialize`")
    
    @abstractmethod
    def should_update(self, timestamp: Timestamp = Timestamp.now()) -> bool:
        raise NotImplementedError("Subclass must implement `should_update`")
    
    @abstractmethod
    def update(self) -> None:
        """更新数据（无参，因为类型已固定）"""
        raise NotImplementedError("Subclass must implement `update`")
    
    def load(self) -> List[T]:
        """加载数据（无需传 cls，类型已知）"""
        return csv_to_slice(self._file_name, self._data_type)
    
    def save(self, data: List[T]) -> None:
        """保存数据"""
        slice_to_csv(self._file_name, data)
    
    def checkout(self) -> List[T]:
        """检出数据（自动更新 + 加载）"""
        if self.should_initialize() or self.should_update():
            self.update()
        return self.load()

class BasedataFileStorage(FileStorage, Generic[T]):
    """基础数据文件存储类"""
    
    def __init__(self, data_type: Type[T], inst: Instrument) -> None:
        self._inst = inst
        super().__init__(data_type) # 调用父类构造函数
        