# -*- coding: UTF-8 -*-
import threading


class Singleton:
    """
    线程安全的单例模式
    """
    _instance = None
    _lock = threading.Lock()

    def __new__(cls, *args, **kwargs):
        with cls._lock:
            if not cls._instance:
                # cls._instance = super().__new__(cls, *args, **kwargs)
                cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self):
        pass
