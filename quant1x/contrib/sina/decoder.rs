use serde::Serialize;
use serde_json::{Map, Value};
use std::collections::HashMap;

use crate::Timestamp;

/// 解码器输出的单行记录（用于日历/历史数据行）
#[derive(Serialize, Debug, Clone, Default)]
pub struct Record {
    pub date: Option<String>,
    pub close: Option<String>,
    pub prevclose: Option<String>,
    pub open: Option<String>,
    pub high: Option<String>,
    pub low: Option<String>,
    pub volume: Option<String>,
    pub avg_price: Option<String>,
}

/// FinanceDecoder: 基于自定义 base64/位流的解码器，用于从压缩编码中解析日期/数值序列
pub struct FinanceDecoder {
    pub indices: Vec<u8>,
    pub e: usize,
    pub o: usize,
    pub n: usize,
    pub base64_chars: String,
    pub r: HashMap<String, i64>,
    pub h: Vec<i64>,
    pub s: i32,
    pub u_val: i64,
    pub l_val: i64,
    pub d_mask: i64,
    pub f_mask: i64,
    pub p: Vec<i32>,
    pub branch_type: i64,
    pub header_done: bool,
}

impl FinanceDecoder {
    pub fn new(_data: &str) -> Self {
        let mut base64_chars = String::new();
        for i in 0..26 {
            base64_chars.push((65 + i) as u8 as char);
        }
        for i in 0..26 {
            base64_chars.push((97 + i) as u8 as char);
        }
        for i in 0..10 {
            base64_chars.push((48 + i) as u8 as char);
        }
        base64_chars.push('+');
        base64_chars.push('/');

        let mut h = vec![0i64; 64];
        for i in 0..64 {
            h[i] = 1i64 << i;
        }

        FinanceDecoder {
            indices: Vec::new(),
            e: 0,
            o: 0,
            n: 0,
            base64_chars,
            r: HashMap::new(),
            h,
            s: 0,
            u_val: 7657,
            l_val: 86400000,
            d_mask: !(3i64 << 30),
            f_mask: 1i64 << 30,
            p: vec![0, 3, 5, 6, 9, 10, 12, 15, 17, 18, 20, 23, 24, 27, 29, 30],
            branch_type: 0,
            header_done: false,
        }
    }

    pub fn decode_base64(&mut self, data: &str) {
        self.indices.clear();
        for c in data.chars() {
            if let Some(pos) = self.base64_chars.find(c) {
                self.indices.push(pos as u8);
            } else {
                self.indices.push(0);
            }
        }
        self.n = self.indices.len();
        self.e = 0;
        self.o = 0;
    }

    pub fn set_branch(&mut self, branch: i64, s_val: i32) {
        self.branch_type = branch;
        self.s = s_val;
    }

    fn y(&mut self) -> bool {
        if self.e >= self.n {
            return false;
        }
        let t = (self.indices[self.e] & (1 << self.o)) != 0;
        self.o += 1;
        if self.o >= 6 {
            self.o -= 6;
            self.e += 1;
        }
        t
    }

    fn n_bits(&mut self) -> i32 {
        let t = self.y();
        let mut e = 1;
        while self.y() {
            e += 1;
        }
        if t {
            e
        } else {
            -e
        }
    }

