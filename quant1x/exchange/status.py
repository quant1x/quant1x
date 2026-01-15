# -*- coding: UTF-8 -*-
"""
Ported from exchange/status.go

Provides helpers to obtain a file's modification timestamp and
determine whether a file should be updated.

Note: This module will try to call `CanInitialize` from
`quant1x.exchange.session` if available. If that function is
not present, `should_update_file` conservatively returns True.
"""
import os
from datetime import datetime
from typing import Optional

from .timestamp import Timestamp


def get_filename_modified_time(fname: str) -> Timestamp:
    """Get the modification time of a file as a `Timestamp`.

    Raises `OSError` if the file metadata cannot be read.
    """
    info = os.lstat(fname)
    dt = datetime.fromtimestamp(info.st_mtime)
    return Timestamp.from_datetime(dt)


def should_initialize_file(fname: str) -> bool:
    """Check whether the given file should be initialized.

    Mirrors `ShouldInitializeFile` in Go: returns True on metadata error
    and otherwise delegates to `session.CanInitialize`.
    """
    try:
        mod_time = get_filename_modified_time(fname)
    except OSError:
        return True

    from .session import CanInitialize

    return CanInitialize(mod_time)


def should_update_file(fname: str) -> bool:
    """Check whether the given file should be updated.

    Mirrors `ShouldUpdateFile` in Go: on metadata error returns True,
    otherwise uses `session.CheckTradingTimestamp` and returns
    the `update_in_real_time` flag.
    """
    try:
        mod_time = get_filename_modified_time(fname)
    except OSError:
        return True

    from .session import CheckTradingTimestamp

    rs = CheckTradingTimestamp(mod_time)
    return rs.update_in_real_time
