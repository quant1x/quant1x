from abc import ABC, ABCMeta, abstractmethod
from typing import Type, Any, Dict, Optional, ClassVar, final
import threading
from threading import Lock, RLock
import time
from contextlib import contextmanager
import weakref
from enum import Enum, auto
from functools import wraps
import logging

# 配置日志
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class SingletonInitPolicy(Enum):
    """单例初始化策略"""
    LAZY = auto()      # 懒加载（默认）
    EAGER = auto()     # 提前初始化
    ON_DEMAND = auto() # 按需初始化


class ThreadSafeStrategy(Enum):
    """线程安全策略"""
    LOCK = auto()          # 简单锁
    DOUBLE_CHECKED = auto()  # 双重检查锁定
    CLASS_LOCK = auto()    # 类级别锁
    THREAD_LOCAL = auto()  # 线程本地存储
    NO_LOCK = auto()       # 无锁（仅单线程）


class SingletonInitializationError(Exception):
    """单例初始化异常"""
    pass


class ThreadSafeSingletonMeta(ABCMeta):
    """
    线程安全的单例元类
    支持多种线程安全策略
    """
    
    # 类级别的锁，用于保护实例创建
    _class_lock: ClassVar[Lock] = Lock()
    
    # 存储所有单例实例
    _instances: ClassVar[Dict[Type, Any]] = {}
    
    # 初始化状态跟踪
    _initializing: ClassVar[Dict[Type, bool]] = {}
    _initialized: ClassVar[Dict[Type, bool]] = {}
    
    # 线程本地存储
    _thread_local = threading.local()
    
    def __init__(cls, name, bases, namespace):
        super().__init__(name, bases, namespace)
        
        # 初始化类级别的锁
        if not hasattr(cls, '_instance_lock'):
            cls._instance_lock = RLock()  # 可重入锁
        
        # 设置默认策略
        if not hasattr(cls, '_init_policy'):
            cls._init_policy = SingletonInitPolicy.LAZY
        
        if not hasattr(cls, '_thread_safe_strategy'):
            cls._thread_safe_strategy = ThreadSafeStrategy.DOUBLE_CHECKED
        
        # 如果是提前初始化策略，在类定义时创建实例
        if cls._init_policy == SingletonInitPolicy.EAGER:
            cls._eager_init()
    
    def _eager_init(cls):
        """提前初始化单例实例"""
        with cls._class_lock:
            if cls not in cls._instances and len(cls.__abstractmethods__) == 0:
                try:
                    instance = super().__call__()
                    cls._instances[cls] = instance
                    cls._initialized[cls] = True
                    logger.info(f"类 {cls.__name__} 已提前初始化")
                except Exception as e:
                    logger.error(f"类 {cls.__name__} 提前初始化失败: {e}")
                    raise
    
    def _check_abstract(cls):
        """检查是否是抽象类"""
        if len(cls.__abstractmethods__) > 0:
            abstract_methods = list(cls.__abstractmethods__)
            raise TypeError(
                f"无法实例化抽象类 {cls.__name__}。"
                f"未实现的抽象方法: {abstract_methods}"
            )
    
    def _create_instance_safely(cls, *args, **kwargs) -> Any:
        """安全创建实例，防止重复初始化"""
        with cls._instance_lock:
            if cls in cls._initializing and cls._initializing[cls]:
                raise SingletonInitializationError(
                    f"检测到循环依赖或重复初始化: {cls.__name__}"
                )
            
            cls._initializing[cls] = True
            try:
                instance = super().__call__(*args, **kwargs)
                cls._initialized[cls] = True
                return instance
            finally:
                cls._initializing[cls] = False
    
    def _get_instance_with_lock(cls, *args, **kwargs) -> Any:
        """使用简单锁策略"""
        with cls._instance_lock:
            if cls not in cls._instances:
                cls._check_abstract()
                cls._instances[cls] = cls._create_instance_safely(*args, **kwargs)
            return cls._instances[cls]
    
    def _get_instance_with_double_checked(cls, *args, **kwargs) -> Any:
        """使用双重检查锁定策略（推荐）"""
        if cls not in cls._instances:
            with cls._instance_lock:
                if cls not in cls._instances:
                    cls._check_abstract()
                    cls._instances[cls] = cls._create_instance_safely(*args, **kwargs)
        return cls._instances[cls]
    
    def _get_instance_with_class_lock(cls, *args, **kwargs) -> Any:
        """使用类级别锁策略"""
        with cls._class_lock:
            if cls not in cls._instances:
                cls._check_abstract()
                cls._instances[cls] = cls._create_instance_safely(*args, **kwargs)
            return cls._instances[cls]
    
    def _get_instance_with_thread_local(cls, *args, **kwargs) -> Any:
        """使用线程本地存储策略"""
        if not hasattr(cls._thread_local, 'instances'):
            cls._thread_local.instances = {}
        
        if cls not in cls._thread_local.instances:
            cls._check_abstract()
            cls._thread_local.instances[cls] = cls._create_instance_safely(*args, **kwargs)
        
        return cls._thread_local.instances[cls]
    
    def __call__(cls, *args, **kwargs) -> Any:
        """根据线程安全策略创建/获取实例"""
        
        # 如果已经有实例，直接返回
        if cls in cls._instances and cls._initialized.get(cls, False):
            return cls._instances[cls]
        
        # 根据策略选择获取实例的方法
        strategy_methods = {
            ThreadSafeStrategy.LOCK: cls._get_instance_with_lock,
            ThreadSafeStrategy.DOUBLE_CHECKED: cls._get_instance_with_double_checked,
            ThreadSafeStrategy.CLASS_LOCK: cls._get_instance_with_class_lock,
            ThreadSafeStrategy.THREAD_LOCAL: cls._get_instance_with_thread_local,
            ThreadSafeStrategy.NO_LOCK: lambda *a, **kw: (
                cls._instances.setdefault(cls, cls._create_instance_safely(*a, **kw))
                if cls not in cls._instances else cls._instances[cls]
            ),
        }
        
        method = strategy_methods.get(
            cls._thread_safe_strategy,
            cls._get_instance_with_double_checked
        )
        
        return method(*args, **kwargs)
    
    def reset_instance(cls):
        """重置单例实例（主要用于测试）"""
        with cls._class_lock:
            if cls in cls._instances:
                if hasattr(cls._instances[cls], '_cleanup'):
                    cls._instances[cls]._cleanup()
                cls._instances.pop(cls, None)
                cls._initialized.pop(cls, False)
                cls._initializing.pop(cls, False)
                logger.info(f"已重置 {cls.__name__} 的单例实例")
    
    def is_initialized(cls) -> bool:
        """检查单例是否已初始化"""
        return cls._initialized.get(cls, False)
    
    def get_instance_count(cls) -> int:
        """获取单例实例数量（用于调试）"""
        if cls._thread_safe_strategy == ThreadSafeStrategy.THREAD_LOCAL:
            if hasattr(cls._thread_local, 'instances'):
                return len([1 for c in cls._thread_local.instances.keys() 
                           if c.__name__ == cls.__name__])
            return 0
        return 1 if cls in cls._instances else 0