    fn w(&mut self, t: &[i32], r_param: Option<&[i32]>, a_param: Option<&[i32]>) -> Vec<i64> {
        let mut l = vec![0i64; t.len()];
        let r = if let Some(rr) = r_param { rr } else { &[] };
        let a = if let Some(aa) = a_param { aa } else { &[] };
        for (i, &ti) in t.iter().enumerate() {
            let mut c = ti;
            if ti <= 0 {
                l[i] = 0;
                continue;
            }
            let mut u: i64 = 0;
            if c != 0 {
                if self.e >= self.n {
                    for j in 0..l.len() {
                        l[j] = 0;
                    }
                    return l;
                }
                if c <= 30 {
                    while c > 0 {
                        if self.e >= self.n {
                            for j in 0..l.len() {
                                l[j] = 0;
                            }
                            return l;
                        }
                        let mut delta = 6 - (self.o as i32);
                        if c < delta {
                            delta = c;
                        }
                        let bits = ((self.indices[self.e] >> self.o) & ((1 << delta) - 1)) as i64;
                        let shift = ti - c;
                        u |= bits << shift;
                        self.o += delta as usize;
                        if self.o >= 6 {
                            self.o -= 6;
                            self.e += 1;
                        }
                        c -= delta;
                    }
                    if i < r.len() && r[i] != 0 {
                        let idx = (ti - 1) as isize;
                        if idx >= 0 && (idx as usize) < self.h.len() {
                            if u >= self.h[idx as usize] {
                                let idx2 = ti as usize;
                                if idx2 < self.h.len() {
                                    u -= self.h[idx2];
                                }
                            }
                        }
                    }
                } else {
                    let sub_t = vec![30, c - 30];
                    let mut sub_r = vec![0i32, 0i32];
                    if i < r.len() {
                        sub_r[1] = r[i];
                    }
                    let sub = self.w(&sub_t, Some(&sub_r), None);
                    if i < a.len() && a[i] == 0 {
                        u = sub[0] + sub[1] * self.h[30];
                    } else {
                        u = sub[0];
                    }
                }
            }
            l[i] = u;
        }
        l
    }

    fn x(&mut self, t: i32) -> String {
        for _ in 0..t {
            let dval = self.r.entry("d".to_string()).or_insert(0);
            *dval += 1;
            let n = (*dval % 7) as i64;
            if n == 3 || n == 4 {
                *self.r.get_mut("d").unwrap() += 5 - n;
            }
        }
        let dcur = *self.r.get("d").unwrap_or(&0);
        let timestamp = (self.u_val + dcur) * self.l_val;
        Timestamp::new(timestamp).only_date()
    }

    pub fn decode(&mut self) -> Vec<Record> {
        if !self.header_done {
            let u = self.w(&[12, 6], None, None);
            if u.len() >= 2 {
                self.branch_type = u[0];
                self.s = (63 ^ (u[1] as i32)) as i32;
            }
            self.header_done = true;
        }

        match self.branch_type {
            1479 | 136 => {
                return self.t_branch();
            }
            200 => {
                return self.s_branch();
            }
            139 => {
                let dates = self.k_list();
                let mut out = Vec::new();
                for dstr in dates {
                    let mut r = Record::default();
                    r.date = Some(dstr);
                    out.push(r);
                }
                return out;
            }
            197 => {
                let data = self.mi_run();
                let mut out = Vec::new();
                for (i, _row) in data.iter().enumerate() {
                    let mut r = Record::default();
                    r.date = Some(format!("{}", i));
                    out.push(r);
                }
                return out;
            }
            _ => return Vec::new(),
        }
    }

    pub fn decode_json(&mut self) -> Value {
        if !self.header_done {
            let u = self.w(&[12, 6], None, None);
            if u.len() >= 2 {
                self.branch_type = u[0];
                self.s = (63 ^ (u[1] as i32)) as i32;
            }
            self.header_done = true;
        }

        match self.branch_type {
            1479 | 136 => {
                let rows = self.t_branch();
                let mut arr = Vec::new();
                for r in rows {
                    let mut map = Map::new();
                    if let Some(d) = r.date {
                        map.insert("date".to_string(), Value::String(d));
                    }
                    if let Some(v) = r.open {
                        map.insert("open".to_string(), Value::String(v));
                    }
                    if let Some(v) = r.high {
                        map.insert("high".to_string(), Value::String(v));
                    }
                    if let Some(v) = r.low {
                        map.insert("low".to_string(), Value::String(v));
                    }
                    if let Some(v) = r.close {
                        map.insert("close".to_string(), Value::String(v));
                    }
                    if let Some(v) = r.volume {
                        map.insert("volume".to_string(), Value::String(v));
                    }
                    arr.push(Value::Object(map));
                }
                Value::Array(arr)
            }
            200 => {
                let rows = self.s_branch();
                let mut arr = Vec::new();
                for r in rows {
                    let mut map = Map::new();
                    if let Some(d) = r.date {
                        map.insert("date".to_string(), Value::String(d));
                    }
                    if let Some(v) = r.close {
                        map.insert("close".to_string(), Value::String(v));
                    }
                    if let Some(v) = r.prevclose {
                        map.insert("prevclose".to_string(), Value::String(v));
                    }
                    arr.push(Value::Object(map));
                }
                Value::Array(arr)
            }
            139 => {
                let dates = self.k_list();
                let arr = dates.into_iter().map(Value::String).collect();
                Value::Array(arr)
            }
            197 => {
                let data = self.mi_run();
                let mut arr = Vec::new();
                for row in data {
                    let inner = row
                        .into_iter()
                        .map(|v| Value::Number(serde_json::Number::from(v)))
                        .collect();
                    arr.push(Value::Array(inner));
                }
                Value::Array(arr)
            }
            _ => Value::Array(Vec::new()),
        }
    }

