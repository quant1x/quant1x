# -*- coding: UTF-8 -*-
from multiprocessing import cpu_count


def cpu_num() -> int:
    """
    获得CPU核数
    :return:
    """
    return cpu_count()


def max_procs() -> int:
    """
    最大可使用CPU核数
    默认使用一半的CPU资源
    :return:
    """
    num = cpu_num()
    return int(num / 2)


if __name__ == '__main__':
    print(cpu_num())
    print(max_procs())
