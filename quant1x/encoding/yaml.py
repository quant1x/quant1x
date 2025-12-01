# -*- coding: UTF-8 -*-

import os
import yaml
from types import SimpleNamespace

try:
    from yaml import CSafeLoader as Loader, CSafeDumper as Dumper
except ImportError:
    from yaml import SafeLoader as Loader, SafeDumper as Dumper


def to_namespace(data):
    """
    递归将字典转换为 SimpleNamespace，支持属性访问
    """
    if isinstance(data, dict):
        return SimpleNamespace(**{k: to_namespace(v) for k, v in data.items()})
    elif isinstance(data, list):
        return [to_namespace(i) for i in data]
    return data


def load(path, to_object=False):
    """
    读取YAML文件
    :param path: 文件路径
    :param to_object: 是否转换为对象(SimpleNamespace)
    :return: 字典或对象
    """
    if not os.path.exists(path):
        return None

    with open(path, 'r', encoding='utf-8') as f:
        data = yaml.load(f, Loader=Loader)

    if to_object:
        return to_namespace(data)
    return data


def dump(data, path):
    """
    保存为YAML文件
    :param data: 数据
    :param path: 文件路径
    """
    # 注意：SimpleNamespace 不能直接被 yaml dump，如果需要保存对象，需先转回 dict
    # 这里假设 data 是 dict, list 或基础类型
    with open(path, 'w', encoding='utf-8') as f:
        yaml.dump(data, f, Dumper=Dumper, allow_unicode=True)


class ReadConfigFiles(object):
    """
    带缓存的配置文件读取类
    """
    _caches = {}  # path -> {'mtime': float, 'data': dict}

    @classmethod
    def cfg(cls, path, item=None):
        """
        调用该方法获取需要的配置，带缓存机制
        :param path: 配置文件路径
        :param item: 配置项名称
        :return: SimpleNamespace 对象或具体值
        """
        if not path or not os.path.exists(path):
            return None

        # 检查文件修改时间，如果有变化则重新加载
        try:
            mtime = os.path.getmtime(path)
            cache = cls._caches.get(path)

            if cache is None or mtime > cache['mtime']:
                data = load(path)
                cls._caches[path] = {'mtime': mtime, 'data': data}
            else:
                data = cache['data']
        except Exception:
            return None

        if not data:
            return None

        if item:
            val = data.get(item)
            return to_namespace(val) if isinstance(val, (dict, list)) else val

        return to_namespace(data)


if __name__ == '__main__':
    try:
        from icecream import ic
        # 示例：需要传入具体路径
        # cfg = ReadConfigFiles.cfg('path/to/config.yaml')
        pass
    except ImportError:
        pass