    pub fn branch_info(&mut self) -> (i64, i32) {
        if self.header_done {
            return (self.branch_type, self.s);
        }
        let save_e = self.e;
        let save_o = self.o;
        let u = self.w(&[12, 6], None, None);
        self.e = save_e;
        self.o = save_o;
        if u.len() >= 2 {
            let b = u[0];
            let s = (63 ^ (u[1] as i32)) as i32;
            (b, s)
        } else {
            (0, 0)
        }
    }

    pub fn t_branch(&mut self) -> Vec<Record> {
        let mut res: Vec<Record> = Vec::new();
        if self.s >= 1 {
            return res;
        }
        self.r.insert("lv".to_string(), 0);
        self.r.insert("ld".to_string(), 0);
        self.r.insert("cd".to_string(), 0);
        self.r.insert("cv0".to_string(), 0);
        self.r.insert("cv1".to_string(), 0);
        let pval = self.w(&[6], None, None);
        if pval.len() > 0 {
            self.r.insert("p".to_string(), pval[0]);
        }
        let initd = self.w(&[18], Some(&[1]), None);
        if initd.len() == 0 {
            return res;
        }
        self.r.insert("d".to_string(), initd[0] - 1);
        let m_pow = *self.r.get("p").unwrap_or(&0) as u32;
        self.r.insert("m".to_string(), 10i64.pow(m_pow));
        let a = self.w(&[3, 3], None, None);
        if a.len() > 1 {
            self.r.insert("md".to_string(), a[0]);
            self.r.insert("mv".to_string(), a[1]);
        }

        let mut loop_guard: usize = 0;
        loop {
            if self.e >= self.n {
                break;
            }
            if loop_guard > 1_000_000 {
                break;
            }
            loop_guard += 1;
            let a6 = self.w(&[6], None, None);
            if a6.len() == 0 {
                break;
            }
            let mut i_map: HashMap<String, i64> = HashMap::new();
            i_map.insert("c".to_string(), a6[0]);
            i_map.insert("d".to_string(), 1);

            if (32 & (a6[0] as i64)) != 0 {
                loop {
                    let a2 = self.w(&[6], None, None);
                    if a2.len() == 0 {
                        break;
                    }
                    let a2v = a2[0] as i64;
                    if 63 == (16 | a2v) {
                        let l: String;
                        if (16 & a2v) != 0 {
                            l = "x".to_string();
                        } else {
                            l = "u".to_string();
                        }
                        let a3 = self.w(&[3, 3], None, None);
                        i_map.insert(
                            format!("{}_d", l),
                            a3.get(0).copied().unwrap_or(0) + *self.r.get("md").unwrap_or(&0),
                        );
                        i_map.insert(
                            format!("{}_v", l),
                            a3.get(1).copied().unwrap_or(0) + *self.r.get("mv").unwrap_or(&0),
                        );
                        break;
                    }
                    if (32 & a2v) != 0 {
                        let o: String;
                        if (8 & a2v) != 0 {
                            o = "d".to_string();
                        } else {
                            o = "v".to_string();
                        }
                        let l: String;
                        if (16 & a2v) != 0 {
                            l = "x".to_string();
                        } else {
                            l = "u".to_string();
                        }
                        i_map.insert(
                            format!("{}_{}", l, o),
                            (7 & a2v) as i64 + *self.r.get(&format!("m{}", o)).unwrap_or(&0),
                        );
                        break;
                    }
                    let o = (15 & a2v) as i64;
                    if o == 0 {
                        let v = self.w(&[6], None, None);
                        i_map.insert("d".to_string(), v.get(0).copied().unwrap_or(0));
                    } else if o == 1 {
                        let tmp = self.w(&[18], None, None);
                        if tmp.len() > 0 {
                            self.r.insert("d".to_string(), tmp[0]);
                        }
                        i_map.insert("d".to_string(), 0);
                    } else {
                        i_map.insert("d".to_string(), o);
                    }
                    if (16 & a2v) == 0 {
                        break;
                    }
                }
            }

            let mut rec = Record::default();
            let id = *i_map.get("d").unwrap_or(&1) as i32;
            rec.date = Some(self.x(id));

            // construct llist
            let mut llist = vec![0i32; 5];
            let u_d = *i_map.get("u_d").unwrap_or(&0) as i32;
            let u_v = *i_map.get("u_v").unwrap_or(&0) as i32;
            for e in 0..4 {
                llist[e] = u_d;
            }
            llist[4] = u_v;
            let lidx = (15 & (*i_map.get("c").unwrap_or(&0) as i32)) as usize;
            let mut lval = if lidx < self.p.len() { self.p[lidx] } else { 0 };
            if (1 & u_v) != 0 {
                lval = 31 - lval;
            }
            if (16 & (*i_map.get("c").unwrap_or(&0) as i64)) != 0 {
                llist[4] += 2;
            }
            for e in 0..5 {
                let bit = 1 << (4 - e);
                if (lval & bit) != 0 {
                    llist[e] += 1;
                }
                llist[e] *= 3;
            }

            let dvals = self.w(&llist, Some(&[1, 0, 0, 1, 1]), Some(&[0, 0, 0, 0, 1]));
            let mut o0 = 0i64;
            if dvals.len() > 0 {
                o0 = dvals[0];
            }
            let cd = *self.r.get("cd").unwrap_or(&0);
            let m = *self.r.get("m").unwrap_or(&1) as f64;
            let open = (cd as f64 + o0 as f64) / m;
            let mut high = open;
            let mut low = open;
            let mut closev = open;
            if dvals.len() > 1 {
                high = (cd as f64 + dvals[1] as f64) / m;
            }
            if dvals.len() > 2 {
                low = (cd as f64 - dvals[2] as f64) / m;
            }
            if dvals.len() > 3 {
                closev = (cd as f64 + dvals[3] as f64) / m;
                self.r.insert("cd".to_string(), cd + dvals[3]);
            }

            rec.open = Some(format!("{:.6}", open));
            rec.high = Some(format!("{:.6}", high));
            rec.low = Some(format!("{:.6}", low));
            rec.close = Some(format!("{:.6}", closev));

            let a4 = if dvals.len() > 4 { dvals[4] } else { 0 };
            let mut cv0 = *self.r.get("cv0").unwrap_or(&0);
            let mut cv1 = *self.r.get("cv1").unwrap_or(&0);
            let d_mask_i = self.d_mask;
            if (cv0 & d_mask_i) != 0 {
                let carry = ((cv0 & (!d_mask_i) + ((a4 as i64) & (!d_mask_i))) >> 30) as i64;
                cv1 += carry;
                cv0 &= !d_mask_i;
            }
            self.r.insert("cv0".to_string(), cv0);
            self.r.insert("cv1".to_string(), cv1);
            let volume = (cv0 & (self.f_mask - 1)) + cv1 * self.f_mask;
            rec.volume = Some(format!("{}", volume));

            res.push(rec);
        }

        res
    }

