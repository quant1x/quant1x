# -*- coding: utf-8 -*-
import os
import threading
from pathlib import Path
from dataclasses import dataclass, field
from typing import Any, Dict
import yaml
from quant1x.std.filepath import expand_user
from .defaults import apply_defaults

LANGUAGE = "py"
DEFAULT_BASE_PATH = f"~/.q1x-{LANGUAGE}"
QUANT1X_CONFIG_FILENAME = "quant1x.yaml"

@dataclass
class BaseConfig:
    debug: bool = field(default=False)
    basedir: str = ""
    logdir: str = ""
    filename: str = ""
    config_map: Dict[str, Any] = field(default_factory=dict)

_QUANT1X_BASE_PATH = None

def get_base_path() -> str:
    """
    返回默认的基础路径，如果无法展开用户目录则返回默认路径
    """
    global _QUANT1X_BASE_PATH
    if _QUANT1X_BASE_PATH is None:
        _QUANT1X_BASE_PATH = expand_user(DEFAULT_BASE_PATH)
    return _QUANT1X_BASE_PATH

def get_meta_path() -> str:
    """
    返回元数据存储的基础路径
    meta目录位于基础路径下的meta子目录中
    """
    return os.path.join(get_base_path(), "meta")

# singleton-ish state mirroring Go package vars
_cache_once_lock = threading.Lock()
_cache_inited = False
_cache_cfg = BaseConfig()

def parse_yaml_config(filename: str, config: BaseConfig) -> None:
    # Apply defaults
    apply_defaults(config)
    config.filename = filename.strip()
    config.config_map = {}

    if not os.path.exists(filename):
        config.basedir = get_base_path()
        config.logdir = os.path.join(config.basedir, "logs")
        return

    with open(filename, "rb") as f:
        data = f.read()

    # first keep a generic map
    try:
        node = yaml.safe_load(data)
        if isinstance(node, dict):
            config.config_map = node
    except Exception as e:
        raise RuntimeError(f"failed to parse config file: {e}")

    # then fill typed fields
    try:
        parsed = yaml.safe_load(data)
        if isinstance(parsed, dict):
            if parsed.get("basedir"):
                config.basedir = os.path.expanduser(str(parsed.get("basedir")))
            else:
                config.basedir = get_base_path()

            if parsed.get("logdir"):
                config.logdir = os.path.expanduser(str(parsed.get("logdir")))
            else:
                config.logdir = os.path.join(config.basedir, "logs")

            config.config_map["basedir"] = config.basedir
            config.config_map["logdir"] = config.logdir
            config.debug = bool(parsed.get("debug", False))
    except Exception as e:
        raise RuntimeError(f"failed to parse typed config: {e}")

    # Apply dataclass metadata defaults after parsing/normalization
    try:
        apply_defaults(config)
    except Exception:
        pass

def lazy_init_cache_config() -> None:
    global _cache_inited, _cache_cfg
    with _cache_once_lock:
        if _cache_inited:
            return
        cfg_filename = os.path.join(get_base_path(), QUANT1X_CONFIG_FILENAME)
        try:
            parse_yaml_config(cfg_filename, _cache_cfg)
        except Exception as e:
            raise RuntimeError(f"failed to parse config file: {e}")
        _cache_inited = True

def get_configfile_path() -> str:
    lazy_init_cache_config()
    return _cache_cfg.filename

def get_logs_path() -> str:
    lazy_init_cache_config()
    return _cache_cfg.logdir

def get_data_path() -> str:
    lazy_init_cache_config()
    return _cache_cfg.basedir

def get_config_map() -> Dict[str, Any]:
    lazy_init_cache_config()
    return dict(_cache_cfg.config_map) if _cache_cfg.config_map is not None else {}

def get_config_map_ref() -> Dict[str, Any]:
    lazy_init_cache_config()
    return _cache_cfg.config_map


if __name__ == "__main__":
    print("Quant1X Config Test")
    print("Base Path:", get_base_path())
    print("Meta Path:", get_meta_path())
    print("Config File Path:", get_configfile_path())
    print("Logs Path:", get_logs_path())
    print("Data Path:", get_data_path())
    print("Config Map:", get_config_map())