class ThreadSafeSingletonABC(ABC, metaclass=ThreadSafeSingletonMeta):
    """
    线程安全的单例抽象基类
    
    特性：
    1. 线程安全的单例模式
    2. 支持多种线程安全策略
    3. 支持多种初始化策略
    4. 防止抽象类实例化
    5. 提供资源清理钩子
    6. 支持实例重置（主要用于测试）
    """
    
    # 线程安全策略
    _thread_safe_strategy: ClassVar[ThreadSafeStrategy] = ThreadSafeStrategy.DOUBLE_CHECKED
    
    # 初始化策略
    _init_policy: ClassVar[SingletonInitPolicy] = SingletonInitPolicy.LAZY
    
    def __new__(cls, *args, **kwargs):
        # 防止直接实例化抽象基类
        if cls is ThreadSafeSingletonABC or cls.__name__ == 'ThreadSafeSingletonABC':
            raise TypeError(f"{cls.__name__} 是抽象基类，不能直接实例化")
        return super().__new__(cls)
    
    def __init__(self, *args, **kwargs):
        # 防止重复初始化
        if not hasattr(self, '_singleton_initialized'):
            self.initialize()
            self._singleton_initialized = True
    
    def __init_subclass__(cls, **kwargs):
        """子类初始化处理"""
        super().__init_subclass__(**kwargs)
        
        # 为每个子类创建独立的锁
        if not hasattr(cls, '_instance_lock'):
            cls._instance_lock = RLock()
        
        # 重置相关状态
        if cls in ThreadSafeSingletonMeta._instances:
            del ThreadSafeSingletonMeta._instances[cls]
        ThreadSafeSingletonMeta._initialized[cls] = False
        ThreadSafeSingletonMeta._initializing[cls] = False
        
        logger.debug(f"初始化单例子类: {cls.__name__}")
    
    @abstractmethod
    def initialize(self) -> None:
        """
        初始化方法（抽象方法）
        子类必须实现此方法来完成初始化
        """
        pass
    
    @abstractmethod
    def cleanup(self) -> None:
        """
        清理方法（抽象方法）
        子类必须实现此方法来清理资源
        """
        pass
    
    def _cleanup(self) -> None:
        """
        内部清理方法
        调用子类的 cleanup 方法
        """
        try:
            self.cleanup()
        except Exception as e:
            logger.error(f"清理资源时出错: {e}")
    
    @classmethod
    def get_instance(cls, *args, **kwargs) -> 'ThreadSafeSingletonABC':
        """
        获取单例实例的显式方法
        在某些情况下比直接调用类更清晰
        """
        return cls(*args, **kwargs)
    
    @classmethod
    def get_or_create(cls, *args, **kwargs) -> 'ThreadSafeSingletonABC':
        """别名方法，用于提高代码可读性"""
        return cls.get_instance(*args, **kwargs)
    
    @classmethod
    def ensure_initialized(cls, *args, **kwargs) -> None:
        """确保单例已初始化"""
        if not cls.is_initialized():
            _ = cls.get_instance(*args, **kwargs)
    
    @final
    def __repr__(self) -> str:
        """统一的字符串表示"""
        return f"<{self.__class__.__name__} singleton at {id(self):#x}>"
    
    @final
    def __del__(self):
        """析构函数，记录清理信息"""
        logger.debug(f"单例实例 {self.__class__.__name__} 正在被销毁")


