# -*- coding: utf-8 -*-
# Copyright (c) 2026 Quant1X. All rights reserved.
# Author: wangfeng <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

from __future__ import annotations

import secrets
import threading
import time
from typing import Callable, Optional, Tuple

from .state_store import FileStateStore, PersistentState, advance_persistent_state


class HLC:
    def __init__(self, *options) -> None:
        self._lock = threading.Lock()
        self.physical = 0
        self.logical = 0
        self.seq = 0
        self._now: Callable[[], int] = lambda: int(time.time() * 1000)
        self.seed = secrets.randbits(16)
        self.sync_every = 1
        self.store: Optional[FileStateStore] = None

        for option in options:
            if option is not None:
                option(self)

        if self.store is not None:
            self.store.sync_every = self.sync_every
            state, ok = self.load_state()
            if ok:
                self.physical = state.physical
                self.logical = state.logical
                self.seq = state.seq
            else:
                self.physical = self._now()
                self.logical = self.seed
        else:
            self.physical = self._now()
            self.logical = self.seed

    def now(self) -> Tuple[int, int]:
        with self._lock:
            current = PersistentState(self.physical, self.logical, self.seq)
            now_ms = self._now()
            if self.store is not None:
                next_state = self.store.next_state(current, now_ms, self.seed)
            else:
                next_state = advance_persistent_state(current, now_ms, self.seed)

            self.physical = next_state.physical
            self.logical = next_state.logical
            self.seq = next_state.seq

            return ((self.physical << 16) | self.logical, self.seq)

    def timestamp(self) -> int:
        with self._lock:
            return self.physical

    def load_state(self) -> Tuple[PersistentState, bool]:
        if self.store is None:
            return PersistentState(0, 0, 0), False
        return self.store.load()