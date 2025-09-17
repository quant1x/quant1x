package main

import (
	"encoding/json"
	"fmt"
	"math"
	"strings"
	"time"
)

// v = String.fromCharCode,
func v(x int) string { return string(rune(x)) }

// b = function (t) { return t === {}._ }
func b(t interface{}) bool { return t == nil }

// N = function () {
func N(e *int, o *int, i []int, n int) int {
	t := y(e, o, i, n)
	e2 := 1
	for {
		if !y(e, o, i, n) {
			if t {
				return e2
			}
			return -e2
		}
		e2++
	}
}

// y = function () {
func y(e *int, o *int, i []int, n int) bool {
	if *e >= n {
		return false
	}
	t := i[*e] & (1 << *o)
	*o++
	if *o >= 6 {
		*o -= 6
		*e++
	}
	return t != 0
}

// w = function (t, r, a) {
func w(t []int, r []bool, a []bool, e *int, o *int, i []int, h []int, n int) []int {
	var s, u, c, d int
	lArr := make([]int, len(t))
	if r == nil {
		r = []bool{}
	}
	if a == nil {
		a = []bool{}
	}
	for s = 0; s < len(t); s++ {
		c = t[s]
		u = 0
		if c != 0 {
			if *e >= n {
				return lArr
			}
			if t[s] <= 0 {
				u = 0
			} else if t[s] <= 30 {
				for {
					d = 6 - *o
					if c > d {
						d = d
					} else {
						d = c
					}
					u |= ((i[*e] >> *o) & ((1 << d) - 1)) << (t[s] - c)
					*o += d
					if *o >= 6 {
						*o -= 6
						*e++
					}
					c -= d
					if c <= 0 {
						break
					}
				}
				if len(r) > s && r[s] && u >= h[t[s]-1] {
					u -= h[t[s]]
				}
			} else {
				tmp := w([]int{30, t[s] - 30}, []bool{false, r[s]}, nil, e, o, i, h, n)
				if len(a) > s && !a[s] {
					u = tmp[0] + tmp[1]*h[30]
				} else {
					u = tmp[0]
				}
			}
			lArr[s] = u
		} else {
			lArr[s] = 0
		}
	}
	return lArr
}

// x = function (t) {
func x(t int, r map[string]int, u int, l int) time.Time {
	for e2 := 0; t > e2; e2++ {
		r["d"]++
		nn := r["d"] % 7
		if nn == 3 || nn == 4 {
			r["d"] += 5 - nn
		}
	}
	return time.Unix(int64((u+r["d"])*l/1000), 0).UTC()
}

// S = function () {
func S(e *int, o *int, i []int, h []int, n int, r map[string]int, l int, u int) []map[string]interface{} {
	r["d"] = w([]int{18}, []bool{true}, nil, e, o, i, h, n)[0] - 1
	aArr := w([]int{3, 3, 30, 6}, nil, nil, e, o, i, h, n)
	r["p"] = aArr[0]
	r["ld"] = aArr[1]
	r["cd"] = aArr[2]
	r["c"] = aArr[3]
	r["m"] = int(math.Pow(10, float64(r["p"])))
	r["pc"] = r["cd"] / r["m"]
	var iArr []map[string]interface{}
	t2 := 0
	for {
		oMap := map[string]int{"d": 1}
		if y(e, o, i, n) {
			a2 := w([]int{3}, nil, nil, e, o, i, h, n)[0]
			if a2 == 0 {
				oMap["d"] = w([]int{6}, nil, nil, e, o, i, h, n)[0]
			} else if a2 == 1 {
				r["d"] = w([]int{18}, nil, nil, e, o, i, h, n)[0]
				oMap["d"] = 0
			} else {
				oMap["d"] = a2
			}
		}
		lMap := map[string]interface{}{"day": x(oMap["d"], r, u, l)}
		if y(e, o, i, n) {
			r["ld"] += N(e, o, i, n)
		}
		aArr2 := w([]int{3 * r["ld"]}, []bool{true}, nil, e, o, i, h, n)
		r["cd"] += aArr2[0]
		lMap["close"] = float64(r["cd"]) / float64(r["m"])
		iArr = append(iArr, lMap)
		t2++
		if *e >= n {
			break
		}
		if *e == n-1 && (63&(r["c"]^t2+1)) == 0 {
			break
		}
	}
	if len(iArr) > 0 {
		iArr[0]["prevclose"] = r["pc"]
	}
	return iArr
}

