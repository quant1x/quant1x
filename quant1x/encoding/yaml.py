# -*- coding: UTF-8 -*-

import os

import yaml
from icecream import ic


class DictToClass(object):
    """
    将字典准换为 class 类型
    """

    @classmethod
    def to_object(cls, _obj):
        _obj_ = type('new', (object,), _obj)
        [setattr(_obj_, key, cls.to_object(value)) if isinstance(value, dict) else setattr(_obj_, key, value) for
         key, value in _obj.items()]
        return _obj_


class ReadConfigFiles(object):
    def __init__(self):
        """
        获取当前工作路径
        """
        self.works_path = os.path.dirname(os.path.realpath(__file__))

    @classmethod
    def open_file(cls):
        """
        读取当前工作目录下cfg.yaml文件内容，并返回字典类型
        :return:
        """
        return yaml.load(
            open(os.path.join(cls().works_path, r'cfg.yaml'), 'r', encoding='utf-8').read(), Loader=yaml.FullLoader
        )

    @classmethod
    def cfg(cls, item=None):
        """
        调用该方法获取需要的配置，item如果为None，返回则是全部配置
        :param item:
        :return:
        """
        return DictToClass.to_object(cls.open_file().get(item) if item else cls.open_file())


if __name__ == '__main__':
    cfg = ReadConfigFiles.cfg()
    # 测试输出
    ic(cfg.host.group)
