# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import time
import os
import json
import threading

class UnbreakableClock:
    def __init__(self, state_file=".clock_state"):
        self.state_file = state_file
        self._lock = threading.Lock()
        
        # 【解决点1：跨生命周期】从持久化加载历史最大时间
        self._history_max_ts = self._load_history_max()
        
        # 获取当前系统时间
        current_real_ts = time.time()
        
        # 如果当前真实时间比历史最大时间还小，说明重启后时间回拨了
        if current_real_ts < self._history_max_ts:
            # 强制修正基准，把历史最大值作为当前的起点
            current_real_ts = self._history_max_ts 
            
        # 【解决点2：运行期间】计算 Base，后续全靠 monotonic 撑住
        # Base = 修正后的真实时间 - 当前的单调时间
        self._base = current_real_ts - time.monotonic()
        
        # 记录运行期间的内存最大值（双重保险）
        self._runtime_max_ts = current_real_ts

    def get_safe_timestamp(self):
        """获取绝对安全、永不回拨的秒级时间戳"""
        with self._lock:
            # 运行期间，完全依赖 Base + monotonic，无视底层 NTP 回拨
            safe_ts = self._base + time.monotonic()
            
            # 内存级兜底（防御极端并发或浮点精度问题）
            if safe_ts < self._runtime_max_ts:
                safe_ts = self._runtime_max_ts + 0.000001
                
            self._runtime_max_ts = safe_ts
            
            # 定期或退出时持久化，防止下次启动回拨 (这里简化为每次写入)
            self._save_history_max(safe_ts) 
            
            return safe_ts

    def _load_history_max(self):
        if os.path.exists(self.state_file):
            with open(self.state_file, 'r') as f:
                return float(json.load(f).get('max_ts', 0.0))
        return 0.0

    def _save_history_max(self, ts):
        # 生产环境建议用原子写入
        with open(self.state_file, 'w') as f:
            json.dump({'max_ts': ts}, f)

# 使用
clock = UnbreakableClock()
print(clock.get_safe_timestamp())