    pub fn s_branch(&mut self) -> Vec<Record> {
        let mut result: Vec<Record> = Vec::new();
        if self.s >= 1 {
            return result;
        }
        let init = self.w(&[18], Some(&[1]), None);
        if init.len() == 0 {
            return result;
        }
        self.r.insert("d".to_string(), init[0] - 1);
        let a = self.w(&[3, 3, 30, 6], None, None);
        self.r.insert("p".to_string(), a[0]);
        self.r.insert("ld".to_string(), a[1]);
        self.r.insert("cd".to_string(), a[2]);
        self.r.insert("c".to_string(), a[3]);
        let p = *self.r.get("p").unwrap_or(&0) as u32;
        let m = 10i64.pow(p);
        self.r.insert("m".to_string(), m);
        self.r
            .insert("pc".to_string(), *self.r.get("cd").unwrap_or(&0));

        let mut tcounter = 0i64;
        loop {
            let mut day_data_d: i32 = 1;
            if self.y() {
                let a_val = self.w(&[3], None, None);
                if a_val[0] == 0 {
                    day_data_d = self.w(&[6], None, None)[0] as i32;
                } else if a_val[0] == 1 {
                    let tmp = self.w(&[18], None, None)[0];
                    self.r.insert("d".to_string(), tmp);
                    day_data_d = 0;
                } else {
                    day_data_d = a_val[0] as i32;
                }
            }
            let mut rec = Record::default();
            rec.date = Some(self.x(day_data_d));
            if self.y() {
                let n = self.n_bits();
                let ld = self.r.entry("ld".to_string()).or_insert(0);
                *ld += n as i64;
            }
            let ld_val = *self.r.get("ld").unwrap_or(&0) as i32;
            let a_close = self.w(&[3 * ld_val], Some(&[1]), None);
            let addv = a_close.get(0).copied().unwrap_or(0);
            let cd_val = *self.r.get("cd").unwrap_or(&0) + addv as i64;
            self.r.insert("cd".to_string(), cd_val);
            let mval = *self.r.get("m").unwrap_or(&1);
            rec.close = Some(format!("{:.6}", (cd_val as f64) / (mval as f64)));
            result.push(rec);
            if self.e >= self.n
                || (self.e == self.n - 1 && (63 & (*self.r.get("c").unwrap_or(&0) ^ tcounter)) == 0)
            {
                break;
            }
            tcounter += 1;
        }

        if result.len() > 0 {
            let pc = *self.r.get("pc").unwrap_or(&0);
            let mval = *self.r.get("m").unwrap_or(&1);
            result[0].prevclose = Some(format!("{:.6}", (pc as f64) / (mval as f64)));
        }

        result
    }

