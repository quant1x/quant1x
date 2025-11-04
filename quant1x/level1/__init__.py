"""level1 package (Python)

This package provides a lightweight Python client factory that mirrors the C++
`level1::client()` behavior: it constructs a singleton connection pool which
must be initialized explicitly by the application (see `init_pool` in
`level1.client`) and returns a pooled connection handle.
"""
