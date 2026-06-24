# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from .meta.exchange import Exchange

from . import cache

def should_initialize_file(fname: str, exchange: Exchange = Exchange.SSE) -> bool:
    """
    检查是否应该初始化文件, 基于文件修改时间和交易所交易时段
    
    Args:
        fname (str): 要检查的文件路径
        exchange (Exchange): 交易所枚举, 默认为SSE
    
    Returns:
        bool: 如果文件不存在或需要重新初始化则返回True, 否则返回False
    
    Raises:
        OSError: 如果文件访问出现错误(函数内部已处理, 但调用方可能需要知道)
    """
    try:
        mod_time = cache.get_filename_modified_time(fname)
    except OSError:
        return True

    from .meta.session import can_initialize

    return can_initialize(exchange=exchange, last_modified=mod_time)


def should_update_file(fname: str, exchange: Exchange = Exchange.SSE) -> bool:
    """
    检查文件是否需要更新, 基于文件修改时间和交易所交易时间
    
    Args:
        fname (str): 需要检查的文件路径
        exchange (Exchange): 交易所对象, 默认为SSE
    
    Returns:
        bool: 如果文件需要更新则返回True, 否则返回False
    
    Raises:
        OSError: 当无法获取文件修改时间时抛出
    """
    try:
        mod_time = cache.get_filename_modified_time(fname)
    except OSError:
        return True

    from .meta.session import check_trading_timestamp

    rs = check_trading_timestamp(exchange=exchange, last_modified=mod_time)
    return rs.update_in_real_time