    pub fn mi_run(&mut self) -> Vec<Vec<i64>> {
        let mut result: Vec<Vec<i64>> = Vec::new();
        if self.s >= 1 {
            return result;
        }
        let wf = self.w(&[6], None, None);
        if wf.len() == 0 {
            return result;
        }
        let rf = wf[0] as usize;
        let wc = self.w(&[6], None, None);
        if wc.len() == 0 {
            return result;
        }
        let rc = wc[0] as i64;
        let mut dv = vec![0i64; rf];
        let mut dl = vec![0i32; rf];
        let mut t = 0i64;
        while !(self.e >= self.n) && !(self.e == self.n - 1 && (7 & (rc ^ t)) == 0) {
            let mut o = vec![0i64; rf];
            for i in 0..rf {
                if self.y() {
                    dl[i] += self.n_bits();
                }
                let vals = self.w(&[3 * dl[i]], Some(&[1]), None);
                let v = if vals.len() > 0 { vals[0] } else { 0 };
                dv[i] += v;
                o[i] = dv[i];
            }
            result.push(o);
            t += 1;
        }
        result
    }

    pub fn k_list(&mut self) -> Vec<String> {
        let mut result: Vec<String> = Vec::new();
        if self.s > 1 {
            return result;
        }
        self.r.insert("l".to_string(), 0);
        let mut n_count = -1i32;
        let mut t_initialized = false;
        let dstart = self.w(&[18], None, None);
        if dstart.len() > 0 {
            self.r.insert("d".to_string(), dstart[0] - 1);
        }
        let target = self.w(&[18], None, None);
        let target_date = if target.len() > 0 {
            target[0] as i64
        } else {
            0
        };
        while *self.r.get("d").unwrap_or(&0) < target_date {
            let current_date = self.x(1);
            if n_count <= 0 {
                if self.y() {
                    let v = self.n_bits();
                    let l = self.r.entry("l".to_string()).or_insert(0);
                    *l += v as i64;
                }
                let count_data = self.w(
                    &[3 * (*self.r.get("l").unwrap_or(&0) as i32)],
                    Some(&[0]),
                    None,
                );
                n_count = (count_data.get(0).copied().unwrap_or(0) + 1) as i32;
                if !t_initialized {
                    result.push(current_date);
                    n_count -= 1;
                    t_initialized = true;
                }
            } else {
                result.push(current_date);
            }
            n_count -= 1;
        }
        result
    }

