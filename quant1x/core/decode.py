# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import yaml
from typing import Any
from dataclasses import is_dataclass, fields
from .defaults import apply_defaults


def decode_to(dst_type_or_obj: Any, src: Any) -> Any:
    """Decode src (usually a dict) into dst_type_or_obj.

    - If dst_type_or_obj is a type (class), we construct an instance and populate attributes from src.
    - If dst_type_or_obj is an instance, we set attributes on it and return it.
    - For basic target types (int/str/bool/float), we attempt conversion and return the value.
    """
    # basic scalar target conversion
    basic_types = (int, str, bool, float)
    if isinstance(dst_type_or_obj, type) and dst_type_or_obj in basic_types:
        try:
            return dst_type_or_obj(src)
        except Exception as e:
            raise ValueError(f"cannot convert {src!r} to {dst_type_or_obj}: {e}")

    # prepare destination instance
    if isinstance(dst_type_or_obj, type):
        dst = dst_type_or_obj()
        # ensure defaults are present before applying config values
        try:
            apply_defaults(dst)
        except Exception:
            pass
    else:
        dst = dst_type_or_obj

    # if src is not a dict and dst is a simple object, attempt conversion via yaml bridge
    if not isinstance(src, dict):
        dumped = yaml.safe_dump(src)
        loaded = yaml.safe_load(dumped)
        if not hasattr(dst, "__dict__"):
            return loaded
        src = loaded

    # merge dict src into dst instance attributes, skipping None values
    if isinstance(src, dict) and hasattr(dst, "__dict__"):
        for k, v in src.items():
            # skip explicit nulls to preserve defaults
            if v is None:
                continue

            # if existing attribute is a dataclass and incoming value is a dict, merge recursively
            try:
                if hasattr(dst, k):
                    cur = getattr(dst, k)
                else:
                    cur = None
                if cur is not None and is_dataclass(cur) and isinstance(v, dict):
                    decode_to(cur, v)
                    continue

                # dict merger: update existing dict rather than replace entirely
                if isinstance(cur, dict) and isinstance(v, dict):
                    for kk, vv in v.items():
                        if vv is None:
                            continue
                        cur[kk] = vv
                    continue

                # otherwise set attribute (perform basic conversion via yaml bridge)
                try:
                    # attempt to convert scalars via yaml to respect booleans/numbers
                    dumped = yaml.safe_dump(v)
                    loaded = yaml.safe_load(dumped)
                    setattr(dst, k, loaded)
                except Exception:
                    try:
                        setattr(dst, k, v)
                    except Exception:
                        pass
            except Exception:
                # ignore unknown/read-only attributes
                pass
        return dst

    # fallback: use yaml bridge to construct a plain value
    dumped = yaml.safe_dump(src)
    return yaml.safe_load(dumped)


def lookup_config(path: str, root: dict) -> tuple[Any, bool]:
    """Lookup dot-separated path inside root dict. Returns (value, True) or (None, False)."""
    if path == "":
        return root, True
    parts = [p.strip() for p in path.split(".") if p.strip()]
    cur = root
    for p in parts:
        if isinstance(cur, dict) and p in cur:
            cur = cur[p]
        else:
            return None, False
    return cur, True


def decode_config(path: str, dst_type_or_obj: Any, root: dict) -> Any:
    val, ok = lookup_config(path, root)
    if not ok:
        raise KeyError(f"path not found: {path}")
    # create/prepare destination instance first and apply defaults
    if isinstance(dst_type_or_obj, type):
        obj = dst_type_or_obj()
    else:
        obj = dst_type_or_obj

    try:
        apply_defaults(obj)
    except Exception:
        pass

    # then merge configuration values into the object, only overwriting when present
    result = decode_to(obj, val)
    return result