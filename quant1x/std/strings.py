# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import re

def to_snake_case(word: str) -> str:
    """
    将驼峰命名法或混合格式的字符串转换为蛇形命名法(snake_case)
    
    Args:
        word (str): 需要转换的原始字符串，可以是驼峰命名(CamelCase)、帕斯卡命名(PascalCase)或混合格式
    
    Returns:
        str: 转换后的蛇形命名法字符串，全部小写并用下划线分隔单词
    
    处理规则:
        1. 在大小写字母交界处插入下划线
        2. 处理连续大写字母后跟小写字母的情况
        3. 将所有非字母数字字符替换为下划线
        4. 最终转换为全小写并去除首尾下划线
    """
    # 将连续的大写字母视为一个词（如 XMLParser → xml_parser）
    # 先在大小写交界处插入下划线（CamelCase → Camel_Case）
    s1 = re.sub('([a-z0-9])([A-Z])', r'\1_\2', word)
    # 处理连续大写字母后跟小写字母的情况（如 XMLParser → XML_Parser）
    s2 = re.sub('([A-Z]+)([A-Z][a-z])', r'\1_\2', s1)
    # 将非字母数字字符（如空格、连字符等）替换为下划线
    s3 = re.sub(r'[^a-zA-Z0-9]+', '_', s2)
    # 转小写并去除首尾下划线
    return s3.lower().strip('_')