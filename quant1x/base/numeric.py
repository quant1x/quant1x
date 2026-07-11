# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import math
import bisect
from typing import Union, List, Tuple, Iterable
import numpy as np


def is_nan(n) -> bool:
    """
    判断是否nan或inf
    :param n:
    :return:
    """
    return np.isnan(n) or np.isinf(n)


def float_round(num: float, digits: int = 2) -> float:
    """
    浮点四舍五入
    :param num:
    :param digits: 小数点后几位数字, 默认两位
    :return:
    """
    # retain previous behaviour using Decimal for exact rounding
    from decimal import Decimal, ROUND_HALF_UP
    s = str(num)
    x = Decimal(s).quantize((Decimal('0.' + '0' * digits)), rounding=ROUND_HALF_UP)
    return float(x)


def fix_float(f: float) -> float:
    """
    修复f, 处理存在Nan和±Inf的情况
    :param f:
    :return:
    """
    return f if not is_nan(f) else 0


def change_rate(base: float, value: float) -> float:
    """
    计算变化率
    :param base: 基准值
    :param value: 变化值
    :return: 变化率
    """
    if base == 0:
        return 0.0
    return value / base


def decimal(f: float, digits: int = 2) -> float:
    """
    保留小数位, 使用与 C++ 等效的无分支算法
    :param f: 浮点数
    :param digits: 小数位数
    :return:
    """
    # Implement same branchless algorithm as C++ numeric::decimal
    # clamp digits to [0,9]
    if math.isnan(f):
        return 0.0
    if digits < 0:
        digits = 0
    if digits > 9:
        digits = 9

    kPowersOf10 = [10.0 ** i for i in range(0, 11)]

    half = math.copysign(5.0, f)
    nj1 = kPowersOf10[digits + 1]
    scaled = f * nj1 + half
    truncated = math.trunc(scaled / 10.0)
    return truncated / (nj1 / 10.0)

