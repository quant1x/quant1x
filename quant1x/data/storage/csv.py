# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations
import csv
from pathlib import Path
from typing import List, Type, Union, get_type_hints
from dataclasses import dataclass, fields, is_dataclass
from quant1x.types import T
from quant1x.std import get_field_names, strings


def csv_to_slice(filepath: Union[str, Path], cls: Type[T]) -> List[T]:
    """
    从 CSV 加载为 dataclass 实例列表
    
    :param filepath: CSV 文件路径
    :param cls: 目标 dataclass 类（必须用 @dataclass 装饰）
    :return: List[cls]
    """
    if not is_dataclass(cls):
        raise TypeError("cls must be a dataclass (decorated with @dataclass)")

    filepath = Path(filepath)
    if not filepath.exists():
        return []

    #field_names = get_field_names(cls)
    # 使用 get_type_hints 解析字符串注解为实际类型
    original_field_types = get_type_hints(cls)
    print(f'field_types={original_field_types}')
    # 转换为字段名称 -> 类型映射
    field_map = {}
    field_names = []
    field_types = {}
    for cls_field_name, v in original_field_types.items():
        csv_field_name = strings.to_snake_case(cls_field_name)
        field_types[cls_field_name]=v
        field_map[csv_field_name] = cls_field_name
        field_names.append(csv_field_name)
    
    instances = []
    with open(filepath, 'r', encoding='utf-8', newline='') as f:
        reader = csv.reader(f)
        try:
            file_header = next(reader)
        except StopIteration:
            return []  # 空文件

        # 验证 header 是否匹配（顺序和名称必须一致）
        if file_header != field_names:
            raise ValueError(
                f"CSV header {file_header} does not match dataclass fields {field_names}. "
                "Field order and names must be identical."
            )

        for row_index, row in enumerate(reader, start=1):
            if len(row) != len(field_names):
                raise ValueError(f"Row {row_index} has {len(row)} columns, expected {len(field_names)}")

            kwargs = {}
            for csv_field_name, value_str in zip(field_names, row):
                name = field_map[csv_field_name]
                print(f'name={name}, value_str={value_str}')
                target_type = field_types[name]
                try:
                    # 基本类型自动转换
                    if target_type is str:
                        kwargs[name] = value_str
                    elif target_type is int:
                        kwargs[name] = int(value_str)
                    elif target_type is float:
                        kwargs[name] = float(value_str)
                    elif target_type is bool:
                        kwargs[name] = value_str.lower() in ('true', '1', 'yes', 'on')
                    else:
                        # 未知类型：保留字符串（或可抛异常）
                        kwargs[name] = value_str
                except Exception as e:
                    raise ValueError(
                        f"Failed to convert value '{value_str}' to {target_type} "
                        f"for field '{name}' at row {row_index}: {e}"
                    )

            instances.append(cls(**kwargs))

    return instances

def slice_to_csv(filepath: Union[str, Path],  data: List[T]) -> None:
    """保存 dataclass 实例列表到 CSV"""
    if not data:
        # 空列表：无法推断结构，拒绝保存（或可创建空 header？）
        # 这里选择：不写文件
        return
    
    first = data[0]
    cls = type(first)
    if not is_dataclass(cls):
        raise TypeError("Only dataclass instances are supported. "
                        "Each item must be created with @dataclass.")
    
    cls_fields = fields(cls)
    header = get_field_names(cls)
    
    # 将每个实例转为行（按字段顺序）
    rows = []
    for obj in data:
        if type(obj) is not cls:
            raise TypeError(f"All items must be of the same dataclass type: {cls.__name__}")
        row = [getattr(obj, f.name) for f in cls_fields]
        rows.append(row)

    # 写入文件
    filepath = Path(filepath)
    filepath.parent.mkdir(parents=True, exist_ok=True)
    with open(filepath, 'w', encoding='utf-8', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(header)
        writer.writerows(rows)

if __name__ == "__main__":
    @dataclass
    class Person:
        Name: str
        Age: int

    filename = "people.csv"
    
    # 保存
    people = [Person("Alice", 30), Person("Bob", 25)]
    slice_to_csv("people.csv", people)

    # 加载
    loaded: List[Person] = csv_to_slice("people.csv", Person)
    print(loaded)  # [Person(name='Alice', age=30), Person(name='Bob', age=25)]

    # 验证类型
    assert isinstance(loaded[0], Person)
    print(f'age={loaded[0].Age}')
    assert loaded[0].Age == 30