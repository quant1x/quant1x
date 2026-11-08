# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

import os
import sys

# Ensure repository root is on sys.path so tests can import the `quant1x` package.
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
if ROOT not in sys.path:
    sys.path.insert(0, ROOT)
