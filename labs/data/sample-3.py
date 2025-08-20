#!/usr/bin/python
# -*- coding: UTF-8 -*-
import scipy.stats as stats

ci = 90


def isf(ci: int):
    alpha = 1 - ci / 10000
    length = 1000000
    n = length

    df = n - 2  # degrees of freedom
    tval = stats.t.isf(alpha, df)  # appropriate t value
    return tval


for i in range(0, 10001):
    print(i, isf(i), ',')
# print('tval=', tval)
# print(isf(9500))
