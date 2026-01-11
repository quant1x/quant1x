import os
import threading
import time
import tempfile
from datetime import datetime, timedelta
from typing import Callable, Optional


class RollingOnce:
    """A Python translation of RollingOnce: run a callable once per window.

    Optional `marker` is a filesystem path used to persist today's run (date string).
    If seconds is provided, a background thread will reset every `seconds` seconds.
    If hour/minute provided, a background thread will reset daily at that local time.
    """

    def __init__(self, marker: Optional[str] = None):
        self._done = False
        self._lock = threading.Lock()
        self._marker = marker and os.fspath(marker)
        self._stop_event = None
        self._thread = None

    @classmethod
    def seconds(cls, seconds: int, marker: Optional[str] = None) -> "RollingOnce":
        if seconds <= 0:
            seconds = 5
        inst = cls(marker=marker)
        inst._stop_event = threading.Event()
        t = threading.Thread(target=inst._seconds_loop, args=(seconds,))
        t.daemon = True
        inst._thread = t
        t.start()
        return inst

    @classmethod
    def daily(cls, hour: int, minute: int, marker: Optional[str] = None) -> "RollingOnce":
        inst = cls(marker=marker)
        inst._stop_event = threading.Event()
        t = threading.Thread(target=inst._daily_loop, args=(hour, minute))
        t.daemon = True
        inst._thread = t
        t.start()
        return inst

    @classmethod
    def from_spec(cls, spec: str, marker: Optional[str] = None) -> "RollingOnce":
        s = (spec or "").strip()
        # match "*/N ..."
        if s.startswith("*/"):
            try:
                rest = s[2:]
                n = int(rest.split()[0])
                if n > 0:
                    return cls.seconds(n, marker=marker)
            except Exception:
                pass
        parts = s.split()
        if len(parts) >= 3 and parts[0] == "0":
            try:
                mi = int(parts[1])
                hi = int(parts[2])
                return cls.daily(hi, mi, marker=marker)
            except Exception:
                pass
        return cls(marker=marker)

    def _seconds_loop(self, seconds: int) -> None:
        ev = self._stop_event
        while not ev.is_set():
            # wait in chunks so we respond to stop requests
            remaining = seconds
            while remaining > 0 and not ev.is_set():
                wait = min(remaining, 1)
                ev.wait(wait)
                remaining -= wait
            if ev.is_set():
                break
            self.reset()

    def _daily_loop(self, hour: int, minute: int) -> None:
        ev = self._stop_event
        while not ev.is_set():
            now = datetime.now()
            try:
                next_run = now.replace(hour=hour, minute=minute, second=0, microsecond=0)
            except Exception:
                # if invalid values, sleep briefly and continue
                ev.wait(1)
                continue
            if next_run <= now:
                next_run = next_run + timedelta(days=1)
            wait = (next_run - now).total_seconds()
            # wait but remain responsive to stop
            waited = 0.0
            while waited < wait and not ev.is_set():
                to_wait = min(60.0, wait - waited)
                ev.wait(to_wait)
                waited += to_wait
            if ev.is_set():
                break
            self.reset()

    def do_once_try(self, f: Callable[[], object]) -> Optional[object]:
        # Fast path
        if self._done:
            return None

        with self._lock:
            if self._done:
                return None
            try:
                v = f()
                # persist marker if requested
                if self._marker:
                    parent = os.path.dirname(self._marker)
                    if parent:
                        try:
                            os.makedirs(parent, exist_ok=True)
                        except Exception:
                            pass
                    try:
                        fd, tmp = tempfile.mkstemp(dir=parent or None)
                        try:
                            with os.fdopen(fd, "w") as fh:
                                fh.write(datetime.now().strftime("%Y-%m-%d"))
                            os.replace(tmp, self._marker)
                        finally:
                            if os.path.exists(tmp):
                                try:
                                    os.remove(tmp)
                                except Exception:
                                    pass
                    except Exception:
                        # fallback write
                        try:
                            with open(self._marker, "w") as fh:
                                fh.write(datetime.now().strftime("%Y-%m-%d"))
                        except Exception:
                            pass

                self._done = True
                return v
            except Exception:
                # Mark done to avoid retry storm, then re-raise
                self._done = True
                raise

    def do(self, f: Callable[[], None]) -> None:
        # convenience wrapper for callables that don't return
        self.do_once_try(f)

    def reset(self) -> None:
        # wait for any in-progress call by acquiring lock
        with self._lock:
            self._done = False
            # remove marker file if present
            if self._marker:
                try:
                    os.remove(self._marker)
                except Exception:
                    pass

    def close(self) -> None:
        if self._stop_event:
            self._stop_event.set()
            if self._thread:
                self._thread.join()

    def mark_run(self) -> None:
        # persist marker and set done
        if self._marker:
            parent = os.path.dirname(self._marker)
            if parent:
                try:
                    os.makedirs(parent, exist_ok=True)
                except Exception:
                    pass
            try:
                with open(self._marker, "w") as fh:
                    fh.write(datetime.now().strftime("%Y-%m-%d"))
            except Exception:
                pass
        self._done = True

    def __repr__(self) -> str:
        return f"RollingOnce(done={self._done})"