# 辅助装饰器
def synchronized(lock_attr: str = '_instance_lock'):
    """
    方法同步装饰器
    用于确保单个方法的线程安全
    """
    def decorator(method):
        @wraps(method)
        def wrapper(self, *args, **kwargs):
            lock = getattr(self, lock_attr, None)
            if lock is None:
                lock = getattr(self.__class__, lock_attr, threading.Lock())
            
            with lock:
                return method(self, *args, **kwargs)
        return wrapper
    return decorator


def singleton_method(method):
    """
    标记单例关键方法
    主要用于文档和调试
    """
    @wraps(method)
    def wrapper(self, *args, **kwargs):
        logger.debug(f"调用单例方法: {method.__name__} on {self}")
        return method(self, *args, **kwargs)
    wrapper._is_singleton_method = True
    return wrapper


# 使用示例
class DatabaseConfig(ThreadSafeSingletonABC):
    """
    数据库配置管理器示例
    使用双重检查锁定策略
    """
    
    _thread_safe_strategy = ThreadSafeStrategy.DOUBLE_CHECKED
    
    def initialize(self) -> None:
        """初始化数据库配置"""
        self.host = "localhost"
        self.port = 3306
        self.username = "admin"
        self.password = "secret"
        self.connection_pool = []
        self._connection_count = 0
        logger.info("数据库配置已初始化")
    
    def cleanup(self) -> None:
        """清理数据库连接"""
        for conn in self.connection_pool:
            try:
                # 模拟关闭连接
                pass
            except Exception as e:
                logger.error(f"关闭连接时出错: {e}")
        self.connection_pool.clear()
        logger.info("数据库连接已清理")
    
    @synchronized()
    def get_connection(self) -> dict:
        """获取数据库连接（线程安全）"""
        self._connection_count += 1
        conn = {
            "id": self._connection_count,
            "host": self.host,
            "port": self.port
        }
        self.connection_pool.append(conn)
        return conn
    
    @singleton_method
    def get_config(self) -> dict:
        """获取配置信息"""
        return {
            "host": self.host,
            "port": self.port,
            "username": self.username,
            "connections": len(self.connection_pool)
        }


