from abc import ABC, abstractmethod
from typing import List, Union, Literal
from .market import Exchange, Instrument, Sector
from enum import Enum

class PlateCategory(Enum):
    """板块类别：用于区分不同逻辑类型的股票分组"""
    UNKNOWN = (0, "未知")
    INDUSTRY = (2, "行业")
    REGION = (3, "地区")
    THEMATIC = (4, "概念")
    STYLE = (5, "风格")
    INDEX = (6, "指数")
    RESEARCH_INDUSTRY = (12, "研究行业")

    def __init__(self, code: int, label: str):
        self.code = code      # 保留原始数字值（用于兼容旧系统、数据库等）
        self.label = label    # 中文显示名

    @property
    def value(self) -> int:
        """覆盖 value 属性，使其返回 code（默认行为）"""
        return self.code

    def __str__(self) -> str:
        return self.label

    def __repr__(self) -> str:
        return f"{self.__class__.__name__}.{self.name} ({self.code}, '{self.label}')"

class DataHandler(ABC):
    """
    市场接口抽象基类

    所有具体市场（如 A 股、港股、美股）应继承此接口并实现方法。
    """
    
    @abstractmethod
    def get_market_list(self) -> List[Exchange]:
        """
        返回该市场对应的市场列表
        
        Returns:
            List[Exchange]: 市场对象列表，包含该市场所有市场信息
            
        Raises:
            NotImplementedError: 如果子类未实现此方法
        """
        raise NotImplementedError("Subclass of DataHandler must implement `get_market_list` method")
    
    @abstractmethod
    def get_index_list(self, market: Union[List, str] = "all") -> List[Instrument]:
        """
        返回指定市场对应的指数列表
        
        Args:
            market (Union[List, str]): 市场标识，可以是字符串或列表。默认为"all"表示所有市场
        
        Returns:
            List[Instrument]: 包含指定市场所有指数对象的列表
        
        Raises:
            NotImplementedError: 如果子类未实现此方法
        """
        raise NotImplementedError("Subclass of DataHandler must implement `get_market_list` method")

    @abstractmethod
    def get_sector_list(self, category: PlateCategory=PlateCategory.UNKNOWN) -> List[Sector]:
        """
        获取指定类别的板块列表
        
        Args:
            category (PlateCategory): 板块类别，默认为 PlateCategory.UNKNOWN
        
        Returns:
            List[Sector]: 返回指定类别的板块列表
        
        Raises:
            NotImplementedError: 子类必须实现此方法
        """
        raise NotImplementedError("Subclass of DataHandler must implement `get_sector_list` method")
    
    @abstractmethod
    def list_instruments(self, market: Union[List, str] = "all") -> List[Instrument]:
        """
        返回指定市场对应的所有证券列表
        
        Args:
            market (Union[List, str]): 市场标识，可以是字符串或列表。默认为"all"表示所有市场
        
        Returns:
            List[Instrument]: 包含指定市场所有证券对象的列表
        
        Raises:
            NotImplementedError: 如果子类未实现此方法
        """
        raise NotImplementedError("Subclass of DataHandler must implement `list_instruments` method")