// k = function () {
func k(e *int, o *int, i []int, h []int, n int, r map[string]int, l int, u int) []time.Time {
	// r.l = 0,
	r["l"] = 0
	// n = -1,
	nn := -1
	// r.d = w([18], null, null)[0] - 1,
	r["d"] = w([]int{18}, nil, nil, e, o, i, h, n)[0] - 1
	// i = w([18], null, null)[0],
	i2 := w([]int{18}, nil, nil, e, o, i, h, n)[0]
	// t = [];
	var tArr []time.Time
	// for (; r.d < i;) {
	for r["d"] < i2 {
		// e = x(1);
		e3 := x(1, r, u, l)
		// if (n <= 0) {
		if nn <= 0 {
			// if (y()) r.l += N();
			if y(e, o, i, n) {
				r["l"] += N(e, o, i, n)
			}
			// n = w([3 * r.l], [0], null)[0] + 1;
			nn = w([]int{3 * r["l"]}, []bool{false}, nil, e, o, i, h, n)[0] + 1
			// 0 == t.length && (t.push(e), n--);
			if len(tArr) == 0 {
				tArr = append(tArr, e3)
				nn--
			}
		} else {
			// t.push(e);
			tArr = append(tArr, e3)
		}
		// n--;
		nn--
	}
	// return t;
	return tArr
}

// T = function () {
func T(e *int, o *int, i []int, h []int, n int, r map[string]int, l int, u int, p []int, d_ int, f int) [][]int64 {
	// r.d = w([18], [1], nil)[0] - 1;
	r["d"] = w([]int{18}, []bool{true}, nil, e, o, i, h, n)[0] - 1
	// r.p = w([3], nil, nil)[0];
	r["p"] = w([]int{3}, nil, nil, e, o, i, h, n)[0]
	// r.m = Math.pow(10, r.p);
	r["m"] = int(math.Pow(10, float64(r["p"])))
	// r.ld = w([3], nil, nil)[0];
	r["ld"] = w([]int{3}, nil, nil, e, o, i, h, n)[0]
	// r.cd = w([30], nil, nil)[0];
	r["cd"] = w([]int{30}, nil, nil, e, o, i, h, n)[0]
	// r.c = w([6], nil, nil)[0];
	r["c"] = w([]int{6}, nil, nil, e, o, i, h, n)[0]
	// r.pc = r.cd / r.m;
	r["pc"] = r["cd"] / r["m"]

	// var arr = [];
	arr := [][]int64{}
	// var t = 0;
	t := 0
	// for (;;) {
	for {
		// var day = x(1);
		day := x(1, r, u, l)
		// if (y()) r.ld += N();
		if y(e, o, i, n) {
			r["ld"] += N(e, o, i, n)
		}
		// var vals = w(p, [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], nil);
		vals := w(p, []bool{true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true}, nil, e, o, i, h, n)
		// r.cd += vals[0];
		r["cd"] += vals[0]
		// var open = r.cd / r.m;
		open := float64(r["cd"]) / float64(r["m"])
		// r.cd += vals[1];
		r["cd"] += vals[1]
		// var high = r.cd / r.m;
		high := float64(r["cd"]) / float64(r["m"])
		// r.cd += vals[2];
		r["cd"] += vals[2]
		// var low = r.cd / r.m;
		low := float64(r["cd"]) / float64(r["m"])
		// r.cd += vals[3];
		r["cd"] += vals[3]
		// var close = r.cd / r.m;
		close := float64(r["cd"]) / float64(r["m"])
		// var vol = vals[4];
		vol := vals[4]
		// var amount = vals[5];
		amount := vals[5]
		// arr.push([day, open, high, low, close, vol, amount]);
		arr = append(arr, []int64{
			day.Unix() * 1000,
			int64(open * 10000),
			int64(high * 10000),
			int64(low * 10000),
			int64(close * 10000),
			int64(vol),
			int64(amount),
		})
		t++
		// if (*e >= n) break;
		if *e >= n {
			break
		}
		// if (*e == n-1 && (63 & (r["c"] ^ t + 1)) == 0) break;
		if *e == n-1 && (63&(r["c"]^t+1)) == 0 {
			break
		}
	}
	return arr
}

