#!/usr/bin/python
# -*- coding: UTF-8 -*-

from datetime import datetime

now = datetime.now()
start = datetime(year=now.year, month=now.month, day=now.day, hour=9, minute=30)
print(now)
print(now.minute)
ms = (now - start).seconds
print(ms)
