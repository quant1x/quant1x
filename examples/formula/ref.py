#!/usr/bin/python
# -*- coding: UTF-8 -*-
import numpy as np

from quant1x import formula
from quant1x.data import D

data = D.dataset('000002')
HIGH = data['high']
N = np.repeat(0, len(HIGH))
data['ref1'] = formula.REF(HIGH, N)

print(data)