// decode_ = function () { ... }
func decode_(s int, e *int, o *int, i []int, h []int, n int, r map[string]int, l int, u int) []map[string]interface{} {
	// r.d = w([18], [1], nil)[0] - 1;
	r["d"] = w([]int{18}, []bool{true}, nil, e, o, i, h, n)[0] - 1
	// r.p = w([3], nil, nil)[0];
	r["p"] = w([]int{3}, nil, nil, e, o, i, h, n)[0]
	// r.m = Math.pow(10, r.p);
	r["m"] = int(math.Pow(10, float64(r["p"])))
	// r.ld = w([]int{3}, nil, nil)[0];
	r["ld"] = w([]int{3}, nil, nil, e, o, i, h, n)[0]
	// r.cd = w([]int{30}, nil, nil)[0];
	r["cd"] = w([]int{30}, nil, nil, e, o, i, h, n)[0]
	// r.c = w([]int{6}, nil, nil)[0];
	r["c"] = w([]int{6}, nil, nil, e, o, i, h, n)[0]
	// r.pc = r.cd / r.m;
	r["pc"] = r["cd"] / r["m"]

	// var arr = [];
	arr := []map[string]interface{}{}
	// var t = 0;
	t := 0
	// for (;;) {
	for {
		// var time = w([12], nil, nil)[0];
		timeVal := w([]int{12}, nil, nil, e, o, i, h, n)[0]
		// if (y()) r.ld += N();
		if y(e, o, i, n) {
			r["ld"] += N(e, o, i, n)
		}
		// var price = w([]int{3 * r.ld}, [1], nil)[0] + r.cd;
		price := w([]int{3 * r["ld"]}, []bool{true}, nil, e, o, i, h, n)[0] + r["cd"]
		// var vol = w([]int{6}, nil, nil)[0];
		vol := w([]int{6}, nil, nil, e, o, i, h, n)[0]
		// arr.push({ time: time, price: price / r.m, vol: vol });
		arr = append(arr, map[string]interface{}{
			"time":  timeVal,
			"price": float64(price) / float64(r["m"]),
			"vol":   vol,
		})
		t++
		// if (*e >= n) break;
		if *e >= n {
			break
		}
		// if (*e == n-1 && (63 & (r["c"] ^ t + 1)) == 0) break;
		if *e == n-1 && (63&(r["c"]^t+1)) == 0 {
			break
		}
	}
	return arr
}

// _mi_run = function () {
func _mi_run(e *int, o *int, i []int, h []int, n int, r map[string]int) [][]int64 {
	// r.d = w([18], [1], nil)[0] - 1;
	r["d"] = w([]int{18}, []bool{true}, nil, e, o, i, h, n)[0] - 1
	// r.p = w([3], nil, nil)[0];
	r["p"] = w([]int{3}, nil, nil, e, o, i, h, n)[0]
	// r.m = Math.pow(10, r.p);
	r["m"] = int(math.Pow(10, float64(r["p"])))
	// r.ld = w([3], nil, nil)[0];
	r["ld"] = w([]int{3}, nil, nil, e, o, i, h, n)[0]
	// r.cd = w([30], nil, nil)[0];
	r["cd"] = w([]int{30}, nil, nil, e, o, i, h, n)[0]
	// r.c = w([6], nil, nil)[0];
	r["c"] = w([]int{6}, nil, nil, e, o, i, h, n)[0]
	// r.pc = r.cd / r.m;
	r["pc"] = r["cd"] / r["m"]

	// var arr = [];
	arr := [][]int64{}
	// var t = 0;
	t := 0
	// for (;;) {
	for {
		// var day = w([12], nil, nil)[0];
		day := w([]int{12}, nil, nil, e, o, i, h, n)[0]
		// if (y()) r.ld += N();
		if y(e, o, i, n) {
			r["ld"] += N(e, o, i, n)
		}
		// var vals = w([]int{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}, nil, nil);
		vals := w([]int{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}, nil, nil, e, o, i, h, n)
		// r.cd += vals[0];
		r["cd"] += vals[0]
		// var open = r.cd / r.m;
		open := float64(r["cd"]) / float64(r["m"])
		// r.cd += vals[1];
		r["cd"] += vals[1]
		// var high = r.cd / r.m;
		high := float64(r["cd"]) / float64(r["m"])
		// r.cd += vals[2];
		r["cd"] += vals[2]
		// var low = r.cd / r.m;
		low := float64(r["cd"]) / float64(r["m"])
		// r.cd += vals[3];
		r["cd"] += vals[3]
		// var close = r.cd / r.m;
		close := float64(r["cd"]) / float64(r["m"])
		// var vol = vals[4];
		vol := vals[4]
		// var amount = vals[5];
		amount := vals[5]
		// arr.push([day, open, high, low, close, vol, amount]);
		arr = append(arr, []int64{
			int64(day),
			int64(open * 10000),
			int64(high * 10000),
			int64(low * 10000),
			int64(close * 10000),
			int64(vol),
			int64(amount),
		})
		t++
		// if (*e >= n) break;
		if *e >= n {
			break
		}
		// if (*e == n-1 && (63 & (r["c"] ^ t + 1)) == 0) break;
		if *e == n-1 && (63&(r["c"]^t+1)) == 0 {
			break
		}
	}
	return arr
}

