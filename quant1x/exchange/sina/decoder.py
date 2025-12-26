# -*- coding: utf-8 -*-
"""
Finance Decoder - Python implementation converted from Go
用于解码新浪财经数据的解码器
"""

import math
import time
from typing import List, Dict, Any, Optional


class FinanceDecoder:
    def __init__(self, data: str):
        self.branch_type: int = 0
        self.encoded_data: str = data
        self.indices: List[int] = []
        self.base64_chars: str = ""
        self.e: int = 0
        self.o: int = 0
        self.n: int = 0
        self.r: Dict[str, int] = {}
        self.h: List[int] = []
        self.s: int = 0
        self.u_val: int = 7657
        self.l_val: int = 86400000
        self.d_mask: int = ~(3 << 30)
        self.f_mask: int = 1 << 30
        self.p: List[int] = [0, 3, 5, 6, 9, 10, 12, 15, 17, 18, 20, 23, 24, 27, 29, 30]

        self.init_base64()
        self.init_powers()
        self.decode_base64()
        self.r = {}
        self.e, self.o = 0, 0
        u = self.w([12, 6], None, None)
        self.s = int(63 ^ u[1])
        self.branch_type = int(u[0])

    def init_base64(self) -> None:
        """初始化 Base64 字符集"""
        sb = []
        for i in range(26):
            sb.append(chr(i + 65))  # A-Z
        for i in range(26):
            sb.append(chr(i + 97))  # a-z
        for i in range(10):
            sb.append(chr(i + 48))  # 0-9
        sb.append('+')
        sb.append('/')
        self.base64_chars = ''.join(sb)

    def init_powers(self) -> None:
        """初始化 2 的幂"""
        self.h = [1 << i for i in range(64)]

    def decode_base64(self) -> None:
        """解码 Base64 数据"""
        self.indices = []
        for c in self.encoded_data:
            pos = self.base64_chars.find(c)
            if pos != -1:
                self.indices.append(pos)
            else:
                self.indices.append(0)
        self.n = len(self.indices)

    def y(self) -> bool:
        """读取一位"""
        if self.e >= self.n:
            return False
        t = (self.indices[self.e] & (1 << self.o)) != 0
        self.o += 1
        if self.o >= 6:
            self.o -= 6
            self.e += 1
        return t

    def N(self) -> int:
        """读取变长整数"""
        t = self.y()
        e = 1
        while self.y():
            e += 1
        return e if t else -e

    def w(self, t: List[int], r_param: Optional[List[int]], a_param: Optional[List[int]]) -> List[int]:
        """核心解码函数"""
        l = [0] * len(t)
        r = r_param if r_param else [0] * len(t)
        a = a_param if a_param else [0] * len(t)

        for i in range(len(t)):
            c = t[i]
            u = 0
            if c != 0:
                if self.e >= self.n:
                    return [0] * len(l)

                if c <= 0:
                    u = 0
                elif c <= 30:
                    while c > 0:
                        delta = 6 - self.o
                        if c < delta:
                            delta = c
                        bits = (self.indices[self.e] >> self.o) & ((1 << delta) - 1)
                        shift = t[i] - c
                        u |= bits << shift
                        self.o += delta
                        if self.o >= 6:
                            self.o -= 6
                            self.e += 1
                        c -= delta

                    if i < len(r) and r[i] != 0 and u >= self.h[t[i] - 1]:
                        u -= self.h[t[i]]
                else:
                    sub_t = [30, c - 30]
                    sub_r = [0, 0]
                    if i < len(r):
                        sub_r[1] = r[i]
                    sub_result = self.w(sub_t, sub_r, None)
                    if i < len(a) and a[i] == 0:
                        u = sub_result[0] + sub_result[1] * self.h[30]
                    else:
                        u = sub_result[0]

            l[i] = u
        return l

    def x(self, t: int) -> str:
        """生成日期字符串"""
        for i in range(t):
            self.r["d"] = self.r.get("d", 0) + 1
            n = self.r["d"] % 7
            if n == 3 or n == 4:
                self.r["d"] += 5 - n

        timestamp = (self.u_val + self.r["d"]) * self.l_val
        tm = time.gmtime(timestamp / 1000)
        return time.strftime("%Y-%m-%d", tm)

    def S(self) -> List[Dict[str, str]]:
        """解码日线数据"""
        result = []
        if self.s >= 1:
            return result

        init_data = self.w([18], [1], None)
        self.r["d"] = int(init_data[0] - 1)
        a = self.w([3, 3, 30, 6], None, None)
        self.r["p"] = int(a[0])
        self.r["ld"] = int(a[1])
        self.r["cd"] = int(a[2])
        self.r["c"] = int(a[3])
        self.r["m"] = int(math.pow(10, self.r["p"]))
        self.r["pc"] = self.r["cd"]

        t = 0
        while True:
            day_data = {"d": 1}
            if self.y():
                a_val = self.w([3], None, None)
                if a_val[0] == 0:
                    day_data["d"] = int(self.w([6], None, None)[0])
                elif a_val[0] == 1:
                    self.r["d"] = int(self.w([18], None, None)[0])
                    day_data["d"] = 0
                else:
                    day_data["d"] = int(a_val[0])

            l = {}
            l["day"] = self.x(day_data["d"])

            if self.y():
                self.r["ld"] += self.N()

            a_close = self.w([3 * self.r["ld"]], [1], None)
            self.r["cd"] += int(a_close[0])
            l["close"] = f"{self.r['cd'] / self.r['m']:.6f}"
            result.append(l)

            if self.e >= self.n or (self.e == self.n - 1 and (63 & (self.r["c"] ^ (t + 1))) == 0):
                break
            t += 1

        if result:
            if self.r["m"] != 0:
                result[0]["prevclose"] = f"{self.r['pc'] / self.r['m']:.6f}"
            else:
                result[0]["prevclose"] = f"{self.r['pc']:.6f}"

        return result

    def _decode_minute_data(self) -> List[Dict[str, str]]:
        """解码分时数据"""
        result = []
        if self.s > 2:
            return result

        c = []
        self.r["d"] = int(self.w([18], [1], None)[0]) - 1
        header = {"day": self.x(1)}

        if self.s < 1:
            a = self.w([3, 3, 4, 1, 1, 1, 5], None, None)
        else:
            a = self.w([4, 4, 4, 1, 1, 1, 3], None, None)

        names = ["la", "lp", "lv", "tv", "rv", "zv", "pp"]
        for i in range(min(7, len(a))):
            self.r[names[i]] = int(a[i])

        self.r["m"] = int(math.pow(10, self.r["pp"]))

        t = 0
        while not (self.e >= self.n) and (self.e != self.n - 1 or (7 & (self.r["c"] ^ t)) != 0):
            lmap = {}
            o = {}

            total_v = 0
            for i in range(3):
                pkey = ["v", "p", "a"][i]
                has = True
                if pkey == "v" and self.r["tv"] == 0:
                    has = False
                if has and self.y():
                    self.r["l" + pkey] += self.N()

                u = 1
                if pkey == "v" and self.r["rv"] != 0:
                    u = 1 if self.y() else 0

                val = 0
                extra = 7 if (pkey == "v" and u == 1) else 0
                call_len = 3 * self.r["l" + pkey] + extra
                vals = self.w([call_len], [1 if i != 0 else 0], None)
                if vals:
                    val = vals[0]

                if u == 0:
                    val *= 100

                o[pkey] = val
                if pkey == "v":
                    total_v += val
                    if val == 0 and (self.s > 1 or 241 > t) and (self.r["zv"] != 0 or not self.y()):
                        o["p"] = 0

            self.r["sv"] = self.r.get("sv", 0) + int(total_v)
            self.r["cp"] = int(self.r.get("cp", 0) + o["p"])

            open_price = self.r["cp"] / self.r["m"]
            lmap["volume"] = str(total_v)
            lmap["price"] = f"{open_price:.6f}"
            lmap["avg_price"] = f"{open_price:.6f}"

            c.append(lmap)
            t += 1

        if c:
            c[0]["date"] = header["day"]
            c[0]["prevclose"] = f"{self.r.get('pc', 0) / self.r['m']:.6f}"

        return c

    def T(self) -> List[Dict[str, str]]:
        """解码K线数据"""
        result = []
        if self.s >= 1:
            return result

        res = []
        self.r["lv"] = 0
        self.r["ld"] = 0
        self.r["cd"] = 0
        self.r["cv0"] = 0
        self.r["cv1"] = 0

        pval = self.w([6], None, None)
        if pval:
            self.r["p"] = int(pval[0])

        self.r["d"] = int(self.w([18], [1], None)[0]) - 1
        self.r["m"] = int(math.pow(10, self.r["p"]))

        a = self.w([3, 3], None, None)
        if len(a) > 1:
            self.r["md"] = int(a[0])
            self.r["mv"] = int(a[1])

        while True:
            a6 = self.w([6], None, None)
            if not a6:
                break

            i = {"c": int(a6[0]), "d": 1}

            if 32 & i["c"] != 0:
                while True:
                    a2 = int(self.w([6], None, None)[0])
                    if 63 == (16 | a2):
                        l = "x" if 16 & a2 != 0 else "u"
                        a3 = self.w([3, 3], None, None)
                        i[l + "_d"] = int(a3[0]) + self.r["md"]
                        i[l + "_v"] = int(a3[1]) + self.r["mv"]
                        break
                    if 32 & a2 != 0:
                        o = "d" if 8 & a2 != 0 else "v"
                        l = "x" if 16 & a2 != 0 else "u"
                        i[l + "_" + o] = (7 & a2) + self.r["m" + o]
                        break

                    o = 15 & a2
                    if o == 0:
                        i["d"] = int(self.w([6], None, None)[0])
                    elif o == 1:
                        tmp = int(self.w([18], None, None)[0])
                        self.r["d"] = tmp
                        i["d"] = 0
                    else:
                        i["d"] = o

                    if 16 & a2 == 0:
                        break

            rec = {}
            rec["date"] = self.x(i["d"])

            llist = [0] * 5
            u_d = i.get("u_d", 0)
            u_v = i.get("u_v", 0)
            for e in range(4):
                llist[e] = u_d
            llist[4] = u_v

            lidx = 15 & i["c"]
            lval = self.p[lidx] if 0 <= lidx < len(self.p) else 0

            if (1 & u_v) != 0:
                lval = 31 - lval

            if (16 & i["c"]) != 0:
                llist[4] += 2

            for e in range(5):
                bit = 1 << (4 - e)
                if (lval & bit) != 0:
                    llist[e] += 1
                llist[e] *= 3

            dvals = self.w(llist, [1, 0, 0, 1, 1], [0, 0, 0, 0, 1])

            o0 = dvals[0] if dvals else 0
            open_price = (self.r["cd"] + o0) / self.r["m"]
            high = open_price
            low = open_price
            close_val = open_price

            if len(dvals) > 1:
                high = (self.r["cd"] + dvals[1]) / self.r["m"]
            if len(dvals) > 2:
                low = (self.r["cd"] - dvals[2]) / self.r["m"]
            if len(dvals) > 3:
                close_val = (self.r["cd"] + dvals[3]) / self.r["m"]
                self.r["cd"] += int(dvals[3])

            rec["open"] = f"{open_price:.6f}"
            rec["high"] = f"{high:.6f}"
            rec["low"] = f"{low:.6f}"
            rec["close"] = f"{close_val:.6f}"

            a4 = dvals[4] if len(dvals) > 4 else 0
            cv0 = self.r["cv0"] + int(a4)
            cv1 = self.r["cv1"]

            if (cv0 & self.d_mask) != 0:
                carry = ((cv0 & ~self.d_mask) + (int(a4) & ~self.d_mask)) >> 30
                cv1 += carry
                cv0 &= ~self.d_mask

            self.r["cv0"] = cv0
            self.r["cv1"] = cv1
            volume = (cv0 & (self.f_mask - 1)) + cv1 * self.f_mask
            rec["volume"] = str(volume)

            res.append(rec)

        return res

    def k(self) -> List[str]:
        """解码日期列表"""
        result = []
        if self.s > 1:
            return result

        self.r["l"] = 0
        n_count = -1
        t_initialized = False
        self.r["d"] = int(self.w([18], None, None)[0] - 1)
        target_date = int(self.w([18], None, None)[0])

        while self.r["d"] < target_date:
            current_date = self.x(1)
            if n_count <= 0:
                if self.y():
                    self.r["l"] += self.N()
                count_data = self.w([3 * self.r["l"]], [0], None)
                n_count = int(count_data[0]) + 1 if count_data else 1
                if not t_initialized:
                    result.append(current_date)
                    n_count -= 1
                    t_initialized = True
            else:
                result.append(current_date)
            n_count -= 1

        return result

    def _mi_run(self) -> List[List[int]]:
        """解码分钟间隔数据"""
        result = []
        if self.s >= 1:
            return result

        wf = self.w([6], None, None)
        if not wf:
            return result
        rf = int(wf[0])

        wc = self.w([6], None, None)
        if not wc:
            return result
        rc = int(wc[0])

        dv = [0] * rf
        dl = [0] * rf

        t = 0
        while not (self.e >= self.n) and (self.e != self.n - 1 or (7 & (rc ^ t)) != 0):
            o = []
            for i in range(rf):
                if self.y():
                    dl[i] += self.N()
                vals = self.w([3 * dl[i]], [1], None)
                v = vals[0] if vals else 0
                dv[i] += v
                o.append(dv[i])
            result.append(o)
            t += 1

        return result

    def decode(self) -> Any:
        """主解码函数"""
        branch = self.branch_type
        if branch in [1479, 136]:
            return self.T()
        elif branch == 200:
            return self.S()
        elif branch == 139:
            return self.k()
        elif branch == 197:
            data = self._mi_run()
            result = []
            for i, item in enumerate(data):
                result.append({"index": str(i)})
            return result
        else:
            return []