#!/usr/bin/python
# -*- coding: UTF-8 -*-
import numpy as np

d = [5, 32, 38, -33, -19, -10, 1, -8, -20, 10, -1, 4, 11, -1, -7, -2]
f = np.fft.fft(d)
print(f.real)
