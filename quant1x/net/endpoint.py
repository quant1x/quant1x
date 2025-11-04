"""EndpointManager - Python port matching the C++ EndpointManager API.

This module provides EndpointManager with the same semantics as the C++
implementation in `net/endpoint.h` used by the connection pool.

Endpoints are represented as (host, port) tuples.
"""
from typing import Dict, List, Optional, Tuple
import threading

Endpoint = Tuple[str, int]


class EndpointManager:
    def __init__(self):
        self._endpoints_list: List[Endpoint] = []
        # endpoint -> { 'max_connections': int, 'active_connections': int }
        self._endpoints_data: Dict[Endpoint, Dict[str, int]] = {}
        self._lock = threading.Lock()

    def add_endpoint(self, ip: str, port: int, max_connections: int) -> bool:
        # Validate port range
        if port == 0 or port >= 65535:
            return False
        try:
            ep = (ip, int(port))
            return self.add_endpoint_obj(ep, max_connections)
        except Exception:
            return False

    def add_endpoint_obj(self, endpoint: Endpoint, max_connections: int) -> bool:
        with self._lock:
            if endpoint in self._endpoints_data:
                return False
            self._endpoints_list.append(endpoint)
            self._endpoints_data[endpoint] = {"max_connections": int(max_connections), "active_connections": 0}
            return True

    # Compatibility alias with C++ naming
    addEndpoint = add_endpoint
    addEndpointObj = add_endpoint_obj

    def remove_endpoint(self, endpoint: Endpoint) -> None:
        with self._lock:
            self._endpoints_data.pop(endpoint, None)
            try:
                self._endpoints_list.remove(endpoint)
            except ValueError:
                pass

    def acquire_endpoint(self) -> Optional[Endpoint]:
        with self._lock:
            for ep in list(self._endpoints_list):
                data = self._endpoints_data.get(ep)
                if data is None:
                    continue
                if data["active_connections"] < data["max_connections"]:
                    data["active_connections"] += 1
                    return ep
            return None

    def release_endpoint(self, endpoint: Endpoint) -> None:
        with self._lock:
            data = self._endpoints_data.get(endpoint)
            if data and data["active_connections"] > 0:
                data["active_connections"] -= 1

    def get_endpoint_stats(self, endpoint: Endpoint):
        with self._lock:
            data = self._endpoints_data.get(endpoint)
            if data is None:
                raise KeyError("Endpoint not found")
            return data["max_connections"], data["active_connections"]

    def get_all_endpoints(self) -> List[Endpoint]:
        with self._lock:
            return list(self._endpoints_list)

    def get_available_resources(self) -> int:
        with self._lock:
            total = 0
            for data in self._endpoints_data.values():
                if data["active_connections"] < data["max_connections"]:
                    total += data["max_connections"] - data["active_connections"]
            return total

    # C++ style method names expected by the pool
    acquireEndpoint = acquire_endpoint
    releaseEndpoint = release_endpoint
    getEndpointStats = get_endpoint_stats
    getAllEndpoints = get_all_endpoints
    getAvailableResources = get_available_resources