class Logger(ThreadSafeSingletonABC):
    """
    线程本地日志记录器示例
    每个线程有自己的实例
    """
    
    _thread_safe_strategy = ThreadSafeStrategy.THREAD_LOCAL
    
    def initialize(self) -> None:
        """初始化日志记录器"""
        self.logs = []
        self.thread_id = threading.get_ident()
        logger.info(f"日志记录器已初始化 (线程: {self.thread_id})")
    
    def cleanup(self) -> None:
        """清理日志"""
        self.logs.clear()
        logger.info(f"日志已清理 (线程: {self.thread_id})")
    
    @synchronized('_log_lock')
    def log(self, message: str, level: str = "INFO") -> None:
        """记录日志（使用单独的锁）"""
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
        log_entry = f"[{timestamp}] [{level}] {message}"
        self.logs.append(log_entry)
        
        # 模拟输出到控制台
        print(log_entry)
    
    def get_logs(self) -> list:
        """获取日志记录"""
        return self.logs.copy()


class CacheManager(ThreadSafeSingletonABC):
    """
    缓存管理器示例
    使用提前初始化策略
    """
    
    _init_policy = SingletonInitPolicy.EAGER
    _thread_safe_strategy = ThreadSafeStrategy.DOUBLE_CHECKED
    
    def initialize(self) -> None:
        """初始化缓存"""
        self.cache = {}
        self.hits = 0
        self.misses = 0
        logger.info("缓存管理器已提前初始化")
    
    def cleanup(self) -> None:
        """清理缓存"""
        self.cache.clear()
        logger.info("缓存已清理")
    
    @synchronized()
    def get(self, key: str, default=None):
        """从缓存获取数据"""
        if key in self.cache:
            self.hits += 1
            return self.cache[key]
        self.misses += 1
        return default
    
    @synchronized()
    def set(self, key: str, value) -> None:
        """设置缓存数据"""
        self.cache[key] = value
    
    @singleton_method
    def get_stats(self) -> dict:
        """获取缓存统计"""
        return {
            "size": len(self.cache),
            "hits": self.hits,
            "misses": self.misses,
            "hit_rate": self.hits / (self.hits + self.misses) if (self.hits + self.misses) > 0 else 0
        }


# 多线程测试函数
def test_database_config(thread_id: int):
    """测试数据库配置的单例性"""
    config = DatabaseConfig.get_instance()
    conn = config.get_connection()
    print(f"线程 {thread_id}: 获取连接 {conn['id']}")
    time.sleep(0.1)
    return config


def test_thread_local_logger(thread_id: int):
    """测试线程本地单例"""
    logger = Logger.get_instance()
    logger.log(f"来自线程 {thread_id} 的消息")
    return logger


