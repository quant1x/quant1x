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


def get_filename_modified_time(fname: str) -> Optional[Timestamp]:
    """Get the modification time of a file as a `Timestamp`.

    Raises `OSError` if the file metadata cannot be read.
    """
    info = os.lstat(fname)
    dt = datetime.fromtimestamp(info.st_mtime)
    return Timestamp.from_datetime(dt)


def should_update_file(fname: str) -> bool:
    """Check whether the given file should be updated.

    - If reading file metadata fails, returns True (conservative).
    - If `CanInitialize` exists in `exchange.session`, delegates decision to it.
    - If `CanInitialize` is not available, returns True.
    """
    try:
        mod_time = get_filename_modified_time(fname)
    except OSError:
        return True

    try:
        from .session import CanInitialize

        return CanInitialize(mod_time)
    except Exception:
        # If session.CanInitialize is not implemented in Python yet,
        # conservatively signal that the file should be updated.
        return True
