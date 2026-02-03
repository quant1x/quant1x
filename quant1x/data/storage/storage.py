from abc import ABC, abstractmethod

from typing import Any

class FileStorage(ABC):
    
    @abstractmethod
    def load(self, filepath: str) -> Any:
        """加载数据"""
        raise NotImplementedError("Subclass of FileStorage must implement `load` method")
    
    @abstractmethod
    def save(self, filepath: str, data: Any) -> None:
        """保存数据"""
        raise NotImplementedError("Subclass of FileStorage must implement `save` method")
    
    def update(self, filepath: str, data: Any) -> None:
        """更新数据"""
        raise NotImplementedError("Subclass of FileStorage must implement `update` method")
    
    def checkout(self, filepath: str) -> Any:
        """
        从文件存储中检出指定文件
        
        Args:
            filepath (str): 需要检出的文件路径
        
        Returns:
            Any: 返回检出的文件内容或对象
        
        Raises:
            NotImplementedError: 如果子类未实现此方法
        """
        raise NotImplementedError("Subclass of FileStorage must implement `checkout` method")
