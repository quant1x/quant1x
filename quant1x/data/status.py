# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from . import cache

def should_initialize_file(fname: str) -> bool:
    try:
        mod_time = cache.get_filename_modified_time(fname)
    except OSError:
        return True

    from .meta.session import can_initialize

    return can_initialize(mod_time)


def should_update_file(fname: str) -> bool:
    try:
        mod_time = cache.get_filename_modified_time(fname)
    except OSError:
        return True

    from .meta.session import check_trading_timestamp

    rs = check_trading_timestamp(mod_time)
    return rs.update_in_real_time