def test_concurrent_access():
    """测试并发访问"""
    print("\n=== 测试并发访问 ===")
    
    # 测试数据库配置（全局单例）
    print("\n1. 测试全局单例（DatabaseConfig）:")
    threads = []
    configs = []
    
    for i in range(5):
        t = threading.Thread(
            target=lambda idx=i: configs.append(test_database_config(idx)),
            daemon=True
        )
        threads.append(t)
        t.start()
    
    for t in threads:
        t.join()
    
    # 检查是否是同一个实例
    if all(c is configs[0] for c in configs):
        print("✓ 所有线程获取到同一个实例")
    else:
        print("✗ 实例不一致！")
    
    # 测试线程本地单例
    print("\n2. 测试线程本地单例（Logger）:")
    threads.clear()
    loggers = []
    
    for i in range(3):
        t = threading.Thread(
            target=lambda idx=i: loggers.append(test_thread_local_logger(idx)),
            daemon=True
        )
        threads.append(t)
        t.start()
    
    for t in threads:
        t.join()
    
    # 检查是否是不同实例
    unique_instances = len(set(id(l) for l in loggers))
    print(f"✓ 创建了 {unique_instances} 个不同的 Logger 实例（每个线程一个）")
    
    # 测试缓存管理器
    print("\n3. 测试缓存管理器（提前初始化）:")
    cache1 = CacheManager.get_instance()
    cache2 = CacheManager.get_instance()
    
    print(f"cache1 is cache2: {cache1 is cache2}")
    print(f"缓存统计: {cache1.get_stats()}")


def demonstrate_features():
    """演示各种特性"""
    print("=== 线程安全单例抽象类特性演示 ===\n")
    
    # 1. 测试抽象类不能实例化
    print("1. 测试抽象类不能实例化:")
    try:
        # 这会失败，因为 ThreadSafeSingletonABC 是抽象类
        abstract_instance = ThreadSafeSingletonABC()
    except TypeError as e:
        print(f"   预期异常: {e}\n")
    
    # 2. 测试单例性
    print("2. 测试单例性:")
    config1 = DatabaseConfig.get_instance()
    config2 = DatabaseConfig.get_instance()
    config3 = DatabaseConfig()  # 也可以直接调用
    
    print(f"   config1 is config2: {config1 is config2}")
    print(f"   config2 is config3: {config2 is config3}")
    print(f"   所有引用指向同一个实例: {config1 is config2 is config3}\n")
    
    # 3. 测试初始化
    print("3. 测试初始化状态:")
    print(f"   DatabaseConfig 已初始化: {DatabaseConfig.is_initialized()}")
    print(f"   CacheManager 已初始化: {CacheManager.is_initialized()}\n")
    
    # 4. 测试实例重置
    print("4. 测试实例重置（用于测试）:")
    original_config = DatabaseConfig.get_instance()
    DatabaseConfig.reset_instance()
    new_config = DatabaseConfig.get_instance()
    
    print(f"   重置前后是不同实例: {original_config is not new_config}")
    print(f"   新实例已初始化: {DatabaseConfig.is_initialized()}\n")
    
    # 5. 测试配置获取
    print("5. 测试配置获取:")
    config = DatabaseConfig.get_instance()
    conn1 = config.get_connection()
    conn2 = config.get_connection()
    
    print(f"   创建的连接: {conn1['id']}, {conn2['id']}")
    print(f"   配置信息: {config.get_config()}")


# 主测试函数
def main():
    """主测试函数"""
    print("=" * 60)
    print("线程安全单例抽象基类测试")
    print("=" * 60)
    
    # 演示基本特性
    demonstrate_features()
    
    # 测试并发访问
    test_concurrent_access()
    
    # 清理
    print("\n" + "=" * 60)
    print("清理所有单例实例:")
    DatabaseConfig.reset_instance()
    CacheManager.reset_instance()
    print("测试完成！")
    print("=" * 60)


if __name__ == "__main__":
    main()