// function d(t) {
func d(t string) interface{} {
	var e, o, n, s int
	var r = map[string]int{}
	l := 86400000
	u := 7657
	c := make([]int, 64)
	h := make([]int, 64)
	d_ := ^(3 << 30)
	f := 1 << 30
	p := []int{0, 3, 5, 6, 9, 10, 12, 15, 17, 18, 20, 23, 24, 27, 29, 30}

	// for (l = 0; 64 > l; l++) h[l] = m.pow(2, l),
	for l2 := 0; l2 < 64; l2++ {
		h[l2] = int(math.Pow(2, float64(l2)))
		if l2 < 26 {
			c[l2] = l2 + 65
			c[l2+26] = l2 + 97
			if l2 < 10 {
				c[l2+52] = l2 + 48
			}
		}
	}
	cstr := ""
	for _, cc := range c {
		cstr += string(cc)
	}
	cstr += "+/"
	i_str := strings.Split(t, "")
	n = len(i_str)
	iArr := make([]int, n)
	for l2 := 0; l2 < n; l2++ {
		iArr[l2] = strings.Index(cstr, i_str[l2])
	}
	e, o = 0, 0
	uArr := w([]int{12, 6}, nil, nil, &e, &o, iArr, h, n)
	s = 63 ^ uArr[1]
	fmt.Println("u[0]=", uArr[0], "分支:", map[string]string{
		"_1479": "T",
		"_136":  "_",
		"_200":  "S",
		"_139":  "k",
		"_197":  "_mi_run",
	}[fmt.Sprintf("_%d", uArr[0])])
	branchMap := map[string]func() interface{}{
		"_1479": func() interface{} { return T(&e, &o, iArr, h, n, r, l, u, p, d_, f) },
		"_136":  func() interface{} { return decode_(s, &e, &o, iArr, h, n, r, l, u) },
		"_200":  func() interface{} { return S(&e, &o, iArr, h, n, r, l, u) },
		"_139":  func() interface{} { return k(&e, &o, iArr, h, n, r, l, u) },
		"_197":  func() interface{} { return _mi_run(&e, &o, iArr, h, n, r) },
	}
	key := fmt.Sprintf("_%d", uArr[0])
	if fn, ok := branchMap[key]; ok {
		return fn()
	}
	return []interface{}{}
}

// 测试用例
func main() {
	encoded_data := "LC/AAAf8CXCw6mHbaPgkryxXv10eAJP1LW0SD39aT7+NV44Xba3PxCgTdrFc3FepphjnTBw1X4hmGu+ypVAcvFenpBXPqCc6F4ZmGueLFwbIN8QTDXPsCc1FepphjvOoCc8FepphjvcgFO3CP00wxXXWhrkUdZrIJpw9X3ThrlEp6hlGc88Kcem0VeFpZM46VV4MrTC2KScKc811U4aLXUdlzINc9lTrwFW3T52KPj0mDueVFuUR1RtiEoCXfdgFOOSGRXnUhrXWhb0kt6Rk2pU44JV4SrTyU9wSDHPwCnXdP1FuiUM44r7qwdKqcYrIZpw1DqgrlU5IrHRawxjrwBaqcbrIt9gr3UhDtOpyVNjEnCHPnC3royNWvi0gj/"
	out := d(encoded_data)
	// try direct marshal first
	b, err := json.MarshalIndent(out, "", "  ")
	if err == nil {
		fmt.Println(string(b))
		return
	}

	// fallback normalizers
	switch v := out.(type) {
	case []time.Time:
		ss := make([]string, len(v))
		for i, t := range v {
			ss[i] = t.Format("2006-01-02")
		}
		if b, err = json.MarshalIndent(ss, "", "  "); err == nil {
			fmt.Println(string(b))
			return
		}
	case []map[string]interface{}:
		// convert any time.Time values to ISO strings
		for _, m := range v {
			for k, val := range m {
				if tt, ok := val.(time.Time); ok {
					m[k] = tt.Format("2006-01-02 15:04:05")
				}
			}
		}
		if b, err = json.MarshalIndent(v, "", "  "); err == nil {
			fmt.Println(string(b))
			return
		}
	case [][]int64:
		if b, err = json.MarshalIndent(v, "", "  "); err == nil {
			fmt.Println(string(b))
			return
		}
	}

	// final fallback: print error + Go value
	fmt.Printf("json.Marshal failed: %v\n%#v\n", err, out)
}
