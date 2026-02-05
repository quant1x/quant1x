# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

"""
Dataclass 序列化与反射工具
提供 CSV 与 dataclass 实例列表之间的双向转换，以及字段元信息提取。
"""
from typing import Type, List, Dict,  get_type_hints
from dataclasses import fields, is_dataclass
from quant1x.types import T
from . import strings

def get_field_names(cls: Type[T]) -> List[str]:
    """
    获取 dataclass 类的所有字段名称列表
    
    Args:
        cls (Type[T]): 需要获取字段名的 dataclass 类
    
    Returns:
        List[str]: 包含所有字段名称的列表
    
    Raises:
        TypeError: 如果传入的类不是 dataclass 类型
    """
    if not is_dataclass(cls):
        raise TypeError(
            f"Expected a dataclass type, but got {cls!r}. "
            "Please decorate your class with @dataclass."
        )
    cls_fields = fields(cls)
    field_names = [strings.to_snake_case(f.name) for f in cls_fields]
    return field_names

# def get_field_types(cls: Type[T]) -> Dict[str, Type]:
#     """
#     获取 dataclass 类的所有字段类型列表
    
#     Args:
#         cls (Type[T]): 需要获取字段类型的 dataclass 类
    
#     Returns:
#         List[Type[Any]]: 包含所有字段类型的列表
    
#     Raises:
#         TypeError: 如果传入的类不是 dataclass 类型
#     """
#     if not is_dataclass(cls):
#         raise TypeError(
#             f"Expected a dataclass type, but got {cls!r}. "
#             "Please decorate your class with @dataclass."
#         )
#     field_types = get_type_hints(cls)
#     field_types = {strings.to_snake_case(k): v for k, v in field_types.items()}
#     return field_types