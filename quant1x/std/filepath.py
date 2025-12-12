# -*- coding: utf-8 -*-
import os
import tempfile

def homedir() -> str:
    """
    获取当前用户的主目录
    优先级:
    1. QUANT1X_HOME 环境变量
    2. GOX_HOME 环境变量
    3. HOME 环境变量
    4. USERPROFILE 环境变量 (Windows only)
    5. 系统临时目录 (fallback)
    """
    home = os.getenv("QUANT1X_HOME")
    if home:
        return home
    
    home = os.getenv("GOX_HOME")
    if home:
        return home
        
    home = os.getenv("HOME")
    if home:
        return home
        
    if os.name == 'nt':
        home = os.getenv("USERPROFILE")
        if home:
            return home
            
    return tempfile.gettempdir()

def expand_user(path: str) -> str:
    """
    展开用户主目录 (例如 "~/data" -> "/home/user/data")
    """
    if not path:
        return path
        
    path = path.strip()
    if not path.startswith('~'):
        return path
        
    home = homedir()
    if len(path) == 1:
        return home
        
    if path[1] == '/' or path[1] == '\\':
        return os.path.join(home, path[2:])
        
    return path
