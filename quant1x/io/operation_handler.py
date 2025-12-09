"""Operation handler interface ported from C++ NetworkOperationHandlerr.

Provides a base class that matches the C++ semantics: default timeouts
and intervals, setters, and abstract methods for handshake and keepalive.
"""
from __future__ import annotations

import datetime
from typing import Union


class NetworkOperationHandler:
    """Simple interface for network operation handler.

    This class intentionally does not use `abc.ABC`. It provides default
    timeout/check-interval storage and declares the protocol methods
    `handshake` and `keepalive` that concrete handlers must implement by
    overriding. If they are not overridden, NotImplementedError is raised at
    runtime — this keeps the class as a lightweight interface (no ABC
    metaclass enforcement).
    """

    def __init__(self):
        # default semantics: interval=5s, timeout=10s
        self._interval = datetime.timedelta(seconds=5)
        self._timeout = datetime.timedelta(seconds=10)

    def timeout(self) -> float:
        """Return timeout in seconds (float) to be compatible with socket APIs."""
        return self._timeout.total_seconds()

    def set_timeout(self, seconds: Union[int, float]) -> None:
        self._timeout = datetime.timedelta(seconds=float(seconds))

    def check_interval(self) -> float:
        return self._interval.total_seconds()

    def set_check_interval(self, seconds: Union[int, float]) -> None:
        self._interval = datetime.timedelta(seconds=float(seconds))

    # The following methods define the interface contract. They raise
    # NotImplementedError to indicate they must be implemented by concrete
    # handlers.
    def handshake(self, sock) -> bool:
        """Perform protocol handshake on a connected socket. Return True if OK."""
        raise NotImplementedError("handshake must be implemented by subclasses")

    def keepalive(self, sock) -> bool:
        """Return True if socket is still healthy."""
        raise NotImplementedError("keepalive must be implemented by subclasses")