    pub fn intraday(&mut self) -> Vec<Record> {
        let mut c: Vec<Record> = Vec::new();
        if self.s > 2 {
            return c;
        }
        // init
        let init = self.w(&[18], Some(&[1]), None);
        if init.len() == 0 {
            return c;
        }
        self.r.insert("d".to_string(), init[0] - 1);
        let header = if self.s < 1 {
            self.w(&[3, 3, 4, 1, 1, 1, 5], None, None)
        } else {
            self.w(&[4, 4, 4, 1, 1, 1, 3], None, None)
        };
        let names = ["la", "lp", "lv", "tv", "rv", "zv", "pp"];
        for i in 0..header.len().min(7) {
            self.r.insert(names[i].to_string(), header[i]);
        }
        let m = 10i64.pow(*self.r.get("pp").unwrap_or(&0) as u32);
        self.r.insert("m".to_string(), m);
        let mut t = 0i64;
        while !(self.e >= self.n)
            && !(self.e == self.n - 1 && (7 & (*self.r.get("c").unwrap_or(&0) ^ t)) == 0)
        {
            let mut lmap = Record::default();
            let mut o_map: HashMap<&str, i64> = HashMap::new();
            let mut total_v: i64 = 0;
            for i in 0..3 {
                let pkey = ["v", "p", "a"][i];
                let mut has = true;
                if pkey == "v" && *self.r.get("tv").unwrap_or(&0) == 0 {
                    has = false;
                }
                if has && self.y() {
                    let v = self.n_bits();
                    let key = format!("l{}", pkey);
                    let cur = self.r.entry(key).or_insert(0);
                    *cur += v as i64;
                }
                let mut u = 1;
                if pkey == "v" && *self.r.get("rv").unwrap_or(&0) != 0 {
                    if self.y() {
                        u = 1;
                    } else {
                        u = 0;
                    }
                }
                let mut extra = 0;
                if pkey == "v" && u == 1 {
                    extra = 7;
                }
                let call_len =
                    3 * (*self.r.get(&format!("l{}", pkey)).unwrap_or(&0) as i32) + extra;
                let vals = self.w(&[call_len], Some(&[if i != 0 { 1 } else { 0 }]), None);
                let mut val = if vals.len() > 0 { vals[0] } else { 0 };
                if u == 0 {
                    val *= 100;
                }
                o_map.insert(pkey, val);
                if pkey == "v" {
                    total_v += val;
                    if val == 0 && (*self.r.get("zv").unwrap_or(&0) != 0 || !self.y()) {
                        o_map.insert("p", 0);
                    }
                }
            }
            let cp = self.r.entry("cp".to_string()).or_insert(0);
            *cp += *o_map.get("p").unwrap_or(&0) as i64;
            let open = (*cp as f64) / (*self.r.get("m").unwrap_or(&1) as f64);
            lmap.volume = Some(format!("{}", total_v));
            lmap.open = Some(format!("{:.6}", open));
            lmap.avg_price = Some(format!("{:.6}", open));
            c.push(lmap);
            t += 1;
        }
        if c.len() > 0 {
            c[0].date = Some(self.x(1));
            c[0].prevclose = Some(format!(
                "{:.6}",
                (*self.r.get("pc").unwrap_or(&0) as f64) / (*self.r.get("m").unwrap_or(&1) as f64)
            ));
        }
        c
    }
}