class NumberRange:
    def __init__(self, *args, **kwargs):
        """
        初始化数值范围对象, 支持多种参数格式
        
        Args:
            *args: 可变参数, 支持以下格式: 
                - 两个数值 (start, end)
                - 列表/元组包含多个数值或范围对
                - 集合包含多个离散值
                - 另一个 NumberRange 对象
            **kwargs: 可选参数, 包含: 
                include_start (bool): 默认包含范围起始值, 默认为 True
                include_end (bool): 默认包含范围结束值, 默认为 True
        
        Raises:
            TypeError: 如果参数类型不符合要求
        """
        self._ranges = []
        self._starts = []
        
        default_include_start = kwargs.get('include_start', True)
        default_include_end = kwargs.get('include_end', True)
        
        # 如果没有参数, 返回空范围
        if not args:
            return
        
        # 情况1: 两个数值参数 NumberRange(1, 100) 或两个字符串参数 NumberRange("00001", "02799")
        if len(args) == 2:
            if all(isinstance(x, (int, float)) for x in args):
                self.add_range(args[0], args[1], default_include_start, default_include_end)
                return
            if all(isinstance(x, str) for x in args):
                self.add_range(args[0], args[1], default_include_start, default_include_end)
                return
        
        # 情况2: 处理其他参数
        for arg in args:
            if isinstance(arg, (list, tuple)):
                # 如果是元组 (1, 100) 或 ("00001", "02799"), 直接作为范围
                if len(arg) == 2 and all(isinstance(x, (int, float, str)) for x in arg):
                    self.add_range(arg[0], arg[1], default_include_start, default_include_end)
                else:
                    # 如果是列表/元组包含多个元素
                    for item in arg:
                        if isinstance(item, (list, tuple)) and len(item) == 2 and all(isinstance(x, (int, float, str)) for x in item):
                            self.add_range(item[0], item[1], default_include_start, default_include_end)
                        elif isinstance(item, (int, float)):
                            self.add_range(item, item, True, True)
            elif isinstance(arg, set):
                # 离散值集合
                for item in arg:
                    self.add_range(item, item, True, True)
            elif isinstance(arg, NumberRange):
                # 复制
                for r in arg._ranges:
                    self.add_range(*r)
    
    # 保持其他方法不变
    def add_range(self, start, end, include_start=True, include_end=True):
        if start > end:
            raise ValueError("start > end")
        if start == end and not (include_start and include_end):
            return
        
        new_range = (start, end, include_start, include_end)
        
        if not self._ranges:
            self._ranges.append(new_range)
            self._starts.append(start)
            return
        
        idx = bisect.bisect_left(self._starts, start)
        self._ranges.insert(idx, new_range)
        self._starts.insert(idx, start)
        self._merge_ranges()
    
    def _merge_ranges(self):
        """
        合并重叠或相邻的范围区间
        
        将内部存储的多个范围区间(_ranges)合并为不重叠的连续区间, 并更新对应的起始点列表(_starts). 
        每个范围区间由四元组(start, end, include_start, include_end)表示, 其中include_start和include_end表示是否包含端点. 
        
        处理逻辑: 
        1. 当两个区间重叠或相邻且至少有一个包含端点时, 将它们合并为一个新区间
        2. 新区间的起止点为合并区间的最早开始和最晚结束
        3. 新区间的端点包含性由原区间中对应端点的包含性决定
        
        注意: 此方法会直接修改实例的_ranges和_starts属性
        """
        if len(self._ranges) <= 1:
            return
        
        merged = []
        merged_starts = []
        current = self._ranges[0]
        
        for i in range(1, len(self._ranges)):
            next_range = self._ranges[i]
            s1, e1, inc1_s, inc1_e = current
            s2, e2, inc2_s, inc2_e = next_range
            
            if e1 >= s2 or (e1 == s2 and (inc1_e or inc2_s)):
                new_start = min(s1, s2)
                new_end = max(e1, e2)
                include_start = inc1_s if s1 == new_start else inc2_s
                include_end = inc1_e if e1 == new_end else inc2_e
                current = (new_start, new_end, include_start, include_end)
            else:
                merged.append(current)
                merged_starts.append(current[0])
                current = next_range
        
        merged.append(current)
        merged_starts.append(current[0])
        self._ranges = merged
        self._starts = merged_starts
    
    def __contains__(self, value):
        """
        检查给定值是否在当前范围集合中
        
        Args:
            value: 要检查的值
        
        Returns:
            bool: 如果值在任何范围内则返回True, 否则返回False
        
        Note:
            范围检查包含以下逻辑: 
            - 值大于起始且小于结束
            - 如果值等于起始, 则检查是否包含起始(inc_start)
            - 如果值等于结束, 则检查是否包含结束(inc_end)
        """
        if not self._ranges:
            return False
        
        idx = bisect.bisect_right(self._starts, value) - 1
        if idx < 0:
            return False
        
        for i in range(idx, len(self._ranges)):
            start, end, inc_start, inc_end = self._ranges[i]
            if start > value:
                break
            left_ok = (value > start) or (value == start and inc_start)
            right_ok = (value < end) or (value == end and inc_end)
            if left_ok and right_ok:
                return True
        
        return False
    
    def __repr__(self):
        ranges_str = []
        for start, end, inc_s, inc_e in self._ranges:
            left = '[' if inc_s else '('
            right = ']' if inc_e else ')'
            ranges_str.append(f"{left}{start}, {end}{right}")
        return f"NumberRange({', '.join(ranges_str)})"
    
    def max_value_length(self) -> int:
        """
        获取所有范围中最大数值的字符串长度
        
        Returns:
            最大数值的字符串长度
        """
        max_len = 0
        for start, end, _, _ in self._ranges:
            max_len = max(max_len, len(str(start)), len(str(end)))
        return max_len

if __name__ == '__main__':
    # 1. 单个范围
    nr1 = NumberRange(1, 100)
    print(nr1)  # NumberRange([1, 100])

    # 2. 多个范围元组
    nr2 = NumberRange((1, 100), (200, 300))
    print(nr2)  # NumberRange([1, 100], [200, 300])

    # 3. 范围列表
    ranges = [(1, 100), (200, 300), (150, 180)]  # 会自动合并
    nr3 = NumberRange(ranges)
    print(nr3)  # NumberRange([1, 100], [150, 180], [200, 300])

    # 4. 离散值集合(自动转换为单点区间)
    discrete_set = {1, 3, 5, 7, 9}
    nr4 = NumberRange(discrete_set)
    print(nr4)  # NumberRange([1, 1], [3, 3], [5, 5], [7, 7], [9, 9])

    # 5. 复制构造
    nr5 = NumberRange(nr4)
    print(nr5)  # 与nr4相同

    # 6. 混合输入
    nr6 = NumberRange((1, 50), {60, 70, 80}, (90, 100))
    print(nr6)  # NumberRange([1, 50], [60, 60], [70, 70], [80, 80], [90, 100])

    # 7. 指定边界包含性
    nr7 = NumberRange((1, 100), include_start=False, include_end=False)
    print(nr7)  # NumberRange((1, 100))

    # 测试匹配
    print(1 in nr7)   # False (开区间)
    print(50 in nr7)  # True
    print(100 in nr7) # False
    print(99 in nr7)  # True